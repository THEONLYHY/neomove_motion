// copyright 2025 YottaImage. All rights reserved.
#include "neomove_axis.h"

#include <glog/glog_helper.h>

#include <chrono>
#include <cmath>
#include <limits>
#include <sstream>
#include <thread>

#include "neomove_io_monitor_thread.h"
#include "neomove_motion_mgr_context_impl.h"
#include "neomove_sdk_guard.h"
#include "pitch_compensation_config/pitch_compensation_config.h"

using namespace yotta;

namespace {

double NormalizeReadbackMultiplier(double multiplier) {
  if (fabs(multiplier) < std::numeric_limits<double>::epsilon()) {
    return 1.0;
  }
  return multiplier;
}

std::string FormatAxisStatus(const NM_AXISSTATUS& axis_status,
                             double multiplier) {
  const double readback_multiplier = NormalizeReadbackMultiplier(multiplier);
  std::ostringstream oss;
  oss << "servoOn=" << axis_status.servoOn
      << ", motionComplete=" << axis_status.motionComplete
      << ", inPos=" << axis_status.inPos
      << ", ampAlarm=" << axis_status.ampAlarm
      << ", axisAlarm=" << axis_status.axisAlarm
      << ", followingErrorAlarm=" << axis_status.followingErrorAlarm
      << ", ampAlarmCode=" << axis_status.ampAlarmCode
      << ", positiveLS=" << axis_status.positiveLS
      << ", negativeLS=" << axis_status.negativeLS
      << ", positiveSoftLimit=" << axis_status.positiveSoftLimit
      << ", negativeSoftLimit=" << axis_status.negativeSoftLimit
      << ", actualPos=" << axis_status.actualPos / readback_multiplier
      << ", posCmd=" << axis_status.posCmd / readback_multiplier;
  return oss.str();
}

bool HasAxisHardFault(const NM_AXISSTATUS& axis_status) {
  return axis_status.ampAlarm != 0 || axis_status.axisAlarm != 0 ||
         axis_status.followingErrorAlarm != 0;
}

bool IsAtPositiveLimit(const NM_AXISSTATUS& axis_status) {
  return axis_status.positiveLS != 0 || axis_status.positiveSoftLimit != 0;
}

bool IsAtNegativeLimit(const NM_AXISSTATUS& axis_status) {
  return axis_status.negativeLS != 0 || axis_status.negativeSoftLimit != 0;
}

}  // namespace

NeoMoveAxis::NeoMoveAxis(int axis_index) : axis_index_(axis_index) {
  LOG(INFO) << "NeoMoveAxis constructed, axis_index=" << axis_index_;
  // 初始化缓存为无效状态
  cached_status_.valid = false;
}

void NeoMoveAxis::UpdateCachedStatus(const NM_AXISSTATUS& status) {
  {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    cached_status_.axis_status = status;
    cached_status_.last_update = std::chrono::steady_clock::now();
    cached_status_.valid = true;
  }

  // 检查 Wait() 唤醒条件
  bool should_notify = false;
  WaitResult result = WaitResult::kPending;

  if (status.motionComplete && status.inPos) {
    result = WaitResult::kCompleted;
    should_notify = true;
  } else if (HasAxisHardFault(status)) {
    result = WaitResult::kError;
    should_notify = true;
    SetError(MotionErrors::MoveFailed, "Wait 检测到轴硬件报警",
             "axis=" + std::to_string(axis_index_));
  } else if (!status.servoOn) {
    result = WaitResult::kError;
    should_notify = true;
    SetError(MotionErrors::MoveFailed, "Wait 检测到伺服未使能",
             "axis=" + std::to_string(axis_index_));
  } else {
    // 限位方向感知
    if (IsAtPositiveLimit(status) && last_move_direction_ > 0) {
      result = WaitResult::kError;
      should_notify = true;
      SetError(MotionErrors::MoveFailed, "Wait 检测到正限位",
               "axis=" + std::to_string(axis_index_));
    } else if (IsAtNegativeLimit(status) && last_move_direction_ < 0) {
      result = WaitResult::kError;
      should_notify = true;
      SetError(MotionErrors::MoveFailed, "Wait 检测到负限位",
               "axis=" + std::to_string(axis_index_));
    }
  }

  if (should_notify) {
    std::lock_guard<std::mutex> lk(wait_mutex_);
    wait_result_ = result;
    wait_cv_.notify_all();
  }
}

