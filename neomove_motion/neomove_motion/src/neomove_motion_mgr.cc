// copyright 2025 YottaImage. All rights reserved.
#include "neomove_motion_mgr.h"

#include <common/encode_helper.h>
#include <glog/glog_helper.h>

#include <cmath>
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
  // 重复 Initialize() 或传入无效指针时，插件层按无操作成功处理。
  if (initialized_ || !path || !file) {
    LOG(WARNING) << "NeoMoveMotionMgr already initialized or invalid args.";
    return 0;
  }

  std::string path_str(path, path_len);
  std::string file_str(file, file_len);

  auto& context = NeoMoveMotionMgrContextImpl::GetCurrent();
  // context.Init() 负责 SDK 初始化顺序：
  // LoadConfig -> SetControllerType -> 可选 SetControllerIP -> Open。
  int ret = context.Init(path, path_len, file, file_len);

  // 即使 Init() 失败也创建包装功能组，保证后续清理和诊断面对一致的 manager 状态。
  motion_control_ = std::make_unique<NeoMoveMotionControl>();
  event_control_ = std::make_unique<NeoMoveEventControl>();
  io_monitor_thread_ = std::make_shared<NeoMoveIoMonitorThread>();

  if (ret == 0) {
    // 仅在 NM_Open 成功后启动监控；否则轮询会访问尚未打开的控制器。
    io_monitor_thread_->Start();
  } else {
    LOG(ERROR) << "NeoMoveMotionMgrContextImpl::Init failed, ret=" << ret;
    // 优先使用 context 记录的精确失败配置/API 步骤；未记录时退回到 Init() 包装函数名。
    std::string failed_api = context.last_failed_api().empty()
                                 ? "NeoMoveMotionMgrContextImpl::Init"
                                 : context.last_failed_api();
    std::string custom_data =
        "neomove_api=" + failed_api + ", ret=" + ToHex(ret);
    if (ret == MotionErrors::ConfigLoadFailed) {
      SetError(MotionErrors::ConfigLoadFailed,
               "Initialize NeoMove config load failed", custom_data);
    } else if (ret == MotionErrors::ParamInvalid) {
      SetError(MotionErrors::ParamInvalid,
               "Initialize NeoMove config parameter invalid", custom_data);
    } else {
      // 非配置错误来自 NeoMove SDK 连接序列，因此按设备创建/打开失败处理。
      SetError(MotionErrors::DeviceCreateFailed,
               "Initialize NeoMove device open failed", custom_data);
    }
  }

  initialized_ = (ret == 0);
  LOG(INFO) << "NeoMoveMotionMgr::Initialize() ret=" << ret
            << "\npath=" << path_str << "\nfile=" << file_str
            << "\ncurrent thread id:"
            << ThreadIdChecker::GetCurrentThreadIdStr();
  return ret;
}

// 停止监控、关伺服、释放资源
int YOTTA_API_CALL NeoMoveMotionMgr::Finalize() {
  ClearError();
  std::unique_lock<std::shared_mutex> lock(mutex_);
  LOG(INFO) << "NeoMoveMotionMgr::Finalize called.";
  if (!initialized_) {
    LOG(WARNING) << "NeoMoveMotionMgr not initialized.";
    return 0;
  }
  // 设置所有的轴关伺服server off -> MM_ServoOnOff,尽量让设备进入安全状态
  for (auto& axis : axes_) {
    if (axis.second) {
      axis.second->SetServoOff();
    }
  }
  // 停止IO监控线程
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
  std::unique_lock<std::shared_mutex> lock(mutex_);
  if (!std::isfinite(multiplier) || multiplier <= 0.0) {
    LOG(ERROR) << "SetAxisMultiplier failed: invalid multiplier="
               << multiplier;
    SetError(MotionErrors::ParamInvalid, "SetAxisMultiplier 倍率无效",
             "axis_multiplier=" + std::to_string(multiplier));
    return MotionErrors::ParamInvalid;
  }
  if (!axes_.empty()) {
    if (multiplier != axis_multiplier_) {
      LOG(ERROR) << "SetAxisMultiplier failed: axes already created"
                 << ", old_multiplier=" << axis_multiplier_
                 << ", new_multiplier=" << multiplier;
      SetError(MotionErrors::ParamInvalid,
               "SetAxisMultiplier cannot change after axis creation",
               "old_axis_multiplier=" + std::to_string(axis_multiplier_) +
                   ", new_axis_multiplier=" + std::to_string(multiplier));
      return MotionErrors::ParamInvalid;
    }
    return 0;
  }
  // 保存原始倍率值。Axis 对象会通过共享 MotionMgrContext 回调读取它并完成单位转换。
  axis_multiplier_ = multiplier;
  return 0;
}

