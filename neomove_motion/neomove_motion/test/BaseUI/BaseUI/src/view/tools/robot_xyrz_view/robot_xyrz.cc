// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#include "robot_xyrz.h"

#include <QMetaObject>
#include <QThread>
#include <limits>

#include "config/config_factory.h"
#include "controller/device_status_monitor/device_status_monitor.h"
#include "model/model_mgr.h"
#include "model/unit_info/unit_info_mgr.h"
#include "ui_robot_xyrz.h"
#include "view/components/ps_arrow_button/ps_arrow_button.h"
#include "view/components/ps_button/ps_button.h"


RobotXYZR::RobotXYZR() : QDialog(nullptr), ui_(new Ui::RobotXYZRClass()) {
  ui_->setupUi(this);
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint |
                 Qt::WindowStaysOnTopHint);
  InitializeButtons();
  axis_config_ = yotta::ConfigFactory::GetInstance()->GetAxisConfig();
  if (!axis_config_) {
    return;
  }

  robot_manual_ = new RobotManual;
  connect(this, &RobotXYZR::MovePropertyChanged,
                   robot_manual_, &RobotManual::OnMovePropertyChanged);
  connect(this, &RobotXYZR::ButtonClicked,
                   robot_manual_, &RobotManual::OnButtonClicked);
  connect(this, &RobotXYZR::ButtonLongPressed,
                   robot_manual_,
                   &RobotManual::OnButtonLongPressed);
  connect(this, &RobotXYZR::ButtonLongPressReleased,
                   robot_manual_,
                   &RobotManual::OnButtonLongPressReleased);
  connect(this, &RobotXYZR::UnitButtonClicked, this,
          &RobotXYZR::ShowAxisPosWidget);
  UnitInfoMgrPtr motion_module =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!motion_module) {
    return;
  }
  std::vector<UnitInfo> vec_device;
  for (int i = 0; i < motion_module->GetUnitCount(); i++) {
    UnitInfoPtr device_ = motion_module->GetUnitInfo(i);
    vec_device.push_back(*device_);
  }
  SetAxisInfo(vec_device);
  if (step_values_.size() == 2) {
    ui_->pushButton_step_val_->setText(
        "SIZE:\n" + QString::number(step_values_[current_step_value_index_]) +
        "\num");
  }
  // 定时器实时监测io状态
  time_monitor_ = new QTimer(this);
  connect(time_monitor_, &QTimer::timeout, this, &RobotXYZR::UpdateTimer);
  // Don't start timer here - it will be started in onPageShow()
}

RobotXYZR::~RobotXYZR() {
  if (time_monitor_ && time_monitor_->isActive()) {
    time_monitor_->stop();
  }
  delete ui_;
}

void RobotXYZR::ShowPage() {
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(
        this, [this]() { ShowPage(); }, Qt::QueuedConnection);
    return;
  }

  QRect page_geometry = geometry();
  if (page_geometry.width() <= 0 || page_geometry.height() <= 0) {
    page_geometry.setSize(PreferredWindowSize());
  }
  ShowPage(page_geometry.x(), page_geometry.y(), page_geometry.width(),
           page_geometry.height());
}

void RobotXYZR::ShowPage(QWidget* anchor_widget) {
  if (!anchor_widget) {
    ShowPage();
    return;
  }
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(
        this, [this, anchor_widget]() { ShowPage(anchor_widget); },
        Qt::QueuedConnection);
    return;
  }

  QSize robot_size = PreferredWindowSize();

  QPoint anchor_pos = anchor_widget->mapToGlobal(QPoint(0, 0));
  QSize anchor_size = anchor_widget->rect().size();
  QPoint robot_pos =
      anchor_pos + QPoint(anchor_size.width() - robot_size.width(),
                          anchor_size.height() - robot_size.height());
  ShowPage(robot_pos.x(), robot_pos.y(), robot_size.width(),
           robot_size.height());
}

void RobotXYZR::ShowPage(int x, int y, int width, int height) {
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(
        this, [this, x, y, width, height]() { ShowPage(x, y, width, height); },
        Qt::QueuedConnection);
    return;
  }

  setGeometry(x, y, width, height);
  show();
  raise();
  activateWindow();
}

