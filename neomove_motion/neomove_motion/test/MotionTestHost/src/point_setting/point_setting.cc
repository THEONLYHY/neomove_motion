#include "point_setting.h"

#include <QTimer>

#include <common/message_loop.h>
#include <limit_motion/impl/acc_dec_profile_impl.h>
#include <limit_motion/limit_motion_mgr.h>
#include <main_process/module_mgr.h>

#include <QPushButton>

#include "controller/device_status_monitor/device_status_monitor.h"
#include "view/tools/shared_manager/shared_manager.h"
#include "view/tools/utils/style_utils.h"

PointSetting::PointSetting(QWidget *parent)
    : QWidget(parent), ui(new Ui::point_settingClass()) {
  ui->setupUi(this);
  InitUI();

  axis_config_ = yotta::ConfigFactory::GetInstance()->GetAxisConfig();
  if (!axis_config_) {
    LOG(ERROR) << "axis_config_ is nullptr";
  }

  point_config_ = yotta::ConfigFactory::GetInstance()->GetPointConfig();
  if (!point_config_) {
    LOG(ERROR) << "point_config_ is nullptr";
  }

  unit_point_mgr_ = ModelMgrSinglton::GetInstance()->unit_point_mgr();
  if (!unit_point_mgr_) {
    LOG(ERROR) << "unit_point_mgr_ is nullptr";
  }

  unit_info_mgr_ = ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!unit_info_mgr_) {
    LOG(ERROR) << "unit_info_mgr_ is nullptr";
  }

  AddUnitButtons();

  connect(ui->button_page_switch_, &QPushButton::clicked, this,
          &PointSetting::SwitchPage);
  connect(ui->button_point_add_, &QPushButton::clicked, this,
          &PointSetting::AddPoint);
  connect(ui->button_point_delete_, &QPushButton::clicked, this,
          &PointSetting::DeletePoint);
  connect(ui->table_point_list_, &QTableWidget::cellClicked, this,
          &PointSetting::OnPointSelect);
  connect(ui->table_point_list_->horizontalHeader(),
          &QHeaderView::sectionResized,
          [this](int logicalIndex, int, int newSize) {
            if (logicalIndex == 0) {
              ui->table_axis_list_->setColumnWidth(0, newSize);
            }
          });
}

PointSetting::~PointSetting() { delete ui; }

void PointSetting::InitUI() {
  // 禁用 autoDefault，防止第一次点击只激活按钮而不触发 clicked 信号
  ui->button_page_switch_->setAutoDefault(false);
  ui->button_point_add_->setAutoDefault(false);
  ui->button_point_delete_->setAutoDefault(false);

  StyleUtils::ApplyStyle(ui->button_page_switch_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui->button_point_add_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui->button_point_delete_, "btn_icon_deepblue_140_69");
}

void PointSetting::AddUnitButtons() {
  // 空指针检查
  if (!unit_info_mgr_) {
    LOG(ERROR) << "PointSetting::AddUnitButtons - unit_info_mgr_ is null";
    return;
  }

  for (int i = 0; i < unit_info_mgr_->GetUnitCount(); i++) {
    auto unit_info = unit_info_mgr_->GetUnitInfo(i);
    if (!unit_info) {
      LOG(WARNING) << "PointSetting::AddUnitButtons - GetUnitInfo(" << i << ") returned null";
      continue;
    }
    QPushButton *axis_btn = new QPushButton(
        QString::fromStdString(unit_info->unit_ids));
    axis_btn->setFixedSize(kUnitButtonWidth, kUnitButtonHeight);
    axis_btn->setCheckable(true);
    ui->module_list_layout->addWidget(axis_btn);
    btn_group_unit_.addButton(axis_btn, i);
    StyleUtils::ApplyStyle(axis_btn, "btn_unit_tab");

    connect(axis_btn, &QAbstractButton::clicked, this, [this, i](bool checked) {
      if (!checked) {
        return;
      }
      OnUnitClicked(i);
    });
  }
  ui->module_list_layout->addStretch();

  QAbstractButton *btn = btn_group_unit_.button(0);
  if (btn) {
    btn->setChecked(true);
    OnUnitClicked(0);
  }
}

