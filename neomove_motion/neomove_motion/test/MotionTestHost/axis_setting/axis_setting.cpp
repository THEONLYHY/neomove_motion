#include "axis_setting.h"

#include <QComboBox>
#include <QLineEdit>
#include <QListView>
#include <QPointer>
#include <QPushButton>

#include "view/tools/utils/style_utils.h"

AxisSetting::AxisSetting(QWidget *parent)
    : QWidget(parent), ui_(new Ui::axis_settingClass()) {
  ui_->setupUi(this);
  axis_config_ = yotta::ConfigFactory::GetInstance()->GetAxisConfig();
  if (!axis_config_) {
    LOG(ERROR) << "AxisSetting axis_config_ is null";
  }

  UnitInfoMgrPtr motion_module =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!motion_module) {
    return;
  }
  //左侧工位列表
  bool first_check = true;  //选中第一个工位
  for (int i = 0; i < motion_module->GetUnitCount(); i++) {
    QPushButton *axis_btn = new QPushButton(
        QString::fromStdString(motion_module->GetUnitInfo(i)->unit_ids));
    axis_btn->setFixedSize(160, 69);
    axis_btn->setCheckable(true);
    StyleUtils::ApplyStyle(axis_btn, "btn_icon_deepblue_160_69");
    if (first_check) {
      axis_btn->setChecked(first_check);
      module_id_ = motion_module->GetUnitInfo(i)->unit_ids;
      first_check = false;
    }
    ui_->module_list_lay->addWidget(axis_btn);
    btn_group_module_.addButton(axis_btn, i);
  }
  ui_->module_list_lay->addStretch();
  for (QAbstractButton *button : btn_group_module_.buttons()) {
    connect(button, &QAbstractButton::clicked, this,
            [this, button](bool checked) {
              if (!checked) {
                return;
              }
              int id = btn_group_module_.id(button);
              OnModuleClicked(id);
            });
  }
  //参数,速度,区域,限位切换
  btn_group_region_limit_.addButton(ui_->button_axis_parameter_, 0);
  btn_group_region_limit_.addButton(ui_->button_axis_speed_, 1);
  btn_group_region_limit_.addButton(ui_->button_axis_region_, 2);
  btn_group_region_limit_.addButton(ui_->button_axis_limit_, 3);
  ui_->button_axis_parameter_->setChecked(true);
  for (QAbstractButton *button : btn_group_region_limit_.buttons()) {
    connect(button, &QAbstractButton::clicked, this, [this, button]() {
      int id = btn_group_region_limit_.id(button);
      OnParaVelRegionLimitClicked(id);
    });
  }
  ui_->stackedWidget->setCurrentWidget(ui_->para_page_);

  {
    QStringList horizontalHeaders;
    horizontalHeaders << tr("Speed Name") << tr("Velocity") << tr("Acc")
                      << tr("Dec");
    ui_->tableWidget_speed_list_->setHorizontalHeaderLabels(horizontalHeaders);
    ui_->tableWidget_speed_list_->setColumnWidth(0, 150);

    ui_->tableWidget_speed_list_->setShowGrid(true);  //显示表格线
    ui_->tableWidget_speed_list_->setVerticalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);  //隐藏滑动条
    ui_->tableWidget_speed_list_->setSelectionBehavior(
        QAbstractItemView::SelectRows);  //设置行选择模式
    ui_->tableWidget_speed_list_->setSelectionMode(
        QAbstractItemView::SingleSelection);
    ui_->tableWidget_speed_list_->setContextMenuPolicy(Qt::CustomContextMenu);
    ui_->tableWidget_speed_list_->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);  //先自适应宽度
    ui_->tableWidget_speed_list_->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Fixed);  //固定0列宽度
    ui_->tableWidget_speed_list_->setColumnWidth(0, 150);
    ui_->tableWidget_speed_list_->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { color: black; font-weight: normal; }");
    QFont speed_font = ui_->tableWidget_speed_list_->font();
    speed_font.setWeight(QFont::Normal);
    ui_->tableWidget_speed_list_->setFont(speed_font);

    connect(ui_->tableWidget_speed_list_, SIGNAL(cellClicked(int, int)), this,
            SLOT(OnSpeedListSelect(int, int)));
  }
  {
    QStringList horizontalHeaders;
    horizontalHeaders << tr("Region Name") << tr("Negative") << tr("Positive");
    ui_->tableWidget_region_list_->setHorizontalHeaderLabels(horizontalHeaders);
    ui_->tableWidget_region_list_->setColumnWidth(0, 150);

    ui_->tableWidget_region_list_->setShowGrid(true);  //显示表格线
    ui_->tableWidget_region_list_->setVerticalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);  //隐藏滑动条
    ui_->tableWidget_region_list_->setSelectionBehavior(
        QAbstractItemView::SelectRows);  //设置行选择模式
    ui_->tableWidget_region_list_->setSelectionMode(
        QAbstractItemView::SingleSelection);
    ui_->tableWidget_region_list_->setContextMenuPolicy(Qt::CustomContextMenu);
    ui_->tableWidget_region_list_->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);  //先自适应宽度
    ui_->tableWidget_region_list_->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Fixed);  //固定0列宽度
    ui_->tableWidget_region_list_->setColumnWidth(0, 150);
    ui_->tableWidget_region_list_->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { color: black; font-weight: normal; }");
    QFont region_font = ui_->tableWidget_region_list_->font();
    region_font.setWeight(QFont::Normal);
    ui_->tableWidget_region_list_->setFont(region_font);

    connect(ui_->tableWidget_region_list_, SIGNAL(cellClicked(int, int)), this,
            SLOT(OnRegionListSelect(int, int)));
  }
  {
    limit_setting_ = new LimitSettingView;
    ui_->limit_set_layout->addWidget(limit_setting_);
  }

  QObject::connect(ui_->button_save_para_, SIGNAL(clicked()), this,
                   SLOT(OnParaSaveClicked()));

  QObject::connect(ui_->button_add_spped_, SIGNAL(clicked()), this,
                   SLOT(OnAddSpeedButtonClicked()));
  QObject::connect(ui_->button_del_spped_, SIGNAL(clicked()), this,
                   SLOT(OnDelSpeedButtonClicked()));
  QObject::connect(ui_->button_save_spped_, SIGNAL(clicked()), this,
                   SLOT(OnSaveSpeedButtonClicked()));
  QObject::connect(ui_->button_add_region_, SIGNAL(clicked()), this,
                   SLOT(OnAddRegionButtonClicked()));
  QObject::connect(ui_->button_del_region_, SIGNAL(clicked()), this,
                   SLOT(OnDelRegionButtonClicked()));
  QObject::connect(ui_->button_save_region_, SIGNAL(clicked()), this,
                   SLOT(OnSaveRegionButtonClicked()));

  InitUi();  // 应用UI样式

  ShowModule();
}
AxisSetting::~AxisSetting() { delete ui_; }