bool NeoMoveAxis::ReadCachedStatus(NM_AXISSTATUS* status) const {
  std::lock_guard<std::mutex> lock(cache_mutex_);
  if (!cached_status_.valid) {
    LOG(WARNING) << "NeoMoveAxis::ReadCachedStatus: cache not ready, axis="
                 << axis_index_;
    return false;
  }
  if (status) {
    *status = cached_status_.axis_status;
  }
  return true;
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
  if (!ReadCachedStatus(&axis_status)) {
    SetError(MotionErrors::MoveFailed, "获取轴状态失败", "cache_not_ready");
    return -1;
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
  if (!ReadCachedStatus(&axis_status)) {
    SetError(MotionErrors::MoveFailed, "获取轴状态失败", "cache_not_ready");
    return -1;
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
  if (!ReadCachedStatus(&axis_status)) {
    SetError(MotionErrors::MoveFailed, "获取轴状态失败", "cache_not_ready");
    return -1;
  }

  // 所有未完成的运动都归成了kPos，neomove中没有相应opstate
  // 无法可靠区分 Jog、Pos、Stop、插补等意图
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
  int ret = neomove_sdk_guard::Call([&]() {
    return NM_ServoOnOff(GetControllerIndex(), axis_index_, 1);
  });
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "SetServoOn 调用失败",
             "neomove_api=NM_ServoOnOff, ret=" + ToHex(ret));
    LOG(ERROR) << "NeoMoveAxis::SetServoOn failed, axis=" << axis_index_
               << ", ret=" << ToHex(ret);
    return ret;
  }

  NM_AXISSTATUS axis_status{};
  int status_ret = neomove_sdk_guard::Call([&]() {
    return NM_GetAxisStatus(GetControllerIndex(), axis_index_, &axis_status);
  });
  if (status_ret == NM_RETURN_OK) {
    UpdateCachedStatus(axis_status);  // 同步缓存
    LOG(INFO) << "NeoMoveAxis::SetServoOn status, axis=" << axis_index_
              << ", " << FormatAxisStatus(axis_status, GetMultiplier());
  } else {
    LOG(ERROR) << "NeoMoveAxis::SetServoOn status read failed, axis="
               << axis_index_ << ", ret=" << ToHex(status_ret);
    SetError(MotionErrors::MoveFailed, "SetServoOn 状态读回失败",
             "neomove_api=NM_GetAxisStatus, ret=" + ToHex(status_ret));
    return status_ret;
  }

  const std::string status_text = FormatAxisStatus(axis_status, GetMultiplier());
  if (!axis_status.servoOn) {
    SetError(MotionErrors::MoveFailed, "SetServoOn 后伺服未使能",
             "neomove_api=NM_GetAxisStatus, " + status_text);
    LOG(ERROR) << "NeoMoveAxis::SetServoOn readback servo off, axis="
               << axis_index_ << ", " << status_text;
    return -1;
  }
  if (HasAxisHardFault(axis_status)) {
    SetError(MotionErrors::MoveFailed, "SetServoOn 后检测到轴硬件报警",
             "neomove_api=NM_GetAxisStatus, " + status_text);
    LOG(ERROR) << "NeoMoveAxis::SetServoOn readback fault, axis="
               << axis_index_ << ", " << status_text;
    return -1;
  }
  return 0;
}

