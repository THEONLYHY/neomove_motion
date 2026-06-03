// copyright 2025 YottaImage. All rights reserved.
#include "neomove_motion_control.h"

#include <glog/glog_helper.h>

#include "neomove_motion_mgr_context_impl.h"

using namespace yotta;

NeoMoveMotionControl::NeoMoveMotionControl() {
  LOG(INFO) << "NeoMoveMotionControl constructed.";
}

NeoMoveMotionControl::~NeoMoveMotionControl() {
  LOG(INFO) << "NeoMoveMotionControl destructed.";
  motion_modes_.Clear();
}

int NeoMoveMotionControl::GetControllerIndex() {
  return NeoMoveMotionMgrContextImpl::GetCurrent().controller_index();
}

int YOTTA_API_CALL NeoMoveMotionControl::RegisterMotionMode(MotionMode* mode) {
  ClearError();
  char buf[256]{};
  if (!mode || !mode->name(buf, 256)) {
    LOG(ERROR) << "RegisterMotionMode failed: invalid mode.";
    SetError(MotionErrors::ParamInvalid, "RegisterMotionMode 参数无效");
    return -1;
  }
  std::string name(buf);
  if (motion_modes_.Exists(name)) {
    LOG(WARNING) << "RegisterMotionMode: already registered, name=" << name;
    SetError(MotionErrors::ParamInvalid, "RegisterMotionMode 已注册",
             "already_registered");
    return -2;
  }
  motion_modes_.Insert(name, mode);
  LOG(INFO) << "MotionMode registered: " << name;
  return 0;
}

int YOTTA_API_CALL NeoMoveMotionControl::UnregisterMotionMode(
    const char* name) {
  ClearError();
  std::string mode_name(name);
  motion_modes_.Erase(mode_name);
  LOG(WARNING) << "UnregisterMotionMode: name=" << mode_name;
  return 0;
}

int YOTTA_API_CALL NeoMoveMotionControl::UnregisterMotionMode(
    MotionMode* mode) {
  ClearError();
  if (!mode) {
    LOG(ERROR) << "UnregisterMotionMode failed: mode is null.";
    SetError(MotionErrors::ParamInvalid, "UnregisterMotionMode mode 为空");
    return 1;
  }
  char buf[256]{};
  if (!mode->name(buf, 256)) {
    LOG(ERROR) << "UnregisterMotionMode failed: invalid mode name.";
    SetError(MotionErrors::ParamInvalid, "UnregisterMotionMode 参数无效");
    return -1;
  }
  return UnregisterMotionMode(buf);
}

MotionMode* YOTTA_API_CALL NeoMoveMotionControl::GetMotionMode(
    const char* name) {
  std::string mode_name(name);
  return motion_modes_.Get(mode_name);
}

int YOTTA_API_CALL NeoMoveMotionControl::AsyncLinearIntplPos(
    Axis* axes[], double dest_pos[], AccDecProfile* profiles[],
    size_t array_count) {
  ClearError();
  LOG(INFO) << "AsyncLinearIntplPos called, axis count=" << array_count;

  if (array_count == 0 || !axes || !profiles) {
    LOG(ERROR) << "AsyncLinearIntplPos failed: invalid args.";
    SetError(MotionErrors::ParamInvalid, "AsyncLinearIntplPos 参数无效");
    return -1;
  }

  if (array_count == 1) {
    return axes[0]->AsyncMoveTo(dest_pos[0], profiles[0]);
  }

  // Multi-axis interpolation using NeoMove path interpolation
  NM_PATHINTPLLOOKAHEADCONFIGURATION path_config{};
  path_config.axisCount = static_cast<int>(array_count);
  path_config.compositeVel = profiles[0]->velocity();
  path_config.compositeAcc = profiles[0]->acceleration();
  path_config.compositeDec = profiles[0]->deceleration();

  for (size_t i = 0; i < array_count && i < 8; ++i) {
    path_config.axisList[i] = static_cast<unsigned int>(axes[i]->id());
  }

  int ret = NM_SetPathIntplLookaheadConfiguration(GetControllerIndex(), 0,
                                                    path_config);
  if (ret != NM_RETURN_OK) {
    LOG(ERROR) << "NM_SetPathIntplLookaheadConfiguration failed, ret=" << ret;
    SetError(MotionErrors::InterpolationFailed,
             "AsyncLinearIntplPos 配置失败",
             "neomove_api=NM_SetPathIntplLookaheadConfiguration, ret=" +
                 ToHex(ret));
    return ret;
  }

  NM_PATHINTPLLOOKAHEADCOMMANDPOINT point{};
  point.segmentType = NM_SEGMENTTYPE_LINE;
  for (size_t i = 0; i < array_count && i < 8; ++i) {
    point.position[i] = dest_pos[i];
  }

  ret = NM_AddPathIntplLookahead(GetControllerIndex(), 0, 1, &point);
  if (ret != NM_RETURN_OK) {
    LOG(ERROR) << "NM_AddPathIntplLookahead failed, ret=" << ret;
    SetError(MotionErrors::InterpolationFailed,
             "AsyncLinearIntplPos 添加点失败",
             "neomove_api=NM_AddPathIntplLookahead, ret=" + ToHex(ret));
    return ret;
  }

  ret = NM_Motion_PathIntplLookahead(GetControllerIndex(), 0);
  if (ret != NM_RETURN_OK) {
    LOG(ERROR) << "NM_Motion_PathIntplLookahead failed, ret=" << ret;
    SetError(MotionErrors::InterpolationFailed,
             "AsyncLinearIntplPos 启动失败",
             "neomove_api=NM_Motion_PathIntplLookahead, ret=" + ToHex(ret));
    return ret;
  }

  return 0;
}