void AxisSetting::ReTranslate() { ui_->retranslateUi(this); }

void AxisSetting::InitUi() {
  // 参数页面输入框样式
  StyleUtils::ApplyStyle(ui_->limit_positive, "input_box");
  StyleUtils::ApplyStyle(ui_->limit_negative, "input_box");
  StyleUtils::ApplyStyle(ui_->limit_max_velocity, "input_box");
  StyleUtils::ApplyStyle(ui_->limit_min_velocity, "input_box");
  StyleUtils::ApplyStyle(ui_->limit_max_acc, "input_box");
  StyleUtils::ApplyStyle(ui_->limit_min_acc, "input_box");
  StyleUtils::ApplyStyle(ui_->limit_max_dec, "input_box");
  StyleUtils::ApplyStyle(ui_->limit_min_dec, "input_box");
  StyleUtils::ApplyStyle(ui_->limit_torque_positive, "input_box");
  StyleUtils::ApplyStyle(ui_->limit_torque_negative, "input_box");
  StyleUtils::ApplyStyle(ui_->base_offset, "input_box");
  StyleUtils::ApplyStyle(ui_->axis_multi, "input_box");
  StyleUtils::ApplyStyle(ui_->limit_offset, "input_box");
  StyleUtils::ApplyStyle(ui_->nEnabled, "input_box");
  StyleUtils::ApplyStyle(ui_->nElectricSize, "input_box");
  StyleUtils::ApplyStyle(ui_->nPropertyMode, "input_box");
  StyleUtils::ApplyStyle(ui_->nElectricMulti, "input_box");
  StyleUtils::ApplyStyle(ui_->nElectricMode, "input_box");
  StyleUtils::ApplyStyle(ui_->nElectricEcID, "input_box");
  StyleUtils::ApplyStyle(ui_->nElectricIndex, "input_box");
  StyleUtils::ApplyStyle(ui_->nElectricSubIndex, "input_box");

  // 参数页面标签样式
  StyleUtils::ApplyStyle(ui_->label_100, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_101, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_102, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_103, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_104, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_105, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_106, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_107, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_108, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_109, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_110, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_111, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_112, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_113, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_114, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_115, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_116, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_117, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_118, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_119, "label_lineEdit_text_prefix");
  StyleUtils::ApplyStyle(ui_->label_120, "label_lineEdit_text_prefix");

  // 按钮样式 - 深蓝 140×69
  StyleUtils::ApplyStyle(ui_->button_axis_parameter_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui_->button_axis_speed_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui_->button_axis_region_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui_->button_axis_limit_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui_->button_add_region_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui_->button_del_region_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui_->button_save_region_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui_->button_add_spped_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui_->button_del_spped_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui_->button_save_spped_, "btn_icon_deepblue_140_69");
  StyleUtils::ApplyStyle(ui_->button_save_para_, "btn_icon_deepblue_140_69");
}

