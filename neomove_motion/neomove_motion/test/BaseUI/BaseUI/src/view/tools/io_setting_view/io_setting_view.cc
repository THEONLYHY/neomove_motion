// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#include "io_setting_view.h"

#include <QDialog>
#include <QMouseEvent>

#include "controller/device_status_monitor/device_status_monitor.h"
#include "main_process/module_mgr.h"
#include "view/components/ps_button/ps_button.h"
#include "view/components/ps_label/ps_label.h"
#include "view/tools/limit_setting_view/limit_setting_view.h"

constexpr int kNumSelectData = 256;

IoSettingView::IoSettingView(QWidget* parent)
    : QWidget(parent), ui(new Ui::IoSettingviewClass()) {
  ui->setupUi(this);
  io_config_ = yotta::ConfigFactory::GetInstance()->GetIoConfig();

  UnitInfoMgrPtr unit_mgr = ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!unit_mgr) {
    return;
  }
  bool first_check = true;  //选中第一个工位
  for (int i = 0; i < unit_mgr->GetUnitCount(); i++) {
    QPushButton* axis_btn = new PsButton(
        QString::fromStdString(unit_mgr->GetUnitInfo(i)->unit_ids));
    axis_btn->setStyleSheet(R"(QPushButton {
        background-color: #4A86E8;
                border: none;
                border-radius: 8px;
        font-size: 24px;
        color: white;
    }
        QPushButton:checked  {
        background-color: #F39C12;
                border: none;
                border-radius: 8px;
        font-size: 24px;
        color: black;
    })");
    axis_btn->setFixedSize(160, 69);
    axis_btn->setCheckable(true);
    if (first_check) {
      axis_btn->setChecked(first_check);
      module_id_ = unit_mgr->GetUnitInfo(i)->unit_ids;
      first_check = false;
    }
    ui->module_list_lay->addWidget(axis_btn);
    btn_group_module_.addButton(axis_btn, i);
  }
  ui->module_list_lay->addStretch();

  for (QAbstractButton* button : btn_group_module_.buttons()) {
    connect(button, &QAbstractButton::toggled, this,
            [this, button](bool checked) {
              if (!checked) {
                return;
              }
              int id = btn_group_module_.id(button);
              OnModuleClicked(id);
            });
  }

  ui->input_io_tab_btn->Color().SetBaseColor(PsColor::Color::kBlue);
  ui->output_io_tab_btn->Color().SetBaseColor(PsColor::Color::kBlue);
  ui->axis_io_tab_btn->Color().SetBaseColor(PsColor::Color::kBlue);

  ui->input_io_tab_btn->setCheckable(true);
  ui->output_io_tab_btn->setCheckable(true);
  ui->axis_io_tab_btn->setCheckable(true);

  tab_btn_group_.addButton(ui->input_io_tab_btn, 0);
  tab_btn_group_.addButton(ui->output_io_tab_btn, 1);
  tab_btn_group_.addButton(ui->axis_io_tab_btn, 2);
  tab_btn_group_.setExclusive(true);

  for (QAbstractButton* button : tab_btn_group_.buttons()) {
    connect(button, &QAbstractButton::toggled, this,
            [this, button](bool checked) {
              if (!checked) {
                return;
              }
              ui->stackedWidget->setCurrentIndex(tab_btn_group_.id(button));
            });
  }

  ui->input_io_tab_btn->setChecked(true);
  ui->stackedWidget->setCurrentIndex(0);

  ShowModule();

  // 定时器实时监测io状态
  time_monitor_ = new QTimer(this);
  connect(time_monitor_, &QTimer::timeout, this, &IoSettingView::UpdateTimer);
  // Don't start timer here - it will be started in onPageShow()
}

IoSettingView::~IoSettingView() {
  if (time_monitor_ && time_monitor_->isActive()) {
    time_monitor_->stop();
  }
  delete ui;
}

void IoSettingView::showEvent(QShowEvent* event) {
  if (time_monitor_) {
    time_monitor_->start(500);  // Start timer when page is shown
    LOG(INFO) << "IoSettingView: Timer started";
  }
}

void IoSettingView::hideEvent(QHideEvent* event) {
  if (time_monitor_ && time_monitor_->isActive()) {
    time_monitor_->stop();  // Stop timer when page is hidden
    LOG(INFO) << "IoSettingView: Timer stopped";
  }
}

void IoSettingView::OnModuleClicked(int module_index) {
  UnitInfoMgrPtr motion_module =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!motion_module) {
    return;
  }
  if (module_index < motion_module->GetUnitCount()) {
    module_id_ = motion_module->GetUnitInfo(module_index)->unit_ids;
    time_monitor_->stop();
    ShowModule();
    time_monitor_->start(100);
  }
}

