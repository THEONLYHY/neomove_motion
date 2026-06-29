// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/31 12:24

#ifndef BASE_UI_SRC_CONTROLLER_ASYNC_WAIT_ASYNC_WAIT_H_
#define BASE_UI_SRC_CONTROLLER_ASYNC_WAIT_ASYNC_WAIT_H_

#include <singleton.h>

#include <future>
#include <map>
#include <mutex>
#include <utility>

/*
用法示例1：单次异步等待

AsyncWait<int> wait;
auto future = wait.Register();

在回调线程或工作线程中设置结果：
wait.Complete(123);

在等待线程中获取结果：
if (future.wait_for(std::chrono::seconds(3)) == std::future_status::ready) {
  int value = future.get();
} else {
  wait.Remove();
}

用法示例2：按key管理的等待管理器单例

using ResultWaitMgrSinglton = AsyncWaitManagerSinglton<int64_t, ResultData>;
auto future = ResultWaitMgrSinglton::GetInstance()->Register(task_id);

在回调线程或工作线程中设置结果：
ResultWaitMgrSinglton::GetInstance()->Complete(task_id, result);

在等待线程中获取结果：
if (future.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
  auto value = future.get();
} else {
  ResultWaitMgrSinglton::GetInstance()->Remove(task_id);
}
*/

/* 单次异步等待对象，适合一个请求对应一个future的场景。 */
template <typename TValue>
class AsyncWait {
 public:
  AsyncWait() = default;
  ~AsyncWait() = default;

  AsyncWait(const AsyncWait&) = delete;
  AsyncWait& operator=(const AsyncWait&) = delete;

  /* 注册等待，返回future。 */
  std::future<TValue> Register() {
    std::lock_guard<std::mutex> lock(mutex_);
    promise_ = std::promise<TValue>();
    is_pending_ = true;
    return promise_.get_future();
  }

  /* 完成等待，设置结果。 */
  bool Complete(const TValue& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!is_pending_) {
      return false;
    }
    promise_.set_value(value);
    is_pending_ = false;
    return true;
  }

  /* 完成等待，设置右值结果。 */
  bool Complete(TValue&& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!is_pending_) {
      return false;
    }
    promise_.set_value(std::move(value));
    is_pending_ = false;
    return true;
  }

  /* 移除等待，通常用于超时清理。 */
  bool Remove() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!is_pending_) {
      return false;
    }
    promise_ = std::promise<TValue>();
    is_pending_ = false;
    return true;
  }

  /* 当前是否仍然处于等待状态。 */
  bool IsPending() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return is_pending_;
  }

 private:
  mutable std::mutex mutex_;
  std::promise<TValue> promise_;
  bool is_pending_ = false;
};

/* 通用异步等待管理器。每个<TKey, TValue>模板特化都会对应一份单例实例。 */
template <typename TKey, typename TValue>
class AsyncWaitManager {
  using Self = AsyncWaitManager<TKey, TValue>;
  friend class yotta::Singleton<Self>;

 public:
  static Self* GetInstance() { return yotta::Singleton<Self>::GetInstance(); }

  /* 注册一个等待项，返回future。 */
  std::future<TValue> Register(const TKey& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::promise<TValue> promise;
    auto future = promise.get_future();
    pending_[key] = std::move(promise);
    return future;
  }

  /* 完成等待，设置结果。 */
  bool Complete(const TKey& key, const TValue& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pending_.find(key);
    if (it == pending_.end()) {
      return false;
    }
    it->second.set_value(value);
    pending_.erase(it);
    return true;
  }

  /* 完成等待，设置右值结果。 */
  bool Complete(const TKey& key, TValue&& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pending_.find(key);
    if (it == pending_.end()) {
      return false;
    }
    it->second.set_value(std::move(value));
    pending_.erase(it);
    return true;
  }

  /* 移除等待项，通常用于超时清理。 */
  bool Remove(const TKey& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return pending_.erase(key) > 0;
  }

  /* 判断等待项是否存在。 */
  bool Contains(const TKey& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pending_.find(key) != pending_.end();
  }

  /* 清空所有等待项。 */
  void Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    pending_.clear();
  }

  /* 完成所有等待项并设置结果。 */
  void CompleteAll(const TValue& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& it : pending_) {
      it.second.set_value(value);
    }
    pending_.clear();
  }

 private:
  AsyncWaitManager() = default;
  ~AsyncWaitManager() = default;

  AsyncWaitManager(const AsyncWaitManager&) = delete;
  AsyncWaitManager& operator=(const AsyncWaitManager&) = delete;

  mutable std::mutex mutex_;
  std::map<TKey, std::promise<TValue>> pending_;
};

template <typename TKey, typename TValue>
using AsyncWaitManagerSinglton =
    yotta::Singleton<AsyncWaitManager<TKey, TValue>>;

#endif  // BASE_UI_SRC_CONTROLLER_ASYNC_WAIT_ASYNC_WAIT_H_
