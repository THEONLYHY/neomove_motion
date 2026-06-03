// copyright 2025 YottaImage. All rights reserved.
#include "neomove_motion_mgr.h"

#include <common/encode_helper.h>
#include <glog/glog_helper.h>

#include <memory>

using namespace yotta;

NeoMoveMotionMgr::NeoMoveMotionMgr() {}

NeoMoveMotionMgr::~NeoMoveMotionMgr() { Finalize(); }

void NeoMoveMotionMgr::ClearResources() {
  motion_control_.reset();
  event_control_.reset();
  analog_ios_.clear();
  axes_.clear();
  input_ios_.clear();
  ouput_ios_.clear();
  services_.clear();
}

int YOTTA_API_CALL NeoMoveMotionMgr::Initialize(const char* path, size_t path_len,
                                                 const char* file,
                                                 size_t file_len) {
  ClearError();
  std::unique_lock<std::shared_mutex> lock(mutex_);
  if (initialized_ || !path || !file) {
    LOG(WARNING) << "NeoMoveMotionMgr already initialized or invalid args.";
    return 0;
  }

  std::string path_str(path, path_len);
  std::string file_str(file, file_len);
  NeoMoveMotionMgrContextImpl::SetGlobalConfig(path_str, file_str);

  int ret = NeoMoveMotionMgrContextImpl::GetCurrent().Init(
      path, path_len, file, file_len);

  motion_control_ = std::make_unique<NeoMoveMotionControl>();
  event_control_ = std::make_unique<NeoMoveEventControl>();
  io_monitor_thread_ = std::make_shared<NeoMoveIoMonitorThread>();

  if (ret == 0) {
    io_monitor_thread_->Start();
  } else {
    LOG(ERROR) << "NeoMoveMotionMgrContextImpl::Init failed, ret=" << ret;
    SetError(MotionErrors::DeviceCreateFailed, "Initialize 设备打开失败",
             "neomove_api=NM_Open, ret=" + ToHex(ret));
  }

  initialized_ = (ret == 0);
  LOG(INFO) << "NeoMoveMotionMgr::Initialize() ret=" << ret
            << "\npath=" << path_str << "\nfile=" << file_str
            << "\ncurrent thread id:"
            << ThreadIdChecker::GetCurrentThreadIdStr();
  return ret;
}

int YOTTA_API_CALL NeoMoveMotionMgr::Finalize() {
  ClearError();
  std::unique_lock<std::shared_mutex> lock(mutex_);
  LOG(INFO) << "NeoMoveMotionMgr::Finalize called.";
  if (!initialized_) {
    LOG(WARNING) << "NeoMoveMotionMgr not initialized.";
    return 0;
  }

  for (auto& axis : axes_) {
    if (axis.second) {
      axis.second->SetServoOff();
    }
  }

  if (io_monitor_thread_) {
    io_monitor_thread_->Stop();
  }

  NeoMoveMotionMgrContextImpl::GetCurrent().Finalize();
  ClearResources();

  initialized_ = false;
  LOG(INFO) << "NeoMoveMotionMgr finalized.";
  return 0;
}

int YOTTA_API_CALL NeoMoveMotionMgr::SetAxisMultiplier(double multiplier) {
  ClearError();
  axis_multiplier_ = multiplier;
  return 0;
}

Axis* YOTTA_API_CALL NeoMoveMotionMgr::GetAxis(int id) {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  if (!initialized_ || id < 0) {
    LOG(ERROR) << "GetAxis failed: not initialized or invalid id.";
    return nullptr;
  }

  auto it = axes_.find(id);
  if (it != axes_.end()) {
    return it->second.get();
  }

  auto new_axis = std::make_unique<NeoMoveAxis>(id);
  new_axis->SetIoMonitorThread(io_monitor_thread_);
  axes_[id] = std::move(new_axis);
  LOG(INFO) << "NeoMoveAxis created, id=" << id;
  return axes_[id].get();
}