void clearLayout(QLayout* layout) {
  if (!layout) return;
  QLayoutItem* item;
  while ((item = layout->takeAt(0)) != nullptr) {
    if (item->widget()) {
      item->widget()->deleteLater();  // 移除部件
    } else if (item->layout()) {
      clearLayout(item->layout());    // 递归移除子布局
      item->layout()->deleteLater();  // 删除子布局
    } else if (item->spacerItem()) {
      delete item->spacerItem();  // 处理间隔项
    }
    //  delete item;  // 删除布局项
  }
}

void IoSettingView::ShowModule() {
  UnitInfoMgrPtr motion_module =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!motion_module) {
    return;
  }
  UnitInfoPtr module_axis_info = motion_module->GetUnitInfo(module_id_);
  if (!module_axis_info) {
    return;
  }
  io_input_.clear();
  io_output_.clear();
  io_axis_.clear();
  io_in_relax.clear();
  io_out_relax.clear();
  io_axis_relax.clear();

  // Clear cache when switching modules
  cached_io_values_.clear();
  // 输入IO
  QLayout* input_io_layout_old = ui->io_input->layout();
  if (input_io_layout_old) {
    clearLayout(input_io_layout_old);
    delete input_io_layout_old;
  }
  QGridLayout* input_io_layout = new QGridLayout();
  input_io_layout->setSizeConstraint(QLayout::SetFixedSize);
  input_io_layout->setContentsMargins(0, 0, 0, 0);
  input_io_layout->setSpacing(10);
  /* 输入排列[7 -> 5] */
  uint8_t num_of_column = 5;
  yotta::MultiIoPortConfigPtr io_in_config = io_config_->GetInputIo();
  int input_io_index = 0;
  for (int i = 0; i < io_in_config->GetIoPortConfigCount(); i++) {
    yotta::IoPortConfigPtr io_in_port = io_in_config->GetIoPortConfig(i);

    std::string io_name;
    int io_addr = 0;
    int io_bit = 0;
    char io_name_char[256];
    io_in_port->GetIds(io_name_char, 256);
    io_name = io_name_char;
    io_in_port->GetAddr(&io_addr);
    io_in_port->GetBit(&io_bit);

    int belong_module = 0;
    for (int j = 0; j < module_axis_info->io_input_names.size(); j++) {
      if (io_name == module_axis_info->io_input_names[j]) {
        belong_module = 1;
      }
    }
    if (!belong_module) {
      continue;
    }

    // 创建布局
    QHBoxLayout* io_device = new QHBoxLayout();
    io_device->setContentsMargins(0, 0, 0, 0);
    io_device->setSpacing(0);

    // 只读, 使用标签
    QLabel* io_pbt = new PsLabel();
    io_pbt->setText(QString(io_name.c_str()));
    io_pbt->setFixedSize(160, 69);
    io_pbt->setAlignment(Qt::AlignCenter);
    static_cast<PsLabel*>(io_pbt)->Color().SetBaseColor(PsColor::Color::kBlue);

    // 数据框
    QLabel* io_edit = new PsLabel();
    io_edit->setFixedSize(60, 69);
    io_edit->setFont(QFont(font().family(), 20));
    io_edit->setCursor(Qt::PointingHandCursor);
    io_edit->installEventFilter(this);
    io_edit->setText(QString::number(io_addr) + "." + QString::number(io_bit));
    static_cast<PsLabel*>(io_edit)->Color().SetBaseColor(PsColor::Color::kGreen);

    // 将数据框和按钮加入布局
    io_device->addWidget(io_pbt);
    io_device->addWidget(io_edit);
    io_input_.push_back(io_edit);
    io_in_relax.insert(std::pair<QLabel*, QLabel*>(io_edit, io_pbt));
    // 外部大窗口添加布局
    input_io_layout->addLayout(io_device, input_io_index / num_of_column,
                               input_io_index % num_of_column);
    input_io_index++;
  }
  ui->io_input->setLayout(input_io_layout);

  //输出IO
  QLayout* output_io_layout_old = ui->io_output->layout();
  if (output_io_layout_old) {
    clearLayout(output_io_layout_old);
    delete output_io_layout_old;
  }
  QGridLayout* output_io_layout = new QGridLayout();
  output_io_layout->setSizeConstraint(QLayout::SetFixedSize);
  output_io_layout->setContentsMargins(0, 0, 0, 0);
  output_io_layout->setSpacing(10);
  /* 输出排列[5 -> 4] */
  num_of_column = 4;
  yotta::MultiIoPortConfigPtr io_out_config = io_config_->GetOutputIo();
  int output_io_index = 0;
  for (int i = 0; i < io_out_config->GetIoPortConfigCount(); i++) {
    yotta::IoPortConfigPtr io_out_port = io_out_config->GetIoPortConfig(i);

    std::string io_ids;
    int io_addr = 0;
    int io_bit = 0;
    char io_name_char[256];
    io_out_port->GetIds(io_name_char, 256);
    io_out_port->GetAddr(&io_addr);
    io_out_port->GetBit(&io_bit);
    io_ids = io_name_char;

    int belong_module = 0;
    for (int j = 0; j < module_axis_info->io_output_names.size(); j++) {
      if (io_ids == module_axis_info->io_output_names[j]) {
        belong_module = 1;
      }
    }
    if (!belong_module) {
      continue;
    }

    // 创建容器 Widget
    QWidget* container = new QWidget(ui->io_output);
    container->setFixedSize(290, 70);

    // 创建布局
    QHBoxLayout* io_device = new QHBoxLayout(container);
    io_device->setContentsMargins(0, 0, 0, 0);
    io_device->setSpacing(0);
    // 按钮
    QPushButton* io_pbt = new PsButton(container);
    io_pbt->setText(QString(io_ids.c_str()));
    io_pbt->setFixedSize(160, 69);
    // 取消checkable
    io_pbt->setCheckable(false);

    // 按钮 用来打开限制界面
    QPushButton* io_info_limit = new PsButton(container);
    io_info_limit->setText(tr("限制"));
    io_info_limit->setFixedSize(69, 69);

    connect(io_info_limit, &QPushButton::clicked, this,
            [this, io_info_limit, io_ids] {
              LimitSettingView limit_view;
              limit_view.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
              limit_view.setAttribute(Qt::WA_TranslucentBackground);
              limit_view.SetCondition(1, "", io_ids);
              limit_view.exec();
            });

    // 数据框
    QLabel* io_edit = new PsLabel(container);
    io_edit->setFixedSize(60, 69);
    io_edit->setFont(QFont(font().family(), 20));
    io_edit->setCursor(Qt::PointingHandCursor);
    io_edit->installEventFilter(this);
    io_edit->setText(QString::number(io_addr) + "." + QString::number(io_bit));
    static_cast<PsLabel*>(io_edit)->Color().SetBaseColor(PsColor::Color::kGreen);
    // 将数据框和按钮加入容器
    io_device->addWidget(io_pbt);
    io_device->addWidget(io_edit);
    io_device->addWidget(io_info_limit);
    io_output_.push_back(io_edit);
    io_out_relax.insert(std::pair<QLabel*, QPushButton*>(io_edit, io_pbt));
    // 添加控件
    output_io_layout->addWidget(container, output_io_index / num_of_column,
                                output_io_index % num_of_column);
    output_io_index++;

    connect(io_pbt, &QPushButton::clicked, this, [this, io_pbt, io_ids] {
      uint8_t value;
      bool got_value = DeviceStatusMonitorSinglton::GetInstance()->GetOutputIoValue(io_ids, value);
      if (!got_value) {
        return;
      }
      SlotIoClick(io_ids, value == 0);
    });
  }
  ui->io_output->setLayout(output_io_layout);

  //轴IO
  QLayout* axis_io_layout_old = ui->io_axis->layout();
  if (axis_io_layout_old) {
    clearLayout(axis_io_layout_old);
    delete axis_io_layout_old;
  }
  QVBoxLayout* axis_io_layout = new QVBoxLayout();
  axis_io_layout->setSizeConstraint(QLayout::SetFixedSize);
  axis_io_layout->setContentsMargins(0, 0, 0, 0);
  axis_io_layout->setSpacing(10);
  for (int i = 0; i < io_config_->GetAxisIoCount(); i++) {
    QHBoxLayout* axis_io_layout_1 = new QHBoxLayout();
    yotta::AxisIoConfigPtr axis_io_config = io_config_->GetAxisIo(i);

    std::string axis_name;
    char axis_name_char[256];
    axis_io_config->GetIds(axis_name_char, 256);
    axis_name = axis_name_char;
    int belong_module = 0;
    for (int j = 0; j < module_axis_info->axis_ids.size(); j++) {
      if (axis_name == module_axis_info->axis_ids[j]) {
        belong_module = 1;
      }
    }
    if (!belong_module) {
      continue;
    }

    for (int j = 0; j < axis_io_config->GetIoPortConfigCount(); j++) {
      yotta::IoPortConfigPtr io_axis_port = axis_io_config->GetIoPortConfig(j);

      std::string io_name;
      int io_addr = 0;
      int io_bit = 0;
      char io_name_char[256];
      io_axis_port->GetIds(io_name_char, 256);
      io_axis_port->GetAddr(&io_addr);
      io_axis_port->GetBit(&io_bit);
      io_name = io_name_char;

      // 创建布局
      QHBoxLayout* io_device = new QHBoxLayout();
      io_device->setContentsMargins(0, 0, 0, 0);
      io_device->setSpacing(0);
      // 只读, 使用标签
      QLabel* io_pbt = new PsLabel();
      io_pbt->setText(QString(io_name.c_str()));
      io_pbt->setFixedSize(160, 69);
      io_pbt->setAlignment(Qt::AlignCenter);
      static_cast<PsLabel*>(io_pbt)->Color().SetBaseColor(PsColor::Color::kBlue);

      // 数据框
      QLabel* io_edit = new PsLabel();
      io_edit->setFixedSize(60, 69);
      io_edit->setFont(QFont(font().family(), 20));
      io_edit->setCursor(Qt::PointingHandCursor);
      io_edit->installEventFilter(this);
      io_edit->setText(QString::number(io_addr) + "." +
                       QString::number(io_bit));
      static_cast<PsLabel*>(io_edit)->Color().SetBaseColor(PsColor::Color::kGreen);
      // 将数据框和按钮加入容器
      io_device->addWidget(io_pbt);
      io_device->addWidget(io_edit);
      io_axis_.push_back(io_edit);
      io_axis_relax.insert(
          std::pair<QLabel*, QLabel*>(io_edit, io_pbt));
      axis_io_layout_1->addLayout(io_device);
    }
    axis_io_layout->addLayout(axis_io_layout_1);
  }
  ui->io_axis->setLayout(axis_io_layout);
}
void IoSettingView::SlotIoClick(std::string io_ids, bool state) {
  yotta::LimitMotionMgrPtr limit_motion =
      main_process::GetModuleMgr()->GetLimitMotionMgr();
  if (!limit_motion) {
    LOG(ERROR) << "get limit_motion error";
    return;
  }
  yotta::OutputIO* output_io = limit_motion->GetOutputIoByIds(io_ids.c_str());
  if (!output_io) {
    LOG(ERROR) << "get output_io error";
    return;
  }
  output_io->WriteValue(state);
}

