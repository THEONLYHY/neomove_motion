#include "limit_setting_view.h"
#include "view/tools/utils/style_utils.h"

LimitSettingView::LimitSettingView(QWidget* parent)
    : QDialog(parent), ui(new Ui::limit_setting_viewClass()) {
  ui->setupUi(this);
  io_config_ = yotta::ConfigFactory::GetInstance()->GetIoConfig();
  axis_config_ = yotta::ConfigFactory::GetInstance()->GetAxisConfig();
  limit_rule_mgr_config_ =
      yotta::ConfigFactory::GetInstance()->GetLimitRuleMgrConfig();
  {
    ui->tableWidget_axis_states_->setShowGrid(true);  // 显示表格线
    ui->tableWidget_axis_states_->setVerticalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);  // 隐藏滑动条
    ui->tableWidget_axis_states_->setSelectionBehavior(
        QAbstractItemView::SelectRows);  // 设置行选择模式
    ui->limit_list->setSelectionMode(QAbstractItemView::SingleSelection);
    // ui->tableWidget_decice_list->setRowHeight(69);
    ui->tableWidget_axis_states_->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget_axis_states_->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);  // 先自适应宽度
    // ui->tableWidget_axis_states_->horizontalHeader()->setSectionResizeMode(
    //    0, QHeaderView::ResizeToContents);  // 按文本长度自动确定每列宽度
    ui->tableWidget_axis_states_->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Fixed);  // 固定0列宽度
    ui->tableWidget_axis_states_->setColumnWidth(0, 150);
    ui->tableWidget_axis_states_->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { color: black; font-weight: normal; "
        "background-color: white; }");
  }
  {
    ui->tableWidget_axis_region_states_->setShowGrid(true);  // 显示表格线
    ui->tableWidget_axis_region_states_->setVerticalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);  // 隐藏滑动条
    ui->tableWidget_axis_region_states_->setSelectionBehavior(
        QAbstractItemView::SelectRows);  // 设置行选择模式
    ui->limit_list->setSelectionMode(QAbstractItemView::SingleSelection);
    // ui->tableWidget_decice_list->setRowHeight(69);
    ui->tableWidget_axis_region_states_->setContextMenuPolicy(
        Qt::CustomContextMenu);
    ui->tableWidget_axis_region_states_->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::Stretch);  // 先自适应宽度
    // ui->tableWidget_axis_region_states_->horizontalHeader()
    //    ->setSectionResizeMode(
    //        0, QHeaderView::ResizeToContents);  // 按文本长度自动确定每列宽度
    ui->tableWidget_axis_region_states_->horizontalHeader()
        ->setSectionResizeMode(0, QHeaderView::Fixed);  // 固定0列宽度
    ui->tableWidget_axis_region_states_->setColumnWidth(0, 150);
    ui->tableWidget_axis_region_states_->horizontalHeader()
        ->setSectionResizeMode(1, QHeaderView::Fixed);  // 固定1列宽度
    ui->tableWidget_axis_region_states_->setColumnWidth(1, 150);
    ui->tableWidget_axis_region_states_->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { color: black; font-weight: normal; "
        "background-color: white; }");
  }
  {
    ui->tableWidget_io_states_->setShowGrid(true);  // 显示表格线
    ui->tableWidget_io_states_->setVerticalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);  // 隐藏滑动条
    ui->tableWidget_io_states_->setSelectionBehavior(
        QAbstractItemView::SelectRows);  // 设置行选择模式
    ui->limit_list->setSelectionMode(QAbstractItemView::SingleSelection);
    // ui->tableWidget_decice_list->setRowHeight(69);
    ui->tableWidget_io_states_->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget_io_states_->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);  // 先自适应宽度
    // ui->tableWidget_io_states_->horizontalHeader()->setSectionResizeMode(
    //    0, QHeaderView::ResizeToContents);  // 按文本长度自动确定每列宽度
    ui->tableWidget_io_states_->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Fixed);  // 固定0列宽度
    ui->tableWidget_io_states_->setColumnWidth(0, 150);
    ui->tableWidget_io_states_->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { color: black; font-weight: normal; "
        "background-color: white; }");
  }

  QObject::connect(ui->button_add_axis_states_, &QPushButton::clicked, this,
                   &LimitSettingView::OnAddLimitAxisStatesButtonClicked);
  QObject::connect(ui->button_del_axis_states_, &QPushButton::clicked, this,
                   &LimitSettingView::OnDelLimitAxisStatesButtonClicked);
  QObject::connect(ui->button_add_axis_region_, &QPushButton::clicked, this,
                   &LimitSettingView::OnAddLimitAxisRegionStatesButtonClicked);
  QObject::connect(ui->button_del_axis_region_, &QPushButton::clicked, this,
                   &LimitSettingView::OnDelLimitAxisRegionStatesButtonClicked);
  QObject::connect(ui->button_add_io_states_, &QPushButton::clicked, this,
                   &LimitSettingView::OnAddLimitIOStatesButtonClicked);
  QObject::connect(ui->button_del_io_states_, &QPushButton::clicked, this,
                   &LimitSettingView::OnDelLimitIOStatesButtonClicked);
  QObject::connect(ui->button_add_limit_, &QPushButton::clicked, this,
                   &LimitSettingView::OnAddLimitButtonClicked);
  QObject::connect(ui->button_del_limit_, &QPushButton::clicked, this,
                   &LimitSettingView::OnDelLimitButtonClicked);
  QObject::connect(ui->button_save_limit_, &QPushButton::clicked, this,
                   &LimitSettingView::OnSaveLimitButtonClicked);

  ui->button__io_limit->setEnabled(false);
  ui->button__axis_limit->setEnabled(false);
}
LimitSettingView::~LimitSettingView() { delete ui; }

