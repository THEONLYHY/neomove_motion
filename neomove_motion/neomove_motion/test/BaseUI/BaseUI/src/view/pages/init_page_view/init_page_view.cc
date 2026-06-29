// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/18 11:05

#include "init_page_view.h"

#include <common/message_loop.h>
#include <limit_motion/impl/acc_dec_profile_impl.h>
#include <limit_motion/limit_motion_mgr.h>

#include <QLayout>
#include <QLayoutItem>
#include <QPushButton>

#include "config/config_factory.h"
#include "controller/device_status_monitor/device_status_monitor.h"
#include "controller/motion_control/motion_control.h"
#include "controller/real_time_data/real_time_data.h"
#include "controller/task/task_callback_impl.h"
#include "main_process/module_mgr.h"
#include "model/error_define.h"
#include "model/model_mgr.h"
#include "model/unit_info/unit_info_mgr.h"
#include "view/components/ps_button/ps_button.h"
#include "view/components/ps_label/ps_label.h"
#include "view/tools/popup_dialog/popup_dialog.h"

using namespace main_process;

InitPageView::InitPageView(QWidget *parent)
    : QWidget(parent), ui(new Ui::init_page_viewClass()) {
  ui->setupUi(this);

  axis_config_ = yotta::ConfigFactory::GetInstance()->GetAxisConfig();
  if (!axis_config_) {
    return;
  }
  UnitInfoMgrPtr motion_module =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!motion_module) {
    return;
  }
  //工位列表
  bool first_check = true;  //选中第一个工位
  for (int i = 0; i < motion_module->GetUnitCount(); i++) {
    PsButton *axis_btn = new PsButton(
        QString::fromStdString(motion_module->GetUnitInfo(i)->unit_ids));
    axis_btn->setFixedSize(160, 69);
    axis_btn->setCheckable(true);
    if (first_check) {
      axis_btn->click();
      module_id_ = motion_module->GetUnitInfo(i)->unit_ids;
      first_check = false;
    }
    ui->module_list_layout->addWidget(axis_btn);
    btn_group_module_.addButton(axis_btn, i);
  }

  // 定时器实时监测io状态
  time_monitor_ = new QTimer(this);
  connect(time_monitor_, &QTimer::timeout, this, &InitPageView::UpdateTimer);
  // Don't start timer here - it will be started in onPageShow()

  ui->module_list_layout->addStretch();
  for (QAbstractButton *button : btn_group_module_.buttons()) {
    connect(button, &QAbstractButton::clicked, this,
            [this, button](bool checked) {
              if (!checked) {
                return;
              }
              int id = btn_group_module_.id(button);
              OnUnitClicked(id);
            });
  }
  if (motion_module->GetUnitCount()) {
    OnUnitClicked(0);
  }

  connect(ui->button_equipment_init, &QPushButton::clicked, this,
          &InitPageView::OnEquipmentInitClicked);
  connect(this, &InitPageView::SigHomeResult, this,
          &InitPageView::OnHomeResult);
  connect(this, &InitPageView::SigServoOnResult, this,
          &InitPageView::OnServoOnResult);
  connect(this, &InitPageView::SigClearWarningResult, this,
          &InitPageView::OnClearWarningResult);
  connect(ui->button_serve_on_off, &QPushButton::clicked, this,
          &InitPageView::OnButtonClickedServoOnOff);
  connect(ui->button_clear_all_alarm, &QPushButton::clicked, this,
          &InitPageView::OnButtonClickedClearAllAlarm);

  ApplyUserLevelVisibility();
}

InitPageView::~InitPageView() {
  if (time_monitor_ && time_monitor_->isActive()) {
    time_monitor_->stop();
  }
  ClearWatchers();
  delete ui;
}

void InitPageView::showEvent(QShowEvent *event) {
  ApplyUserLevelVisibility();
  if (time_monitor_) {
    time_monitor_->start(500);  // Start timer when page is shown
    LOG(INFO) << "InitPageView: Timer started";
  }
}

void InitPageView::hideEvent(QHideEvent *event) {
  if (time_monitor_ && time_monitor_->isActive()) {
    time_monitor_->stop();  // Stop timer when page is hidden
    LOG(INFO) << "InitPageView: Timer stopped";
  }
}

void InitPageView::OnUnitClicked(int module_index) {
  UnitInfoMgrPtr motion_module =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!motion_module) {
    return;
  }
  if (module_index < motion_module->GetUnitCount()) {
    module_id_ = motion_module->GetUnitInfo(module_index)->unit_ids;
    time_monitor_->stop();
    // ClearWatchers();
    ShowUnit();
    time_monitor_->start(500);
  }
}