bool IoSettingView::eventFilter(QObject* watched, QEvent* event) {
  // 判断事件源是否为QLabel类型
  QLabel* edit = qobject_cast<QLabel*>(watched);
  if (edit) {
    // 处理鼠标按下事件
    if (event->type() == QEvent::MouseButtonPress) {
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

      // 创建数据选择对话框 由于只需要一次所以 使用局部变量
      // 每次调用会生成新的对话框
      QDialog select_io_parameter;
      select_io_parameter.setMinimumSize(640, 480);
      // 设置布局
      QGridLayout input_io_layout(&select_io_parameter);
      // 控件排列
      uint8_t num_of_row = 0;
      uint8_t num_of_column = 0;
      for (int i = 0; i < kNumSelectData; ++i) {
        // 这个按钮由于外部使用和呈现，如果是局部变量那么每次for循环都会删除上一个创建的控件，所以必须在堆上创建
        // 设置了父对象是外面的局部变量 析构时会自动删除
        QPushButton* select_num_button = new PsButton(&select_io_parameter);
        select_num_button->setFixedSize(50, 50);
        select_num_button->setStyleSheet(
            "QPushButton {"
            "    font-family: 'SimHei', 'Microsoft YaHei', sans-serif;"
            "    font-size: 15px;"
            "    color: black;"
            "}");
        // 格式化数字选择
        double value = static_cast<double>(num_of_column + 0.1 * num_of_row);
        QString formatted = QString::number(value, 'f', 1);
        select_num_button->setText(formatted);

        input_io_layout.addWidget(select_num_button, num_of_row, num_of_column);
        // 更新行列位置
        ++num_of_column;
        if (num_of_column >= 32) {
          ++num_of_row;
          num_of_column = 0;
        }

        // 连接点击信号 选择数据 这里可以获取用户选择的数据 按需提供给外部使用
        QString tmp_num = select_num_button->text();
        connect(select_num_button, &QPushButton::clicked, this,
                [edit, tmp_num, &select_io_parameter, this,
                 io_index = tmp_num.toInt()] {
                  edit->setText(tmp_num);
                  select_io_parameter.close();
                  OnSaveClicked();
                });
      }
      select_io_parameter.exec();
    }
  }
  return false;
}