void RobotXYZR::HidePage() {
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(
        this, [this]() { HidePage(); }, Qt::QueuedConnection);
    return;
  }

  hide();
}

QSize RobotXYZR::PreferredWindowSize() {
  if (layout()) {
    layout()->activate();
  }

  QSize preferred_size = sizeHint();
  if (!preferred_size.isValid() || preferred_size.width() <= 0 ||
      preferred_size.height() <= 0) {
    preferred_size = size();
  }
  if (!preferred_size.isValid() || preferred_size.width() <= 0 ||
      preferred_size.height() <= 0) {
    preferred_size = QSize(550, 560);
  }
  return preferred_size.expandedTo(minimumSize());
}

void RobotXYZR::showEvent(QShowEvent* event) {
  if (time_monitor_) {
    time_monitor_->start(500);  // Start timer when page is shown
    LOG(INFO) << "RobotXYZR: Timer started";
  }
}

void RobotXYZR::hideEvent(QHideEvent* event) {
  if (time_monitor_ && time_monitor_->isActive()) {
    time_monitor_->stop();  // Stop timer when page is hidden
    LOG(INFO) << "RobotXYZR: Timer stopped";
  }
}

void RobotXYZR::SetAxisInfo(const std::vector<UnitInfo>& devices) {
  unit_ = devices;
  current_module_index_ = 0;

  QHBoxLayout* device_layout = new QHBoxLayout;
  const int kButtonWidth = 100;
  const int kButtonHeight = 50;
  const int kColumns = 2;
  QGridLayout* grid_layout = new QGridLayout;

  unit_buttons_.clear();
  for (int i = 0; i < static_cast<int>(devices.size()); ++i) {
    QPushButton* device_button =
        new PsButton(QString::fromStdString(devices[i].unit_ids));
    device_button->setStyleSheet(
        "QPushButton {"
        "background-color: #4A86E8;"
        "border: none;"
        "border-radius: 6px;"
        "color: #FFFFFF;"
        "}"
        "QPushButton:checked {"
        "background-color: #27AE60;"
        "color: #FFFFFF;"
        "}");

    device_button->setFixedSize(kButtonWidth, kButtonHeight);
    QFont btn_font = font();
    btn_font.setPointSize(12);
    btn_font.setBold(true);
    device_button->setFont(btn_font);
    device_button->setCheckable(true);

    grid_layout->addWidget(device_button, i / kColumns, i % kColumns);
    unit_buttons_.push_back(device_button);
    connect(device_button, &QPushButton::clicked, this,
            &RobotXYZR::OnUnitChanged);
  }
  if (unit_buttons_.size()) {
    unit_buttons_[0]->click();
  }

  device_layout->addLayout(grid_layout);
  device_layout->addStretch();
  ui_->verticalLayout_4->addLayout(device_layout);
  ui_->verticalLayout_4->addStretch();
}
void RobotXYZR::SetStepInfo(const std::vector<double>& step_values,
                            const std::vector<int>& steps) {
  if (step_values.size() != 2 || steps.size() != 5) {
    LOG(ERROR) << "SetStepInfo : step_values.size() != 2 || steps.size() != 5";
    return;
  }
  step_values_ = step_values;
  current_step_value_index_ = 0;
  steps_ = steps;
  current_step_index_ = 0;

  ui_->pushButton_step_val_->setText(
      "SIZE:\n" + QString::number(step_values_[current_step_value_index_]) +
      "\num");

  for (size_t i = 0; i < steps_.size(); ++i) {
    step_buttons_[i]->setText(QString::number(steps_[i]));
  }

  move_property_.step =
      step_values_[current_step_value_index_] * steps_[current_step_index_];
  move_property_.move_model = current_type_index_;
  emit MovePropertyChanged(move_property_);
}
void RobotXYZR::SetIndexStep(const std::string& axis_ids, double index_step) {
  if (robot_manual_) {
    robot_manual_->SetIndexStep(axis_ids, index_step);
  }
}
void RobotXYZR::ReTranslate() { ui_->retranslateUi(this); }
void RobotXYZR::SetUnit(const std::string& module_name) {
  for (int i = 0; i < unit_buttons_.size(); i++) {
    if (unit_buttons_[i]->text().toStdString() == module_name) {
      unit_buttons_[i]->click();
    }
  }
}