void InitPageView::ClearLayout(QLayout *layout) {
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
  home_buttons_.clear();
  servo_on_buttons_.clear();
  clear_warning_buttons_.clear();
}

void InitPageView::ClearWatchers() {
  UnitInfoMgrPtr unit_info_mgr =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (unit_info_mgr) {
    UnitInfoPtr unit_info = unit_info_mgr->GetUnitInfo(module_id_);
    yotta::LimitMotionMgrPtr limit_motion =
        main_process::GetModuleMgr()->GetLimitMotionMgr();
    if (limit_motion) {
      for (size_t i = 0; i < axis_watchers_.size(); ++i) {
        AxisAlarmWatcher *watcher = axis_watchers_[i];
        if (watcher && i < unit_info->axis_ids.size()) {
          std::string axis_ids = unit_info->axis_ids[i];
          yotta::LimitAxis *axis_motion =
              limit_motion->GetAxisByIds(axis_ids.c_str());
          if (axis_motion) {
            // 注销监听器
            axis_motion->SetAxisWatcher(nullptr);
          }
        }
      }
    }
  }

  // 释放监听器对象
  for (AxisAlarmWatcher *watcher : axis_watchers_) {
    if (watcher) {
      delete watcher;
    }
  }
  axis_watchers_.clear();
}

void InitPageView::ApplyUserLevelVisibility() {
  const bool can_show_axis_controls =
      RealTimeDataSinglton::GetInstance()->current_user_level() >= kL3SU1;

  ui->frame->setVisible(can_show_axis_controls);
  ui->frame_2->setVisible(can_show_axis_controls);

  ui->button_serve_on_off->setVisible(can_show_axis_controls);
  ui->button_clear_all_alarm->setVisible(can_show_axis_controls);
}

void InitPageView::ShowUnit() {
  ClearLayout(ui->axis_home_layout);
  UnitInfoMgrPtr unit_info_mgr =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!unit_info_mgr) {
    return;
  }
  UnitInfoPtr module_axis_info = unit_info_mgr->GetUnitInfo(module_id_);
  // Initialize cache vectors for this unit
  cached_home_states_.clear();
  cached_servo_states_.clear();
  cached_home_states_.resize(module_axis_info->axis_ids.size(),
                             -1);  // -1 means uninitialized
  cached_servo_states_.resize(module_axis_info->axis_ids.size(), -1);
  alarmed_axes_.clear();

  bool first_check = true;  //选中第一个轴
  for (int i = 0; i < module_axis_info->axis_ids.size(); i++) {
    QHBoxLayout *axis_layout = new QHBoxLayout();
    axis_layout->setContentsMargins(0, 0, 0, 0);
    axis_layout->setSpacing(6);
    // 标签
    PsLabel *axis_label =
        new PsLabel(QString::fromStdString(module_axis_info->axis_ids[i]));
    axis_label->setFixedSize(160, 69);

    // 按钮 归零
    PsButton *axis_btn_1 = new PsButton(tr("Home"));
    axis_btn_1->setFixedSize(89, 69);
    axis_btn_1->Color().SetBaseColor(PsColor::Color::kBlue);
    connect(axis_btn_1, &QPushButton::clicked, this,
            [this, i] { OnButtonClickedHome(i); });
    home_buttons_.push_back(axis_btn_1);

    // 按钮二 清除轴警告
    PsButton *axis_btn_2 = new PsButton(tr("Clear\nAlarm"));
    axis_btn_2->setFixedSize(89, 69);
    axis_btn_2->Color().SetBaseColor(PsColor::Color::kBlue);
    connect(axis_btn_2, &QPushButton::clicked, this,
            [this, i] { OnButtonClickedClearWarning(i); });
    clear_warning_buttons_.push_back(axis_btn_2);

    // 按钮三 上使能
    PsButton *axis_btn_3 = new PsButton(tr("Servo\nOn"));
    axis_btn_3->setFixedSize(89, 69);
    axis_btn_3->Color().SetBaseColor(PsColor::Color::kBlue);
    connect(axis_btn_3, &QPushButton::clicked, this,
            [this, i] { OnButtonClickedServoOn(i); });
    servo_on_buttons_.push_back(axis_btn_3);

    // 按钮四 负限位
    PsButton *axis_btn_4 = new PsButton(tr("Limit-"));
    axis_btn_4->setFixedSize(89, 69);
    axis_btn_4->Color().SetBaseColor(PsColor::Color::kBlue);
    connect(axis_btn_4, &QPushButton::clicked, this,
            [this, i] { OnButtonClickedLimitMin(i); });
    // 按钮五 正限位
    PsButton *axis_btn_5 = new PsButton(tr("Limit+"));
    axis_btn_5->setFixedSize(89, 69);
    axis_btn_5->Color().SetBaseColor(PsColor::Color::kBlue);
    connect(axis_btn_5, &QPushButton::clicked, this,
            [this, i] { OnButtonClickedLimitMax(i); });

    // 加入布局
    axis_layout->addWidget(axis_label);
    axis_layout->addWidget(axis_btn_1);
    axis_layout->addWidget(axis_btn_2);
    axis_layout->addWidget(axis_btn_3);
    axis_layout->addWidget(axis_btn_4);
    axis_layout->addWidget(axis_btn_5);
    axis_layout->addStretch();

    ui->axis_home_layout->addLayout(axis_layout);
    // 创建并设置报警监听器
    yotta::LimitMotionMgrPtr limit_motion = GetModuleMgr()->GetLimitMotionMgr();
    if (limit_motion) {
      yotta::LimitAxis *axis_motion =
          limit_motion->GetAxisByIds(module_axis_info->axis_ids[i].c_str());
      if (axis_motion) {
        AxisAlarmWatcher *watcher = new AxisAlarmWatcher(i, this);
        connect(watcher, &AxisAlarmWatcher::SigAlarmTriggered, this,
                &InitPageView::OnAxisAlarm, Qt::QueuedConnection);
        axis_motion->SetAxisWatcher(watcher);
        axis_watchers_.push_back(watcher);
      }
    }
  }
  ui->axis_home_layout->addStretch();
}