void IoSettingView::OnSaveClicked() {
  for (QLabel* input_item : io_input_) {
    QStringList parts = input_item->text().split('.');
    int addr = parts[0].toInt();
    int bit = parts[1].toInt();
    std::string input_name = io_in_relax[input_item]->text().toStdString();

    yotta::IoPortConfigPtr io_port =
        io_config_->GetIoPortConfig(input_name.c_str());
    if (io_port) {
      io_port->SetAddr(addr);
      io_port->SetBit(bit);
    }
  }
  for (QLabel* output_item : io_output_) {
    QStringList parts = output_item->text().split('.');
    int addr = parts[0].toInt();
    int bit = parts[1].toInt();
    std::string output_name = io_out_relax[output_item]->text().toStdString();

    yotta::IoPortConfigPtr io_port =
        io_config_->GetIoPortConfig(output_name.c_str());
    if (io_port) {
      io_port->SetAddr(addr);
      io_port->SetBit(bit);
    }
  }
  for (QLabel* axis_item : io_axis_) {
    QStringList parts = axis_item->text().split('.');
    int addr = parts[0].toInt();
    int bit = parts[1].toInt();
    std::string axis_name = io_axis_relax[axis_item]->text().toStdString();

    yotta::IoPortConfigPtr io_port =
        io_config_->GetIoPortConfig(axis_name.c_str());
    if (io_port) {
      io_port->SetAddr(addr);
      io_port->SetBit(bit);
    }
  }

  io_config_->Save();
  DeviceStatusMonitorSinglton::GetInstance()->RefreshDeviceList();
}

