// copyright 2025 YottaImage. All rights reserved.
#include "neomove_motion_mgr_context_impl.h"

#include <common/encode_helper.h>
#include <glog/glog_helper.h>

#include <cstring>

std::string NeoMoveMotionMgrContextImpl::config_path_;
std::string NeoMoveMotionMgrContextImpl::config_file_;
std::string NeoMoveMotionMgrContextImpl::controller_ip_;
int NeoMoveMotionMgrContextImpl::controller_type_ = NM_CONTROLLERTYPE_E2_M300;

NeoMoveMotionMgrContextImpl& NeoMoveMotionMgrContextImpl::GetCurrent() {
  static NeoMoveMotionMgrContextImpl instance;
  return instance;
}

NeoMoveMotionMgrContextImpl::~NeoMoveMotionMgrContextImpl() {
  if (initialized_) {
    Finalize();
  }
}

void NeoMoveMotionMgrContextImpl::SetGlobalConfig(const std::string& path,
                                                   const std::string& file) {
  config_path_ = path;
  config_file_ = file;
}

void NeoMoveMotionMgrContextImpl::SetControllerIP(const std::string& ip) {
  controller_ip_ = ip;
}

int NeoMoveMotionMgrContextImpl::Init(const char* path, size_t path_len,
                                       const char* file, size_t file_len) {
  if (initialized_) {
    LOG(WARNING) << "NeoMoveMotionMgrContextImpl already initialized";
    return 0;
  }

  LOG(INFO) << "NeoMoveMotionMgrContextImpl::Init, path="
            << std::string(path, path_len)
            << ", file=" << std::string(file, file_len);

  NM_SetControllerType(controller_type_);

  if (!controller_ip_.empty()) {
    char ip[256]{};
    strncpy_s(ip, sizeof(ip), controller_ip_.c_str(), controller_ip_.size());
    NM_SetControllerIP(controller_index_, ip);
  }

  int ret = NM_Open(controller_index_);
  if (ret != NM_RETURN_OK) {
    LOG(ERROR) << "NM_Open failed, ret=" << ret;
    return ret;
  }

  initialized_ = true;
  LOG(INFO) << "NeoMoveMotionMgrContextImpl initialized successfully";
  return 0;
}

int NeoMoveMotionMgrContextImpl::Finalize() {
  if (!initialized_) {
    return 0;
  }

  LOG(INFO) << "NeoMoveMotionMgrContextImpl::Finalize";
  int ret = NM_Close(controller_index_);
  if (ret != NM_RETURN_OK) {
    LOG(ERROR) << "NM_Close failed, ret=" << ret;
  }
  initialized_ = false;
  return ret;
}

double YOTTA_API_CALL NeoMoveMotionMgrContextImpl::GetAxisMultiplier(void) {
  // Forward-declared: will be implemented after NeoMoveMotionMgr is defined
  extern double GetNeoMoveAxisMultiplier();
  return GetNeoMoveAxisMultiplier();
}
