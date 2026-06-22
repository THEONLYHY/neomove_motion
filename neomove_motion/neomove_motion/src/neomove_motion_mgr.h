// copyright 2025 YottaImage. All rights reserved.
#ifndef NEOMOVE_MOTION_MGR_H_
#define NEOMOVE_MOTION_MGR_H_

#include <base.h>
#include <common/thread_id_checker.h>
#include <error_info/error_codes.h>
#include <error_info/impl/execution_error_impl.h>
#include <motion/motion_mgr.h>
#include <singleton.h>

#include <iomanip>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <vector>

#include "neomove_analog_io.h"
#include "neomove_axis.h"
#include "neomove_event_control.h"
#include "neomove_input_io.h"
#include "neomove_io_monitor_thread.h"
#include "neomove_motion_control.h"
#include "neomove_motion_mgr_context_impl.h"
#include "neomove_output_io.h"

// NeoMove 运动插件导出的 MotionMgr 实现。该类把面向宿主的 yotta::MotionMgr
// 接口适配到 NeoMove 专用的 Axis/IO/MotionControl 对象，并按地址缓存这些对象。
class NeoMoveMotionMgr : public yotta::MotionMgr {
public:
  SINGLETON(NeoMoveMotionMgr);
  virtual ~NeoMoveMotionMgr();

  // 返回该管理器最近一次生成的对外错误信息。底层 SDK/配置失败会在 Initialize()
  // 中转换后保存到这里。
  ExecutionErrorPtr GetError() const override {
    std::lock_guard<std::mutex> lock(error_mutex_);
    return last_error_;
  }

  // 初始化共享 NeoMove 上下文，创建包装对象，并在控制器打开成功后启动 IO 监控。
  int YOTTA_API_CALL Initialize(const char* path, size_t path_len,
                                const char* file, size_t file_len) override;
  // 停止活动输出/监控，关闭 SDK 上下文，并清理缓存的包装对象。
  int YOTTA_API_CALL Finalize() override;
  // 宿主可配置的轴单位倍率。Axis 对象通过
  int YOTTA_API_CALL SetAxisMultiplier(double multiplier) override;
  double YOTTA_API_CALL GetAxisMultiplier(void) override {
    return axis_multiplier_;
  }

  yotta::Axis* YOTTA_API_CALL GetAxis(int id) override;
  yotta::InputIO* YOTTA_API_CALL GetInputIo(int addr, int bit) override;
  yotta::OutputIO* YOTTA_API_CALL GetOutputIo(int addr, int bit) override;
  yotta::MotionControl* YOTTA_API_CALL GetMotionControl(int major_ver,
                                                        int minor_ver) override;
  yotta::MotionMgrContext* YOTTA_API_CALL GetMotionMgrContext() override;
  yotta::EventControl* YOTTA_API_CALL GetEventControl() override;
  yotta::AnalogIO* YOTTA_API_CALL GetAnalogIo(int addr) override;

  int YOTTA_API_CALL RegisterService(const char* service_name,
                                     void* service) override;
  int YOTTA_API_CALL QueryService(const char* service_name,
                                  void** service) override;

private:
  NeoMoveMotionMgr();
  // 只释放 manager 拥有的 C++ 包装资源；SDK 连接关闭由
  // NeoMoveMotionMgrContextImpl::Finalize() 单独处理。
  void ClearResources();

  bool initialized_ = false;  // 防止未初始化时使用设备
  // 轴位置/速度单位转换使用的倍率。
  double axis_multiplier_ = 1.0;

  std::unique_ptr<NeoMoveMotionControl> motion_control_;
  std::unique_ptr<NeoMoveEventControl> event_control_;
  std::map<int, std::unique_ptr<NeoMoveAxis>> axes_;  // 缓存轴对象
  std::map<int, std::unique_ptr<NeoMoveAnalogIo>> analog_ios_;  // 缓存模拟量 IO 对象
  std::map<std::pair<int, int>, std::unique_ptr<NeoMoveInputIo>> input_ios_;  // 缓存输入IO对象
  std::map<std::pair<int, int>, std::unique_ptr<NeoMoveOutputIo>> ouput_ios_; // 缓存输出IO对象

  std::map<std::string, void*> services_; // 保护外部注册服务

  // Axis 和 IO 包装对象共享的轮询线程，用于输入/状态监控；
  // 生命周期应覆盖 manager 的初始化周期。
  std::shared_ptr<NeoMoveIoMonitorThread> io_monitor_thread_;
  std::shared_mutex mutex_; // 接口多线程安全

  // GetError() 对外暴露的错误对象。这里使用独立锁保护，
  // 避免调用者查询错误时占用完整 manager 锁。
  mutable ExecutionErrorPtr last_error_;
  mutable std::mutex error_mutex_;

  // 在可能设置新错误的公开操作开始时清理旧错误。
  void ClearError() const {
    std::lock_guard<std::mutex> lock(error_mutex_);
    last_error_ = nullptr;
  }

  // 构造宿主可见的错误对象。custom_data 携带失败的 NeoMove API 名称、
  // 原始返回码等诊断细节。
  void SetError(int32_t code, const std::string& message,
                const std::string& custom_data = "") const {
    std::lock_guard<std::mutex> lock(error_mutex_);
    last_error_ = ExecutionError::Create(code, "Motion");
    last_error_->WithMessage(message).WithEntity("neomove_motion_mgr");
    if (!custom_data.empty()) {
      last_error_->WithCustomData(custom_data);
    }
  }

  // 将原始 SDK 返回码格式化到 custom_data 中，让日志和宿主错误界面使用一致的十六进制表示。
  static std::string ToHex(int value) {
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase << value;
    return oss.str();
  }

  // 允许上下文回调读取 axis_multiplier_，同时不通过公开 MotionMgr 接口暴露 manager 内部实现。
  friend double GetNeoMoveAxisMultiplier();
};

using NeoMoveMotionMgrSingleton = yotta::Singleton<NeoMoveMotionMgr>;

double GetNeoMoveAxisMultiplier();

#endif  // NEOMOVE_MOTION_MGR_H_