void AxisSetting::ClearLayout(QLayout *layout) {
  if (!layout) return;
  QLayoutItem *item;
  while ((item = layout->takeAt(0)) != nullptr) {
    if (item->widget()) {
      item->widget()->deleteLater();  // 移除部件
    } else if (item->layout()) {
      ClearLayout(item->layout());    // 递归移除子布局
      item->layout()->deleteLater();  // 删除子布局
    } else if (item->spacerItem()) {
      delete item->spacerItem();  // 处理间隔项
    }
  }
}
void AxisSetting::OnModuleClicked(int module_index) {
  UnitInfoMgrPtr motion_module =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!motion_module) {
    return;
  }
  if (module_index < motion_module->GetUnitCount()) {
    module_id_ = motion_module->GetUnitInfo(module_index)->unit_ids;
    ShowModule();
  }
}
void AxisSetting::OnAxisClicked(int id) {
  if (id < axis_id_list_.size()) {
    axis_ids_ = axis_id_list_[id];
    ShowModuleAxis();
  }
}
void AxisSetting::OnParaVelRegionLimitClicked(int id) {
  switch (id) {
    case 0:
      ui_->stackedWidget->setCurrentWidget(ui_->para_page_);
      break;
    case 1:
      ui_->stackedWidget->setCurrentWidget(ui_->speed_page_);
      break;
    case 2:
      ui_->stackedWidget->setCurrentWidget(ui_->region_page_);
      break;
    case 3:
      ui_->stackedWidget->setCurrentWidget(ui_->limit_page_);
      break;
    default:
      break;
  }
}