int YOTTA_API_CALL NeoMoveAxis::SetServoOff() {
  ClearError();
  LOG(INFO) << "NeoMoveAxis::SetServoOff, axis=" << axis_index_;
  int ret = neomove_sdk_guard::Call([&]() {
    return NM_ServoOnOff(GetControllerIndex(), axis_index_, 0);
  });
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "SetServoOff 调用失败",
             "neomove_api=NM_ServoOnOff, ret=" + ToHex(ret));
    return ret;
  }

  NM_AXISSTATUS axis_status{};
  int elapsed = 0;
  const int timeout = 3000;
  const int poll_interval_ms = 20;
  while (elapsed < timeout) {
    int status_ret = neomove_sdk_guard::Call([&]() {
      return NM_GetAxisStatus(GetControllerIndex(), axis_index_, &axis_status);
    });
    if (status_ret != NM_RETURN_OK) {
      SetError(MotionErrors::MoveFailed, "获取轴状态失败",
               "neomove_api=NM_GetAxisStatus, ret=" + ToHex(status_ret));
      return status_ret;
    }
    if (!axis_status.servoOn) {
      LOG(INFO) << "NeoMoveAxis::SetServoOff completed, axis=" << axis_index_
                << ", elapsed_ms=" << elapsed << ", "
                << FormatAxisStatus(axis_status, GetMultiplier());
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(poll_interval_ms));
    elapsed += poll_interval_ms;
  }
  if (axis_status.servoOn) {
    SetError(MotionErrors::MoveFailed, "SetServoOff 等待伺服关闭超时",
             "neomove_api=NM_GetAxisStatus, " +
                 FormatAxisStatus(axis_status, GetMultiplier()));
    return -1;
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
  int ret = neomove_sdk_guard::Call([&]() {
    return NM_ClearAlarm(GetControllerIndex(), axis_index_);
  });
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "ClearAxisAlarm 调用失败",
             "neomove_api=NM_ClearAlarm, ret=" + ToHex(ret));
    LOG(ERROR) << "NM_ClearAlarm failed, axis=" << axis_index_
               << ", ret=" << ToHex(ret);
    return ret;
  }

  NM_AXISSTATUS axis_status{};
  int status_ret = neomove_sdk_guard::Call([&]() {
    return NM_GetAxisStatus(GetControllerIndex(), axis_index_, &axis_status);
  });
  if (status_ret == NM_RETURN_OK) {
    LOG(INFO) << "NeoMoveAxis::ClearAxisAlarm after clear, axis=" << axis_index_
              << ", " << FormatAxisStatus(axis_status, GetMultiplier());
  } else {
    LOG(ERROR) << "NeoMoveAxis::ClearAxisAlarm status read failed, axis="
               << axis_index_ << ", ret=" << ToHex(status_ret);
    SetError(MotionErrors::MoveFailed, "ClearAxisAlarm 状态读回失败",
             "neomove_api=NM_GetAxisStatus, ret=" + ToHex(status_ret));
    return status_ret;
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

  int ret = neomove_sdk_guard::Call([&]() {
    return NM_SetAxisMode(GetControllerIndex(), axis_index_, neoMode);
  });
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

  int ret = neomove_sdk_guard::Call([&]() {
    return NM_AxisHome(GetControllerIndex(), axis_index_, home_param);
  });
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

  neomove_sdk_guard::Call([&]() {
    return NM_SetPosition(GetControllerIndex(), axis_index_, 0.0);
  });
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
  if (!ReadCachedStatus(&axis_status)) {
    SetError(MotionErrors::MoveFailed, "GetActualPosition 失败", "cache_not_ready");
    return -1;
  }

  double multiplier = NormalizeReadbackMultiplier(GetMultiplier());
  *position = axis_status.actualPos / multiplier;
  return 0;
}

int YOTTA_API_CALL NeoMoveAxis::GetActualVelocity(double* velocity) {
  ClearError();
  if (!velocity) {
    SetError(MotionErrors::ParamInvalid, "velocity 参数为空");
    return 1;
  }

  NM_AXISSTATUS axis_status{};
  if (!ReadCachedStatus(&axis_status)) {
    SetError(MotionErrors::MoveFailed, "GetActualVelocity 失败", "cache_not_ready");
    return -1;
  }

  double multiplier = NormalizeReadbackMultiplier(GetMultiplier());
  *velocity = axis_status.actualVelocity / multiplier;
  return 0;
}

int YOTTA_API_CALL NeoMoveAxis::GetTargetPosition(double* target_position) {
  ClearError();
  if (!target_position) {
    SetError(MotionErrors::ParamInvalid, "target_position 参数为空");
    return 1;
  }

  NM_AXISSTATUS axis_status{};
  if (!ReadCachedStatus(&axis_status)) {
    SetError(MotionErrors::MoveFailed, "GetTargetPosition 失败", "cache_not_ready");
    return -1;
  }

  double multiplier = NormalizeReadbackMultiplier(GetMultiplier());
  *target_position = axis_status.posCmd / multiplier;
  return 0;
}