void InitPageView::OnButtonClickedHome(int index) {
  if (RealTimeDataSinglton::GetInstance()->current_user_level() < kL2EG) {
    return;
  }
  axis_index_ = index;
  common::MessageLoop::GetMessageLoop(common::kMotion)
      ->PostTask(std::bind(&InitPageView::AxisHome, this));
}
void InitPageView::OnButtonClickedClearWarning(int index) {
  if (RealTimeDataSinglton::GetInstance()->current_user_level() < kL2EG) {
    return;
  }
  axis_index_ = index;
  common::MessageLoop::GetMessageLoop(common::kMotion)
      ->PostTask(std::bind(&InitPageView::AxisClearWarning, this));
}
void InitPageView::OnButtonClickedServoOn(int index) {
  if (RealTimeDataSinglton::GetInstance()->current_user_level() < kL2EG) {
    return;
  }
  axis_index_ = index;
  common::MessageLoop::GetMessageLoop(common::kMotion)
      ->PostTask(std::bind(&InitPageView::AxisServoOn, this));
}

void InitPageView::OnButtonClickedLimitMin(int index) {
  if (RealTimeDataSinglton::GetInstance()->current_user_level() < kL2EG) {
    return;
  }
  axis_index_ = index;
  UnitInfoMgrPtr unit_info_mgr =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!unit_info_mgr) {
    return;
  }
  UnitInfoPtr unit_info = unit_info_mgr->GetUnitInfo(module_id_);
  std::string axis_ids = unit_info->axis_ids[axis_index_];
  yotta::LimitMotionMgrPtr limit_motion =
      main_process::GetModuleMgr()->GetLimitMotionMgr();
  if (!limit_motion) {
    LOG(ERROR) << "get limit_motion error";
    return;
  }

  yotta::Axis *axis_motion = limit_motion->GetAxisByIds(axis_ids.c_str());
  if (!axis_motion) {
    LOG(ERROR) << "get axis_motion error";
    return;
  }
  double position = 0;
  axis_motion->GetActualPosition(&position);

  yotta::AxisConfigItemPtr axis_config_item =
      axis_config_->GetAxisByIds(axis_ids.c_str());
  if (!axis_config_item) {
    LOG(ERROR) << "get axis_config_item error";
    return;
  }
  yotta::AxisAttributeConfigPtr axis_config_item_attribute =
      axis_config_item->GetAttribute();
  if (!axis_config_item_attribute) {
    LOG(ERROR) << "get axis_config_item_attribute error";
    return;
  }
  axis_config_item_attribute->SetLimitNegative(position);
  axis_config_->Save();
}
void InitPageView::OnButtonClickedLimitMax(int index) {
  if (RealTimeDataSinglton::GetInstance()->current_user_level() < kL2EG) {
    return;
  }
  axis_index_ = index;
  UnitInfoMgrPtr unit_info_mgr =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!unit_info_mgr) {
    return;
  }
  UnitInfoPtr unit_info = unit_info_mgr->GetUnitInfo(module_id_);
  std::string axis_ids = unit_info->axis_ids[axis_index_];
  yotta::LimitMotionMgrPtr limit_motion =
      main_process::GetModuleMgr()->GetLimitMotionMgr();
  if (!limit_motion) {
    LOG(ERROR) << "get limit_motion error";
    return;
  }

  yotta::Axis *axis_motion = limit_motion->GetAxisByIds(axis_ids.c_str());
  if (!axis_motion) {
    LOG(ERROR) << "get axis_motion error";
    return;
  }
  double position = 0;
  axis_motion->GetActualPosition(&position);

  yotta::AxisConfigItemPtr axis_config_item =
      axis_config_->GetAxisByIds(axis_ids.c_str());
  if (!axis_config_item) {
    LOG(ERROR) << "get axis_config_item error";
    return;
  }
  yotta::AxisAttributeConfigPtr axis_config_item_attribute =
      axis_config_item->GetAttribute();
  if (!axis_config_item_attribute) {
    LOG(ERROR) << "get axis_config_item_attribute error";
    return;
  }
  axis_config_item_attribute->SetLimitPositive(position);
  axis_config_->Save();
}