void AxisSetting::OnSpeedListSelect(int row, int column) {
  speed_index_ = row;
}
void AxisSetting::OnAddSpeedButtonClicked() {
  yotta::AxisConfigItemPtr axis_config = GetCurrentAxisConfig();
  if (!axis_config) {
    return;
  }

  int RowCont = ui_->tableWidget_speed_list_->rowCount();
  ui_->tableWidget_speed_list_->insertRow(RowCont);

  std::string speed_ids = "new_speed";
  double velocity = 1;
  double acc = 0.1;
  double dec = 0.1;

  yotta::AxisSpeedConfigPtr axis_region =
      axis_config->AddSpeed(speed_ids.c_str());
  axis_region->SetVelocity(velocity);
  axis_region->SetAcc(acc);
  axis_region->SetDec(dec);

  ui_->tableWidget_speed_list_->setItem(
      RowCont, 0, new QTableWidgetItem(QString::fromStdString(speed_ids)));
  ui_->tableWidget_speed_list_->setItem(
      RowCont, 1, new QTableWidgetItem(QString::number(velocity)));
  ui_->tableWidget_speed_list_->setItem(
      RowCont, 2, new QTableWidgetItem(QString::number(acc)));
  ui_->tableWidget_speed_list_->setItem(
      RowCont, 3, new QTableWidgetItem(QString::number(dec)));
  ui_->tableWidget_speed_list_->setRowHeight(RowCont, 69);
}
void AxisSetting::OnDelSpeedButtonClicked() {
  yotta::AxisConfigItemPtr axis_config = GetCurrentAxisConfig();
  if (!axis_config) {
    return;
  }

  int RowCont = ui_->tableWidget_speed_list_->rowCount();
  if (!RowCont) {
    return;
  }
  speed_index_ = speed_index_ < RowCont ? speed_index_ : RowCont - 1;
  speed_index_ = speed_index_ < 0 ? 0 : speed_index_;

  std::string speed_ids =
      ui_->tableWidget_speed_list_->item(speed_index_, 0)->text().toStdString();

  axis_config->DeleteSpeed(speed_ids.c_str());

  ui_->tableWidget_speed_list_->removeRow(speed_index_);
}
void AxisSetting::OnSaveSpeedButtonClicked() {
  int RowCont = ui_->tableWidget_speed_list_->rowCount();
  if (!RowCont) {
    return;
  }
  OnParaSaveClicked();
}

void AxisSetting::OnRegionListSelect(int row, int column) {
  region_index_ = row;
}
void AxisSetting::OnAddRegionButtonClicked() {
  yotta::AxisConfigItemPtr axis_config = GetCurrentAxisConfig();
  if (!axis_config) {
    return;
  }

  int RowCont = ui_->tableWidget_region_list_->rowCount();
  ui_->tableWidget_region_list_->insertRow(RowCont);

  std::string region_ids = "new_region";
  double negative = 0.1;
  double postive = 0.2;
  yotta::AxisRegionConfigPtr axis_region =
      axis_config->AddRegion(region_ids.c_str());
  axis_region->SetRegionNegative(negative);
  axis_region->SetRegionPositive(postive);

  ui_->tableWidget_region_list_->setItem(
      RowCont, 0, new QTableWidgetItem(QString::fromStdString(region_ids)));
  ui_->tableWidget_region_list_->setItem(
      RowCont, 1, new QTableWidgetItem(QString::number(negative)));
  ui_->tableWidget_region_list_->setItem(
      RowCont, 2, new QTableWidgetItem(QString::number(postive)));
  ui_->tableWidget_region_list_->setRowHeight(RowCont, 69);
}
void AxisSetting::OnDelRegionButtonClicked() {
  yotta::AxisConfigItemPtr axis_config = GetCurrentAxisConfig();
  if (!axis_config) {
    return;
  }

  int RowCont = ui_->tableWidget_region_list_->rowCount();
  if (!RowCont) {
    return;
  }
  region_index_ = region_index_ < RowCont ? region_index_ : RowCont - 1;
  region_index_ = region_index_ < 0 ? 0 : region_index_;

  // 空指针检查
  QTableWidgetItem* item = ui_->tableWidget_region_list_->item(region_index_, 0);
  if (!item) {
    LOG(ERROR) << "AxisSetting::OnDelRegionButtonClicked - item is null at row " << region_index_;
    return;
  }
  std::string region_ids = item->text().toStdString();

  axis_config->DeleteRegion(region_ids.c_str());

  ui_->tableWidget_region_list_->removeRow(region_index_);
}
void AxisSetting::OnSaveRegionButtonClicked() {
  int RowCont = ui_->tableWidget_region_list_->rowCount();
  if (!RowCont) {
    return;
  }
  OnParaSaveClicked();
}