void PointSetting::OnUnitClicked(int unit_index) {
  if (!unit_info_mgr_) {
    return;
  }
  current_unit_index_ = unit_index;
  current_unit_info_ = *unit_info_mgr_->GetUnitInfo(unit_index);
  current_unit_ids_ = current_unit_info_.unit_ids;

  ui->table_axis_list_->setRowCount(0);
  ui->table_point_list_->setRowCount(0);
  ui->table_point_list_->setColumnCount(0);
  axis_names_.clear();
  axis_num_ = 0;
  checkboxs_axis_.clear();
  buttons_set_.clear();
  buttons_goto_point_.clear();
  current_row_ = -1;
  is_save_ = true;

  axis_num_ = static_cast<int>(current_unit_info_.axis_ids.size());
  for (int i = 0; i < axis_num_; i++) {
    axis_names_.push_back(
        QString::fromStdString(current_unit_info_.axis_ids[i]));
  }
  InitTitleTable();
  InitPointListTable();
}
void PointSetting::InitTitleTable() {
  ui->table_axis_list_->setColumnCount(axis_num_ + 3);
  ui->table_axis_list_->setRowCount(1);
  ui->table_axis_list_->setRowHeight(0, kTableButtonHeight);
  ui->table_axis_list_->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->table_axis_list_->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  ui->table_axis_list_->horizontalHeader()->setSectionResizeMode(
      0, QHeaderView::Interactive);

  ui->table_axis_list_->horizontalHeader()->setSectionResizeMode(
      axis_num_ + 1, QHeaderView::Fixed);
  ui->table_axis_list_->horizontalHeader()->setSectionResizeMode(
      axis_num_ + 2, QHeaderView::Fixed);
  ui->table_axis_list_->setColumnWidth(axis_num_ + 1, kTableButtonWidth);
  ui->table_axis_list_->setColumnWidth(axis_num_ + 2, kTableButtonWidth);
  ui->table_axis_list_->horizontalHeader()->setVisible(false);
  ui->table_axis_list_->verticalHeader()->setVisible(false);

  QTableWidgetItem *item = new QTableWidgetItem();
  item->setText(tr("Point\\Axis"));
  item->setFlags(Qt::ItemIsEnabled);
  ui->table_axis_list_->setItem(0, 0, item);

  for (int i = 0; i < axis_num_; ++i) {
    QPointer<QCheckBox> checkBox = new QCheckBox(axis_names_[i]);
    ui->table_axis_list_->setCellWidget(0, i + 1, checkBox);
    checkboxs_axis_.push_back(checkBox);
  }

  QPushButton *prev_row = new QPushButton(tr("上一行"));
  QPushButton *next_row = new QPushButton(tr("下一行"));
  StyleUtils::ApplyStyle(prev_row, "btn_table_action");
  StyleUtils::ApplyStyle(next_row, "btn_table_action");

  connect(prev_row, &QPushButton::clicked, prev_row, [this] {
    int currentRow = ui->table_point_list_->currentRow();
    int prevRow = qMax(currentRow - 1, 0);
    current_row_ = prevRow;
    ui->table_point_list_->setCurrentCell(prevRow, 0);
  });
  connect(next_row, &QPushButton::clicked, next_row, [this] {
    int currentRow = ui->table_point_list_->currentRow();
    int nextRow = qMin(currentRow + 1, ui->table_point_list_->rowCount() - 1);
    current_row_ = nextRow;
    ui->table_point_list_->setCurrentCell(nextRow, 0);
  });
  ui->table_axis_list_->setCellWidget(0, axis_num_ + 1, prev_row);
  ui->table_axis_list_->setCellWidget(0, axis_num_ + 2, next_row);
}
void PointSetting::InitPointListTable() {
  ui->table_point_list_->setColumnCount(axis_num_ + 3);
  ui->table_point_list_->setShowGrid(true);
  ui->table_point_list_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui->table_point_list_->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->table_point_list_->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->table_point_list_->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  ui->table_point_list_->horizontalHeader()->setSectionResizeMode(
      axis_num_ + 1, QHeaderView::Fixed);
  ui->table_point_list_->horizontalHeader()->setSectionResizeMode(
      axis_num_ + 2, QHeaderView::Fixed);
  ui->table_point_list_->horizontalHeader()->setVisible(false);
  ui->table_point_list_->verticalHeader()->setVisible(false);

  // 按行填充
  for (int i = 0; i < unit_point_mgr_->GetUnitPointCount(); i++) {
    UnitPoint point_info = unit_point_mgr_->GetUnitPoint(i);
    if (point_info.unit_ids == current_unit_ids_) {
      std::string point_ids = point_info.point_ids;

      int row_count = ui->table_point_list_->rowCount();
      ui->table_point_list_->insertRow(row_count);

      AppendItemByRow(row_count, point_info.point_ids);
    }
  }
}
void PointSetting::AppendItemByRow(int row, std::string point_ids) {
  QTableWidgetItem *item_point_name =
      new QTableWidgetItem(QString::fromStdString(point_ids));
  ui->table_point_list_->setItem(row, 0, item_point_name);

  for (int col = 0; col < axis_num_; col++) {
    QTableWidgetItem *cell = new QTableWidgetItem("0.0000");
    cell->setFlags(cell->flags() & ~Qt::ItemIsEditable);
    ui->table_point_list_->setItem(row, col + 1, cell);

    yotta::PointConfigItemPtr point =
        point_config_->GetPointByIds(point_ids.c_str());
    if (!point) {
      continue;
    }
    for (int j = 0; j < point->GetPointAxisItemCount(); ++j) {
      yotta::PointAxisConfigItemPtr point_axis = point->GetPointAxisItem(j);

      std::string axis_ids;
      char axis_ids_char[kAxisIdBufferSize];
      double pos = 0;
      point_axis->GetAxisIds(axis_ids_char, kAxisIdBufferSize);
      point_axis->GetPosition(&pos);
      axis_ids = axis_ids_char;

      if (axis_names_[col].toStdString() == axis_ids) {
        ui->table_point_list_->item(row, col + 1)
            ->setText(QString::number(pos, 'f', kPositionPrecision));
        cell->setForeground(Qt::green);
        break;
      }
    }
  }

  QPointer<QPushButton> btn_goto_point = new QPushButton(tr("移动到点"));
  btn_goto_point->setFixedSize(kTableButtonWidth, kTableButtonHeight);
  StyleUtils::ApplyStyle(btn_goto_point, "btn_table_action");
  QPointer<QPushButton> btn_set = new QPushButton(tr("设置点位"));
  btn_set->setFixedSize(kTableButtonWidth, kTableButtonHeight);
  StyleUtils::ApplyStyle(btn_set, "btn_table_action");
  ui->table_point_list_->setCellWidget(row, axis_num_ + 1, btn_goto_point);
  ui->table_point_list_->setCellWidget(row, axis_num_ + 2, btn_set);
  buttons_goto_point_.push_back(btn_goto_point);
  buttons_set_.push_back(btn_set);

  connect(btn_goto_point, &QPushButton::clicked, this,
          &PointSetting::OnGotoButtonClicked);
  connect(btn_set, &QPushButton::clicked, this,
          &PointSetting::OnSetButtonClicked);

  ui->table_point_list_->setRowHeight(row, kTableButtonHeight);
  ui->table_point_list_->setColumnWidth(axis_num_ + 1, kTableButtonWidth);
  ui->table_point_list_->setColumnWidth(axis_num_ + 2, kTableButtonWidth);
}