void LimitSettingView::SetCondition(int limit_type, std::string axis_ids,
                                    std::string io_ids) {
  limit_type_ = limit_type;
  if (limit_type_) {
    ui->button__io_limit->setChecked(true);
    ui->button__axis_limit->setChecked(false);

    ui->limit_list->setColumnCount(2);
    QStringList horizontalHeaders;
    horizontalHeaders << tr("Limit ID") << tr("Mode");
    ui->limit_list->setHorizontalHeaderLabels(horizontalHeaders);

  } else {
    ui->button__io_limit->setChecked(false);
    ui->button__axis_limit->setChecked(true);

    ui->limit_list->setColumnCount(4);
    QStringList horizontalHeaders;
    horizontalHeaders << tr("Limit ID") << tr("Mode") << tr("Negative")
                      << tr("Positive");
    ui->limit_list->setHorizontalHeaderLabels(horizontalHeaders);
  }
  {
    ui->limit_list->setShowGrid(true);  // 显示表格线
    ui->limit_list->setVerticalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);  // 隐藏滑动条
    ui->limit_list->setSelectionBehavior(
        QAbstractItemView::SelectRows);  // 设置行选择模式
    ui->limit_list->setSelectionMode(QAbstractItemView::SingleSelection);
    // ui->tableWidget_decice_list->setRowHeight(69);
    ui->limit_list->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->limit_list->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);  // 先自适应宽度
    ui->limit_list->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Fixed);  // 固定0列宽度
    ui->limit_list->setColumnWidth(0, 150);
    if (limit_type_) {
      ui->limit_list->setColumnWidth(0, 250);  // IO名字一般比较长
    }
    ui->limit_list->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { color: black; font-weight: normal; "
        "background-color: white; }");
    connect(ui->limit_list, &QTableWidget::cellClicked, this,
            &LimitSettingView::OnLimitListSelect);
  }
  axis_ids_ = axis_ids;
  io_ids_ = io_ids;
  ShowLimitCondition();
  InitUI();
}

