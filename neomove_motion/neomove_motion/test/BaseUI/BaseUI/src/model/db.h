// copyright 2025 YottaImage. All rights reserved.
// author panming
// date 2025/05/09 14:35
#ifndef BASE_UI_SRC_MODEL_DB_H_
#define BASE_UI_SRC_MODEL_DB_H_

#include <sqlite3/CppSQLite3.h>
#include <sqlite3/sqlite3_helper.h>

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>

using DBPtr = std::shared_ptr<CppSQLite3DB>;
using DBWeakPtr = std::weak_ptr<CppSQLite3DB>;

// RAII 事务，支持嵌套（使用 SAVEPOINT），默认 BEGIN IMMEDIATE。
// 若未显式 Commit，析构时将自动回滚；Commit 成功后析构不回滚。
class ScopedTransaction {
 public:
  explicit ScopedTransaction(DBPtr db, bool immediate = true)
      : db_(std::move(db)) {
    if (!db_) {
      throw std::runtime_error("ScopedTransaction: null db");
    }
    // 事务嵌套深度：线程内计数
    depth_ = ++DepthRef();
    if (depth_ == 1) {
      db_->execDML(immediate ? "BEGIN IMMEDIATE;" : "BEGIN;");
    } else {
      savepoint_name_ = MakeSavepointName(depth_);
      db_->execDML(("SAVEPOINT " + savepoint_name_ + ";").c_str());
    }
    active_ = true;
  }

  ScopedTransaction(const ScopedTransaction&) = delete;
  ScopedTransaction& operator=(const ScopedTransaction&) = delete;

  ~ScopedTransaction() {
    // 始终减少嵌套深度
    if (depth_ > 0) {
      if (active_) {
        // 未提交则回滚
        try {
          if (depth_ == 1) {
            db_->execDML("ROLLBACK;");
          } else {
            // 回滚到本 SAVEPOINT 并释放
            db_->execDML(("ROLLBACK TO " + savepoint_name_ + ";").c_str());
            db_->execDML(("RELEASE " + savepoint_name_ + ";").c_str());
          }
        } catch (...) {
          // 构析期吞掉异常
        }
      }
      // 减少深度
      int& d = DepthRef();
      if (d > 0) --d;
    }
  }

  void Commit() {
    if (!active_) return;
    if (depth_ == 1) {
      db_->execDML("COMMIT;");
    } else {
      db_->execDML(("RELEASE " + savepoint_name_ + ";").c_str());
    }
    active_ = false;
  }

 private:
  static std::string MakeSavepointName(int depth) {
    return std::string("sp_txn_") + std::to_string(depth);
  }
  static int& DepthRef() {
    static thread_local int depth = 0;
    return depth;
  }

  DBPtr db_;
  bool active_ = false;
  int depth_ = 0;
  std::string savepoint_name_;
};

// ------------------------------
// 通用重试工具
// ------------------------------
namespace detail {
inline bool IsBusyOrLocked(int code) {
  return code == SQLITE_BUSY || code == SQLITE_LOCKED;
}
}  // namespace detail

// 非 void 返回值模板
template <typename Fn, typename R = typename std::result_of<Fn&()>::type,
          typename std::enable_if<!std::is_void<R>::value, int>::type = 0>
auto RetryOnBusy(Fn&& fn, int max_retries = 3, int initial_delay_ms = 50,
                 double backoff = 2.0) -> R {
  int attempt = 0;
  int delay = initial_delay_ms;
  for (;;) {
    try {
      return fn();
    } catch (CppSQLite3Exception& e) {
      if (attempt >= max_retries || !detail::IsBusyOrLocked(e.errorCode())) {
        throw;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(delay));
      ++attempt;
      delay = static_cast<int>(delay * backoff);
      if (delay <= 0) delay = initial_delay_ms;  // 溢出保护
      continue;
    }
  }
}

// void 返回值模板
template <typename Fn, typename R = typename std::result_of<Fn&()>::type,
          typename std::enable_if<std::is_void<R>::value, int>::type = 0>
void RetryOnBusy(Fn&& fn, int max_retries = 3, int initial_delay_ms = 50,
                 double backoff = 2.0) {
  int attempt = 0;
  int delay = initial_delay_ms;
  for (;;) {
    try {
      fn();
      return;
    } catch (CppSQLite3Exception& e) {
      if (attempt >= max_retries || !detail::IsBusyOrLocked(e.errorCode())) {
        throw;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(delay));
      ++attempt;
      delay = static_cast<int>(delay * backoff);
      if (delay <= 0) delay = initial_delay_ms;
      continue;
    }
  }
}

#endif  // BASE_UI_SRC_MODEL_DB_H_
