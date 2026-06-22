// copyright 2025 YottaImage. All rights reserved.
#include "neomove_axis.h"

#include <glog/glog_helper.h>

#include <cmath>
#include <limits>
#include <thread>

#include "neomove_io_monitor_thread.h"
#include "neomove_motion_mgr_context_impl.h"
#include "pitch_compensation_config/pitch_compensation_config.h"

using namespace yotta;

namespace {

double NormalizeReadbackMultiplier(double multiplier) {
  if (fabs(multiplier) < std::numeric_limits<double>::epsilon()) {
    return 1.0;
  }
  return multiplier;
}

}  // namespace

NeoMoveAxis::NeoMoveAxis(int axis_index) : axis_index_(axis_index) {
  LOG(INFO) << "NeoMoveAxis constructed, axis_index=" << axis_index_;
}

int NeoMoveAxis::GetControllerIndex() {
  return NeoMoveMotionMgrContextImpl::GetCurrent().controller_index();
}

double NeoMoveAxis::GetMultiplier() {
  return NeoMoveMotionMgrContextImpl::GetCurrent().GetAxisMultiplier();
}

int YOTTA_API_CALL NeoMoveAxis::id() {
  LOG(INFO) << "NeoMoveAxis::id called, asix_index = " << axis_index_;
  return axis_index_;
}

int YOTTA_API_CALL NeoMoveAxis::name(char* name_buf, size_t name_buf_size) {
  LOG(INFO) << "NeoMoveAxis::name called, axis_index=" << axis_index_;
  snprintf(name_buf, name_buf_size, "NeoMove-Axis-%d", axis_index_);
  return 0;
}

// 伺服状态
int YOTTA_API_CALL NeoMoveAxis::state(AxisState* state) {
  ClearError();
  if (!state) {
    SetError(MotionErrors::ParamInvalid, "state 参数为空");
    return 1;
  }

  NM_AXISSTATUS axis_status{};
  int ret = NM_GetAxisStatus(GetControllerIndex(), axis_index_, &axis_status);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "获取轴状态失败",
             "neomove_api=NM_GetAxisStatus, ret=" + ToHex(ret));
    return ret;
  }

  *state = axis_status.servoOn ? AxisState::kServoOn : AxisState::kServoOff;
  return 0;
}

// 回零状态
int YOTTA_API_CALL NeoMoveAxis::home_state(AxisHomeState* state) {
  ClearError();
  if (!state) {
    SetError(MotionErrors::ParamInvalid, "home_state 参数为空");
    return 1;
  }

  NM_AXISSTATUS axis_status{};
  int ret = NM_GetAxisStatus(GetControllerIndex(), axis_index_, &axis_status);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "获取轴状态失败",
             "neomove_api=NM_GetAxisStatus, ret=" + ToHex(ret));
    return ret;
  }

  *state = axis_status.homeDone ? AxisHomeState::kHomeOn : AxisHomeState::kHomeOff;
  return 0;
}

// 运动状态
int YOTTA_API_CALL NeoMoveAxis::operation_state(AxisOperationState* operation_state) {
  ClearError();
  if (!operation_state) {
    SetError(MotionErrors::ParamInvalid, "operation_state 参数为空");
    return 1;
  }

  NM_AXISSTATUS axis_status{};
  int ret = NM_GetAxisStatus(GetControllerIndex(), axis_index_, &axis_status);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "获取轴状态失败",
             "neomove_api=NM_GetAxisStatus, ret=" + ToHex(ret));
    return ret;
  }
  // 所有未完成的运动都归成了kPos，neomove中没有相应opstate
  if (axis_status.homing) {
    *operation_state = AxisOperationState::kHome;
  } else if (!axis_status.motionComplete) {
    *operation_state = AxisOperationState::kPos;
  } else {
    *operation_state = AxisOperationState::kIdle;
  }
  return 0;
}