void RobotXYZR::InitializeButtons() {
  // Type buttons: white default, blue when selected
  type_buttons_ = {ui_->pushButton_type_1_, ui_->pushButton_type_2_,
                   ui_->pushButton_type_3_};
  for (QPushButton* button : type_buttons_) {
    connect(button, &QPushButton::clicked, this, &RobotXYZR::OnTypeChanged);
    auto* ps_btn = qobject_cast<PsButton*>(button);
    // if (ps_btn) {
    //   ps_btn->Color().SetBaseColor(PsColor::Color::kWhite);
    //   ps_btn->CheckedColor().SetBaseColor(PsColor::Color::kBlue);
    // }
  }

  // Step buttons: white default, blue when selected
  connect(ui_->pushButton_step_val_, &QPushButton::clicked, this,
          &RobotXYZR::OnStepValChanged);
  step_buttons_ = {ui_->pushButton_step_1_, ui_->pushButton_step_2_,
                   ui_->pushButton_step_3_, ui_->pushButton_step_4_,
                   ui_->pushButton_step_5_};
  for (QPushButton* button : step_buttons_) {
    connect(button, &QPushButton::clicked, this, &RobotXYZR::OnStepChanged);
    auto* ps_btn = qobject_cast<PsButton*>(button);
    if (ps_btn) {
      // ps_btn->Color().SetBaseColor(PsColor::Color::kWhite);
      // ps_btn->CheckedColor().SetBaseColor(PsColor::Color::kBlue);
    }
  }

  // Axis buttons: white
  axis_buttons_ = {
      ui_->pushButton_xadd_,   ui_->pushButton_xsub_,
      ui_->pushButton_yadd_,   ui_->pushButton_ysub_,
      ui_->pushButton_r_add_,  ui_->pushButton_r_sub_,
      ui_->pushButton_r2_add_, ui_->pushButton_r2_sub_,
      ui_->pushButton_z1_up_,  ui_->pushButton_z1_down_,
      ui_->pushButton_z2_up_,  ui_->pushButton_z2_down_,
      ui_->pushButton_txadd_,  ui_->pushButton_txsub_,
      ui_->pushButton_tyadd_,  ui_->pushButton_tysub_,
  };
  current_unit_axis_ids_ = {"", "", "", "", "", "", "", "",
                            "", "", "", "", "", "", "", ""};

  for (QPushButton* button : axis_buttons_) {
    connect(button, &QPushButton::clicked, this, &RobotXYZR::OnButtonClicked);
    connect(button, &QPushButton::pressed, this, &RobotXYZR::OnButtonPressed);
    connect(button, &QPushButton::released, this, &RobotXYZR::OnButtonReleased);
    button->setVisible(false);
  }

  ui_->pushButton_txadd_->setVisible(true);
  ui_->pushButton_txadd_->setText("");
  ConfigureArrowButton(12, PsArrowButton::Direction::UpRight);

  ui_->pushButton_txsub_->setVisible(true);
  ui_->pushButton_txsub_->setText("");
  ConfigureArrowButton(13, PsArrowButton::Direction::DownRight);

  ui_->pushButton_tyadd_->setVisible(true);
  ui_->pushButton_tyadd_->setText("");
  ConfigureArrowButton(14, PsArrowButton::Direction::UpLeft);

  ui_->pushButton_tysub_->setVisible(true);
  ui_->pushButton_tysub_->setText("");
  ConfigureArrowButton(15, PsArrowButton::Direction::DownLeft);

  auto* center_btn = qobject_cast<PsButton*>(ui_->pushButton_14);
  if (center_btn) {
    center_btn->Color().SetBaseColor(PsColor::Color::kWhite);
  }
}
void RobotXYZR::OnUnitChanged() {
  if (time_monitor_) {
    time_monitor_->stop();
  }
  QPushButton* sender_button = qobject_cast<QPushButton*>(sender());
  for (size_t i = 0; i < unit_buttons_.size(); ++i) {
    unit_buttons_[i]->setChecked(false);
  }
  for (int i = 0; i < static_cast<int>(unit_buttons_.size()); ++i) {
    if (sender_button == unit_buttons_[i]) {
      unit_buttons_[i]->setChecked(true);
      current_module_index_ = i;
      RefreshDevice(current_module_index_);
      emit UnitButtonClicked(current_module_index_);
      break;
    }
  }
  if (time_monitor_) {
    time_monitor_->start(100);
  }
}
void RobotXYZR::OnStepValChanged() {
  current_step_value_index_ =
      (static_cast<size_t>(current_step_value_index_) + 1) %
      step_values_.size();
  ui_->pushButton_step_val_->setText(
      "SIZE:\n" +
      QString::number(
          step_values_[static_cast<size_t>(current_step_value_index_)]) +
      "\num");

  UpdateMoveProperty();
}
void RobotXYZR::OnStepChanged() {
  QPushButton* sender_button = qobject_cast<QPushButton*>(sender());
  for (int i = 0; i < static_cast<int>(step_buttons_.size()); ++i) {
    if (sender_button == step_buttons_[i]) {
      current_step_index_ = i;
      RefreshStep(current_step_index_);
      break;
    }
  }
  UpdateMoveProperty();
}
void RobotXYZR::OnTypeChanged() {
  QPushButton* sender_button = qobject_cast<QPushButton*>(sender());
  for (int i = 0; i < static_cast<int>(type_buttons_.size()); ++i) {
    if (sender_button == type_buttons_[i]) {
      current_type_index_ = i;
      RefreshType(current_type_index_);
      break;
    }
  }
  UpdateMoveProperty();
}

