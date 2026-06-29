// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/05/07 14:46

#include "robot_manual.h"

#include <limit_motion/impl/acc_dec_profile_impl.h>
#include <limit_motion/limit_motion_mgr.h>

#include "config/config_factory.h"
#include "controller/log_manager/log_view_sink.h"
#include "main_process/module_mgr.h"

RobotManual::RobotManual(QWidget* parent) : QDialog(parent) {
  // yotta::ConfigFactory::GetInstance()->Init(0);
  axis_config_ = yotta::ConfigFactory::GetInstance()->GetAxisConfig();
}

RobotManual::~RobotManual() {}

void RobotManual::SetIndexStep(const std::string& axis_ids,
                               double index_step) {
  index_steps_[axis_ids] = index_step;
}

void RobotManual::OnMovePropertyChanged(MoveProperty property) {
  move_property_ = property;
}

void RobotManual::OnButtonClicked(int device_index, std::string axis_ids,
                                  int direction) {
  Q_UNUSED(device_index);

  yotta::LimitMotionMgrPtr limit_motion =
      main_process::GetModuleMgr()->GetLimitMotionMgr();
  if (!limit_motion) {
    LOG(ERROR) << "get limit_motion error";
    return;
  }
  yotta::AxisConfigItemPtr axis_config_item =
      axis_config_->GetAxisByIds(axis_ids.c_str());
  if (!axis_config_item) {
    LOG(ERROR) << "get axis_config_item error" << axis_ids;
    return;
  }
  yotta::Axis* axis_motion = limit_motion->GetAxisByIds(axis_ids.c_str());
  if (!axis_motion) {
    LOG(ERROR) << "get axis_motion error";
    return;
  }
  yotta::AccDecProfileImpl acc_dec_profile(0);
  std::string speed_ids = "单轴高速";
  yotta::AxisSpeedConfigPtr speed = nullptr;
  if (speed_ids.empty()) {
    speed = axis_config_item->GetDefaultSpeed();
  } else {
    speed = axis_config_item->GetSpeedByIds(speed_ids.c_str());
  }
  if (!speed) {
    LOG(ERROR) << "speed_ids: " << speed_ids << " not found";
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

  double multiplier = 0.001;
  if (direction) {
    multiplier = -0.001;
  }
  // SACN
  if (move_property_.move_model == 1) {
    double current_position = 0;
    axis_motion->GetActualPosition(&current_position);
    axis_motion->AsyncMoveTo(
        current_position + move_property_.step * multiplier, &acc_dec_profile);
  }
  // INDEX
  else if (move_property_.move_model == 2) {
    double current_position = 0;
    axis_motion->GetActualPosition(&current_position);
    auto iter = index_steps_.find(axis_ids);
    if (iter == index_steps_.end()) {
      LOG(ERROR) << "index step not set, axis_ids: " << axis_ids;
      return;
    }
    multiplier *= 1000;
    axis_motion->AsyncMoveTo(current_position + iter->second * multiplier,
                             &acc_dec_profile);
  }
}

void RobotManual::OnButtonLongPressed(int device_index, std::string axis_ids,
                                      int direction) {
  Q_UNUSED(device_index);

  yotta::LimitMotionMgrPtr limit_motion =
      main_process::GetModuleMgr()->GetLimitMotionMgr();
  if (!limit_motion) {
    LOG(ERROR) << "get limit_motion error";
    return;
  }
  yotta::AxisConfigItemPtr axis_config_item =
      axis_config_->GetAxisByIds(axis_ids.c_str());
  if (!axis_config_item) {
    LOG(ERROR) << "get axis_config_item error : axis_ids = " << axis_ids;
    return;
  }
  yotta::Axis* axis_motion = limit_motion->GetAxisByIds(axis_ids.c_str());
  if (!axis_motion) {
    LOG(ERROR) << "get axis_motion error";
    return;
  }
  yotta::AccDecProfileImpl acc_dec_profile(0);
  // yotta::AxisSpeedConfigPtr speed = axis_config_item->GetSpeed(0);
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

  std::string speed_ids = "单轴低速";
  yotta::AxisSpeedConfigPtr speed = nullptr;
  if (speed_ids.empty()) {
    speed = axis_config_item->GetDefaultSpeed();
  } else {
    speed = axis_config_item->GetSpeedByIds(speed_ids.c_str());
  }
  if (!speed) {
    LOG(ERROR) << "speed_ids: " << speed_ids << " not found";
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

  // JOG
  if (move_property_.move_model == 0) {
    axis_motion->StartJog(&acc_dec_profile, direction);
  }
}

void RobotManual::OnButtonLongPressReleased(int device_index,
                                            std::string axis_ids,
                                            int direction) {
  Q_UNUSED(device_index);
  Q_UNUSED(direction);

  yotta::LimitMotionMgrPtr limit_motion =
      main_process::GetModuleMgr()->GetLimitMotionMgr();
  if (!limit_motion) {
    LOG(ERROR) << "get limit_motion error";
    return;
  }
  yotta::AxisConfigItemPtr axis_config =
      axis_config_->GetAxisByIds(axis_ids.c_str());
  if (!axis_config) {
    LOG(ERROR) << "get axis_config error";
    return;
  }
  yotta::Axis* axis_motion = limit_motion->GetAxisByIds(axis_ids.c_str());
  if (!axis_motion) {
    LOG(ERROR) << "get axis_motion error";
    return;
  }

  axis_motion->Stop();
}
