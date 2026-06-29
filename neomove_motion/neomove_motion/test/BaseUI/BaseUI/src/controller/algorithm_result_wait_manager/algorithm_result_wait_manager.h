// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/23 19:28

#ifndef BASE_UI_SRC_CONTROLLER_ALGORITHM_RESULT_WAIT_MANAGER_ALGORITHM_RESULT_WAIT_MANAGER_H_
#define BASE_UI_SRC_CONTROLLER_ALGORITHM_RESULT_WAIT_MANAGER_ALGORITHM_RESULT_WAIT_MANAGER_H_

#include <singleton.h>

#include <cstdint>
#include <future>
#include <string>
#include <vector>

#include "controller/async_wait/async_wait.h"

// 算法结果结构体
struct AlgorithmResult {
  int code;
  std::string mod_id;
  int64_t sub_step_id;
  int32_t msg_type;
  std::string msg;
  std::vector<double> result;

  AlgorithmResult() : code(-1), sub_step_id(0), msg_type(0) {}
};

// 结果管理器，管理所有等待中的结果
class AlgorithmResultWaitMgr {
  SINGLETON(AlgorithmResultWaitMgr);

 public:
  static AlgorithmResultWaitMgr* GetInstance() {
    return yotta::Singleton<AlgorithmResultWaitMgr>::GetInstance();
  }

  // 注册一个等待操作，返回 future
  std::future<AlgorithmResult> Register(int64_t sub_step_id) {
    return AsyncWaitManagerSinglton<int64_t, AlgorithmResult>::GetInstance()
        ->Register(sub_step_id);
  }

  // 完成一个操作，设置结果
  void Complete(int64_t sub_step_id, const AlgorithmResult& result) {
    AsyncWaitManagerSinglton<int64_t, AlgorithmResult>::GetInstance()
        ->Complete(sub_step_id, result);
  }

  // 移除操作（超时等情况）
  void Remove(int64_t sub_step_id) {
    AsyncWaitManagerSinglton<int64_t, AlgorithmResult>::GetInstance()->Remove(
        sub_step_id);
  }

 private:
  AlgorithmResultWaitMgr() = default;
  ~AlgorithmResultWaitMgr() = default;

  AlgorithmResultWaitMgr(const AlgorithmResultWaitMgr&) = delete;
  AlgorithmResultWaitMgr& operator=(const AlgorithmResultWaitMgr&) = delete;
};

using AlgorithmResultWaitMgrSinglton = yotta::Singleton<AlgorithmResultWaitMgr>;

#endif  // BASE_UI_SRC_CONTROLLER_ALGORITHM_RESULT_WAIT_MANAGER_ALGORITHM_RESULT_WAIT_MANAGER_H_