void PointSetting::OnSetButtonClicked() {
  QPushButton *current_button = qobject_cast<QPushButton *>(sender());
  int current_row = 0;
  for (QPushButton *button : buttons_set_) {
    if (current_button == button) {
      break;
    }
    ++current_row;
  }

  for (int col = 0; col < axis_num_; ++col) {
    auto monitor = DeviceStatusMonitorSinglton::GetInstance();
    double position = 0;
    monitor->GetAxisPos(axis_names_[col].toStdString().c_str(), position);

    QTableWidgetItem *item = ui->table_point_list_->item(current_row, col + 1);
    if (!item) {
      LOG(WARNING) << "PointSetting::OnSetButtonClicked - item is null at row "
                   << current_row << ", col " << (col + 1);
      continue;
    }
    if (checkboxs_axis_[col].isNull() || !checkboxs_axis_[col]->isChecked()) {
      item->setText(QString::number(0.0000, 'f', kPositionPrecision));
      item->setForeground(Qt::black);
      continue;
    }
    item->setText(QString::number(position, 'f', kPositionPrecision));
    item->setForeground(Qt::green);
  }

  SavePoint(current_row);
}
void PointSetting::OnGotoButtonClicked() {
  QPushButton *current_button = qobject_cast<QPushButton *>(sender());
  int current_row = 0;
  for (QPushButton *button : buttons_goto_point_) {
    if (current_button == button) {
      break;
    }
    ++current_row;
  }

  ReadPoint(current_point_, current_row);
  common::MessageLoop::GetMessageLoop(common::kMotion)
      ->PostTask(std::bind(&PointSetting::GotoPoint, this));
}