void InitPageView::AxisHome() {
  UnitInfoMgrPtr unit_info_mgr =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!unit_info_mgr) {
    return;
  }
  UnitInfoPtr unit_info = unit_info_mgr->GetUnitInfo(module_id_);
  std::string axis_ids = unit_info->axis_ids[axis_index_];

  yotta::LimitMotionMgrPtr limit_motion =
      main_process::GetModuleMgr()->GetLimitMotionMgr();
  if (!limit_motion) {
    LOG(ERROR) << "get limit_motion error";
    return;
  }

  yotta::Axis *axis_motion = limit_motion->GetAxisByIds(axis_ids.c_str());
  if (!axis_motion) {
    LOG(ERROR) << "get axis_motion error";
    return;
  }
  int result = axis_motion->Home();
  // emit SigHomeResult(axis_index_, result);
}
void InitPageView::AxisClearWarning() {
  UnitInfoMgrPtr motion_module =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!motion_module) {
    return;
  }
  UnitInfoPtr module_axis_info = motion_module->GetUnitInfo(module_id_);
  std::string axis_ids = module_axis_info->axis_ids[axis_index_];

  yotta::LimitMotionMgrPtr limit_motion =
      main_process::GetModuleMgr()->GetLimitMotionMgr();
  if (!limit_motion) {
    LOG(ERROR) << "get limit_motion error";
    return;
  }

  yotta::Axis *axis_motion = limit_motion->GetAxisByIds(axis_ids.c_str());
  if (!axis_motion) {
    LOG(ERROR) << "get axis_motion error";
    return;
  }
  int result = axis_motion->ClearAmpAlarm();
  emit SigClearWarningResult(axis_index_, result);
}
void InitPageView::AxisServoOn() {
  UnitInfoMgrPtr motion_module =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!motion_module) {
    return;
  }
  UnitInfoPtr module_axis_info = motion_module->GetUnitInfo(module_id_);
  std::string axis_ids = module_axis_info->axis_ids[axis_index_];

  yotta::LimitMotionMgrPtr limit_motion =
      main_process::GetModuleMgr()->GetLimitMotionMgr();
  if (!limit_motion) {
    LOG(ERROR) << "get limit_motion error";
    return;
  }

  yotta::Axis *axis_motion = limit_motion->GetAxisByIds(axis_ids.c_str());
  if (!axis_motion) {
    LOG(ERROR) << "get axis_motion error";
    return;
  }
  int result = axis_motion->SetServoOn();
  // emit SigServoOnResult(axis_index_, result);
}