void IoSettingView::UpdateTimer() {
  DeviceStatusMonitor* monitor = DeviceStatusMonitorSinglton::GetInstance();

  for (QLabel* input_item : io_input_) {
    std::string input_io_ids = io_in_relax[input_item]->text().toStdString();
    uint8_t value = 0;
    bool got_value = monitor->GetInputIoValue(input_io_ids, value);

    // Convert to state: 0=offline/unknown, 1=off, 2=on
    uint8_t state = got_value ? (value == 1 ? 2 : 1) : 0;

    // Only update UI if state changed
    if (cached_io_values_[input_io_ids] != state) {
      if (got_value) {
        value == 1 ? static_cast<PsLabel*>(input_item)->Color().SetBaseColor(PsColor::Color::kGreen)
                   : static_cast<PsLabel*>(input_item)->Color().SetBaseColor(PsColor::Color::kRed);
      } else {
        static_cast<PsLabel*>(input_item)->Color().SetBaseColor(PsColor::Color::kGray);
      }
      cached_io_values_[input_io_ids] = state;
    }
  }

  for (QLabel* output_item : io_output_) {
    std::string output_io_ids = io_out_relax[output_item]->text().toStdString();
    uint8_t value = 0;
    bool got_value = monitor->GetOutputIoValue(output_io_ids, value);

    // Convert to state: 0=offline/unknown, 1=off, 2=on
    uint8_t state = got_value ? (value == 1 ? 2 : 1) : 0;

    // Only update UI if state changed
    if (cached_io_values_[output_io_ids] != state) {
      if (got_value) {
        value == 1 ? static_cast<PsLabel*>(output_item)->Color().SetBaseColor(PsColor::Color::kGreen)
                   : static_cast<PsLabel*>(output_item)->Color().SetBaseColor(PsColor::Color::kRed);
      } else {
        static_cast<PsLabel*>(output_item)->Color().SetBaseColor(PsColor::Color::kGray);
      }
      cached_io_values_[output_io_ids] = state;
    }
  }

  for (QLabel* axis_item : io_axis_) {
    std::string axis_io_ids = io_axis_relax[axis_item]->text().toStdString();
    uint8_t value = 0;
    bool got_value = monitor->GetInputIoValue(axis_io_ids, value);

    // Convert to state: 0=offline/unknown, 1=off, 2=on
    uint8_t state = got_value ? (value == 1 ? 2 : 1) : 0;

    // Only update UI if state changed
    if (cached_io_values_[axis_io_ids] != state) {
      if (got_value) {
        value == 1 ? static_cast<PsLabel*>(axis_item)->Color().SetBaseColor(PsColor::Color::kGreen)
                   : static_cast<PsLabel*>(axis_item)->Color().SetBaseColor(PsColor::Color::kRed);
      } else {
        static_cast<PsLabel*>(axis_item)->Color().SetBaseColor(PsColor::Color::kGray);
      }
      cached_io_values_[axis_io_ids] = state;
    }
  }
}