void PointSetting::SwitchPage() {
  point_page_ = !point_page_;
  ui->stack_widget_content_->setCurrentWidget(
      point_page_ ? ui->page_image_view_ : ui->page_point_list_);
}
void PointSetting::AddPoint() {
  if (!is_save_) {
    return;
  }
  is_save_ = false;

  ui->table_point_list_->insertRow(ui->table_point_list_->rowCount());
  current_row_ = ui->table_point_list_->rowCount() - 1;
  ui->table_point_list_->selectRow(current_row_);
  UnitPoint default_point_para;
  AppendItemByRow(current_row_, default_point_para.point_ids);
}
void PointSetting::DeletePoint() {
  int save_rows = GetRowCount(current_unit_ids_);
  if (current_row_ > save_rows || current_row_ < 0) {
    return;
  }

  ui->table_point_list_->removeRow(current_row_);
  buttons_goto_point_.erase(buttons_goto_point_.begin() + current_row_);
  buttons_set_.erase(buttons_set_.begin() + current_row_);

  if (current_row_ == save_rows) {
    --current_row_;
    ui->table_point_list_->selectRow(current_row_);
    is_save_ = true;
    return;
  }

  std::string current_point_ids = current_point_.point_ids;
  unit_point_mgr_->DelUnitPoint(current_point_ids);
  point_config_->DeletePoint(current_point_ids.c_str());
  point_config_->Save();
  --current_row_;
  ui->table_point_list_->selectRow(current_row_);
}
void PointSetting::SavePoint(int row) {
  int save_rows = GetRowCount(current_unit_ids_);
  // 行数错误
  if (current_row_ > save_rows || current_row_ < 0) {
    return;
  }

  UnitPoint point_read;
  ReadPoint(point_read, current_row_);
  point_read.unit_ids = current_unit_ids_;
  // 名称不能为空
  if (point_read.point_ids.empty()) {
    return;
  }

  // 判断是否重复添加
  UnitPoint point_exit =
      unit_point_mgr_->GetUnitPointByIds(point_read.unit_ids);
  if (point_exit.point_ids != "temp_point") {
    if (current_row_ == save_rows) {
      ui->table_point_list_->item(current_row_, 0)->setText("");
    }
  }
  unit_point_mgr_->AddUnitPoint(point_read);
  OnUnitClicked(current_unit_index_);
}
void PointSetting::OnPointSelect(int row, int col) {
  current_row_ = row;
  for (int i = 0; i < axis_num_; ++i) {
    if (!checkboxs_axis_[i].isNull()) {
      checkboxs_axis_[i]->setChecked(false);
    }
  }
  int save_rows = GetRowCount(current_unit_ids_);
  if (current_row_ >= save_rows) {
    return;
  }
  std::string point_ids =
      ui->table_point_list_->item(row, 0)->text().toStdString();
  current_point_ = unit_point_mgr_->GetUnitPointByIds(point_ids);

  yotta::PointConfigItemPtr point_config =
      point_config_->GetPointByIds(point_ids.c_str());
  if (!point_config) {
    return;
  }
  std::vector<std::string> vec_axis;
  for (int j = 0; j < point_config->GetPointAxisItemCount(); ++j) {
    yotta::PointAxisConfigItemPtr point_axis =
        point_config->GetPointAxisItem(j);
    std::string axis_ids;
    char axis_ids_char[kAxisIdBufferSize];
    double pos = 0;
    point_axis->GetAxisIds(axis_ids_char, kAxisIdBufferSize);
    axis_ids = axis_ids_char;
    vec_axis.push_back(axis_ids);
  }

  for (int i = 0; i < vec_axis.size(); ++i) {
    for (int j = 0; j < axis_names_.size(); ++j) {
      if (vec_axis[i] == axis_names_[j].toStdString()) {
        if (!checkboxs_axis_[j].isNull()) {
          checkboxs_axis_[j]->setChecked(true);
        }
        break;
      }
    }
  }
}
void PointSetting::ReadPoint(UnitPoint &point_para, int row) {
  point_para.point_ids =
      ui->table_point_list_->item(row, 0)->text().toStdString();
  if (point_para.point_ids == "") {
    return;
  }
  yotta::PointConfigItemPtr point_config_item =
      point_config_->GetPointByIds(point_para.point_ids.c_str());
  if (!point_config_item) {
    point_config_item = point_config_->AddPoint(point_para.point_ids.c_str());
  }

  for (int col = 0; col < axis_num_; ++col) {
    if (!checkboxs_axis_[col]->isChecked()) {
      continue;
    }
    double pos = ui->table_point_list_->item(row, col + 1)->text().toDouble();
    yotta::PointAxisConfigItemPtr point_axis =
        point_config_item->GetPointAxisItemByIds(
            axis_names_[col].toStdString().c_str());
    if (!point_axis) {
      point_axis = point_config_item->AddPointAxisItem(
          axis_names_[col].toStdString().c_str());
    }
    if (!point_axis) {
      continue;
    }
    point_axis->SetPosition(pos);
  }
  point_config_->Save();
}