void InitPageView::OnEquipmentInitClicked() {
  PopupDialogSingleton::GetInstance()->ShowPage();
  PopupDialogSingleton::GetInstance()->PopupOperationStatus(
      tr("设备开始初始化......"), {tr("轴上使能"), tr("轴初始化")}, 600);
  PopupDialogSingleton::GetInstance()->PopupOperationStatusAppend("轴上使能",
                                                                  2);
  yotta::AxisConfigPtr axis_config =
      yotta::ConfigFactory::GetInstance()->GetAxisConfig();
  for (int i = 0; i < axis_config->GetAxisCount(); i++) {
    yotta::AxisConfigItemPtr axis_config_item = axis_config->GetAxis(i);

    char char_ids[256];
    axis_config_item->GetIds(char_ids, 256);

    std::string axis_ids = char_ids;
    yotta::LimitMotionMgrPtr limit_motion =
        main_process::GetModuleMgr()->GetLimitMotionMgr();
    if (!limit_motion) {
      LOG(ERROR) << "get limit_motion error";
      return;
    }

    yotta::Axis *axis_motion = limit_motion->GetAxisByIds(axis_ids.c_str());
    if (!axis_motion) {
      LOG(ERROR) << "get axis_motion error";
      return;
    }
    int result = axis_motion->ClearAmpAlarm();
    result += axis_motion->SetServoOn();
    if (result) {
      LOG(ERROR) << "轴初始化失败!请重试";
      return;
    }
  }
  PopupDialogSingleton::GetInstance()->PopupOperationStatusAppend("轴上使能",
                                                                  1);

  common::MessageLoop::GetMessageLoop(common::kMotion)->PostTask([this] {
    PopupDialogSingleton::GetInstance()->PopupOperationStatusAppend("轴初始化",
                                                                    2);
    const int result = MotionControlSinglton::GetInstance()->DoTaskSync(
        "轴初始化");
    if (result == kMotionOk) {
      PopupDialogSingleton::GetInstance()->PopupOperationStatusAppend(
          "轴初始化", 1);
      emit SigInitEnd();
    } else {
      PopupDialogSingleton::GetInstance()->PopupError(
          result, tr("轴初始化错误"), QString());
    }
  });
}

void InitPageView::OnEquipmentInitEndClicked() { emit SigInitEnd(); }

void InitPageView::UpdateTimer() {
  UnitInfoMgrPtr motion_module =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!motion_module) {
    return;
  }

  DeviceStatusMonitor *monitor = DeviceStatusMonitorSinglton::GetInstance();
  UnitInfoPtr module_axis_info = motion_module->GetUnitInfo(module_id_);

  bool all_servo_on = true;
  bool any_servo_on = false;
  for (int i = 0; i < module_axis_info->axis_ids.size(); i++) {
    // 跳过无效索引
    if (i >= servo_on_buttons_.size()) continue;
    if (i >= cached_home_states_.size() || i >= cached_servo_states_.size())
      continue;

    const std::string &axis_ids = module_axis_info->axis_ids[i];

    // 获取轴状态
    int axis_state_int = 0;
    if (!monitor->GetAxisState(axis_ids, axis_state_int)) continue;
    yotta::Axis::AxisState axis_state =
        static_cast<yotta::Axis::AxisState>(axis_state_int);

    int home_state_int = 0;
    if (!monitor->GetAxisHomeState(axis_ids, home_state_int)) continue;
    yotta::Axis::AxisHomeState home_state =
        static_cast<yotta::Axis::AxisHomeState>(home_state_int);

    if (axis_state == yotta::Axis::AxisState::kServoOn) {
      any_servo_on = true;
    } else {
      all_servo_on = false;
    }

    // 只在状态改变时更新UI
    if (cached_home_states_[i] != home_state_int) {
      home_buttons_[i]->Color().SetBaseColor(
          (home_state == yotta::Axis::AxisHomeState::kHomeOn)
              ? PsColor::Color::kBlue
              : PsColor::Color::kRed);
      cached_home_states_[i] = home_state_int;
    }

    if (cached_servo_states_[i] != axis_state_int) {
      servo_on_buttons_[i]->Color().SetBaseColor(
          (axis_state == yotta::Axis::AxisState::kServoOn)
              ? PsColor::Color::kBlue
              : PsColor::Color::kRed);
      cached_servo_states_[i] = axis_state_int;
    }
  }

  // 全使能按钮颜色：全部使能蓝 / 部分使能黄 / 全部未使能红
  int axis_count = static_cast<int>(module_axis_info->axis_ids.size());
  if (all_servo_on) {
    ui->button_serve_on_off->Color().SetBaseColor(PsColor::Color::kBlue);
  } else if (any_servo_on) {
    ui->button_serve_on_off->Color().SetBaseColor(PsColor::Color::kYellow);
  } else {
    ui->button_serve_on_off->Color().SetBaseColor(PsColor::Color::kRed);
  }

  // 清除所有报警按钮颜色：无报警蓝 / 部分报警黄 / 全部报警红
  int alarmed_count = alarmed_axes_.size();
  if (alarmed_count == 0) {
    ui->button_clear_all_alarm->Color().SetBaseColor(PsColor::Color::kBlue);
  } else if (alarmed_count < axis_count) {
    ui->button_clear_all_alarm->Color().SetBaseColor(PsColor::Color::kYellow);
  } else {
    ui->button_clear_all_alarm->Color().SetBaseColor(PsColor::Color::kRed);
  }
}