Axis* YOTTA_API_CALL NeoMoveMotionMgr::GetAxis(int id) {
  ClearError();
  std::unique_lock<std::shared_mutex> lock(mutex_);
  // 控制器打开后轴对象才有效；负数 id 不映射到硬件轴。
  if (!initialized_) {
    LOG(ERROR) << "GetAxis failed: not initialized.";
    SetError(MotionErrors::DeviceCreateFailed, "GetAxis 未初始化");
    return nullptr;
  }
  if (id < 0) {
    LOG(ERROR) << "GetAxis failed: invalid id=" << id;
    SetError(MotionErrors::ParamInvalid, "GetAxis 轴号无效",
             "axis=" + std::to_string(id));
    return nullptr;
  }

  auto it = axes_.find(id);
  if (it != axes_.end()) {
    return it->second.get();
  }

  // 懒加载：如果没有则创建。通用 axis.json 软限位由上层 limit_motion 负责，
  // 这里不把轴创建绑定到控制器软限位下发。
  auto new_axis = std::make_unique<NeoMoveAxis>(id);
  new_axis->SetIoMonitorThread(io_monitor_thread_);
  auto* axis = new_axis.get();
  axes_.emplace(id, std::move(new_axis));

  LOG(INFO) << "NeoMoveAxis created, id=" << id;
  return axis;
}

InputIO* YOTTA_API_CALL NeoMoveMotionMgr::GetInputIo(int addr, int bit) {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  if (!initialized_) return nullptr;

  // 输入 IO 按地址和 bit 共同缓存，因为一个地址可以包含多个逻辑 IO 点。
  auto key = std::make_pair(addr, bit);
  auto it = input_ios_.find(key);
  if (it != input_ios_.end()) {
    // 返回缓存对象前刷新地址字段，防止包装对象内部另有地址副本。
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
  // 输出 IO 使用与输入 IO 相同的地址/bit 缓存方式，保证重复调用拿到同一包装对象。
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

  // 当前插件只暴露一个 MotionControl 实现，不区分调用方请求的接口版本。
  return motion_control_.get();
}

MotionMgrContext* YOTTA_API_CALL NeoMoveMotionMgr::GetMotionMgrContext() {
  // 返回进程级上下文，确保子对象和宿主代码观察到同一份控制器连接状态。
  return &NeoMoveMotionMgrContextImpl::GetCurrent();
}

yotta::EventControl* YOTTA_API_CALL NeoMoveMotionMgr::GetEventControl() {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  if (!initialized_) return nullptr;

  return event_control_.get();
}

yotta::AnalogIO* YOTTA_API_CALL NeoMoveMotionMgr::GetAnalogIo(int addr) {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  // 模拟量通道只按 addr 寻址；创建包装对象前先拒绝负数逻辑通道。
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

  // service 是宿主/插件扩展点。仅在初始化后保存裸指针，避免消费者在设备关闭时看到服务。
  if (!initialized_) {
    SetError(MotionErrors::ParamInvalid, "RegisterService 未初始化");
    return -1;
  }
  std::string name(service_name);
  if (services_.find(name) != services_.end()) {
    LOG(WARNING) << "Service already exists: " << service_name;
    SetError(MotionErrors::ParamInvalid,
             "RegisterService service already exists");
    return -1;
  }
  // 对象所有权仍属于调用方；这里的 map 只作为查找注册表。
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
    // 未找到时返回 -1 并清空输出指针；不设置 last_error_，
    // 因为可选服务缺失可能是正常路径。
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