int YOTTA_API_CALL NeoMoveAxis::SetServoOn() {
  ClearError();
  LOG(INFO) << "NeoMoveAxis::SetServoOn, axis=" << axis_index_;
  int ret = NM_ServoOnOff(GetControllerIndex(), axis_index_, 1);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "SetServoOn 调用失败",
             "neomove_api=NM_ServoOnOff, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAxis::SetServoOff() {
  ClearError();
  LOG(INFO) << "NeoMoveAxis::SetServoOff, axis=" << axis_index_;
  int ret = NM_ServoOnOff(GetControllerIndex(), axis_index_, 0);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "SetServoOff 调用失败",
             "neomove_api=NM_ServoOnOff, ret=" + ToHex(ret));
    return ret;
  }
  Wait();

  NM_AXISSTATUS axis_status{};
  while (true) {
    int status_ret =NM_GetAxisStatus(GetControllerIndex(), axis_index_, &axis_status);
    if (status_ret != NM_RETURN_OK) {
      SetError(MotionErrors::MoveFailed, "获取轴状态失败",
              "neomove_api=NM_GetAxisStatus, ret=" + ToHex(ret));
      return ret;
    }
    if (!axis_status.servoOn) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAxis::ClearAmpAlarm() {
  return ClearAxisAlarm();
}

// 清轴报警
int YOTTA_API_CALL NeoMoveAxis::ClearAxisAlarm() {
  ClearError();
  LOG(INFO) << "NeoMoveAxis::ClearAxisAlarm, axis=" << axis_index_;
  int ret = NM_ClearAlarm(GetControllerIndex(), axis_index_);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "ClearAxisAlarm 调用失败",
             "neomove_api=ClearAxisAlarm, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAxis::SetAxisCommandMode(AxisCommandMode mode) {
  ClearError();
  LOG(INFO) << "NeoMoveAxis::SetAxisCommandMode, axis=" << axis_index_
            << ", mode=" << static_cast<int>(mode);

  // 
  int neoMode = NM_E2M300_AXISMODE_CSP;
  if (mode == AxisCommandMode::kTorque) {
    neoMode = NM_E2M300_AXISMODE_TQ;
  } 

  int ret = NM_SetAxisMode(GetControllerIndex(), axis_index_, neoMode);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "SetAxisCommandMode 调用失败",
             "neomove_api=NM_SetAxisMode, ret=" + ToHex(ret));
    return ret;
  }
  current_mode_ = mode;
  return 0;
}

int YOTTA_API_CALL NeoMoveAxis::GetAxisCommandMode(AxisCommandMode* mode) {
  if (!mode) {
    SetError(MotionErrors::ParamInvalid, "mode 参数为空");
    return -1;
  }

  *mode = current_mode_;
  return 0;
}

int YOTTA_API_CALL NeoMoveAxis::Home() {
  ClearError();
  LOG(INFO) << "NeoMoveAxis::Home, axis=" << axis_index_;
  // 回零前关闭补偿
  DisablePitchCompensation();
  NeoMoveHomeParamConfig home_config =
      NeoMoveMotionMgrContextImpl::GetCurrent().GetHomeParamConfig(axis_index_);

  NM_HOMEPARAM home_param{};
  home_param.homeType = static_cast<unsigned int>(home_config.home_type);
  home_param.homingVelocityFast = home_config.velocity_fast;
  home_param.homingVelocitySlow = home_config.velocity_slow;
  home_param.homingAcc = home_config.acc;
  home_param.homingDec = home_config.dec;

  int ret = NM_AxisHome(GetControllerIndex(), axis_index_, home_param);
  if (ret != NM_RETURN_OK) {
    LOG(ERROR) << "NM_AxisHome failed, ret=" << ret;
    SetError(MotionErrors::HomingFailed, "NM_AxisHome 失败",
             "neomove_api=NM_AxisHome, ret=" + ToHex(ret));
    return ret;
  }

  ret = Wait();
  if (ret != 0) {
    LOG(ERROR) << "Wait failed during homing";
    SetError(MotionErrors::HomingFailed, "Wait 超时");
    return ret;
  }

  NM_SetPosition(GetControllerIndex(), axis_index_, 0.0);
  ApplyPitchCompensation();

  LOG(INFO) << "Homing completed for axis " << axis_index_;
  return 0;
}