int YOTTA_API_CALL NeoMoveAxis::AsyncMoveTo(double dest_pos,
                                             yotta::AccDecProfile* profile) {
  ClearError();
  if (!profile) {
    SetError(MotionErrors::ParamInvalid, "profile 参数为空");
    return 1;
  }

  // 预检限位：读取缓存状态，拒绝朝已触发限位方向的运动
  NM_AXISSTATUS cur_status{};
  if (ReadCachedStatus(&cur_status)) {
    double multiplier_check = NormalizeReadbackMultiplier(GetMultiplier());
    double cur_pos = cur_status.actualPos / multiplier_check;
    bool going_positive = dest_pos > cur_pos;
    bool going_negative = dest_pos < cur_pos;
    if (going_positive && IsAtPositiveLimit(cur_status)) {
      SetError(MotionErrors::MoveFailed, "AsyncMoveTo 被正限位阻止",
               "dest=" + std::to_string(dest_pos) + ", cur=" + std::to_string(cur_pos));
      LOG(WARNING) << "NeoMoveAxis::AsyncMoveTo blocked by positive limit, axis="
                   << axis_index_ << ", dest=" << dest_pos << ", cur=" << cur_pos;
      return -1;
    }
    if (going_negative && IsAtNegativeLimit(cur_status)) {
      SetError(MotionErrors::MoveFailed, "AsyncMoveTo 被负限位阻止",
               "dest=" + std::to_string(dest_pos) + ", cur=" + std::to_string(cur_pos));
      LOG(WARNING) << "NeoMoveAxis::AsyncMoveTo blocked by negative limit, axis="
                   << axis_index_ << ", dest=" << dest_pos << ", cur=" << cur_pos;
      return -1;
    }
    last_move_direction_ = going_positive ? 1 : (going_negative ? -1 : 0);
  }

  double multiplier = GetMultiplier();
  double pulse_dest = dest_pos * multiplier;
  LOG(INFO) << "NeoMoveAxis::AsyncMoveTo, axis=" << axis_index_
            << ", dest=" << dest_pos << ", pulse_dest=" << pulse_dest
            << ", velocity=" << profile->velocity()
            << ", acc=" << profile->acceleration()
            << ", dec=" << profile->deceleration()
            << ", pulse_velocity=" << profile->velocity() * multiplier
            << ", pulse_acc=" << profile->acceleration() * multiplier
            << ", pulse_dec=" << profile->deceleration() * multiplier;

  NM_POSITIONCOMMAND pos_cmd{};
  pos_cmd.axisIndex = static_cast<unsigned int>(axis_index_);
  pos_cmd.target = pulse_dest;
  pos_cmd.velocity = profile->velocity() * multiplier;
  pos_cmd.acc = profile->acceleration() * multiplier;
  pos_cmd.dec = profile->deceleration() * multiplier;
  pos_cmd.smoothTime = profile->moving_average_time_milliseconds();

  int ret = neomove_sdk_guard::Call([&]() {
    return NM_Motion_PositionMode_Abs(GetControllerIndex(), pos_cmd);
  });
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "AsyncMoveTo 调用失败",
             "neomove_api=NM_Motion_PositionMode_Abs, ret=" + ToHex(ret));
    LOG(ERROR) << "NeoMoveAxis::AsyncMoveTo failed, axis=" << axis_index_
               << ", ret=" << ToHex(ret);
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAxis::Wait() {
  ClearError();
  LOG(INFO) << "NeoMoveAxis::Wait, axis=" << axis_index_;

  std::unique_lock<std::mutex> lk(wait_mutex_);
  wait_result_ = WaitResult::kPending;

  bool ok = wait_cv_.wait_for(lk, std::chrono::milliseconds(60000), [this] {
    return wait_result_ != WaitResult::kPending;
  });

  if (!ok) {
    SetError(MotionErrors::MoveFailed, "Wait 超时");
    LOG(ERROR) << "NeoMoveAxis::Wait timeout, axis=" << axis_index_;
    return -1;
  }

  // 错误已由 UpdateCachedStatus() 在唤醒前设置
  if (wait_result_ == WaitResult::kCompleted) {
    LOG(INFO) << "NeoMoveAxis::Wait completed, axis=" << axis_index_;
    return 0;
  } else {
    LOG(ERROR) << "NeoMoveAxis::Wait error, axis=" << axis_index_;
    return -1;
  }
}

