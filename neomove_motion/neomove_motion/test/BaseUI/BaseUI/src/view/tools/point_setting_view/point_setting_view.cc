// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#include "point_setting_view.h"

#include <common/message_loop.h>
#include <limit_motion/impl/acc_dec_profile_impl.h>
#include <limit_motion/limit_motion_mgr.h>
#include <main_process/module_mgr.h>

#include <QApplication>
#include <QHeaderView>
#include <QPushButton>

#include "controller/device_status_monitor/device_status_monitor.h"
#include "model/model_mgr.h"
#include "view/components/ps_button/ps_button.h"
#include "view/tools/multi_image_show_view/multi_image_show_view.h"
#include "view/tools/popup_dialog/popup_dialog.h"
#include "view/tools/robot_xyrz_view/robot_xyrz.h"

PointSettingView::PointSettingView(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::point_setting_viewClass()),
      button_size_(80, 54) {
  ui->setupUi(this);

  axis_config_ = yotta::ConfigFactory::GetInstance()->GetAxisConfig();
  point_config_ = yotta::ConfigFactory::GetInstance()->GetPointConfig();
  unit_point_mgr_ = ModelMgrSinglton::GetInstance()->unit_point_mgr();
  unit_info_mgr_ = ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!unit_info_mgr_ || !axis_config_ || !point_config_ || !unit_point_mgr_) {
    LOG(ERROR) << "point_set_view page init failed!";
    return;
  }
  //{
  //  qss_table_button_ =
  //      "QPushButton {"
  //      "background-image:url(:/icon/深蓝色/"
  //      "80_54--正常.png);"
  //      "background-repeat: no-repeat;"
  //      "background-position: center;"
  //      "color: white;"
  //      "}"
  //      " QPushButton:pressed  {"
  //      "background-color: #3A76D8;"
  //      "}";
  //  qss_unit_button_ = R"(QPushButton {
  //      background-color: #4A86E8;
  //              border: none;
  //              border-radius: 8px;
  //      font-size: 24px;
  //      color: white;
  //  }
  //      QPushButton:checked  {
  //      background-color: #F39C12;
  //              border: none;
  //              border-radius: 8px;
  //      font-size: 24px;
  //      color: black;
  //  })";
  //}
  for (int i = 0; i < unit_info_mgr_->GetUnitCount(); i++) {
    QPushButton* axis_btn = new PsButton(
        QString::fromStdString(unit_info_mgr_->GetUnitInfo(i)->unit_ids));
    axis_btn->setStyleSheet(qss_unit_button_);
    axis_btn->setFixedSize(140, 69);
    axis_btn->setCheckable(true);
    ui->module_list_layout->addWidget(axis_btn);
    btn_group_unit_.addButton(axis_btn, i);
  }
  ui->module_list_layout->addStretch();
  for (QAbstractButton* button : btn_group_unit_.buttons()) {
    connect(button, &QAbstractButton::clicked, this,
            [this, button](bool checked) {
              if (!checked) {
                return;
              }
              int id = btn_group_unit_.id(button);
              OnUnitClicked(id);
            });
  }
  // 选中第零个按钮
  QAbstractButton* btn = btn_group_unit_.button(0);
  if (btn) {
    btn->setChecked(true);
    OnUnitClicked(0);
  }

  // 按钮点击信号槽
  connect(ui->button_switch, &QPushButton::clicked, this,
          &PointSettingView::SwitchPage);  // 切换界面
  connect(ui->button_add, &QPushButton::clicked, this,
          &PointSettingView::AddPoint);  // 添加
  connect(ui->button_delete, &QPushButton::clicked, this,
          &PointSettingView::DeletePoint);  // 删除

  // 上锁按钮：checkable，用 checked 状态（颜色）指示锁定，不依赖文字
  ui->button_lock->setCheckable(true);
  ui->button_lock->setChecked(is_locked_);
  connect(ui->button_lock, &QAbstractButton::toggled, this,
          &PointSettingView::SetLockedState);

  // 更新选中项
  connect(ui->point_list_table, &QTableWidget::cellClicked, this,
          &PointSettingView::OnPointSelect);

  // 监听point_list_table的列宽变化，title_table的第一列与点位信息表格第一列宽度保持一致
  connect(ui->point_list_table->horizontalHeader(),
          &QHeaderView::sectionResized,
          [this](int logicalIndex, int, int newSize) {
            if (logicalIndex == 0) {
              ui->axis_list_table->setColumnWidth(0, newSize);
            }
          });
}

