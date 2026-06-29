// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#include "axis_setting_view.h"

#include <QComboBox>
#include <QLineEdit>
#include <QListView>
#include <QPointer>
#include <QPushButton>
#include "view/components/ps_button/ps_button.h"
#include "view/tools/popup_dialog/popup_dialog.h"

AxisSettingView::AxisSettingView(QWidget *parent)
    : QWidget(parent), ui(new Ui::AxisSettingViewClass()) {
  ui->setupUi(this);
  axis_config_ = yotta::ConfigFactory::GetInstance()->GetAxisConfig();

  UnitInfoMgrPtr motion_module =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!motion_module) {
    return;
  }
  //左侧工位列表
  bool first_check = true;  //选中第一个工位
  for (int i = 0; i < motion_module->GetUnitCount(); i++) {
    QPushButton *axis_btn = new PsButton(
        QString::fromStdString(motion_module->GetUnitInfo(i)->unit_ids));
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
      module_id_ = motion_module->GetUnitInfo(i)->unit_ids;
      first_check = false;
    }
    ui->module_list_lay->addWidget(axis_btn);
    btn_group_module_.addButton(axis_btn, i);
  }
  ui->module_list_lay->addStretch();
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
  btn_group_region_limit_.addButton(ui->button_axis_parameter_, 0);
  btn_group_region_limit_.addButton(ui->button_axis_speed_, 1);
  btn_group_region_limit_.addButton(ui->button_axis_region_, 2);
  btn_group_region_limit_.addButton(ui->button_axis_limit_, 3);
  ui->button_axis_parameter_->setChecked(true);
  for (QAbstractButton *button : btn_group_region_limit_.buttons()) {
    connect(button, &QAbstractButton::clicked, this, [this, button]() {
      int id = btn_group_region_limit_.id(button);
      OnParaVelRegionLimitClicked(id);
    });
  }
  ui->stackedWidget->setCurrentWidget(ui->para_page_);

  {
    QStringList horizontalHeaders;
    horizontalHeaders << tr("Speed Name") << tr("Velocity") << tr("Acc")
                      << tr("Dec");
    ui->tableWidget_speed_list_->setHorizontalHeaderLabels(horizontalHeaders);
    ui->tableWidget_speed_list_->setColumnWidth(0, 150);

    ui->tableWidget_speed_list_->setShowGrid(true);  //显示表格线
    ui->tableWidget_speed_list_->setVerticalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);  //隐藏滑动条
    ui->tableWidget_speed_list_->setSelectionBehavior(
        QAbstractItemView::SelectRows);  //设置行选择模式
    ui->tableWidget_speed_list_->setSelectionMode(
        QAbstractItemView::SingleSelection);
    // ui->tableWidget_decice_list->setRowHeight(69);
    ui->tableWidget_speed_list_->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget_speed_list_->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);  //先自适应宽度
    // ui->tableWidget_region_list_->horizontalHeader()->setSectionResizeMode(
    //    0, QHeaderView::ResizeToContents);  //按文本长度自动确定每列宽度
    ui->tableWidget_speed_list_->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Fixed);  //固定0列宽度
    ui->tableWidget_speed_list_->setColumnWidth(0, 150);

    connect(ui->tableWidget_speed_list_, &QTableWidget::cellClicked, this,
            &AxisSettingView::OnSpeedListSelect);
  }
  {
    QStringList horizontalHeaders;
    horizontalHeaders << tr("Region Name") << tr("Negative") << tr("Positive");
    ui->tableWidget_region_list_->setHorizontalHeaderLabels(horizontalHeaders);
    ui->tableWidget_region_list_->setColumnWidth(0, 150);

    ui->tableWidget_region_list_->setShowGrid(true);  //显示表格线
    ui->tableWidget_region_list_->setVerticalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);  //隐藏滑动条
    ui->tableWidget_region_list_->setSelectionBehavior(
        QAbstractItemView::SelectRows);  //设置行选择模式
    ui->tableWidget_region_list_->setSelectionMode(
        QAbstractItemView::SingleSelection);
    // ui->tableWidget_decice_list->setRowHeight(69);
    ui->tableWidget_region_list_->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget_region_list_->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);  //先自适应宽度
    // ui->tableWidget_region_list_->horizontalHeader()->setSectionResizeMode(
    //    0, QHeaderView::ResizeToContents);  //按文本长度自动确定每列宽度
    ui->tableWidget_region_list_->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Fixed);  //固定0列宽度
    ui->tableWidget_region_list_->setColumnWidth(0, 150);

    connect(ui->tableWidget_region_list_, &QTableWidget::cellClicked, this,
            &AxisSettingView::OnRegionListSelect);
  }
  {
    limit_setting_view_ = new LimitSettingView;
    ui->limit_set_layout->addWidget(limit_setting_view_);
  }

  connect(ui->button_save_para_, &QPushButton::clicked, this,
                   &AxisSettingView::OnParaSaveClicked);

  connect(ui->button_add_spped_, &QPushButton::clicked, this,
                   &AxisSettingView::OnAddSpeedButtonClicked);
  connect(ui->button_del_spped_, &QPushButton::clicked, this,
                   &AxisSettingView::OnDelSpeedButtonClicked);
  connect(ui->button_save_spped_, &QPushButton::clicked, this,
                   &AxisSettingView::OnSaveSpeedButtonClicked);
  connect(ui->button_add_region_, &QPushButton::clicked, this,
                   &AxisSettingView::OnAddRegionButtonClicked);
  connect(ui->button_del_region_, &QPushButton::clicked, this,
                   &AxisSettingView::OnDelRegionButtonClicked);
  connect(ui->button_save_region_, &QPushButton::clicked, this,
                   &AxisSettingView::OnSaveRegionButtonClicked);

  ShowModule();
}
AxisSettingView::~AxisSettingView() { delete ui; }

