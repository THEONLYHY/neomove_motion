// copyright 2025 YottaImage. All rights reserved.
#ifndef NEOMOVE_MOTION_MGR_CONTEXT_IMPL_H_
#define NEOMOVE_MOTION_MGR_CONTEXT_IMPL_H_

#include <motion/motion_mgr_context.h>

#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "NeoMove CPlusPlus.h"

struct NeoMoveHomeParamConfig {
  int home_type = 0;
  double velocity_fast = 0.0;
  double velocity_slow = 0.0;
  double acc = 0.0;
  double dec = 0.0;
};

NeoMoveHomeParamConfig DefaultNeoMoveHomeParamConfig();

// NeoMove 运动上下文保存管理器和子对象共享的设备级状态。
// 这里负责控制器连接生命周期；NeoMoveMotionMgr 负责更上层的
// Axis/IO/MotionControl 包装对象。
class NeoMoveMotionMgrContextImpl : public yotta::MotionMgrContext {
public:
  static NeoMoveMotionMgrContextImpl& GetCurrent();

  // 加载控制器配置，写入 NeoMove SDK 连接参数，并打开配置指定的控制器。
  int Init(const char* path, size_t path_len,
           const char* file, size_t file_len);
  // 关闭 Init() 打开的 NeoMove 控制器；未初始化时直接返回，便于析构流程安全调用。
  int Finalize();

  // MotionMgrContext 回调，供 Axis 实现读取管理器中的倍率配置，
  double YOTTA_API_CALL GetAxisMultiplier(void) override;

  int controller_index() const { return controller_index_; }
  NeoMoveHomeParamConfig GetHomeParamConfig(int axis_index) const;
  bool is_initialized() const { return initialized_; }
  std::mutex& sdk_mutex() { return sdk_mutex_; }
  // 记录 Init() 中最近一次失败的配置步骤或 NeoMove SDK API 名称。
  // NeoMoveMotionMgr 会把它写入 GetError() 的 custom_data，便于定位失败点。
  const std::string& last_failed_api() const { return last_failed_api_; }

private:
  NeoMoveMotionMgrContextImpl() = default;
  ~NeoMoveMotionMgrContextImpl();

  // 从宿主 json 配置读取 "neomove/*" 节点，并在传给 NeoMove SDK 前完成校验。
  int LoadConfig(const char* path, size_t path_len,
                 const char* file, size_t file_len);
  // 集中校验 SDK 控制器类型，避免错误配置值在修改 SDK 状态后才暴露。
  static bool IsValidControllerType(int controller_type);
  // 读取并校验按轴回零参数；未配置的轴继续由读取接口回退默认值。
  int LoadHomeConfig(const std::filesystem::path& config_file);
  static bool IsValidHomeType(int home_type);
  int WaitForMasterRunning();

  // 传给 NM_Open/NM_Close 以及其他按索引访问控制器的 SDK API 的控制器编号。
  int controller_index_ = 0;
  // 仅在 NM_Open 成功后为 true，用于约束 Finalize() 和 NeoMoveMotionMgr 对象访问。
  bool initialized_ = false;
  // Init() 失败标记；NeoMoveMotionMgr::Initialize() 会用它把底层失败转换成
  // 对外可读的 ExecutionError。
  std::string last_failed_api_;

  // 最近一次宿主配置位置和连接参数，归属于当前 context 实例。
  std::string config_path_;
  std::string config_file_;
  std::string controller_ip_ = "172.30.30.10";
  int controller_type_ = NM_CONTROLLERTYPE_E2_M100;
  std::map<int, NeoMoveHomeParamConfig> home_param_configs_;
  std::mutex sdk_mutex_;
};

#endif  // NEOMOVE_MOTION_MGR_CONTEXT_IMPL_H_