PointSettingView::~PointSettingView() { delete ui; }

void PointSettingView::SetLockedState(bool locked) {
  is_locked_ = locked;
  ui->button_lock->blockSignals(true);
  ui->button_lock->setChecked(locked);
  ui->button_lock->blockSignals(false);
}

void PointSettingView::ToggleLockState() { SetLockedState(!is_locked_); }

void PointSettingView::OnUnitClicked(int unit_index) {
  if (!unit_info_mgr_) {
    return;
  }
  current_unit_index_ = unit_index;
  current_unit_info_ = *unit_info_mgr_->GetUnitInfo(unit_index);
  current_unit_ids_ = current_unit_info_.unit_ids;
  RobotXYZRSingleton::GetInstance()->SetUnit(current_unit_ids_);
  current_point_.point_ids.clear();
  current_point_.unit_ids = current_unit_ids_;

  ui->axis_list_table->setRowCount(0);
  ui->axis_list_table->setColumnCount(0);
  ui->axis_list_table->clear();
  ui->point_list_table->setRowCount(0);
  ui->point_list_table->setColumnCount(0);
  // ui->point_list_table->clear();
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
void PointSettingView::InitTitleTable() {
  ui->axis_list_table->setColumnCount(axis_num_ + 3);
  ui->axis_list_table->setRowCount(1);
  ui->axis_list_table->setRowHeight(0, button_size_.height());
  ui->axis_list_table->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->axis_list_table->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);  // 先自适应宽度
  ui->axis_list_table->horizontalHeader()->setSectionResizeMode(
      0, QHeaderView::Interactive);

  // 按钮列设为固定模式
  ui->axis_list_table->horizontalHeader()->setSectionResizeMode(
      axis_num_ + 1, QHeaderView::Fixed);
  ui->axis_list_table->horizontalHeader()->setSectionResizeMode(
      axis_num_ + 2, QHeaderView::Fixed);
  ui->axis_list_table->setColumnWidth(axis_num_ + 1, button_size_.width());
  ui->axis_list_table->setColumnWidth(axis_num_ + 2, button_size_.width());
  ui->axis_list_table->horizontalHeader()->setVisible(false);
  ui->axis_list_table->verticalHeader()->setVisible(false);

  // 0,0单元格
  QTableWidgetItem* item1 = new QTableWidgetItem();
  item1->setText(tr("点位\\轴"));
  item1->setFlags(Qt::ItemIsEnabled);
  ui->axis_list_table->setItem(0, 0, item1);
  // 轴列表勾选
  for (int i = 0; i < axis_num_; ++i) {
    QPointer<QCheckBox> checkBox = new QCheckBox(axis_names_[i]);
    ui->axis_list_table->setCellWidget(0, i + 1, checkBox);
    checkboxs_axis_.push_back(checkBox);
  }
  // 上一行/下一行按钮
  QPushButton* last_row = new PsButton(tr("上一行"));
  QPushButton* next_row = new PsButton(tr("下一行"));
  //last_row->setStyleSheet(qss_table_button_);
  //next_row->setStyleSheet(qss_table_button_);
  // 连接点击信号
  connect(last_row, &QPushButton::clicked, last_row, [this] {
    int row_count = ui->point_list_table->rowCount();
    if (row_count <= 0) {
      return;
    }
    int current_row = ui->point_list_table->currentRow();
    if (current_row < 0) {
      current_row = 0;
    }
    int target_row = qMax(current_row - 1, 0);
    SelectPointRow(target_row);
    OnPointSelect(target_row, 0);
  });
  connect(next_row, &QPushButton::clicked, next_row, [this] {
    int row_count = ui->point_list_table->rowCount();
    if (row_count <= 0) {
      return;
    }
    int current_row = ui->point_list_table->currentRow();
    if (current_row < 0) {
      current_row = 0;
    }
    int target_row = qMin(current_row + 1, row_count - 1);
    SelectPointRow(target_row);
    OnPointSelect(target_row, 0);
  });
  ui->axis_list_table->setCellWidget(0, axis_num_ + 1, last_row);
  ui->axis_list_table->setCellWidget(0, axis_num_ + 2, next_row);
}
void PointSettingView::InitPointListTable() {
  ui->point_list_table->setColumnCount(axis_num_ + 3);
  ui->point_list_table->setShowGrid(true);  // 显示表格线
  ui->point_list_table->setVerticalScrollBarPolicy(
      Qt::ScrollBarAlwaysOff);  // 隐藏滑动条
  ui->point_list_table->setSelectionBehavior(
      QAbstractItemView::SelectRows);  // 设置行选择模式
  ui->point_list_table->setFocusPolicy(Qt::StrongFocus);
  ui->point_list_table->setEditTriggers(QAbstractItemView::DoubleClicked |
                                        QAbstractItemView::EditKeyPressed |
                                        QAbstractItemView::AnyKeyPressed);
  ui->point_list_table->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->point_list_table->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);  // 先自适应宽度
  // 修改特定列的调整模式,设为固定模式
  ui->point_list_table->horizontalHeader()->setSectionResizeMode(
      axis_num_ + 1, QHeaderView::Fixed);
  ui->point_list_table->horizontalHeader()->setSectionResizeMode(
      axis_num_ + 2, QHeaderView::Fixed);
  ui->point_list_table->horizontalHeader()->setVisible(false);
  ui->point_list_table->verticalHeader()->setVisible(false);

  // 按行填充
  for (int i = 0; i < unit_point_mgr_->GetUnitPointCount(current_unit_ids_);
       i++) {
    UnitPoint point_info = unit_point_mgr_->GetUnitPoint(current_unit_ids_, i);
    if (point_info.point_ids.empty()) {
      continue;
    }

    int RowCont = ui->point_list_table->rowCount();
    ui->point_list_table->insertRow(RowCont);

    // ui->point_list_table->setRowCount(ui->point_list_table->rowCount() +
    // 1);
    AppendItemByRow(RowCont, point_info.point_ids);
  }
}
void PointSettingView::AppendItemByRow(int row, std::string point_ids) {
  QTableWidgetItem* item_point_name =
      new QTableWidgetItem(QString::fromStdString(point_ids));
  if (!point_ids.empty()) {
    item_point_name->setFlags(item_point_name->flags() & ~Qt::ItemIsEditable);
  }
  ui->point_list_table->setItem(row, 0, item_point_name);

  for (int col = 0; col < axis_num_; col++) {
    // 轴位置默认值
    QTableWidgetItem* cell = new QTableWidgetItem("0.0000");
    cell->setFlags(cell->flags() & ~Qt::ItemIsEditable);  // 移除可编辑标志
    ui->point_list_table->setItem(row, col + 1, cell);

    yotta::PointConfigItemPtr point = nullptr;
    if (!point_ids.empty()) {
      point = point_config_->GetPointByIds(point_ids.c_str());
    }
    if (!point) {
      continue;
    }
    for (int j = 0; j < point->GetPointAxisItemCount(); ++j) {
      yotta::PointAxisConfigItemPtr point_axis = point->GetPointAxisItem(j);

      std::string axis_ids;
      char axis_ids_char[256];
      double pos = 0;
      point_axis->GetAxisIds(axis_ids_char, 256);
      point_axis->GetPosition(&pos);
      axis_ids = axis_ids_char;

      if (axis_names_[col].toStdString() == axis_ids) {
        // 设置轴位置,并显示蓝色
        ui->point_list_table->item(row, col + 1)
            ->setText(QString::number(pos, 'f', 4));
        cell->setForeground(Qt::blue);
        break;
      }
    }
  }

  // 定点,GOTO按钮
  QPointer<QPushButton> btn_goto_point = new PsButton(tr("定点"));
  btn_goto_point->setFixedSize(button_size_);
  btn_goto_point->setStyleSheet(qss_table_button_);
  QPointer<QPushButton> btn_set = new PsButton(tr("设置"));
  btn_set->setFixedSize(button_size_);
  btn_set->setStyleSheet(qss_table_button_);
  ui->point_list_table->setCellWidget(row, axis_num_ + 1, btn_goto_point);
  ui->point_list_table->setCellWidget(row, axis_num_ + 2, btn_set);
  buttons_goto_point_.push_back(btn_goto_point);
  buttons_set_.push_back(btn_set);
  // 连接信号槽
  connect(btn_goto_point, &QPushButton::clicked, this,
          &PointSettingView::OnGotoButtonClicked);
  connect(btn_set, &QPushButton::clicked, this,
          &PointSettingView::OnSetButtonClicked);
  // 设置行高
  ui->point_list_table->setRowHeight(row, button_size_.height());
  ui->point_list_table->setColumnWidth(axis_num_ + 1, button_size_.width());
  ui->point_list_table->setColumnWidth(axis_num_ + 2, button_size_.width());
}

