// copyright 2025 YottaImage. All rights reserved.
// Database configuration manager base class for singleton configuration objects
// Template base class for managers that handle single configuration records

#ifndef BASE_UI_SRC_MODEL_COMMON_CONFIG_DB_MANAGER_H_
#define BASE_UI_SRC_MODEL_COMMON_CONFIG_DB_MANAGER_H_

#include <glog/glog_helper.h>
#include <sqlite3/CppSQLite3.h>
#include <sqlite3/sqlite3_helper.h>

#include <atomic>
#include <boost/noncopyable.hpp>
#include <memory>
#include <string>

#include "model/common/db.h"
#include "model/common/db_helpers.h"

namespace model_common {

// Base class for database managers that handle single configuration records
// T: The configuration class type (e.g., SystemParameter, RunState, LotInfo)
template <typename T>
class ConfigDbManager
    : boost::noncopyable,
      public std::enable_shared_from_this<ConfigDbManager<T>> {
 public:
  explicit ConfigDbManager(const std::string& table_name)
      : table_name_(table_name), ready_(false) {}

  virtual ~ConfigDbManager() = default;

  // Common interface for all configuration managers
  bool Init(DBWeakPtr db) {
    weak_db_ = db;

    try {
      InitDB();
      DBPtr locked_db = weak_db_.lock();
      if (!locked_db) {
        LOG(ERROR) << "DB instance is null!";
        return false;
      }

      // Load single record from database
      std::string select_sql = "SELECT * FROM " + table_name_ + " LIMIT 1";
      CppSQLite3Query query = locked_db->execQuery(select_sql.c_str());

      if (!query.eof()) {
        LoadFromQuery(query);
      }

    } catch (CppSQLite3Exception& e) {
      LOG(ERROR) << "SQLite error in " << table_name_
                 << " Init: " << e.errorMessage();
      return false;
    }

    ready_ = true;
    LOG(INFO) << table_name_ << " manager init success";
    return true;
  }

  int Save() { return SaveToDB(); }

  void Reload() {
    DBPtr locked_db = weak_db_.lock();
    if (!locked_db) {
      LOG(ERROR) << "DB instance is null in " << table_name_ << " Reload";
      return;
    }
    try {
      std::string select_sql = "SELECT * FROM " + table_name_ + " LIMIT 1";
      CppSQLite3Query query = locked_db->execQuery(select_sql.c_str());
      if (!query.eof()) {
        LoadFromQuery(query);
      }
    } catch (CppSQLite3Exception& e) {
      LOG(ERROR) << "SQLite error in " << table_name_
                 << " Reload: " << e.errorMessage();
    }
  }

  bool IsReady() const { return ready_; }

 protected:
  // Template methods that subclasses must implement
  virtual std::string GetCreateTableSQL() = 0;
  virtual std::string GetInsertSQL() = 0;
  virtual int BindToStatement(CppSQLite3Statement& stmt) = 0;
  virtual void LoadFromQuery(CppSQLite3Query& query) = 0;

 private:
  void InitDB() {
    DBPtr db = weak_db_.lock();
    if (!db) {
      LOG(ERROR) << "DB instance is null!";
      return;
    }

    std::string create_sql = GetCreateTableSQL();
    db->execDML(create_sql.c_str());
  }

  int SaveToDB() {
    DBPtr db = weak_db_.lock();
    if (!db) {
      LOG(ERROR) << "Failed to get database pointer in " << table_name_
                 << " Save";
      return -1;
    }

    model_common::TransactionGuard tx(db);
    if (tx.Begin() != 0) return -1;

    try {
      // Clear table first
      std::string delete_sql = "DELETE FROM " + table_name_;
      int rc = model_common::ExecDMLSafe(db, delete_sql.c_str());
      if (rc != 0) {
        tx.Rollback();
        return -1;
      }

      // Insert current configuration
      CppSQLite3Statement stmt = db->compileStatement(GetInsertSQL().c_str());
      rc = BindToStatement(stmt);
      if (rc != 0) {
        tx.Rollback();
        return -1;
      }
      stmt.execDML();

    } catch (CppSQLite3Exception& e) {
      LOG(ERROR) << "SQLite error in " << table_name_
                 << " Save: " << e.errorMessage();
      tx.Rollback();
      return -1;
    }

    int commit_rc = tx.Commit();
    if (commit_rc == 0) {
      LOG(INFO) << "Save " << table_name_ << " To DB success";
    }
    return commit_rc;
  }

  std::string table_name_;
  DBWeakPtr weak_db_;
  std::atomic_bool ready_;
};

}  // namespace model_common

#endif  // BASE_UI_SRC_MODEL_COMMON_CONFIG_DB_MANAGER_H_
