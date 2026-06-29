// copyright 2025 YottaImage. All rights reserved.
// author panming
// date 2025/05/16 22:30
#ifndef BASE_UI_SRC_CONTROLLER_ALGORITHM_PROCESS_ALGORITHM_PROCESS_CALLBACK_IMPL_H_
#define BASE_UI_SRC_CONTROLLER_ALGORITHM_PROCESS_ALGORITHM_PROCESS_CALLBACK_IMPL_H_

#include <algorithm_process/algorithm_process.h>
#include <main_process/algorithm_step_mgr.h>

#include <cstdint>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "controller/task/task_callback_impl.h"

// 运动控制调用算法处理之后的回调接口
class AlgorithmProcessCallbackImpl : public AlgorithmProcessCallback {
 public:
  //  主进程跟视觉算法进程连接上了的回调，此回调以后就可以调用AlgorithmProcess的接口进行交互
  void OnConnected(const std::string& mod_id) override;

  // 初始化完成，比如需要用参数调用视觉算法进程进行相机和算法模块的初始化等，完成后的回调
  int OnAlgorithmInitEnd(const std::string& mod_id) override;

  // 主进程调用AlgorithmProcess::MotionIsReady，视觉算法会根据配方数据调整相机参数，然后回调
  // 运动控制收到此回调后就可以外触发相机
  int OnAlgorithmIsReady(const std::string& mod_id, int64_t sub_step_id,
                         int32_t msg_type, const char* pb_buf,
                         int32_t pb_len) override;
  // 视觉已经获取到照片的回调
  int OnImageIsReady(const std::string& mod_id, int64_t sub_step_id,
                     int32_t msg_type, const char* pb_buf,
                     int32_t pb_len) override;
  // 视觉算法的结果
  int OnAlgorithmResult(const std::string& mod_id, int64_t sub_step_id,
                        int32_t msg_type, const char* pb_result_buf,
                        int32_t pb_result_len) override;

  void OnDisconnected(const std::string& module_ids) override;

 public:
  void Quit(void);

 private:
  AlgorithmProcessPtr GetAlgProcess(const std::string& mod_id);
  std::string GetLogHeader(const std::string& mod_id, int64_t sub_step_id,
                           int32_t msg_type, int32_t pb_result_len);
  main_process::TaskCallbackPtr GetTaskCallbackPtr(const std::string& mod_id);
  nlohmann::json ReadCameraRealTimeConfigJson(
      const std::wstring& camera_config_path);

 private:
  std::map<std::string, TaskCallbackImplPtr> task_callback_map_;
  nlohmann::json camera_json_;
};

using AlgorithmProcessCallbackImplPtr =
    std::shared_ptr<AlgorithmProcessCallbackImpl>;
using AlgorithmProcessCallbackImplWeakPtr =
    std::weak_ptr<AlgorithmProcessCallbackImpl>;

#endif  // BASE_UI_SRC_CONTROLLER_ALGORITHM_PROCESS_ALGORITHM_PROCESS_CALLBACK_IMPL_H_