void PointSettingView::OnSetButtonClicked() {
  if (is_locked_) {
    PopupDialogSingleton::GetInstance()->PopupInfo("请先解锁点位设置");
    return;
  }
  int current_row = RowFromButton(buttons_set_, sender());
  if (current_row < 0 || current_row >= ui->point_list_table->rowCount()) {
    return;
  }
  if (current_row_ != current_row) {
    SelectPointRow(current_row);
    OnPointSelect(current_row, 0);
  } else {
    SelectPointRow(current_row);
  }

  if (PointIdsAtRow(current_row).empty()) {
    PopupDialogSingleton::GetInstance()->PopupInfo("请输入点位名称");
    if (QTableWidgetItem* item = ui->point_list_table->item(current_row, 0)) {
      ui->point_list_table->editItem(item);
    }
    return;
  }

  if (!HasCheckedAxis()) {
    PopupDialogSingleton::GetInstance()->PopupInfo("请至少选择一个轴");
    return;
  }

  if (!PopupDialogSingleton::GetInstance()->PopupInfo(
          "是否确认保存当前点位?")) {
    return;
  }

  // std::vector<double> current_val;  //读出的实时数据
  // for (int col = 0; col < axis_num_; ++col) {
  //   current_val.push_back(robot_xyrz_->GetAxisPos(col));
  // }

  for (int col = 0; col < axis_num_; ++col) {
    QTableWidgetItem* item = ui->point_list_table->item(current_row, col + 1);
    if (!item) {
      continue;
    }

    if (!checkboxs_axis_[col]->isChecked()) {
      item->setText(QString::number(0.0000, 'f', 4));
      item->setForeground(Qt::black);
      continue;
    }

    DeviceStatusMonitor* monitor = DeviceStatusMonitorSinglton::GetInstance();
    double position = 0;
    if (!monitor->GetAxisPos(axis_names_[col].toStdString().c_str(),
                             position)) {
      LOG(ERROR) << "读取轴位置失败: " << axis_names_[col].toStdString();
      return;
    }
    // 设置从轴读出的数据
    item->setText(QString::number(position, 'f', 4));
    item->setForeground(Qt::blue);
  }

  // 保存
  SavePoint(current_row);
}
void PointSettingView::OnGotoButtonClicked() {
  if (is_locked_) {
    PopupDialogSingleton::GetInstance()->PopupInfo("请先解锁点位设置");
    return;
  }
  int current_row = RowFromButton(buttons_goto_point_, sender());
  if (current_row < 0 || current_row >= ui->point_list_table->rowCount()) {
    return;
  }
  SelectPointRow(current_row);

  std::string point_ids = PointIdsAtRow(current_row);
  if (point_ids.empty()) {
    return;
  }
  if (!unit_point_mgr_->HasUnitPoint(current_unit_ids_, point_ids)) {
    PopupDialogSingleton::GetInstance()->PopupInfo("请先保存点位");
    return;
  }
  current_point_.point_ids = point_ids;
  current_point_.unit_ids = current_unit_ids_;

  // 走位
  common::MessageLoop::GetMessageLoop(common::kMotion)
      ->PostTask(std::bind(&PointSettingView::GotoPoint, this));
}