void RobotXYZR::OnButtonClicked() {
  if (sender() == ui_->pushButton_txadd_ ||
      sender() == ui_->pushButton_txsub_ ||
      sender() == ui_->pushButton_tyadd_ ||
      sender() == ui_->pushButton_tysub_) {
    OnDiagonalButtonClicked();
    return;
  }
  ProcessAxisButtonEvent(&RobotXYZR::ButtonClicked);
}
void RobotXYZR::OnButtonPressed() {
  if (sender() == ui_->pushButton_txadd_ ||
      sender() == ui_->pushButton_txsub_ ||
      sender() == ui_->pushButton_tyadd_ ||
      sender() == ui_->pushButton_tysub_) {
    OnDiagonalButtonPressed();
    return;
  }
  ProcessAxisButtonEvent(&RobotXYZR::ButtonLongPressed);
}
void RobotXYZR::OnButtonReleased() {
  if (sender() == ui_->pushButton_txadd_ ||
      sender() == ui_->pushButton_txsub_ ||
      sender() == ui_->pushButton_tyadd_ ||
      sender() == ui_->pushButton_tysub_) {
    OnDiagonalButtonReleased();
    return;
  }
  ProcessAxisButtonEvent(&RobotXYZR::ButtonLongPressReleased);
}

void RobotXYZR::OnDiagonalButtonClicked() {
  QPushButton* sender_button = qobject_cast<QPushButton*>(sender());
  if (!sender_button) {
    return;
  }

  const std::string axis_x_ids = "平台X轴";
  const std::string axis_y_ids = "平台Y轴";

  if (sender_button == ui_->pushButton_txadd_) {
    emit ButtonClicked(current_module_index_, axis_x_ids, 0);
    emit ButtonClicked(current_module_index_, axis_y_ids, 0);
  } else if (sender_button == ui_->pushButton_txsub_) {
    emit ButtonClicked(current_module_index_, axis_x_ids, 0);
    emit ButtonClicked(current_module_index_, axis_y_ids, 1);
  } else if (sender_button == ui_->pushButton_tyadd_) {
    emit ButtonClicked(current_module_index_, axis_x_ids, 1);
    emit ButtonClicked(current_module_index_, axis_y_ids, 0);
  } else if (sender_button == ui_->pushButton_tysub_) {
    emit ButtonClicked(current_module_index_, axis_x_ids, 1);
    emit ButtonClicked(current_module_index_, axis_y_ids, 1);
  }
}