int PointSetting::GetRowCount(const std::string &unit_ids) {
  int save_rows = 0;
  for (int i = 0; i < unit_point_mgr_->GetUnitPointCount(); ++i) {
    if (unit_ids == unit_point_mgr_->GetUnitPoint(i).unit_ids) {
      ++save_rows;
    }
  }
  return save_rows;
}

void PointSetting::GotoPoint() {
  if (current_point_.point_ids.empty()) {
    return;
  }

  yotta::LimitMotionMgrPtr limit_motion =
      main_process::GetModuleMgr()->GetLimitMotionMgr();
  if (!limit_motion) {
    LOG(ERROR) << "get limit_motion error";
    return;
  }
  yotta::PointConfigItemPtr point_config =
      point_config_->GetPointByIds(current_point_.point_ids.c_str());
  if (!point_config) {
    return;
  }
  for (int j = 0; j < point_config->GetPointAxisItemCount(); ++j) {
    yotta::PointAxisConfigItemPtr point_axis_config =
        point_config->GetPointAxisItem(j);
    std::string axis_ids;
    char axis_ids_char[kAxisIdBufferSize];
    double dis_position = 0;
    point_axis_config->GetAxisIds(axis_ids_char, kAxisIdBufferSize);
    point_axis_config->GetPosition(&dis_position);
    axis_ids = axis_ids_char;

    yotta::AxisConfigItemPtr axis_config =
        axis_config_->GetAxisByIds(axis_ids.c_str());
    if (!axis_config) {
      LOG(ERROR) << "get axis_config error";
      return;
    }
    yotta::Axis *axis_motion = limit_motion->GetAxisByIds(axis_ids.c_str());
    if (!axis_motion) {
      LOG(ERROR) << "get axis_motion error";
      return;
    }
    yotta::AccDecProfileImpl acc_dec_profile(0);
    yotta::AxisSpeedConfigPtr speed = axis_config->GetSpeed(0);
    if (!speed) {
      LOG(ERROR) << "get axis speed error";
      return;
    }
    double val = 0;
    speed->GetAcc(&val);
    acc_dec_profile.set_acceleration(val);
    speed->GetDec(&val);
    acc_dec_profile.set_deceleration(val);
    speed->GetVelocity(&val);
    acc_dec_profile.set_velocity(val);

    axis_motion->AsyncMoveTo(dis_position, &acc_dec_profile);
  }
}

void PointSetting::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);

  // 延迟挂载共享控件，减少闪烁
  QTimer::singleShot(0, this, [this]() {
    auto shared_manager = SharedManager::GetInstance();

    if (ui->image_show_layout) {
      shared_manager->RequestImageMount(this, ui->image_show_layout);
    }

    if (ui->manual_control_layout) {
      shared_manager->RequestControlMount(this, ui->manual_control_layout);
    }
  });
}