void PointSettingView::SwitchPage() {
  SetLockedState(true);
  if (point_page_) {
    MultiImageShowViewSingleton::GetInstance()->ShowEmbedded(
        ui->image_show_layout);
    ui->stackedWidget->setCurrentWidget(ui->page_image);
    point_page_ = false;
  } else {
    MultiImageShowViewSingleton::GetInstance()->HidePage();
    ui->stackedWidget->setCurrentWidget(ui->page_point);
    point_page_ = true;
  }
}
void PointSettingView::AddPoint() {
  if (is_locked_) {
    PopupDialogSingleton::GetInstance()->PopupInfo("请先解锁点位设置");
    return;
  }
  if (!is_save_) {  // 最新行未保存
    PopupDialogSingleton::GetInstance()->PopupInfo(
        "请先设置并保存或删除当前新增点位");
    return;
  }
  is_save_ = false;

  ui->point_list_table->insertRow(ui->point_list_table->rowCount());
  current_row_ = ui->point_list_table->rowCount() - 1;  // 设置当前行索引
  // 为新行设置默认值
  AppendItemByRow(current_row_, "");
  SelectPointRow(current_row_);
  ClearAxisSelection();
  current_point_.point_ids.clear();
  current_point_.unit_ids = current_unit_ids_;
  if (QTableWidgetItem* item = ui->point_list_table->item(current_row_, 0)) {
    ui->point_list_table->editItem(item);
  }
}
void PointSettingView::DeletePoint() {
  if (is_locked_) {
    PopupDialogSingleton::GetInstance()->PopupInfo("请先解锁点位设置");
    return;
  }
  int row = ui->point_list_table->currentRow();
  if (row < 0) {
    row = current_row_;
  }
  if (row < 0 || row >= ui->point_list_table->rowCount()) {
    return;
  }

  int save_rows = GetRowCount(current_unit_ids_);
  if (row > save_rows) {
    return;
  }

  // 删除了新添加但没保存的行，只做更新界面即可
  if (row == save_rows) {
    is_save_ = true;
    OnUnitClicked(current_unit_index_);
    return;
  }

  std::string point_ids = PointIdsAtRow(row);
  if (point_ids.empty()) {
    return;
  }
  if (!PopupDialogSingleton::GetInstance()->PopupInfo(
          "是否确认删除当前点位?")) {
    return;
  }
  if (!unit_point_mgr_->DelUnitPoint(point_ids)) {
    LOG(WARNING) << "删除点位索引失败: " << point_ids;
    OnUnitClicked(current_unit_index_);
    return;
  }
  is_save_ = true;
  OnUnitClicked(current_unit_index_);
  if (ui->point_list_table->rowCount() > 0) {
    int target_row = qMin(row, ui->point_list_table->rowCount() - 1);
    SelectPointRow(target_row);
    OnPointSelect(target_row, 0);
  }
}
bool PointSettingView::SavePoint(int row) {
  int save_rows = GetRowCount(current_unit_ids_);
  // 行数错误
  if (row > save_rows || row < 0 || row >= ui->point_list_table->rowCount()) {
    return false;
  }

  UnitPoint point_read;
  ReadPoint(point_read, row);
  point_read.unit_ids = current_unit_ids_;
  // 名称不能为空
  if (point_read.point_ids.empty()) {
    PopupDialogSingleton::GetInstance()->PopupInfo("请输入点位名称");
    return false;
  }
  if (point_read.point_ids == "temp_point") {
    PopupDialogSingleton::GetInstance()->PopupInfo("请修改默认点位名称");
    return false;
  }
  if (row == save_rows && unit_point_mgr_->HasPointIds(point_read.point_ids)) {
    PopupDialogSingleton::GetInstance()->PopupInfo("点位名称已存在");
    return false;
  }
  if (!SavePointConfig(point_read.point_ids, row)) {
    return false;
  }
  if (!unit_point_mgr_->AddUnitPoint(point_read)) {
    PopupDialogSingleton::GetInstance()->PopupInfo("保存点位索引失败");
    return false;
  }
  is_save_ = true;
  OnUnitClicked(current_unit_index_);
  for (int i = 0; i < ui->point_list_table->rowCount(); ++i) {
    if (PointIdsAtRow(i) == point_read.point_ids) {
      SelectPointRow(i);
      OnPointSelect(i, 0);
      break;
    }
  }
  return true;
}
void PointSettingView::OnPointSelect(int row, int col) {
  current_row_ = row;
  current_point_.point_ids.clear();
  current_point_.unit_ids = current_unit_ids_;
  ClearAxisSelection();
  int save_rows = GetRowCount(current_unit_ids_);
  // 点击了最新行
  if (current_row_ >= save_rows) {
    return;
  }
  // 设置轴选择
  QTableWidgetItem* item_point = ui->point_list_table->item(row, 0);
  if (!item_point) return;
  std::string point_ids = item_point->text().trimmed().toStdString();
  current_point_.point_ids = point_ids;
  current_point_.unit_ids = current_unit_ids_;

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
    char axis_ids_char[256];
    double pos = 0;
    point_axis->GetAxisIds(axis_ids_char, 256);
    axis_ids = axis_ids_char;
    vec_axis.push_back(axis_ids);
  }

  for (int i = 0; i < vec_axis.size(); ++i) {
    for (int j = 0; j < axis_names_.size(); ++j) {
      if (vec_axis[i] == axis_names_[j].toStdString()) {
        checkboxs_axis_[j]->setChecked(true);
        break;
      }
    }
  }
}
void PointSettingView::ReadPoint(UnitPoint& point_para, int row) {
  point_para.point_ids.clear();
  point_para.unit_ids = current_unit_ids_;
  // 获取第零列point_ids
  point_para.point_ids = PointIdsAtRow(row);
}