void RobotXYZR::OnDiagonalButtonPressed() {
  QPushButton* sender_button = qobject_cast<QPushButton*>(sender());
  if (!sender_button) {
    return;
  }

  const std::string axis_x_ids = "平台X轴";
  const std::string axis_y_ids = "平台Y轴";

  if (sender_button == ui_->pushButton_txadd_) {
    emit ButtonLongPressed(current_module_index_, axis_x_ids, 0);
    emit ButtonLongPressed(current_module_index_, axis_y_ids, 0);
  } else if (sender_button == ui_->pushButton_txsub_) {
    emit ButtonLongPressed(current_module_index_, axis_x_ids, 0);
    emit ButtonLongPressed(current_module_index_, axis_y_ids, 1);
  } else if (sender_button == ui_->pushButton_tyadd_) {
    emit ButtonLongPressed(current_module_index_, axis_x_ids, 1);
    emit ButtonLongPressed(current_module_index_, axis_y_ids, 0);
  } else if (sender_button == ui_->pushButton_tysub_) {
    emit ButtonLongPressed(current_module_index_, axis_x_ids, 1);
    emit ButtonLongPressed(current_module_index_, axis_y_ids, 1);
  }
}

void RobotXYZR::OnDiagonalButtonReleased() {
  QPushButton* sender_button = qobject_cast<QPushButton*>(sender());
  if (!sender_button) {
    return;
  }

  const std::string axis_x_ids = "平台X轴";
  const std::string axis_y_ids = "平台Y轴";

  if (sender_button == ui_->pushButton_txadd_) {
    emit ButtonLongPressReleased(current_module_index_, axis_x_ids, 0);
    emit ButtonLongPressReleased(current_module_index_, axis_y_ids, 0);
  } else if (sender_button == ui_->pushButton_txsub_) {
    emit ButtonLongPressReleased(current_module_index_, axis_x_ids, 0);
    emit ButtonLongPressReleased(current_module_index_, axis_y_ids, 1);
  } else if (sender_button == ui_->pushButton_tyadd_) {
    emit ButtonLongPressReleased(current_module_index_, axis_x_ids, 1);
    emit ButtonLongPressReleased(current_module_index_, axis_y_ids, 0);
  } else if (sender_button == ui_->pushButton_tysub_) {
    emit ButtonLongPressReleased(current_module_index_, axis_x_ids, 1);
    emit ButtonLongPressReleased(current_module_index_, axis_y_ids, 1);
  }
}