void AxisSettingView::ClearLayout(QLayout *layout) {
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
    // delete item;  // 删除布局项
  }
}
void AxisSettingView::OnModuleClicked(int module_index) {
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
void AxisSettingView::OnAxisClicked(int id) {
  if (id < axis_id_list_.size()) {
    axis_ids_ = axis_id_list_[id];
    ShowModuleAxis();
  }
}
void AxisSettingView::OnParaVelRegionLimitClicked(int id) {
  switch (id) {
    case 0:
      ui->stackedWidget->setCurrentWidget(ui->para_page_);
      break;
    case 1:
      ui->stackedWidget->setCurrentWidget(ui->speed_page_);
      break;
    case 2:
      ui->stackedWidget->setCurrentWidget(ui->region_page_);
      break;
    case 3:
      ui->stackedWidget->setCurrentWidget(ui->limit_page_);
      break;
    default:
      break;
  }
}

void AxisSettingView::OnSpeedListSelect(int row, int column) {
  speed_index_ = row;
}
void AxisSettingView::OnAddSpeedButtonClicked() {
  yotta::AxisConfigItemPtr axis_config =
      axis_config_->GetAxisByIds(axis_ids_.c_str());
  if (!axis_config) {
    return;
  }

  int RowCont = ui->tableWidget_speed_list_->rowCount();
  ui->tableWidget_speed_list_->insertRow(RowCont);

  std::string speed_ids = "new_speed";
  double velocity = 1;
  double acc = 0.1;
  double dec = 0.1;

  yotta::AxisSpeedConfigPtr axis_region =
      axis_config->AddSpeed(speed_ids.c_str());
  axis_region->SetVelocity(velocity);
  axis_region->SetAcc(acc);
  axis_region->SetDec(dec);

  ui->tableWidget_speed_list_->setItem(
      RowCont, 0, new QTableWidgetItem(QString::fromStdString(speed_ids)));
  ui->tableWidget_speed_list_->setItem(
      RowCont, 1, new QTableWidgetItem(QString::number(velocity)));
  ui->tableWidget_speed_list_->setItem(
      RowCont, 2, new QTableWidgetItem(QString::number(acc)));
  ui->tableWidget_speed_list_->setItem(
      RowCont, 3, new QTableWidgetItem(QString::number(dec)));
}
void AxisSettingView::OnDelSpeedButtonClicked() {
  yotta::AxisConfigItemPtr axis_config =
      axis_config_->GetAxisByIds(axis_ids_.c_str());
  if (!axis_config) {
    return;
  }

  int RowCont = ui->tableWidget_speed_list_->rowCount();
  if (!RowCont) {
    return;
  }
  speed_index_ = speed_index_ < RowCont ? speed_index_ : RowCont - 1;
  speed_index_ = speed_index_ < 0 ? 0 : speed_index_;

  QTableWidgetItem *speed_item = ui->tableWidget_speed_list_->item(speed_index_, 0);
  if (!speed_item) return;
  std::string speed_ids = speed_item->text().toStdString();

  axis_config->DeleteSpeed(speed_ids.c_str());

  ui->tableWidget_speed_list_->removeRow(speed_index_);
}
void AxisSettingView::OnSaveSpeedButtonClicked() {
  int RowCont = ui->tableWidget_speed_list_->rowCount();
  if (!RowCont) {
    return;
  }
  OnParaSaveClicked();
}

void AxisSettingView::OnRegionListSelect(int row, int column) {
  region_index_ = row;
}
void AxisSettingView::OnAddRegionButtonClicked() {
  yotta::AxisConfigItemPtr axis_config =
      axis_config_->GetAxisByIds(axis_ids_.c_str());
  if (!axis_config) {
    return;
  }

  int RowCont = ui->tableWidget_region_list_->rowCount();
  ui->tableWidget_region_list_->insertRow(RowCont);

  std::string region_ids = "new_region";
  double negative = 0.1;
  double postive = 0.2;
  yotta::AxisRegionConfigPtr axis_region =
      axis_config->AddRegion(region_ids.c_str());
  axis_region->SetRegionNegative(negative);
  axis_region->SetRegionPositive(postive);

  ui->tableWidget_region_list_->setItem(
      RowCont, 0, new QTableWidgetItem(QString::fromStdString(region_ids)));
  ui->tableWidget_region_list_->setItem(
      RowCont, 1, new QTableWidgetItem(QString::number(negative)));
  ui->tableWidget_region_list_->setItem(
      RowCont, 2, new QTableWidgetItem(QString::number(postive)));
}
void AxisSettingView::OnDelRegionButtonClicked() {
  yotta::AxisConfigItemPtr axis_config =
      axis_config_->GetAxisByIds(axis_ids_.c_str());
  if (!axis_config) {
    return;
  }

  int RowCont = ui->tableWidget_region_list_->rowCount();
  if (!RowCont) {
    return;
  }
  region_index_ = region_index_ < RowCont ? region_index_ : RowCont - 1;
  region_index_ = region_index_ < 0 ? 0 : region_index_;

  std::string region_ids = ui->tableWidget_region_list_->item(region_index_, 0)
                               ->text()
                               .toStdString();

  axis_config->DeleteRegion(region_ids.c_str());

  ui->tableWidget_region_list_->removeRow(region_index_);
}
void AxisSettingView::OnSaveRegionButtonClicked() {
  int RowCont = ui->tableWidget_region_list_->rowCount();
  if (!RowCont) {
    return;
  }
  OnParaSaveClicked();
}

void AxisSettingView::ShowModule() {
  ClearLayout(ui->axis_list_lay_);
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
  UnitInfoPtr module_axis_info = motion_module->GetUnitInfo(module_id_);
  axis_id_list_.clear();
  bool first_check = true;  //选中第一个轴
  if (module_axis_info) {
    for (int i = 0; i < module_axis_info->axis_ids.size(); i++) {
      // 按钮
      axis_id_list_.push_back(module_axis_info->axis_ids[i]);
      QPushButton *axis_pbt = new PsButton(
          QString::fromStdString(module_axis_info->axis_ids[i]));
      axis_pbt->setCheckable(true);
      axis_pbt->setFixedSize(160, 69);
      if (first_check) {
        axis_pbt->setChecked(true);
        axis_ids_ = module_axis_info->axis_ids[i];
        first_check = false;
      }
      btn_group_axis_.addButton(axis_pbt, i);
      ui->axis_list_lay_->addWidget(axis_pbt);
    }
  }
  ui->axis_list_lay_->addStretch();

  for (QAbstractButton *button : btn_group_axis_.buttons()) {
    connect(button, &QAbstractButton::clicked, this, [this, button]() {
      int id = btn_group_axis_.id(button);
      OnAxisClicked(id);
    });
  }

  ShowModuleAxis();
}
void AxisSettingView::ShowModuleAxis() {
  yotta::AxisConfigItemPtr axis = axis_config_->GetAxisByIds(axis_ids_.c_str());
  if (!axis) {
    return;
    LOG(ERROR) << "get axis failed axis_ids=  " << axis_ids_;
  }
  yotta::AxisAttributeConfigPtr axis_attribute = axis->GetAttribute();

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
    double axis_mileage = 0;
    axis_attribute->GetAxisMileage(&axis_mileage);

    ui->limit_positive->setText(QString::number(limit_positive));
    ui->limit_negative->setText(QString::number(limit_negative));
    ui->limit_max_velocity->setText(QString::number(limit_max_velocity));
    ui->limit_min_velocity->setText(QString::number(limit_min_velocity));
    ui->limit_max_acc->setText(QString::number(limit_max_acc));
    ui->limit_min_acc->setText(QString::number(limit_min_acc));
    ui->limit_max_dec->setText(QString::number(limit_max_dec));
    ui->limit_min_dec->setText(QString::number(limit_min_dec));
    ui->limit_offset->setText(QString::number(limit_offset));
    ui->limit_torque_positive->setText(QString::number(limit_torque_positive));
    ui->limit_torque_negative->setText(QString::number(limit_torque_negative));
    ui->base_offset->setText(QString::number(base_offset));
    ui->axis_multi->setText(QString::number(axis_multi));
    ui->axis_mileage->setText(QString::number(axis_mileage));

    // ui->nEnabled->setText(
    //    QString::number(current_axis_info_.property_info_.nEnabled));
    // ui->nPropertyMode->setText(
    //    QString::number(current_axis_info_.property_info_.nPropertyMode));
    // ui->nElectricMode->setText(
    //    QString::number(current_axis_info_.property_info_.nElectricMode));
    // ui->nElectricEcID->setText(
    //    QString::number(current_axis_info_.property_info_.nElectricEcID));
    // ui->nElectricIndex->setText(
    //    QString::number(current_axis_info_.property_info_.nElectricIndex));
    // ui->nElectricSubIndex->setText(
    //    QString::number(current_axis_info_.property_info_.nElectricSubIndex));
    // ui->nElectricSize->setText(
    //    QString::number(current_axis_info_.property_info_.nElectricSize));
    // ui->nElectricMulti->setText(
    //    QString::number(current_axis_info_.property_info_.nElectricMulti));
  }

  //速度
  {
    ui->tableWidget_speed_list_->horizontalHeader()->show();
    ui->tableWidget_speed_list_->setRowCount(0);
    int speed_count = axis->GetSpeedCount();
    for (int i = 0; i < speed_count; i++) {
      yotta::AxisSpeedConfigPtr axis_speed = axis->GetSpeed(i);
      int RowCont = ui->tableWidget_speed_list_->rowCount();
      ui->tableWidget_speed_list_->insertRow(RowCont);

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

      ui->tableWidget_speed_list_->setItem(
          RowCont, 0, new QTableWidgetItem(QString::fromStdString(speed_ids)));
      ui->tableWidget_speed_list_->setItem(
          RowCont, 1, new QTableWidgetItem(QString::number(velocity)));
      ui->tableWidget_speed_list_->setItem(
          RowCont, 2, new QTableWidgetItem(QString::number(acc)));
      ui->tableWidget_speed_list_->setItem(
          RowCont, 3, new QTableWidgetItem(QString::number(dec)));
      ui->tableWidget_speed_list_->setRowHeight(RowCont, 69);
    }
    if (speed_count) {
      OnSpeedListSelect(speed_count - 1, 0);
    }
  }

  //区域
  {
    ui->tableWidget_region_list_->horizontalHeader()->show();
    ui->tableWidget_region_list_->setRowCount(0);
    int region_count = axis->GetRegionCount();
    for (int i = 0; i < region_count; i++) {
      yotta::AxisRegionConfigPtr axis_region = axis->GetRegion(i);

      int RowCont = ui->tableWidget_region_list_->rowCount();
      ui->tableWidget_region_list_->insertRow(RowCont);

      std::string region_ids;
      char region_ids_char[256];
      axis_region->GetIds(region_ids_char, 256);
      region_ids = region_ids_char;

      double region_negative = 0;
      double region_positive = 0;
      axis_region->GetRegionNegative(&region_negative);
      axis_region->GetRegionPositive(&region_positive);

      ui->tableWidget_region_list_->setItem(
          RowCont, 0, new QTableWidgetItem(QString::fromStdString(region_ids)));
      ui->tableWidget_region_list_->setItem(
          RowCont, 1, new QTableWidgetItem(QString::number(region_negative)));
      ui->tableWidget_region_list_->setItem(
          RowCont, 2, new QTableWidgetItem(QString::number(region_positive)));
      ui->tableWidget_region_list_->setRowHeight(RowCont, 69);
    }
    if (region_count) {
      OnRegionListSelect(region_count - 1, 0);
    }
  }

  //限位
  limit_setting_view_->SetCondition(0, axis_ids_, "");
}
void AxisSettingView::OnParaSaveClicked() {
  if (RealTimeDataSinglton::GetInstance()->current_user_level() < kL3SU1) {
    return;
  }
  // 前置确认：是否保存
  PopupDialogSingleton::GetInstance()->ShowPage();
  if (!PopupDialogSingleton::GetInstance()->PopupInfo("是否保存?")) {
    return;
  }

  yotta::AxisConfigItemPtr axis_config =
      axis_config_->GetAxisByIds(axis_ids_.c_str());
  if (!axis_config) {
    return;
  }

  //参数
  {
    yotta::AxisAttributeConfigPtr axis_attribute = axis_config->GetAttribute();
    if (axis_attribute) {
      axis_attribute->SetLimitPositive(ui->limit_positive->text().toDouble());
      axis_attribute->SetLimitNegative(ui->limit_negative->text().toDouble());
      axis_attribute->SetLimitMaxVelocity(
          ui->limit_max_velocity->text().toDouble());
      axis_attribute->SetLimitMinVelocity(
          ui->limit_min_velocity->text().toDouble());
      axis_attribute->SetLimitMaxAcc(ui->limit_max_acc->text().toDouble());
      axis_attribute->SetLimitMinAcc(ui->limit_min_acc->text().toDouble());
      axis_attribute->SetLimitMaxDec(ui->limit_max_dec->text().toDouble());
      axis_attribute->SetLimitMinDec(ui->limit_min_dec->text().toDouble());
      axis_attribute->SetLimitOffset(ui->limit_offset->text().toDouble());
      axis_attribute->SetLimitTorquePositive(
          ui->limit_torque_positive->text().toDouble());
      axis_attribute->SetLimitTorqueNegative(
          ui->limit_torque_negative->text().toDouble());
      axis_attribute->SetBaseOffset(ui->base_offset->text().toDouble());
      axis_attribute->SetAxisMultiplier(ui->axis_multi->text().toDouble());

      // current_axis_info_.property_info_.nEnabled =
      //    ui->nEnabled->text().toDouble();
      // current_axis_info_.property_info_.nPropertyMode =
      //    ui->nPropertyMode->text().toDouble();
      // current_axis_info_.property_info_.nElectricMode =
      //    ui->nElectricMode->text().toDouble();
      // current_axis_info_.property_info_.nElectricEcID =
      //    ui->nElectricEcID->text().toDouble();
      // current_axis_info_.property_info_.nElectricIndex =
      //    ui->nElectricIndex->text().toDouble();
      // current_axis_info_.property_info_.nElectricSubIndex =
      //    ui->nElectricSubIndex->text().toDouble();
      // current_axis_info_.property_info_.nElectricSize =
      //    ui->nElectricSize->text().toDouble();
      // current_axis_info_.property_info_.nElectricMulti =
      //    ui->nElectricMulti->text().toDouble();
    }
  }

  //速度
  int speed_count = ui->tableWidget_speed_list_->rowCount();
  for (int i = 0; i < speed_count; i++) {
    QTableWidgetItem *speed_name_item = ui->tableWidget_speed_list_->item(i, 0);
    QTableWidgetItem *speed_vel_item = ui->tableWidget_speed_list_->item(i, 1);
    QTableWidgetItem *speed_acc_item = ui->tableWidget_speed_list_->item(i, 2);
    QTableWidgetItem *speed_dec_item = ui->tableWidget_speed_list_->item(i, 3);
    if (!speed_name_item || !speed_vel_item || !speed_acc_item || !speed_dec_item)
      continue;
    std::string axis_speed_name = speed_name_item->text().toStdString();
    yotta::AxisSpeedConfigPtr axis_speed = axis_config->GetSpeed(i);
    if (axis_speed) {
      axis_speed->SetIds(axis_speed_name.c_str());
      axis_speed->SetVelocity(speed_vel_item->text().toDouble());
      axis_speed->SetAcc(speed_acc_item->text().toDouble());
      axis_speed->SetDec(speed_dec_item->text().toDouble());
    }
  }

  //区域
  int region_count = ui->tableWidget_region_list_->rowCount();
  for (int i = 0; i < region_count; i++) {
    QTableWidgetItem *region_name_item = ui->tableWidget_region_list_->item(i, 0);
    QTableWidgetItem *region_neg_item = ui->tableWidget_region_list_->item(i, 1);
    QTableWidgetItem *region_pos_item = ui->tableWidget_region_list_->item(i, 2);
    if (!region_name_item || !region_neg_item || !region_pos_item)
      continue;
    std::string axis_region_name = region_name_item->text().toStdString();
    yotta::AxisRegionConfigPtr axis_region = axis_config->GetRegion(i);
    if (axis_region) {
      axis_region->SetIds(axis_region_name.c_str());
      axis_region->SetRegionNegative(region_neg_item->text().toDouble());
      axis_region->SetRegionPositive(region_pos_item->text().toDouble());
    }
  }

  axis_config_->Save();
}