bool PointSettingView::SavePointConfig(const std::string& point_ids, int row) {
  if (!point_config_ || point_ids.empty()) {
    return false;
  }
  yotta::PointConfigItemPtr point_config_item =
      point_config_->GetPointByIds(point_ids.c_str());
  if (!point_config_item) {
    point_config_item = point_config_->AddPoint(point_ids.c_str());
  }
  if (!point_config_item) {
    LOG(ERROR) << "AddPoint: " << point_ids << " error";
    return false;
  }

  for (int col = 0; col < axis_num_; ++col) {
    // 没有选择该列轴
    if (!checkboxs_axis_[col]->isChecked()) {
      continue;
    }
    // 列号要加一
    QTableWidgetItem* item_pos = ui->point_list_table->item(row, col + 1);
    if (!item_pos) continue;
    double pos = item_pos->text().toDouble();
    // 轴信息
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
  return true;
}

int PointSettingView::RowFromButton(
    const std::vector<QPointer<QPushButton>>& buttons, QObject* sender) const {
  QPushButton* current_button = qobject_cast<QPushButton*>(sender);
  if (!current_button) {
    return -1;
  }
  for (int i = 0; i < static_cast<int>(buttons.size()); ++i) {
    if (buttons[i] == current_button) {
      return i;
    }
  }
  return -1;
}

std::string PointSettingView::PointIdsAtRow(int row) const {
  if (!ui || !ui->point_list_table || row < 0 ||
      row >= ui->point_list_table->rowCount()) {
    return "";
  }
  QTableWidgetItem* item_name = ui->point_list_table->item(row, 0);
  if (!item_name) {
    return "";
  }
  return item_name->text().trimmed().toStdString();
}

bool PointSettingView::HasCheckedAxis() const {
  for (const QPointer<QCheckBox>& checkbox : checkboxs_axis_) {
    if (checkbox && checkbox->isChecked()) {
      return true;
    }
  }
  return false;
}

void PointSettingView::ClearAxisSelection() {
  for (QPointer<QCheckBox>& checkbox : checkboxs_axis_) {
    if (checkbox) {
      checkbox->setChecked(false);
    }
  }
}

void PointSettingView::SelectPointRow(int row) {
  if (!ui || !ui->point_list_table || row < 0 ||
      row >= ui->point_list_table->rowCount()) {
    current_row_ = -1;
    return;
  }
  current_row_ = row;
  ui->point_list_table->setCurrentCell(row, 0);
  ui->point_list_table->selectRow(row);
}

int PointSettingView::GetRowCount(const std::string& unit_ids) {
  return unit_point_mgr_->GetUnitPointCount(unit_ids);
}
void PointSettingView::GotoPoint() {
  if (current_point_.point_ids.empty()) {
    return;
  }
  const QString step_name =
      QString("移动到点位: %1")
          .arg(QString::fromStdString(current_point_.point_ids));

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

  PopupDialogSingleton::GetInstance()->PopupOperationStatus("点位移动",
                                                            {step_name}, 600);
  PopupDialogSingleton::GetInstance()->PopupOperationStatusAppend(step_name, 2);

  std::vector<yotta::Axis*> moving_axes;
  moving_axes.reserve(point_config->GetPointAxisItemCount());

  for (int j = 0; j < point_config->GetPointAxisItemCount(); ++j) {
    yotta::PointAxisConfigItemPtr point_axis_config =
        point_config->GetPointAxisItem(j);
    std::string axis_ids;
    char axis_ids_char[256];
    double dis_position = 0;
    point_axis_config->GetAxisIds(axis_ids_char, 256);
    point_axis_config->GetPosition(&dis_position);
    axis_ids = axis_ids_char;

    yotta::AxisConfigItemPtr axis_config =
        axis_config_->GetAxisByIds(axis_ids.c_str());
    if (!axis_config) {
      LOG(ERROR) << "get axis_config error";
      PopupDialogSingleton::GetInstance()->HidePage();
      return;
    }
    yotta::Axis* axis_motion = limit_motion->GetAxisByIds(axis_ids.c_str());
    if (!axis_motion) {
      LOG(ERROR) << "get axis_motion error";
      PopupDialogSingleton::GetInstance()->HidePage();
      return;
    }
    yotta::AccDecProfileImpl acc_dec_profile(0);
    // yotta::AxisSpeedConfigPtr speed = axis_config->GetSpeed(0);
    // if (!speed) {
    //  LOG(ERROR) << "get axis speed error";
    //  return;
    //}
    // double val = 0;
    // speed->GetAcc(&val);
    // acc_dec_profile.set_acceleration(val);
    // val = 0;
    // speed->GetDec(&val);
    // acc_dec_profile.set_deceleration(val);
    // val = 0;
    // speed->GetVelocity(&val);
    // acc_dec_profile.set_velocity(val);

    std::string speed_ids = "单轴高速";
    yotta::AxisSpeedConfigPtr speed = nullptr;
    if (speed_ids.empty()) {
      speed = axis_config->GetDefaultSpeed();
    } else {
      speed = axis_config->GetSpeedByIds(speed_ids.c_str());
    }
    if (!speed) {
      LOG(ERROR) << "speed_ids: " << speed_ids << " not found";
      PopupDialogSingleton::GetInstance()->HidePage();
      return;
    }

    double acc_val = 0.0;
    double dec_val = 0.0;
    double vel_val = 0.0;
    speed->GetAcc(&acc_val);
    speed->GetDec(&dec_val);
    speed->GetVelocity(&vel_val);
    acc_dec_profile.set_acceleration(acc_val);
    acc_dec_profile.set_deceleration(dec_val);
    acc_dec_profile.set_velocity(vel_val);

    int ret = axis_motion->AsyncMoveTo(dis_position, &acc_dec_profile);
    if (ret != 0) {
      LOG(ERROR) << "axis[" << axis_ids
                 << "] async move to pos failed, ret=" << ret;
      PopupDialogSingleton::GetInstance()->HidePage();
      return;
    }

    moving_axes.push_back(axis_motion);
  }

  for (yotta::Axis* axis_motion : moving_axes) {
    if (!axis_motion) {
      continue;
    }

    int ret = axis_motion->Wait();
    if (ret != 0) {
      LOG(ERROR) << "axis wait failed, ret=" << ret;
      PopupDialogSingleton::GetInstance()->HidePage();
      return;
    }
  }

  PopupDialogSingleton::GetInstance()->PopupOperationStatusAppend(step_name, 1);
  PopupDialogSingleton::GetInstance()->HidePage();
}

void PointSettingView::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);  // 调用基类实现

  // 确保每次显示该页面时，都切换到 page_image
  if (ui && ui->stackedWidget && ui->page_image) {
    MultiImageShowViewSingleton::GetInstance()->ShowEmbedded(
        ui->image_show_layout);
    ui->stackedWidget->setCurrentWidget(ui->page_image);
    point_page_ = false;  // 同步状态标志，避免 SwitchPage 首次双击
  }
  SetLockedState(true);
}