int YOTTA_API_CALL NeoMoveAxis::StartJog(yotta::AccDecProfile* profile,
                                          bool positive) {
  ClearError();
  if (!profile) {
    SetError(MotionErrors::ParamInvalid, "profile 参数为空");
    return 1;
  }

  // 预检限位：拒绝朝已触发限位方向的 Jog
  NM_AXISSTATUS cur_status{};
  if (ReadCachedStatus(&cur_status)) {
    if (positive && IsAtPositiveLimit(cur_status)) {
      SetError(MotionErrors::JogFailed, "StartJog 被正限位阻止", "direction=positive");
      LOG(WARNING) << "NeoMoveAxis::StartJog blocked by positive limit, axis=" << axis_index_;
      return -1;
    }
    if (!positive && IsAtNegativeLimit(cur_status)) {
      SetError(MotionErrors::JogFailed, "StartJog 被负限位阻止", "direction=negative");
      LOG(WARNING) << "NeoMoveAxis::StartJog blocked by negative limit, axis=" << axis_index_;
      return -1;
    }
    last_move_direction_ = positive ? 1 : -1;
  }

  double multiplier = GetMultiplier();
  LOG(INFO) << "NeoMoveAxis::StartJog, axis=" << axis_index_
            << ", positive=" << positive;

  NM_JOGCOMMAND jog_cmd{};
  jog_cmd.axisIndex = static_cast<unsigned int>(axis_index_);
  jog_cmd.velocity = profile->velocity() * multiplier * (positive ? 1.0 : -1.0);
  jog_cmd.acc = profile->acceleration() * multiplier;
  jog_cmd.dec = profile->deceleration() * multiplier;
  jog_cmd.smoothTime = profile->moving_average_time_milliseconds();
  LOG(INFO) << "NeoMoveAxis::StartJog command, axis=" << axis_index_
            << ", direction=" << (positive ? "positive" : "negative")
            << ", velocity=" << profile->velocity()
            << ", acc=" << profile->acceleration()
            << ", dec=" << profile->deceleration()
            << ", smoothTime=" << jog_cmd.smoothTime
            << ", multiplier=" << multiplier
            << ", pulse_velocity=" << jog_cmd.velocity
            << ", pulse_acc=" << jog_cmd.acc
            << ", pulse_dec=" << jog_cmd.dec;

  int ret = neomove_sdk_guard::Call([&]() {
    return NM_Motion_JogMode(GetControllerIndex(), jog_cmd);
  });
  LOG(INFO) << "NeoMoveAxis::StartJog ret, axis=" << axis_index_
            << ", ret=" << ToHex(ret);
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
  neomove_sdk_guard::Call([&]() {
    return NM_AxisStop(GetControllerIndex(), axis_index_);
  });
}

void NeoMoveAxis::QuickStop() {
  neomove_sdk_guard::Call([&]() {
    return NM_AxisQuickStop(GetControllerIndex(), axis_index_);
  });
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
  static constexpr char kNeoMoveAxisInterface[] = "NeoMoveAxis";
  if (length == sizeof(kNeoMoveAxisInterface) - 1 &&
      strncmp(interface_name, kNeoMoveAxisInterface, length) == 0) {
    return static_cast<NeoMoveAxis*>(this);
  }
  return nullptr;
}

int YOTTA_API_CALL NeoMoveAxis::SetAxisWatcher(Watcher* watcher) {
  if (!watcher) {
    return 0;
  }
  axis_watcher_ = watcher;
  if (auto monitor = io_monitor_thread_.lock()) {
    monitor->RegisterAxis(axis_index_, axis_watcher_, this);
  }
  return 0;
}

void NeoMoveAxis::SetIoMonitorThread(
    std::weak_ptr<NeoMoveIoMonitorThread> monitor) {
  io_monitor_thread_ = monitor;
}

void NeoMoveAxis::DisablePitchCompensation() {
  neomove_sdk_guard::Call([&]() {
    return NM_EnableCompensation(GetControllerIndex(), axis_index_, 0);
  });
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

    int ret = neomove_sdk_guard::Call([&]() {
      return NM_SetCompensation(GetControllerIndex(), axis_index_, comp_config,
                                comp_data.data());
    });
    if (ret == NM_RETURN_OK) {
      neomove_sdk_guard::Call([&]() {
        return NM_EnableCompensation(GetControllerIndex(), axis_index_, 1);
      });
      LOG(INFO) << "Enabled 1D pitch compensation for axis " << axis_index_;
    } else {
      LOG(ERROR) << "Failed to set 1D pitch compensation, ret=" << ret;
    }
  } else if (type == "2D") {
    LOG(WARNING) << "2D pitch compensation not yet implemented for NeoMove";
  }
}