void InitPageView::OnHomeResult(int index, int result) {
  if (index < 0 || index >= home_buttons_.size()) {
    return;
  }

  PsButton *btn = home_buttons_[index];
  if (!btn) {
    return;
  }

  btn->Color().SetBaseColor((result == 0) ? PsColor::Color::kBlue
                                          : PsColor::Color::kRed);
}

void InitPageView::OnServoOnResult(int index, int result) {
  if (index < 0 || index >= servo_on_buttons_.size()) return;

  PsButton *btn = servo_on_buttons_[index];
  if (!btn) return;

  btn->Color().SetBaseColor((result == 0) ? PsColor::Color::kBlue
                                          : PsColor::Color::kRed);
}

void InitPageView::OnAxisAlarm(int index) {
  if (index < 0 || index >= clear_warning_buttons_.size()) return;
  PsButton *btn = clear_warning_buttons_[index];
  if (!btn) return;
  // 报警时将清除报警按钮置为红色，并记录报警轴
  btn->Color().SetBaseColor(PsColor::Color::kRed);
  alarmed_axes_.insert(index);
}

void InitPageView::OnClearWarningResult(int index, int result) {
  if (index < 0 || index >= clear_warning_buttons_.size()) return;

  PsButton *btn = clear_warning_buttons_[index];
  if (!btn) return;

  btn->Color().SetBaseColor((result == 0) ? PsColor::Color::kBlue
                                          : PsColor::Color::kRed);
  if (result == 0) {
    alarmed_axes_.remove(index);
  }
}

void InitPageView::OnButtonClickedServoOnOff() {
  yotta::LimitMotionMgrPtr limit_motion = GetModuleMgr()->GetLimitMotionMgr();
  if (!limit_motion || !axis_config_) {
    LOG(ERROR) << "Failed to get limit motion manager or axis config";
    return;
  }

  // 获取所有轴的ID列表
  std::vector<std::string> all_axis_ids;
  for (int i = 0; i < axis_config_->GetAxisCount(); ++i) {
    yotta::AxisConfigItemPtr axis_item = axis_config_->GetAxis(i);
    if (axis_item) {
      char axis_id[256] = {0};
      axis_item->GetIds(axis_id, sizeof(axis_id));
      all_axis_ids.emplace_back(axis_id);
    }
  }

  // 检查当前是否所有轴都已使能
  bool all_enabled = true;
  for (const std::string &axis_id : all_axis_ids) {
    yotta::Axis *axis = limit_motion->GetAxisByIds(axis_id.c_str());
    if (axis) {
      yotta::Axis::AxisState servo_state = yotta::Axis::AxisState::kServoOff;
      axis->state(&servo_state);
      if (servo_state == yotta::Axis::AxisState::kServoOff) {
        all_enabled = false;
        break;
      }
    }
  }

  // 使能/关使能
  for (const std::string &axis_id : all_axis_ids) {
    yotta::Axis *axis = limit_motion->GetAxisByIds(axis_id.c_str());
    if (axis) {
      if (all_enabled) {
        axis->SetServoOff();
      } else {
        axis->SetServoOn();
      }
    }
  }
}

void InitPageView::OnButtonClickedClearAllAlarm() {
  UnitInfoMgrPtr unit_info_mgr =
      ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!unit_info_mgr) return;

  UnitInfoPtr unit_info = unit_info_mgr->GetUnitInfo(module_id_);
  yotta::LimitMotionMgrPtr limit_motion = GetModuleMgr()->GetLimitMotionMgr();
  if (!limit_motion) {
    LOG(ERROR) << "Failed to get limit motion manager";
    return;
  }

  // 清除当前工位所有轴的报警
  for (int i = 0; i < unit_info->axis_ids.size(); ++i) {
    yotta::LimitAxis *axis =
        limit_motion->GetAxisByIds(unit_info->axis_ids[i].c_str());
    if (axis) {
      int result = axis->ClearAmpAlarm();
      emit SigClearWarningResult(i, result);
    }
  }
}