int YOTTA_API_CALL NeoMoveAxis::GetActualPosition(double* position) {
  ClearError();
  if (!position) {
    SetError(MotionErrors::ParamInvalid, "position 参数为空");
    return 1;
  }

  NM_AXISSTATUS axis_status{};
  int ret = NM_GetAxisStatus(GetControllerIndex(), axis_index_, &axis_status);
  if (ret == 0) {
    double multiplier = NormalizeReadbackMultiplier(GetMultiplier());
    *position = axis_status.actualPos / multiplier;
  } else {
    SetError(MotionErrors::MoveFailed, "GetActualPosition 调用失败",
             "neomove_api=NM_GetAxisStatus, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAxis::GetActualVelocity(double* velocity) {
  ClearError();
  if (!velocity) {
    SetError(MotionErrors::ParamInvalid, "velocity 参数为空");
    return 1;
  }

  NM_AXISSTATUS axis_status{};
  int ret = NM_GetAxisStatus(GetControllerIndex(), axis_index_, &axis_status);
  if (ret == 0) {
    double multiplier = NormalizeReadbackMultiplier(GetMultiplier());
    *velocity = axis_status.actualVelocity / multiplier;
  } else {
    SetError(MotionErrors::MoveFailed, "GetActualVelocity 调用失败",
             "neomove_api=NM_GetAxisStatus, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAxis::GetTargetPosition(double* target_position) {
  ClearError();
  if (!target_position) {
    SetError(MotionErrors::ParamInvalid, "target_position 参数为空");
    return 1;
  }

  NM_AXISSTATUS axis_status{};
  int ret = NM_GetAxisStatus(GetControllerIndex(), axis_index_, &axis_status);
  if (ret == 0) {
    double multiplier = NormalizeReadbackMultiplier(GetMultiplier());
    *target_position = axis_status.posCmd / multiplier;
  } else {
    SetError(MotionErrors::MoveFailed, "GetTargetPosition 调用失败",
             "neomove_api=NM_GetAxisStatus, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAxis::AsyncMoveTo(double dest_pos,
                                             yotta::AccDecProfile* profile) {
  ClearError();
  if (!profile) {
    SetError(MotionErrors::ParamInvalid, "profile 参数为空");
    return 1;
  }

  double multiplier = GetMultiplier();
  double pulse_dest = dest_pos * multiplier;
  LOG(INFO) << "NeoMoveAxis::AsyncMoveTo, axis=" << axis_index_
            << ", dest=" << dest_pos << ", pulse_dest=" << pulse_dest;

  NM_POSITIONCOMMAND pos_cmd{};
  pos_cmd.axisIndex = static_cast<unsigned int>(axis_index_);
  pos_cmd.target = pulse_dest;
  pos_cmd.velocity = profile->velocity() * multiplier;
  pos_cmd.acc = profile->acceleration() * multiplier;
  pos_cmd.dec = profile->deceleration() * multiplier;
  pos_cmd.smoothTime = profile->moving_average_time_milliseconds();

  int ret = NM_Motion_PositionMode_Abs(GetControllerIndex(), pos_cmd);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "AsyncMoveTo 调用失败",
             "neomove_api=NM_Motion_PositionMode_Abs, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAxis::Wait() {
  ClearError();
  LOG(INFO) << "NeoMoveAxis::Wait, axis=" << axis_index_;

  NM_AXISSTATUS axis_status{};
  int timeout = 60000;
  int elapsed = 0;
  const int poll_interval_ms = 20;

  while (elapsed < timeout) {
    int ret = NM_GetAxisStatus(GetControllerIndex(), axis_index_, &axis_status);
    if (ret != 0) {
      SetError(MotionErrors::MoveFailed, "Wait 获取状态失败",
               "neomove_api=NM_GetAxisStatus, ret=" + ToHex(ret));
      return ret;
    }
    if (axis_status.motionComplete && axis_status.inPos) {
      return 0;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(poll_interval_ms));
    elapsed += poll_interval_ms;
  }

  SetError(MotionErrors::MoveFailed, "Wait 超时");
  return -1;
}

int YOTTA_API_CALL NeoMoveAxis::StartJog(yotta::AccDecProfile* profile,
                                          bool positive) {
  ClearError();
  if (!profile) {
    SetError(MotionErrors::ParamInvalid, "profile 参数为空");
    return 1;
  }

  double multiplier = GetMultiplier();
  LOG(INFO) << "NeoMoveAxis::StartJog, axis=" << axis_index_
            << ", positive=" << positive;

  NM_JOGCOMMAND jog_cmd{};
  jog_cmd.axisIndex = static_cast<unsigned int>(axis_index_);
  jog_cmd.velocity = profile->velocity() * multiplier * (positive ? 1.0 : -1.0);
  jog_cmd.acc = profile->acceleration() * multiplier;
  jog_cmd.dec = profile->deceleration() * multiplier;

  int ret = NM_Motion_JogMode(GetControllerIndex(), jog_cmd);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::JogFailed, "StartJog 调用失败",
             "neomove_api=NM_Motion_JogMode, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAxis::StartTriggerPos(double dest_pos,
                                            yotta::Trigger* trigger,
                                            yotta::AccDecProfile* profile) {
  ClearError();
  LOG(WARNING) << "NeoMoveAxis::StartTriggerPos not supported by NeoMove API";
  SetError(MotionErrors::MoveFailed, "StartTriggerPos not supported",
           "NeoMove SDK has no direct triggered position move API");
  return 1;
}

void NeoMoveAxis::Pause() {
  ClearError();
  LOG(WARNING) << "NeoMoveAxis::Pause not supported by NeoMove API";
  SetError(MotionErrors::MoveFailed, "Pause not supported",
           "NeoMove SDK has no pause/resume axis API");
  return;
}

void NeoMoveAxis::Resume() {
  ClearError();
  LOG(WARNING) << "NeoMoveAxis::Resume not supported by NeoMove API";
  SetError(MotionErrors::MoveFailed, "Resume not supported",
           "NeoMove SDK has no pause/resume axis API");
  return;
}

void NeoMoveAxis::Stop() {
  NM_AxisStop(GetControllerIndex(), axis_index_);
}

void NeoMoveAxis::QuickStop() {
  NM_AxisQuickStop(GetControllerIndex(), axis_index_);
}

void NeoMoveAxis::TimedStop(double time_milliseconds) {
  ClearError();
  LOG(WARNING) << "NeoMoveAxis::TimedStop not supported by NeoMove API";
  SetError(MotionErrors::MoveFailed, "TimedStop not supported",
           "NeoMove SDK has no timed stop axis API");
  return;
}

void NeoMoveAxis::DecelerationStop(double deceleration) {
  ClearError();
  LOG(WARNING) << "NeoMoveAxis::DecelerationStop not supported by NeoMove API";
  SetError(MotionErrors::MoveFailed, "DecelerationStop not supported",
           "NeoMove SDK has no deceleration stop axis API");
  return;
}

void* NeoMoveAxis::QueryInterface(const char* interface_name, size_t length) {
  return nullptr;
}

int YOTTA_API_CALL NeoMoveAxis::SetAxisWatcher(Watcher* watcher) {
  axis_watcher_ = watcher;
  if (axis_watcher_) {
    if (auto monitor = io_monitor_thread_.lock()) {
      monitor->RegisterAxis(axis_index_, axis_watcher_, this);
    }
  }
  return 0;
}

void NeoMoveAxis::SetIoMonitorThread(
    std::weak_ptr<NeoMoveIoMonitorThread> monitor) {
  io_monitor_thread_ = monitor;
}

void NeoMoveAxis::DisablePitchCompensation() {
  NM_EnableCompensation(GetControllerIndex(), axis_index_, 0);
}

void NeoMoveAxis::ApplyPitchCompensation() {
  PitchCompensationConfig config(axis_index_);
  if (!config.Init()) {
    LOG(INFO) << "Failed to init pitch compensation config for axis "
              << axis_index_;
    return;
  }

  if (!config.is_enabled()) {
    LOG(INFO) << "Pitch compensation disabled for axis " << axis_index_;
    return;
  }

  std::string type = config.get_type();
  if (type == "1D") {
    NM_COMPENSATIONCONFIG comp_config{};
    comp_config.originIndex = 0;
    comp_config.originPosition = config.get_origin_position();
    comp_config.pitchInterval = config.get_interval();

    auto points = config.get_points();
    comp_config.count = static_cast<int>(points.size());

    std::vector<double> comp_data(points.size(), 0.0);
    for (size_t i = 0; i < points.size(); i++) {
      comp_data[i] = points[i];
    }

    int ret = NM_SetCompensation(GetControllerIndex(), axis_index_,
                                  comp_config, comp_data.data());
    if (ret == NM_RETURN_OK) {
      NM_EnableCompensation(GetControllerIndex(), axis_index_, 1);
      LOG(INFO) << "Enabled 1D pitch compensation for axis " << axis_index_;
    } else {
      LOG(ERROR) << "Failed to set 1D pitch compensation, ret=" << ret;
    }
  } else if (type == "2D") {
    LOG(WARNING) << "2D pitch compensation not yet implemented for NeoMove";
  }
}