void LimitSettingView::InitUI() {
  // 160x69 按钮样式
  StyleUtils::ApplyStyle(ui->button__axis_limit, "btn_icon_deepblue_160_69");
  StyleUtils::ApplyStyle(ui->button__io_limit, "btn_icon_deepblue_160_69");
  // 140x69 按钮样式

  StyleUtils::ApplyStyle(ui->button_add_limit_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui->button_del_limit_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui->button_save_limit_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui->button_add_axis_states_,
                         "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui->button_del_axis_states_,
                         "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui->button_add_axis_region_,
                         "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui->button_del_axis_region_,
                         "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui->button_add_io_states_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui->button_del_io_states_, "btn_icon_deepblue_140_69");
}

QComboBox* LimitSettingView::GetAxisLimitTypeComboBox() {
  QComboBox* axis_limit_type_combox = new QComboBox;
  axis_limit_type_combox->addItem(tr("LockMotion"));
  axis_limit_type_combox->addItem(tr("RelativeLimit"));
  axis_limit_type_combox->addItem(tr("AbsoluteLimit"));
  axis_limit_type_combox->addItem(tr("LockHoming"));
  StyleUtils::ApplyStyle(axis_limit_type_combox, "input_box");
  return axis_limit_type_combox;
}
QComboBox* LimitSettingView::GetIoLimitTypeComboBox() {
  QComboBox* io_limit_type_combox = new QComboBox;
  io_limit_type_combox->addItem(tr("LockIoState"));
  io_limit_type_combox->addItem(tr("LockIoPositive"));
  io_limit_type_combox->addItem(tr("LockIoNegative"));
  StyleUtils::ApplyStyle(io_limit_type_combox, "input_box");
  return io_limit_type_combox;
}
QComboBox* LimitSettingView::GetAxisNameComboBox() {
  QComboBox* axis_name_combox = new QComboBox;
  for (int i = 0; i < axis_config_->GetAxisCount(); i++) {
    yotta::AxisConfigItemPtr axis_cofig_item = axis_config_->GetAxis(i);
    if (!axis_cofig_item) {
      return axis_name_combox;
      LOG(ERROR) << "get axis_cofig_item failed axis_index=  " << i;
    }
    std::string axis_ids;
    char axis_ids_char[256];
    axis_cofig_item->GetIds(axis_ids_char, 256);
    axis_ids = axis_ids_char;
    axis_name_combox->addItem(QString::fromStdString(axis_ids));
  }
  StyleUtils::ApplyStyle(axis_name_combox, "input_box");
  return axis_name_combox;
}
QComboBox* LimitSettingView::GetAxisStatesComboBox() {
  QComboBox* axis_move_states_combox = new QComboBox;
  axis_move_states_combox->addItem(tr("STOP"));
  axis_move_states_combox->addItem(tr("MOVING"));
  StyleUtils::ApplyStyle(axis_move_states_combox, "input_box");
  return axis_move_states_combox;
}
QComboBox* LimitSettingView::GetAxisRegionNameComboBox(std::string axis_ids) {
  QComboBox* axis_region_name_combox = new QComboBox;
  yotta::AxisConfigItemPtr axis_cofig_item =
      axis_config_->GetAxisByIds(axis_ids.c_str());
  if (!axis_cofig_item) {
    return axis_region_name_combox;
    LOG(ERROR) << "get axis_cofig_item failed axis_ids=  " << axis_ids;
  }

  for (int i = 0; i < axis_cofig_item->GetRegionCount(); i++) {
    yotta::AxisRegionConfigPtr axis_region = axis_cofig_item->GetRegion(i);
    if (!axis_region) {
      return axis_region_name_combox;
      LOG(ERROR) << "get axis_region failed axis_ids=  " << axis_ids
                 << " axis_region index = " << i;
    }
    std::string region_ids;
    char region_ids_char[256];
    axis_region->GetIds(region_ids_char, 256);
    region_ids = region_ids_char;
    axis_region_name_combox->addItem(QString::fromStdString(region_ids));
  }
  StyleUtils::ApplyStyle(axis_region_name_combox, "input_box");
  return axis_region_name_combox;
}
QComboBox* LimitSettingView::GetAxisRegionStatesComboBox() {
  QComboBox* axis_region_states_combox = new QComboBox;
  axis_region_states_combox->addItem(tr("OUT-REGION"));
  axis_region_states_combox->addItem(tr("IN-REGION"));
  StyleUtils::ApplyStyle(axis_region_states_combox, "input_box");
  return axis_region_states_combox;
}
QComboBox* LimitSettingView::GetIONameComboBox() {
  QComboBox* io_name_combox = new QComboBox;

  // 空指针检查
  if (!io_config_ || !io_config_->GetInputIo()) {
    LOG(ERROR) << "LimitSettingView::GetIONameComboBox - io_config_ or GetInputIo() is null";
    StyleUtils::ApplyStyle(io_name_combox, "input_box");
    return io_name_combox;
  }

  for (int i = 0; i < io_config_->GetInputIo()->GetIoPortConfigCount(); i++) {
    yotta::IoPortConfigPtr io_in_port =
        io_config_->GetInputIo()->GetIoPortConfig(i);
    if (!io_in_port) {
      continue;  // 跳过空指针
    }

    std::string io_ids;
    char io_ids_char[256];
    io_in_port->GetIds(io_ids_char, sizeof(io_ids_char));
    io_ids = io_ids_char;
    io_name_combox->addItem(QString::fromStdString(io_ids));
  }
  StyleUtils::ApplyStyle(io_name_combox, "input_box");
  return io_name_combox;
}
QComboBox* LimitSettingView::GetIOStatesComboBox() {
  QComboBox* axis_io_states_combox = new QComboBox;
  axis_io_states_combox->addItem(tr("OFF"));
  axis_io_states_combox->addItem(tr("ON"));
  StyleUtils::ApplyStyle(axis_io_states_combox, "input_box");
  return axis_io_states_combox;
}

void LimitSettingView::OnLimitListSelect(int row, int column) {
  ui->tableWidget_axis_states_->setRowCount(0);
  ui->tableWidget_axis_region_states_->setRowCount(0);
  ui->tableWidget_io_states_->setRowCount(0);

  limit_index_ = row;
  LimitConditionConfigPtr limit_condition_config;
  if (limit_type_) {
    IOLimitRuleConfigPtr io_limit_rule_config =
        limit_rule_mgr_config_->GetIOLimitRule(io_ids_.c_str());
    if (!io_limit_rule_config) {
      LOG(ERROR) << "LimitSettingView::OnLimitListSelect - GetIOLimitRule failed for " << io_ids_;
      return;
    }
    limit_condition_config =
        io_limit_rule_config->GetLimitCondition(limit_index_);
  } else {
    AxisLimitRuleConfigPtr axis_limit_rule_config =
        limit_rule_mgr_config_->GetAxisLimitRule(axis_ids_.c_str());
    if (!axis_limit_rule_config) {
      LOG(ERROR) << "LimitSettingView::OnLimitListSelect - GetAxisLimitRule failed for " << axis_ids_;
      return;
    }
    limit_condition_config =
        axis_limit_rule_config->GetLimitCondition(limit_index_);
  }
  if (limit_condition_config) {
    // 轴状态
    for (int j = 0; j < limit_condition_config->GetAxisStateConditionCount();
         j++) {
      int RowCont = ui->tableWidget_axis_states_->rowCount();
      ui->tableWidget_axis_states_->insertRow(RowCont);

      AxisStateConditionConfigPtr axis_state_condition_config =
          limit_condition_config->GetAxisStateCondition(j);

      std::string axis_ids;
      char axis_ids_buffer[256];
      axis_state_condition_config->GetAxisIds(axis_ids_buffer,
                                              sizeof(axis_ids_buffer));
      axis_ids = axis_ids_buffer;
      yotta::Axis::AxisOperationState operation_state =
          axis_state_condition_config->GetAxisOperationState();

      QComboBox* axis_name_combox = GetAxisNameComboBox();
      axis_name_combox->setCurrentText(QString::fromStdString(axis_ids));
      ui->tableWidget_axis_states_->setCellWidget(RowCont, 0, axis_name_combox);

      QComboBox* axis_states_combox = GetAxisStatesComboBox();
      axis_states_combox->setCurrentIndex(
          operation_state != yotta::Axis::AxisOperationState::kIdle);
      ui->tableWidget_axis_states_->setCellWidget(RowCont, 1,
                                                  axis_states_combox);
    }
    // 轴区域
    for (int j = 0; j < limit_condition_config->GetAxisRegionConditionCount();
         j++) {
      int RowCont = ui->tableWidget_axis_region_states_->rowCount();
      ui->tableWidget_axis_region_states_->insertRow(RowCont);

      AxisRegionConditionConfigPtr axis_region_condition_config =
          limit_condition_config->GetAxisRegionCondition(j);

      std::string axis_ids;
      char axis_ids_buffer[256];
      axis_region_condition_config->GetAxisIds(axis_ids_buffer,
                                               sizeof(axis_ids_buffer));
      axis_ids = axis_ids_buffer;

      std::string axis_region_ids;
      char axis_region_ids_buffer[256];
      axis_region_condition_config->GetRegionIds(
          axis_region_ids_buffer, sizeof(axis_region_ids_buffer));
      axis_region_ids = axis_region_ids_buffer;
      int in_range = axis_region_condition_config->GetInRange();

      QComboBox* axis_name_combox = GetAxisNameComboBox();
      axis_name_combox->setCurrentText(QString::fromStdString(axis_ids));
      ui->tableWidget_axis_region_states_->setCellWidget(RowCont, 0,
                                                         axis_name_combox);
      connect(axis_name_combox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
              &LimitSettingView::ReadLimitCondition);

      QComboBox* axis_region_name_combox = GetAxisRegionNameComboBox(axis_ids);
      axis_region_name_combox->setCurrentText(
          QString::fromStdString(axis_region_ids));
      ui->tableWidget_axis_region_states_->setCellWidget(
          RowCont, 1, axis_region_name_combox);

      QComboBox* axis_region_states_combox = GetAxisRegionStatesComboBox();
      axis_region_states_combox->setCurrentIndex(in_range);
      ui->tableWidget_axis_region_states_->setCellWidget(
          RowCont, 2, axis_region_states_combox);
    }
    // IO状态
    for (int j = 0; j < limit_condition_config->GetIOConditionCount(); j++) {
      int RowCont = ui->tableWidget_io_states_->rowCount();
      ui->tableWidget_io_states_->insertRow(RowCont);

      IOConditionConfigPtr io_condition_config =
          limit_condition_config->GetIOCondition(j);

      std::string io_ids;
      char io_ids_buffer[256];
      io_condition_config->GetIoIds(io_ids_buffer, sizeof(io_ids_buffer));
      io_ids = io_ids_buffer;
      int require_on = io_condition_config->GetRequireOn();

      QComboBox* io_name_combox = GetIONameComboBox();
      io_name_combox->setCurrentText(QString::fromStdString(io_ids));
      ui->tableWidget_io_states_->setCellWidget(RowCont, 0, io_name_combox);

      QComboBox* io_states_combox = GetIOStatesComboBox();
      io_states_combox->setCurrentIndex(require_on);
      ui->tableWidget_io_states_->setCellWidget(RowCont, 1, io_states_combox);
    }
  }
}
void LimitSettingView::ShowLimitCondition() {
  ui->tableWidget_axis_states_->setRowCount(0);
  ui->tableWidget_axis_region_states_->setRowCount(0);
  ui->tableWidget_io_states_->setRowCount(0);
  ui->limit_list->setRowCount(0);

  // IO限位
  if (limit_type_) {
    IOLimitRuleConfigPtr io_limit_rule_config =
        limit_rule_mgr_config_->GetIOLimitRule(io_ids_.c_str());
    if (io_limit_rule_config) {
      for (int i = 0; i < io_limit_rule_config->GetLimitConditionCount(); i++) {
        int RowCont = ui->limit_list->rowCount();
        ui->limit_list->insertRow(RowCont);

        LimitConditionConfigPtr limit_condition_config =
            io_limit_rule_config->GetLimitCondition(i);
        std::string condition_ids;
        char condition_ids_buffer[256];
        limit_condition_config->GetRuleIds(condition_ids_buffer,
                                           sizeof(condition_ids_buffer));
        condition_ids = condition_ids_buffer;
        yotta::Axis::LimitType type = limit_condition_config->GetType();

        ui->limit_list->setItem(
            RowCont, 0,
            new QTableWidgetItem(QString::fromStdString(condition_ids)));
        QComboBox* mode_combox = GetIoLimitTypeComboBox();
        switch (type) {
          case yotta::Axis::LimitType::kLockIoState:
            mode_combox->setCurrentIndex(0);
            break;
          case yotta::Axis::LimitType::kLockIoPositive:
            mode_combox->setCurrentIndex(1);
            break;
          case yotta::Axis::LimitType::kLockIoNegative:
            mode_combox->setCurrentIndex(2);
            break;
          default:
            break;
        }
        ui->limit_list->setCellWidget(RowCont, 1, mode_combox);
      }
    }
  }
  // 轴限位
  else {
    AxisLimitRuleConfigPtr axis_limit_rule_config =
        limit_rule_mgr_config_->GetAxisLimitRule(axis_ids_.c_str());
    if (axis_limit_rule_config) {
      for (int i = 0; i < axis_limit_rule_config->GetLimitConditionCount();
           i++) {
        int RowCont = ui->limit_list->rowCount();
        ui->limit_list->insertRow(RowCont);

        LimitConditionConfigPtr limit_condition_config =
            axis_limit_rule_config->GetLimitCondition(i);
        std::string condition_ids;
        char condition_ids_buffer[256];
        limit_condition_config->GetRuleIds(condition_ids_buffer,
                                           sizeof(condition_ids_buffer));
        condition_ids = condition_ids_buffer;
        yotta::Axis::LimitType type = limit_condition_config->GetType();
        double min_pos = limit_condition_config->GetMinPos();
        double max_pos = limit_condition_config->GetMaxPos();

        ui->limit_list->setItem(
            RowCont, 0,
            new QTableWidgetItem(QString::fromStdString(condition_ids)));
        QComboBox* mode_combox = GetAxisLimitTypeComboBox();
        switch (type) {
          case yotta::Axis::LimitType::kLockMotion:
            mode_combox->setCurrentIndex(0);
            break;
          case yotta::Axis::LimitType::kRelativeLimit:
            mode_combox->setCurrentIndex(1);
            break;
          case yotta::Axis::LimitType::kAbsoluteLimit:
            mode_combox->setCurrentIndex(2);
            break;
          case yotta::Axis::LimitType::kLockHoming:
            mode_combox->setCurrentIndex(3);
            break;
          default:
            break;
        }
        ui->limit_list->setCellWidget(RowCont, 1, mode_combox);
        ui->limit_list->setItem(RowCont, 2,
                                new QTableWidgetItem(QString::number(min_pos)));
        ui->limit_list->setItem(RowCont, 3,
                                new QTableWidgetItem(QString::number(max_pos)));
      }
    }
  }

  if (ui->limit_list->rowCount()) {
    if (limit_index_ < ui->limit_list->rowCount()) {
    } else {
      limit_index_ = ui->limit_list->rowCount() - 1;
    }
    OnLimitListSelect(limit_index_, 0);
  } else {
    limit_index_ = 0;
  }
}
void LimitSettingView::ReadLimitCondition() {
  LimitConditionConfigPtr limit_condition_config;
  if (limit_type_) {
    IOLimitRuleConfigPtr io_limit_rule_config =
        limit_rule_mgr_config_->GetIOLimitRule(io_ids_.c_str());
    if (!io_limit_rule_config) {
      LOG(ERROR) << "LimitSettingView::ReadLimitCondition - GetIOLimitRule failed for " << io_ids_;
      return;
    }
    limit_condition_config =
        io_limit_rule_config->GetLimitCondition(limit_index_);
    if (limit_condition_config) {
      QTableWidgetItem* item = ui->limit_list->item(limit_index_, 0);
      if (item) {
        std::string rule_ids = item->text().toStdString();
        limit_condition_config->SetRuleIds(rule_ids.c_str());
      }

      QComboBox* io_limit_combox =
          qobject_cast<QComboBox*>(ui->limit_list->cellWidget(limit_index_, 1));
      if (io_limit_combox) {
        switch (io_limit_combox->currentIndex()) {
          case 0:
            limit_condition_config->SetType(yotta::Axis::LimitType::kLockIoState);
            break;
          case 1:
            limit_condition_config->SetType(
                yotta::Axis::LimitType::kLockIoPositive);
            break;
          case 2:
            limit_condition_config->SetType(
                yotta::Axis::LimitType::kLockIoNegative);
            break;
          default:
            break;
        }
      }
    }

  } else {
    AxisLimitRuleConfigPtr axis_limit_rule_config =
        limit_rule_mgr_config_->GetAxisLimitRule(axis_ids_.c_str());
    if (!axis_limit_rule_config) {
      LOG(ERROR) << "LimitSettingView::ReadLimitCondition - GetAxisLimitRule failed for " << axis_ids_;
      return;
    }
    limit_condition_config =
        axis_limit_rule_config->GetLimitCondition(limit_index_);
    if (limit_condition_config) {
      QTableWidgetItem* item = ui->limit_list->item(limit_index_, 0);
      if (item) {
        std::string rule_ids = item->text().toStdString();
        limit_condition_config->SetRuleIds(rule_ids.c_str());
      }
      QComboBox* axis_limit_combox =
          qobject_cast<QComboBox*>(ui->limit_list->cellWidget(limit_index_, 1));
      if (axis_limit_combox) {
        switch (axis_limit_combox->currentIndex()) {
          case 0:
            limit_condition_config->SetType(yotta::Axis::LimitType::kLockMotion);
            break;
          case 1:
            limit_condition_config->SetType(
                yotta::Axis::LimitType::kRelativeLimit);
            break;
          case 2:
            limit_condition_config->SetType(
                yotta::Axis::LimitType::kAbsoluteLimit);
            break;
          case 3:
            limit_condition_config->SetType(yotta::Axis::LimitType::kLockHoming);
            break;
          default:
            break;
        }
      }
      QTableWidgetItem* min_item = ui->limit_list->item(limit_index_, 2);
      QTableWidgetItem* max_item = ui->limit_list->item(limit_index_, 3);
      if (min_item) {
        limit_condition_config->SetMinPos(min_item->text().toDouble());
      }
      if (max_item) {
        limit_condition_config->SetMaxPos(max_item->text().toDouble());
      }
    }
  }

  if (limit_condition_config) {
    limit_condition_config->DeleteAllCondition();

    int row_count_axis_states = ui->tableWidget_axis_states_->rowCount();
    for (int i = 0; i < row_count_axis_states; i++) {
      QComboBox* axis_name_combox = qobject_cast<QComboBox*>(
          ui->tableWidget_axis_states_->cellWidget(i, 0));
      QComboBox* axis_states_combox = qobject_cast<QComboBox*>(
          ui->tableWidget_axis_states_->cellWidget(i, 1));

      if (!axis_name_combox || !axis_states_combox) {
        LOG(WARNING) << "LimitSettingView::ReadLimitCondition - axis combobox is null at row " << i;
        continue;
      }

      std::string axis_ids = axis_name_combox->currentText().toStdString();
      int move_states = axis_states_combox->currentIndex();

      AxisStateConditionConfigPtr axis_state_condition =
          limit_condition_config->AddAxisStateCondition(axis_ids.c_str());
      switch (move_states) {
        case 0:
          axis_state_condition->SetAxisOperationState(
              yotta::Axis::AxisOperationState::kIdle);
          break;
        case 1:
          axis_state_condition->SetAxisOperationState(
              yotta::Axis::AxisOperationState::kPos);
          break;
        default:
          break;
      }
    }

    int row_count_axis_region = ui->tableWidget_axis_region_states_->rowCount();
    for (int i = 0; i < row_count_axis_region; i++) {
      QComboBox* axis_name_combox = qobject_cast<QComboBox*>(
          ui->tableWidget_axis_region_states_->cellWidget(i, 0));
      QComboBox* axis_region_name_combox = qobject_cast<QComboBox*>(
          ui->tableWidget_axis_region_states_->cellWidget(i, 1));
      QComboBox* axis_region_states_combox = qobject_cast<QComboBox*>(
          ui->tableWidget_axis_region_states_->cellWidget(i, 2));

      if (!axis_name_combox || !axis_region_name_combox || !axis_region_states_combox) {
        LOG(WARNING) << "LimitSettingView::ReadLimitCondition - axis region combobox is null at row " << i;
        continue;
      }

      std::string axis_ids = axis_name_combox->currentText().toStdString();
      std::string axis_region_id =
          axis_region_name_combox->currentText().toStdString();
      int region_states = axis_region_states_combox->currentIndex();

      AxisRegionConditionConfigPtr axis_region_condition =
          limit_condition_config->AddAxisRegionCondition(axis_ids.c_str());
      axis_region_condition->SetRegionIds(axis_region_id.c_str());
      axis_region_condition->SetInRange(region_states);
    }

    int row_count_io_states = ui->tableWidget_io_states_->rowCount();
    for (int i = 0; i < row_count_io_states; i++) {
      QComboBox* io_name_combox = qobject_cast<QComboBox*>(
          ui->tableWidget_io_states_->cellWidget(i, 0));
      QComboBox* io_states_combox = qobject_cast<QComboBox*>(
          ui->tableWidget_io_states_->cellWidget(i, 1));

      if (!io_name_combox || !io_states_combox) {
        LOG(WARNING) << "LimitSettingView::ReadLimitCondition - io combobox is null at row " << i;
        continue;
      }

      std::string io_ids = io_name_combox->currentText().toStdString();
      int io_states = io_states_combox->currentIndex();

      IOConditionConfigPtr axis_region_condition =
          limit_condition_config->AddIOCondition(io_ids.c_str());

      axis_region_condition->SetRequireOn(io_states);
    }
  }

  ShowLimitCondition();
}

void LimitSettingView::OnAddLimitButtonClicked() {
  std::string new_limit = io_ids_ + "new_limit_";
  LimitConditionConfigPtr limit_condition_config;
  if (limit_type_) {
    IOLimitRuleConfigPtr io_limit_rule_config =
        limit_rule_mgr_config_->AddIOLimitRule(io_ids_.c_str());
    if (io_limit_rule_config) {
      new_limit = io_ids_ + "new_limit_1";
      limit_condition_config =
          io_limit_rule_config->AddLimitCondition(new_limit.c_str());
    }
  } else {
    AxisLimitRuleConfigPtr axis_limit_rule_config =
        limit_rule_mgr_config_->AddAxisLimitRule(axis_ids_.c_str());
    if (axis_limit_rule_config) {
      new_limit = axis_ids_ + "new_limit_";
      limit_condition_config =
          axis_limit_rule_config->AddLimitCondition(new_limit.c_str());
    }
  }
  limit_index_ = ui->limit_list->rowCount();
  ShowLimitCondition();
}
void LimitSettingView::OnDelLimitButtonClicked() {
  if (limit_type_) {
    LimitConditionConfigPtr limit_condition_config;
    IOLimitRuleConfigPtr io_limit_rule_config =
        limit_rule_mgr_config_->GetIOLimitRule(io_ids_.c_str());
    if (io_limit_rule_config) {
      limit_condition_config =
          io_limit_rule_config->GetLimitCondition(limit_index_);
      if (limit_condition_config) {
        char rule_ids_buffer[256];
        limit_condition_config->GetRuleIds(rule_ids_buffer,
                                           sizeof(rule_ids_buffer));
        io_limit_rule_config->DeleteLimitCondition(rule_ids_buffer);
      }
    }
  } else {
    LimitConditionConfigPtr limit_condition_config;
    AxisLimitRuleConfigPtr axis_limit_rule_config =
        limit_rule_mgr_config_->GetAxisLimitRule(axis_ids_.c_str());
    if (axis_limit_rule_config) {
      limit_condition_config =
          axis_limit_rule_config->GetLimitCondition(limit_index_);
      if (limit_condition_config) {
        char rule_ids_buffer[256];
        limit_condition_config->GetRuleIds(rule_ids_buffer,
                                           sizeof(rule_ids_buffer));
        axis_limit_rule_config->DeleteLimitCondition(rule_ids_buffer);
      }
    }
  }
  ShowLimitCondition();
}
void LimitSettingView::OnSaveLimitButtonClicked() {
  ReadLimitCondition();
  limit_rule_mgr_config_->Save();
}
void LimitSettingView::OnAddLimitAxisStatesButtonClicked() {
  int RowCont = ui->tableWidget_axis_states_->rowCount();
  ui->tableWidget_axis_states_->insertRow(RowCont);

  QComboBox* axis_name_combox = GetAxisNameComboBox();
  axis_name_combox->setCurrentIndex(0);
  ui->tableWidget_axis_states_->setCellWidget(RowCont, 0, axis_name_combox);

  QComboBox* axis_states_combox = GetAxisStatesComboBox();
  axis_states_combox->setCurrentIndex(0);
  ui->tableWidget_axis_states_->setCellWidget(RowCont, 1, axis_states_combox);
}
void LimitSettingView::OnDelLimitAxisStatesButtonClicked() {
  int RowCont = ui->tableWidget_axis_states_->rowCount();
  if (RowCont) {
    ui->tableWidget_axis_states_->removeRow(RowCont - 1);
  }
}
void LimitSettingView::OnAddLimitAxisRegionStatesButtonClicked() {
  int RowCont = ui->tableWidget_axis_region_states_->rowCount();
  ui->tableWidget_axis_region_states_->insertRow(RowCont);

  QComboBox* axis_name_combox = GetAxisNameComboBox();
  axis_name_combox->setCurrentIndex(0);
  ui->tableWidget_axis_region_states_->setCellWidget(RowCont, 0,
                                                     axis_name_combox);
  connect(axis_name_combox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &LimitSettingView::ReadLimitCondition);

  std::string axis_ids;  // 新添加区域默认选第一个轴
  for (int i = 0; i < axis_config_->GetAxisCount(); i++) {
    yotta::AxisConfigItemPtr axis_cofig_item = axis_config_->GetAxis(i);
    if (!axis_cofig_item) {
      continue;
    }
    char axis_ids_char[256];
    axis_cofig_item->GetIds(axis_ids_char, 256);
    axis_ids = axis_ids_char;
    break;
  }
  QComboBox* axis_region_name_combox = GetAxisRegionNameComboBox(axis_ids);
  axis_region_name_combox->setCurrentIndex(0);
  ui->tableWidget_axis_region_states_->setCellWidget(RowCont, 1,
                                                     axis_region_name_combox);

  QComboBox* axis_region_states_combox = GetAxisRegionStatesComboBox();
  axis_region_states_combox->setCurrentIndex(0);
  ui->tableWidget_axis_region_states_->setCellWidget(RowCont, 2,
                                                     axis_region_states_combox);
}
void LimitSettingView::OnDelLimitAxisRegionStatesButtonClicked() {
  int RowCont = ui->tableWidget_axis_region_states_->rowCount();
  if (RowCont) {
    ui->tableWidget_axis_region_states_->removeRow(RowCont - 1);
  }
}
void LimitSettingView::OnAddLimitIOStatesButtonClicked() {
  int RowCont = ui->tableWidget_io_states_->rowCount();
  ui->tableWidget_io_states_->insertRow(RowCont);
  QComboBox* io_name_combox = GetIONameComboBox();
  io_name_combox->setCurrentIndex(0);
  ui->tableWidget_io_states_->setCellWidget(RowCont, 0, io_name_combox);
  QComboBox* io_states_combox = GetIOStatesComboBox();
  io_states_combox->setCurrentIndex(0);
  ui->tableWidget_io_states_->setCellWidget(RowCont, 1, io_states_combox);
}
void LimitSettingView::OnDelLimitIOStatesButtonClicked() {
  int RowCont = ui->tableWidget_io_states_->rowCount();
  if (RowCont) {
    ui->tableWidget_io_states_->removeRow(RowCont - 1);
  }
}