void RobotXYZR::RefreshDevice(int device_index) {
  for (size_t i = 0; i < 12 && i < axis_buttons_.size(); ++i) {
    axis_buttons_[i]->setVisible(false);
  }

  const UnitInfo& device = unit_[device_index];
  for (size_t i = 0; i < device.axis_ids.size(); ++i) {
    const QString axis_name = QString::fromStdString(device.axis_ids[i]);
    QString axis_name_vertical;
    for (QChar c : axis_name) {
      axis_name_vertical.append(c).append('\n');
    }
    const std::string axis_ids = device.axis_ids[i];
    const int axis_type = device.axis_types[i];
    switch (axis_type) {
      case 1:  // X axis
        SetAxisButton(0, axis_name + "+", axis_ids);
        SetAxisButton(1, axis_name + "-", axis_ids);
        ConfigureArrowButton(0, PsArrowButton::Direction::Right);
        ConfigureArrowButton(1, PsArrowButton::Direction::Left);
        break;
      case 2:  // Y axis
        SetAxisButton(2, axis_name_vertical + "+", axis_ids);
        SetAxisButton(3, axis_name_vertical + "-", axis_ids);
        ConfigureArrowButton(2, PsArrowButton::Direction::Up);
        ConfigureArrowButton(3, PsArrowButton::Direction::Down);
        break;
      case 3:  // R1 axis
        SetAxisButton(4, axis_name + "+", axis_ids);
        SetAxisButton(5, axis_name + "-", axis_ids);
        ConfigureArrowButton(4, PsArrowButton::Direction::Right,
                             PsArrowButton::ArrowStyle::Curved);
        ConfigureArrowButton(5, PsArrowButton::Direction::Left,
                             PsArrowButton::ArrowStyle::Curved);
        break;
      case 4:  // R2 axis
        SetAxisButton(6, axis_name + "+", axis_ids);
        SetAxisButton(7, axis_name + "-", axis_ids);
        ConfigureArrowButton(6, PsArrowButton::Direction::Right,
                             PsArrowButton::ArrowStyle::Curved);
        ConfigureArrowButton(7, PsArrowButton::Direction::Left,
                             PsArrowButton::ArrowStyle::Curved);
        break;
      case 5:  // Z1 axis
        SetAxisButton(8, axis_name_vertical + "+", axis_ids);
        SetAxisButton(9, axis_name_vertical + "-", axis_ids);
        ConfigureArrowButton(8, PsArrowButton::Direction::Up);
        ConfigureArrowButton(9, PsArrowButton::Direction::Down);
        break;
      case 6:  // Z2 axis
        SetAxisButton(10, axis_name_vertical + "+", axis_ids);
        SetAxisButton(11, axis_name_vertical + "-", axis_ids);
        ConfigureArrowButton(10, PsArrowButton::Direction::Up);
        ConfigureArrowButton(11, PsArrowButton::Direction::Down);
        break;
        // case 7 / case 8 预留，TX/TY 当前作为斜向按钮固定显示，不跟随 unit
        // 轴配置刷新。
    }
  }
}
void RobotXYZR::RefreshStep(int step_index) {
  for (size_t i = 0; i < step_buttons_.size(); ++i) {
    step_buttons_[i]->setChecked(i == step_index ? true : false);
  }
}
void RobotXYZR::RefreshType(int type_index) {
  for (size_t i = 0; i < type_buttons_.size(); ++i) {
    type_buttons_[i]->setChecked(i == type_index ? true : false);
  }
}

void RobotXYZR::ProcessAxisButtonEvent(void (RobotXYZR::*signal)(int,
                                                                 std::string,
                                                                 int)) {
  QPushButton* sender_button = qobject_cast<QPushButton*>(sender());
  for (size_t i = 0; i < axis_buttons_.size(); ++i) {
    if (sender_button == axis_buttons_[i]) {
      const std::string axis_ids = current_unit_axis_ids_[i];
      const int direction = i % 2;
      (this->*signal)(current_module_index_, axis_ids, direction);
      break;
    }
  }
}
void RobotXYZR::SetAxisButton(int button_index, const QString& text,
                              std::string axis_ids) {
  axis_buttons_[button_index]->setText(text);
  axis_buttons_[button_index]->setVisible(true);
  current_unit_axis_ids_[button_index] = axis_ids;
}

void RobotXYZR::ConfigureArrowButton(int button_index,
                                     PsArrowButton::Direction direction,
                                     PsArrowButton::ArrowStyle style) {
  auto* arrow = qobject_cast<PsArrowButton*>(axis_buttons_[button_index]);
  if (arrow) {
    arrow->SetDirection(direction);
    arrow->SetArrowStyle(style);
  }
}
void RobotXYZR::UpdateMoveProperty() {
  move_property_.step =
      step_values_[current_step_value_index_] * steps_[current_step_index_];
  move_property_.move_model = current_type_index_;
  emit MovePropertyChanged(move_property_);
}