yotta::AxisConfigItemPtr AxisSetting::GetCurrentAxisConfig() const {
  if (!axis_config_) {
    LOG(ERROR) << "axis_config_ is null";
    return nullptr;
  }

  yotta::AxisConfigItemPtr axis = axis_config_->GetAxisByIds(axis_ids_.c_str());
  if (!axis) {
    LOG(ERROR) << "get axis failed axis_ids=  " << axis_ids_;
  }
  return axis;
}

void AxisSetting::ShowModule() {
  ClearLayout(ui_->axis_list_lay_);
  QList<QAbstractButton *> buttons = btn_group_axis_.buttons();
  for (QAbstractButton *button : buttons) {
    btn_group_axis_.removeButton(button);  // 从按钮组中移除
    delete button;                         // 删除按钮对象
  }

  UnitInfoMgrPtr motion_module =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!motion_module) {
    return;
  }
  auto module_axis_info = motion_module->GetUnitInfo(module_id_);
  axis_id_list_.clear();
  bool first_check = true;  //选中第一个轴
  if (module_axis_info) {
    for (int i = 0; i < module_axis_info->axis_ids.size(); i++) {
      // 按钮
      axis_id_list_.push_back(module_axis_info->axis_ids[i]);
      QPushButton *axis_pbt = new QPushButton(
          QString::fromStdString(module_axis_info->axis_ids[i]));
      axis_pbt->setCheckable(true);
      axis_pbt->setFixedSize(160, 69);
      StyleUtils::ApplyStyle(axis_pbt, "btn_icon_deepblue_160_69");
      if (first_check) {
        axis_pbt->setChecked(true);
        axis_ids_ = module_axis_info->axis_ids[i];
        first_check = false;
      }
      btn_group_axis_.addButton(axis_pbt, i);
      ui_->axis_list_lay_->addWidget(axis_pbt);
    }
  }
  ui_->axis_list_lay_->addStretch();
  for (QAbstractButton *button : btn_group_axis_.buttons()) {
    connect(button, &QAbstractButton::clicked, this, [this, button]() {
      int id = btn_group_axis_.id(button);
      OnAxisClicked(id);
    });
  }

  ShowModuleAxis();
}
void AxisSetting::ShowModuleAxis() {
  yotta::AxisConfigItemPtr axis = GetCurrentAxisConfig();
  if (!axis) {
    return;
  }
  yotta::AxisAttributeConfigPtr axis_attribute = axis->GetAttribute();
  if (!axis_attribute) {
    LOG(ERROR) << "get axis attribute failed axis_ids=  " << axis_ids_;
    return;
  }

  //参数
  {
    double limit_positive = 0;
    axis_attribute->GetLimitPositive(&limit_positive);
    double limit_negative = 0;
    axis_attribute->GetLimitNegative(&limit_negative);
    double limit_max_velocity = 0;
    axis_attribute->GetLimitMaxVelocity(&limit_max_velocity);
    double limit_min_velocity = 0;
    axis_attribute->GetLimitMinVelocity(&limit_min_velocity);
    double limit_max_acc = 0;
    axis_attribute->GetLimitMaxAcc(&limit_max_acc);
    double limit_min_acc = 0;
    axis_attribute->GetLimitMinAcc(&limit_min_acc);
    double limit_max_dec = 0;
    axis_attribute->GetLimitMaxDec(&limit_max_dec);
    double limit_min_dec = 0;
    axis_attribute->GetLimitMinDec(&limit_min_dec);
    double limit_offset = 0;
    axis_attribute->GetLimitOffset(&limit_offset);
    double limit_torque_positive = 0;
    axis_attribute->GetLimitTorquePositive(&limit_torque_positive);
    double limit_torque_negative = 0;
    axis_attribute->GetLimitTorqueNegative(&limit_torque_negative);
    double base_offset = 0;
    axis_attribute->GetBaseOffset(&base_offset);
    double axis_multi = 0;
    axis_attribute->GetAxisMultiplier(&axis_multi);

    ui_->limit_positive->setText(QString::number(limit_positive));
    ui_->limit_negative->setText(QString::number(limit_negative));
    ui_->limit_max_velocity->setText(QString::number(limit_max_velocity));
    ui_->limit_min_velocity->setText(QString::number(limit_min_velocity));
    ui_->limit_max_acc->setText(QString::number(limit_max_acc));
    ui_->limit_min_acc->setText(QString::number(limit_min_acc));
    ui_->limit_max_dec->setText(QString::number(limit_max_dec));
    ui_->limit_min_dec->setText(QString::number(limit_min_dec));
    ui_->limit_offset->setText(QString::number(limit_offset));
    ui_->limit_torque_positive->setText(QString::number(limit_torque_positive));
    ui_->limit_torque_negative->setText(QString::number(limit_torque_negative));
    ui_->base_offset->setText(QString::number(base_offset));
    ui_->axis_multi->setText(QString::number(axis_multi));
  }

  //速度
  {
    ui_->tableWidget_speed_list_->horizontalHeader()->show();
    ui_->tableWidget_speed_list_->setRowCount(0);
    int speed_count = axis->GetSpeedCount();
    for (int i = 0; i < speed_count; i++) {
      yotta::AxisSpeedConfigPtr axis_speed = axis->GetSpeed(i);
      int RowCont = ui_->tableWidget_speed_list_->rowCount();
      ui_->tableWidget_speed_list_->insertRow(RowCont);

      std::string speed_ids;
      char speed_ids_char[256];
      axis_speed->GetIds(speed_ids_char, 256);
      speed_ids = speed_ids_char;

      double velocity = 0;
      double acc = 0;
      double dec = 0;
      axis_speed->GetVelocity(&velocity);
      axis_speed->GetAcc(&acc);
      axis_speed->GetDec(&dec);

      ui_->tableWidget_speed_list_->setItem(
          RowCont, 0, new QTableWidgetItem(QString::fromStdString(speed_ids)));
      ui_->tableWidget_speed_list_->setItem(
          RowCont, 1, new QTableWidgetItem(QString::number(velocity)));
      ui_->tableWidget_speed_list_->setItem(
          RowCont, 2, new QTableWidgetItem(QString::number(acc)));
      ui_->tableWidget_speed_list_->setItem(
          RowCont, 3, new QTableWidgetItem(QString::number(dec)));
      ui_->tableWidget_speed_list_->setRowHeight(RowCont, 69);
    }
    if (speed_count) {
      OnSpeedListSelect(speed_count - 1, 0);
    }
  }

  //区域
  {
    ui_->tableWidget_region_list_->horizontalHeader()->show();
    ui_->tableWidget_region_list_->setRowCount(0);
    int region_count = axis->GetRegionCount();
    for (int i = 0; i < region_count; i++) {
      yotta::AxisRegionConfigPtr axis_region = axis->GetRegion(i);

      int RowCont = ui_->tableWidget_region_list_->rowCount();
      ui_->tableWidget_region_list_->insertRow(RowCont);

      std::string region_ids;
      char region_ids_char[256];
      axis_region->GetIds(region_ids_char, 256);
      region_ids = region_ids_char;

      double region_negative = 0;
      double region_positive = 0;
      axis_region->GetRegionNegative(&region_negative);
      axis_region->GetRegionPositive(&region_positive);

      ui_->tableWidget_region_list_->setItem(
          RowCont, 0, new QTableWidgetItem(QString::fromStdString(region_ids)));
      ui_->tableWidget_region_list_->setItem(
          RowCont, 1, new QTableWidgetItem(QString::number(region_negative)));
      ui_->tableWidget_region_list_->setItem(
          RowCont, 2, new QTableWidgetItem(QString::number(region_positive)));
      ui_->tableWidget_region_list_->setRowHeight(RowCont, 69);
    }
    if (region_count) {
      OnRegionListSelect(region_count - 1, 0);
    }
  }

  //限位
  limit_setting_->SetCondition(0, axis_ids_, "");
}
void AxisSetting::OnParaSaveClicked() {
  yotta::AxisConfigItemPtr axis_config = GetCurrentAxisConfig();
  if (!axis_config) {
    return;
  }

  //参数
  {
    yotta::AxisAttributeConfigPtr axis_attribute = axis_config->GetAttribute();
    if (axis_attribute) {
      axis_attribute->SetLimitPositive(ui_->limit_positive->text().toDouble());
      axis_attribute->SetLimitNegative(ui_->limit_negative->text().toDouble());
      axis_attribute->SetLimitMaxVelocity(
          ui_->limit_max_velocity->text().toDouble());
      axis_attribute->SetLimitMinVelocity(
          ui_->limit_min_velocity->text().toDouble());
      axis_attribute->SetLimitMaxAcc(ui_->limit_max_acc->text().toDouble());
      axis_attribute->SetLimitMinAcc(ui_->limit_min_acc->text().toDouble());
      axis_attribute->SetLimitMaxDec(ui_->limit_max_dec->text().toDouble());
      axis_attribute->SetLimitMinDec(ui_->limit_min_dec->text().toDouble());
      axis_attribute->SetLimitOffset(ui_->limit_offset->text().toDouble());
      axis_attribute->SetLimitTorquePositive(
          ui_->limit_torque_positive->text().toDouble());
      axis_attribute->SetLimitTorqueNegative(
          ui_->limit_torque_negative->text().toDouble());
      axis_attribute->SetBaseOffset(ui_->base_offset->text().toDouble());
      axis_attribute->SetAxisMultiplier(ui_->axis_multi->text().toDouble());
    }
  }

  //速度
  int speed_count = ui_->tableWidget_speed_list_->rowCount();
  for (int i = 0; i < speed_count; i++) {
    std::string axis_speed_name =
        ui_->tableWidget_speed_list_->item(i, 0)->text().toStdString();
    yotta::AxisSpeedConfigPtr axis_speed = axis_config->GetSpeed(i);
    if (axis_speed) {
      axis_speed->SetIds(axis_speed_name.c_str());
      axis_speed->SetVelocity(
          ui_->tableWidget_speed_list_->item(i, 1)->text().toDouble());
      axis_speed->SetAcc(
          ui_->tableWidget_speed_list_->item(i, 2)->text().toDouble());
      axis_speed->SetDec(
          ui_->tableWidget_speed_list_->item(i, 3)->text().toDouble());
    }
  }

  //区域
  int region_count = ui_->tableWidget_region_list_->rowCount();
  for (int i = 0; i < region_count; i++) {
    std::string axis_region_name =
        ui_->tableWidget_region_list_->item(i, 0)->text().toStdString();
    yotta::AxisRegionConfigPtr axis_region = axis_config->GetRegion(i);
    if (axis_region) {
      axis_region->SetIds(axis_region_name.c_str());
      axis_region->SetRegionNegative(
          ui_->tableWidget_region_list_->item(i, 1)->text().toDouble());
      axis_region->SetRegionPositive(
          ui_->tableWidget_region_list_->item(i, 2)->text().toDouble());
    }
  }

  axis_config_->Save();
}
