// copyright 2025 YottaImage. All rights reserved.
// Database collection manager base class for common CRUD operations
// Template base class for managers that handle collections of database records

#ifndef BASE_UI_SRC_MODEL_COMMON_COLLECTION_DB_MANAGER_H_
#define BASE_UI_SRC_MODEL_COMMON_COLLECTION_DB_MANAGER_H_

#include <glog/glog_helper.h>
#include <sqlite3/CppSQLite3.h>
#include <sqlite3/sqlite3_helper.h>

#include <atomic>
#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "model/common/db.h"
#include "model/common/db_helpers.h"

namespace model_common {

// Base class for database managers that handle collections of items
// T: The item type (e.g., Consumable, Recipe, etc.)
// ItemPtr: Shared pointer to the item type
template <typename T>
class CollectionDbManager
    : boost::noncopyable,
      public std::enable_shared_from_this<CollectionDbManager<T>> {
 public:
  using ItemType = T;
  using ItemPtr = std::shared_ptr<T>;
  using ItemList = std::vector<ItemPtr>;

  explicit CollectionDbManager(const std::string& table_name)
      : table_name_(table_name), ready_(false) {}

  virtual ~CollectionDbManager() = default;

  // Common interface for all collection managers
  bool Init(DBWeakPtr db) {
    weak_db_ = db;

    try {
      InitDB();
      DBPtr locked_db = weak_db_.lock();
      if (!locked_db) {
        LOG(ERROR) << "DB instance is null!";
        return false;
      }

      // Load all records from database
      std::string select_sql = "SELECT * FROM " + table_name_;
      CppSQLite3Query query = locked_db->execQuery(select_sql.c_str());

      {
        std::lock_guard<std::mutex> lock(items_mutex_);
        items_.clear();

        while (!query.eof()) {
          ItemPtr item = CreateItemFromQuery(query);
          if (item) {
            items_.push_back(item);
          }
          query.nextRow();
        }
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

  bool IsReady() const { return ready_; }

  // Collection operations
  size_t GetCount() {
    std::lock_guard<std::mutex> lock(items_mutex_);
    return items_.size();
  }

  ItemPtr GetItem(size_t idx) {
    std::lock_guard<std::mutex> lock(items_mutex_);
    if (idx >= items_.size()) {
      return nullptr;
    }
    return items_[idx];
  }

  ItemPtr GetItem(const std::string& id) {
    std::lock_guard<std::mutex> lock(items_mutex_);
    for (ItemPtr& item : items_) {
      if (GetIdFromItem(item) == id) {
        return item;
      }
    }
    return nullptr;
  }

  int AddItem(ItemPtr item) {
    if (!item) {
      LOG(ERROR) << "Cannot add null item to " << table_name_;
      return -1;
    }

    std::lock_guard<std::mutex> lock(items_mutex_);

    // Check if item already exists and update if so
    std::string item_id = GetIdFromItem(item);
    for (size_t i = 0; i < items_.size(); ++i) {
      if (GetIdFromItem(items_[i]) == item_id) {
        items_[i] = item;
        return 0;
      }
    }

    // Add new item
    items_.push_back(item);
    return 0;
  }

  void DeleteItem(const std::string& id) {
    std::lock_guard<std::mutex> lock(items_mutex_);
    for (auto it = items_.begin(); it != items_.end(); ++it) {
      if (GetIdFromItem(*it) == id) {
        items_.erase(it);
        break;
      }
    }
  }

 protected:
  // Template methods that subclasses must implement
  virtual std::string GetCreateTableSQL() = 0;
  virtual std::string GetInsertSQL() = 0;
  virtual int BindItemToStatement(CppSQLite3Statement& stmt, ItemPtr item) = 0;
  virtual ItemPtr CreateItemFromQuery(CppSQLite3Query& query) = 0;
  virtual std::string GetIdFromItem(ItemPtr item) = 0;

  // Protected members accessible by derived classes
  std::string table_name_;
  DBWeakPtr weak_db_;
  std::atomic_bool ready_;
  std::mutex items_mutex_;
  ItemList items_;

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

      // Batch insert current list
      ItemList items_copy;
      {
        std::lock_guard<std::mutex> lock(items_mutex_);
        items_copy = items_;
      }

      if (!items_copy.empty()) {
        std::string insert_sql = GetInsertSQL();
        rc = model_common::BatchInsert<ItemPtr>(
            db, insert_sql.c_str(), items_copy,
            [this](CppSQLite3Statement& st, const ItemPtr& item) {
              return BindItemToStatement(st, item);
            });
        if (rc != 0) {
          tx.Rollback();
          return -1;
        }
      }

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
};

}  // namespace model_common

#endif  // BASE_UI_SRC_MODEL_COMMON_COLLECTION_DB_MANAGER_H_
