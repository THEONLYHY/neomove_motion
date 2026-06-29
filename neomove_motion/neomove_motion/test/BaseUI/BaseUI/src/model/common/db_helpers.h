// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#ifndef BASE_UI_SRC_MODEL_COMMON_DB_HELPERS_H_
#define BASE_UI_SRC_MODEL_COMMON_DB_HELPERS_H_

#include <glog/glog_helper.h>
#include <sqlite3/CppSQLite3.h>

#include <functional>
#include <vector>

#include "model/common/db.h"

namespace model_common {

// Simple transaction guard for RAII-style transaction management
class TransactionGuard {
 public:
  explicit TransactionGuard(DBPtr db) : db_(db), committed_(false) {}

  ~TransactionGuard() {
    if (!committed_) {
      try {
        if (db_) {
          db_->execDML("ROLLBACK");
        }
      } catch (...) {
        // Ignore rollback errors in destructor
      }
    }
  }

  int Begin() {
    try {
      if (!db_) return -1;
      db_->execDML("BEGIN TRANSACTION");
      return 0;
    } catch (CppSQLite3Exception& e) {
      LOG(ERROR) << "Failed to begin transaction: " << e.errorMessage();
      return -1;
    }
  }

  int Commit() {
    try {
      if (!db_) return -1;
      db_->execDML("COMMIT");
      committed_ = true;
      return 0;
    } catch (CppSQLite3Exception& e) {
      LOG(ERROR) << "Failed to commit transaction: " << e.errorMessage();
      return -1;
    }
  }

  void Rollback() {
    try {
      if (db_ && !committed_) {
        db_->execDML("ROLLBACK");
        committed_ = true;  // Prevent destructor rollback
      }
    } catch (CppSQLite3Exception& e) {
      LOG(ERROR) << "Failed to rollback transaction: " << e.errorMessage();
    }
  }

 private:
  DBPtr db_;
  bool committed_;
};

// Safe DML execution with error handling
inline int ExecDMLSafe(DBPtr db, const char* sql) {
  if (!db || !sql) {
    LOG(ERROR) << "Invalid parameters for ExecDMLSafe";
    return -1;
  }

  try {
    db->execDML(sql);
    return 0;
  } catch (CppSQLite3Exception& e) {
    LOG(ERROR) << "DML execution failed: " << e.errorMessage();
    return -1;
  }
}

// Batch insert helper template
template <typename T>
int BatchInsert(DBPtr db, const char* sql, const std::vector<T>& items,
                std::function<int(CppSQLite3Statement&, const T&)> bind_func) {
  if (!db || !sql || items.empty()) {
    return 0;  // Empty batch is considered success
  }

  try {
    CppSQLite3Statement stmt = db->compileStatement(sql);

    for (const auto& item : items) {
      int rc = bind_func(stmt, item);
      if (rc != 0) {
        LOG(ERROR) << "Failed to bind parameters for batch insert";
        return -1;
      }
      stmt.execDML();
      stmt.reset();
    }

    return 0;
  } catch (CppSQLite3Exception& e) {
    LOG(ERROR) << "Batch insert failed: " << e.errorMessage();
    return -1;
  }
}

}  // namespace model_common

#endif  // BASE_UI_SRC_MODEL_COMMON_DB_HELPERS_H_