InputIO* YOTTA_API_CALL NeoMoveMotionMgr::GetInputIo(int addr, int bit) {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  if (!initialized_) return nullptr;

  auto key = std::make_pair(addr, bit);
  auto it = input_ios_.find(key);
  if (it != input_ios_.end()) {
    it->second->SetAddress(addr, bit);
    return it->second.get();
  }
  auto new_io = std::make_unique<NeoMoveInputIo>(addr, bit);
  new_io->SetIoMonitorThread(io_monitor_thread_);
  InputIO* raw_ptr = new_io.get();
  input_ios_[key] = std::move(new_io);
  LOG(INFO) << "NeoMoveInputIO created, addr=" << addr << ", bit=" << bit;
  return raw_ptr;
}

OutputIO* YOTTA_API_CALL NeoMoveMotionMgr::GetOutputIo(int addr, int bit) {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  if (!initialized_) return nullptr;
  auto key = std::make_pair(addr, bit);
  auto it = ouput_ios_.find(key);
  if (it != ouput_ios_.end()) {
    it->second->SetAddress(addr, bit);
    return it->second.get();
  }
  auto new_output_io = std::make_unique<NeoMoveOutputIo>(addr, bit);
  new_output_io->SetIoMonitorThread(io_monitor_thread_);
  OutputIO* raw_ptr = new_output_io.get();
  ouput_ios_[key] = std::move(new_output_io);
  LOG(INFO) << "NeoMoveOutputIO created, addr=" << addr << ", bit=" << bit;
  return raw_ptr;
}

MotionControl* YOTTA_API_CALL NeoMoveMotionMgr::GetMotionControl(int major_ver,
                                                                  int minor_ver) {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  if (!initialized_) return nullptr;

  return motion_control_.get();
}

MotionMgrContext* YOTTA_API_CALL NeoMoveMotionMgr::GetMotionMgrContext() {
  return &NeoMoveMotionMgrContextImpl::GetCurrent();
}

yotta::EventControl* YOTTA_API_CALL NeoMoveMotionMgr::GetEventControl() {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  if (!initialized_) return nullptr;

  return event_control_.get();
}

yotta::AnalogIO* YOTTA_API_CALL NeoMoveMotionMgr::GetAnalogIo(int addr) {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  if (!initialized_ || addr < 0) {
    LOG(ERROR) << "GetAnalogIo failed: not initialized or invalid addr.";
    return nullptr;
  }

  auto it = analog_ios_.find(addr);
  if (it != analog_ios_.end()) {
    return it->second.get();
  }

  auto new_io = std::make_unique<NeoMoveAnalogIo>(addr);
  analog_ios_[addr] = std::move(new_io);
  LOG(INFO) << "NeoMoveAnalogIo created, addr=" << addr;
  return analog_ios_[addr].get();
}

int YOTTA_API_CALL NeoMoveMotionMgr::RegisterService(const char* service_name,
                                                      void* service) {
  ClearError();
  std::unique_lock<std::shared_mutex> lock(mutex_);
  LOG(INFO) << "RegisterService called: " << service_name;

  if (!initialized_) {
    SetError(MotionErrors::ParamInvalid, "RegisterService 未初始化");
    return -1;
  }
  std::string name(service_name);
  if (services_.find(name) != services_.end()) {
    LOG(WARNING) << "Service already exists: " << service_name;
    SetError(MotionErrors::ParamInvalid, "RegisterService 服务已存在");
    return -1;
  }
  services_[name] = service;
  return 0;
}

int YOTTA_API_CALL NeoMoveMotionMgr::QueryService(const char* service_name,
                                                   void** service) {
  ClearError();
  std::shared_lock<std::shared_mutex> lock(mutex_);
  LOG(INFO) << "QueryService called: " << service_name;

  if (!initialized_) {
    SetError(MotionErrors::ParamInvalid, "QueryService 未初始化");
    return -1;
  }
  if (!service) {
    SetError(MotionErrors::ParamInvalid, "QueryService service 指针为空");
    return -1;
  }
  std::string name(service_name);
  auto it = services_.find(name);
  if (it == services_.end()) {
    *service = nullptr;
    LOG(WARNING) << "Service not found: " << service_name;
    return -1;
  }
  *service = it->second;
  return 0;
}

double GetNeoMoveAxisMultiplier() {
  return NeoMoveMotionMgrSingleton::GetInstance()->GetAxisMultiplier();
}