void RobotXYZR::ClearLayout(QLayout* layout) {
  if (!layout) return;
  QLayoutItem* item;
  while ((item = layout->takeAt(0)) != nullptr) {
    if (item->widget()) {
      item->widget()->deleteLater();  // 移除部件
    } else if (item->layout()) {
      ClearLayout(item->layout());    // 递归移除子布局
      item->layout()->deleteLater();  // 删除子布局
    } else if (item->spacerItem()) {
      delete item->spacerItem();  // 处理间隔项
    }
    // delete item;  // 删除布局项
  }
}
void RobotXYZR::ShowAxisPosWidget(int module_index) {
  ClearLayout(ui_->axis_pos_layout);
  axis_pos_display_widget_.clear();
  // axis_pos_.clear();

  // Initialize cache for this module
  cached_axis_states_.clear();

  if (module_index >= 0 && module_index < unit_.size()) {
    cached_axis_states_.resize(unit_[module_index].axis_ids.size(),
                               -1);  // -1 means uninitialized

    for (int i = 0; i < unit_[module_index].axis_ids.size(); i++) {
      CoordinateDisplayWidget* axis_pos_display = new CoordinateDisplayWidget;
      axis_pos_display->setFixedSize(275, 40);
      axis_pos_display->label(
          QString::fromStdString(unit_[module_index].axis_ids[i]));

      yotta::AxisConfigItemPtr axis_config =
          axis_config_->GetAxisByIds(unit_[module_index].axis_ids[i].c_str());
      if (!axis_config) {
        LOG(ERROR) << "get axis_config error";
        continue;
      }
      yotta::AxisAttributeConfigPtr axis_attribute_config =
          axis_config->GetAttribute();
      if (!axis_attribute_config) {
        LOG(ERROR) << "get axis_attribute_config error";
        continue;
      }
      double limit_negative = 0;
      double limit_positive = 0;
      axis_attribute_config->GetLimitNegative(&limit_negative);
      axis_attribute_config->GetLimitPositive(&limit_positive);
      axis_pos_display->range(limit_negative, limit_positive);

      axis_pos_display_widget_.push_back(axis_pos_display);
      // axis_pos_.push_back(0);
      ui_->axis_pos_layout->addWidget(axis_pos_display, i / 2, i % 2);
    }
  }
}

void RobotXYZR::UpdateTimer() {
  if (current_module_index_ >= unit_.size()) {
    return;
  }

  DeviceStatusMonitor* monitor = DeviceStatusMonitorSinglton::GetInstance();
  UnitInfo module_axis_info = unit_[current_module_index_];

  for (int i = 0; i < module_axis_info.axis_ids.size(); i++) {
    if (i >= axis_pos_display_widget_.size() ||
        i >= cached_axis_states_.size()) {
      continue;
    }

    std::string axis_ids = module_axis_info.axis_ids[i];

    // Get Axis Position
    double position = 0;
    static double epsilon = std::numeric_limits<double>::epsilon();
    if (monitor->GetAxisPos(axis_ids, position)) {
      // 值改变，并且非0
      if (std::abs(cached_axis_pos_ - position) > epsilon &&
          std::abs(position) > epsilon) {
        axis_pos_display_widget_[i]->value(position);
      }
      // axis_pos_[i] = position;
      cached_axis_pos_ = position;
    }

    // Get Axis State - only update UI if state changed
    int axis_state_int = 0;
    if (monitor->GetAxisState(axis_ids, axis_state_int)) {
      // Only update label color if state changed
      if (cached_axis_states_[i] != axis_state_int) {
        yotta::Axis::AxisState axis_state =
            static_cast<yotta::Axis::AxisState>(axis_state_int);
        switch (axis_state) {
          case yotta::Axis::AxisState::kUnknown:
            axis_pos_display_widget_[i]->lable_color(Qt::gray);
            break;
          case yotta::Axis::AxisState::kServoOn:
            axis_pos_display_widget_[i]->lable_color(Qt::green);
            break;
          case yotta::Axis::AxisState::kServoOff:
            axis_pos_display_widget_[i]->lable_color(Qt::red);
            break;
          default:
            break;
        }
        cached_axis_states_[i] = axis_state_int;
      }
    }
  }
}

// double RobotXYZR::GetAxisPos(int axis_index) {
//  if (axis_index < 0 || axis_index >= axis_pos_.size()) {
//    LOG(ERROR) << "get GetAxisPos error";
//    return 0;
//  }
//
//  return axis_pos_[axis_index];
//}
