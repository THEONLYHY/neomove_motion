// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/05/30 11:24

#include "model/user_manage/user_manage.h"

#include <common/message_loop.h>
#include <glog/glog_helper.h>

int UserInfo::check_password(QString password) {
  QString password_md5 = QString(
      QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5)
          .toHex());
  return (password_ == password_md5) ? 0 : 1;
}

int UserInfo::ChangePassword(QString new_password) {
  password_ = QString(
      QCryptographicHash::hash(new_password.toUtf8(), QCryptographicHash::Md5)
          .toHex());
  return 0;
}
bool UserManager::Init(DBWeakPtr db) {
  weak_db_ = db;
  try {
    DBPtr db = weak_db_.lock();
    db->execDML(
        "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT "
        "UNIQUE, password TEXT, level INTEGER);");
  } catch (CppSQLite3Exception& e) {
    LOG(ERROR) << "Database error: " << e.errorMessage();
    return false;
  }
  InitUser();
  LoadUsersFromDatabase();
  return true;
}

void UserManager::LoadUsersFromDatabase() {
  try {
    users_.clear();
    DBPtr db = weak_db_.lock();
    CppSQLite3Query q =
        db->execQuery("SELECT id, name, password, level FROM users;");
    while (!q.eof()) {
      // 创建对象并添加到users_列表中
      users_.emplace_back(new UserInfo(
          q.getIntField(0), q.getStringField(1, ""), q.getStringField(2, ""),
          static_cast<UserGroupLevel>(q.getIntField(3))));
      q.nextRow();
    }
  } catch (CppSQLite3Exception& e) {
    LOG(WARNING) << "Load users error: " << e.errorMessage();
  }
}

int UserManager::DeleteUser(int user_id) {
  if (GetUser(user_id) == nullptr) {
    return 1;
  }
  {
    std::lock_guard<std::mutex> lock(users_mutex_);
    for (int i = 0; i < users_.size(); i++) {
      if (users_[i]->user_id() == user_id) {
        users_.erase(users_.begin() + i);
        break;
      }
    }
    // users_.remove_if(
    //    [user_id](User* user) { return user->user_id() == user_id; });
  }
  common::MessageLoop::GetMessageLoop(common::kIo)
      ->PostTask(
          std::bind(&UserManager::DeleteUserInDB, shared_from_this(), user_id));
  return 0;
}

int UserManager::DeleteUserInDB(const int& user_id) {
  try {
    // 删除用户
    DBPtr db = weak_db_.lock();
    if (!db) {
      LOG(ERROR) << "DB instance is null";
      return 1;
    }
    CppSQLite3Statement stmt =
        db->compileStatement("DELETE FROM users WHERE id=?;");
    stmt.bind(1, user_id);
    stmt.execDML();
    LOG(INFO) << "delete user id=" << user_id;
    return 0;
  } catch (CppSQLite3Exception& e) {
    LOG(WARNING) << "Delete user error: " << e.errorMessage();
    return 2;
  }
}

User* UserManager::GetUser(const QString& user_name) {
  std::lock_guard<std::mutex> lock(users_mutex_);
  for (const UserInfoPtr& user : users_) {
    if (user->user_name() == user_name) {
      return user.get();
    }
  }
  return nullptr;
}

User* UserManager::GetUser(int user_id) {
  std::lock_guard<std::mutex> lock(users_mutex_);
  for (const UserInfoPtr& user : users_) {
    if (user->user_id() == user_id) {
      return user.get();
    }
  }
  return nullptr;
}

std::list<User*> UserManager::user_list() {
  std::list<User*> list;
  for (UserInfoPtr iter : users_) {
    list.push_back(iter.get());
  }
  return list;
}

User* UserManager::AddUser(const QString& new_user_name,
                           const QString& new_user_password,
                           const UserGroupLevel& new_user_level) {
  if (GetUser(new_user_name) != nullptr) {
    return nullptr;
  }
  int new_user_id = GetUserCount() + 1;
  UserInfoPtr new_user = std::make_shared<UserInfo>(
      new_user_id, new_user_name, new_user_password, new_user_level);
  {
    std::lock_guard<std::mutex> lock(users_mutex_);
    users_.push_back(new_user);
  }
  common::MessageLoop::GetMessageLoop(common::kIo)
      ->PostTask(std::bind(&UserManager::AddUserInDB, shared_from_this(),
                           new_user_name, new_user_password, new_user_level,
                           new_user_id));
  return new_user.get();
}

int UserManager::AddUserInDB(const QString& new_user_name,
                             const QString& new_user_password,
                             const UserGroupLevel& new_user_level, int id) {
  try {
    DBPtr db = weak_db_.lock();
    if (!db) {
      LOG(ERROR) << "DB instance is null";
      return 1;
    }
    // 插入新用户
    QString password_md5 =
        QString(QCryptographicHash::hash(new_user_password.toUtf8(),
                                         QCryptographicHash::Md5)
                    .toHex());
    CppSQLite3Statement stmt = db->compileStatement(
        "INSERT INTO users (id,name, password, level) VALUES (?,?,?,?);");
    stmt.bind(1, id);
    stmt.bind(2, new_user_name.toUtf8().constData());
    stmt.bind(3, password_md5.toUtf8().constData());
    stmt.bind64(4, static_cast<sqlite3_int64>(new_user_level));
    stmt.execDML();
    return 0;
  } catch (CppSQLite3Exception& e) {
    LOG(WARNING) << "Add user error: " << e.errorMessage();
    return 2;
  }
}

int UserManager::ModifyUser(User* user, const QString& new_user_name,
                            const QString& new_password) {
  if (GetUser(new_user_name) == nullptr) {
    return 1;
  }
  {
    std::lock_guard<std::mutex> lock(users_mutex_);
    // auto modified_user = std::find_if(
    //    users_.begin(), users_.end(),
    //    [user](User*& u) { return u->user_id() == user->user_id(); });
    // if (modified_user != users_.end()) {
    //  modified_user modified_user->ChangeUserName(new_user_name);
    //  modified_user->ChangePassword(new_password);
    //}
    for (int i = 0; i < users_.size(); i++) {
      if (users_[i]->user_id() == user->user_id()) {
        users_[i]->ChangeUserName(new_user_name);
        users_[i]->ChangePassword(new_password);
      }
    }
  }
  common::MessageLoop::GetMessageLoop(common::kIo)
      ->PostTask(std::bind(&UserManager::ModifyUserInDB, shared_from_this(),
                           user, new_user_name, new_password));
  return 0;
}

int UserManager::ModifyUserInDB(User* user, const QString& new_user_name,
                                const QString& new_password) {
  try {
    DBPtr db = weak_db_.lock();
    if (!db) {
      LOG(ERROR) << "DB instance is null";
      return 1;
    }
    ScopedTransaction txn(db, true);
    // 更新用户信息
    QString new_password_md5 = QString(
        QCryptographicHash::hash(new_password.toUtf8(), QCryptographicHash::Md5)
            .toHex());
    CppSQLite3Statement stmt =
        db->compileStatement("UPDATE users SET name=?, password=? WHERE id=?;");
    stmt.bind(1, new_user_name.toUtf8().constData());
    stmt.bind(2, new_password_md5.toUtf8().constData());
    stmt.bind(3, static_cast<int>(user->user_id()));
    stmt.execDML();
    txn.Commit();
    return 0;
  } catch (CppSQLite3Exception& e) {
    LOG(WARNING) << "Modify user error: " << e.errorMessage();
    return 2;
  }
}

User* UserManager::GetCurrentUser() { return current_user_; }

void UserManager::SetCurrentUser(const QString& user_name) {
  User* user = GetUser(user_name);
  current_user_ = user;
}

int UserManager::GetUserCount() {
  std::lock_guard<std::mutex> lock(users_mutex_);
  return users_.size();
}

void UserManager::InitUser() {
  try {
    DBPtr db = weak_db_.lock();
    if (!db) {
      LOG(ERROR) << "DB instance is null";
      return;
    }

    // 1. 检查表中是否有数据
    CppSQLite3Query q = db->execQuery("SELECT * FROM users;");

    if (q.eof()) {
      // 2. 表为空，插入初始数据
      QString password_md5 = QString(
          QCryptographicHash::hash("000000", QCryptographicHash::Md5).toHex());
      CppSQLite3Statement stmt = db->compileStatement(
          "INSERT INTO users (id, name, password,level) VALUES (?, ?, ?, ?), "
          "(?, ?, ?, ?),  (?, ?, ?, ?);");
      int idx = 1;
      stmt.bind(idx++, 1);
      stmt.bind(idx++, "factory");
      stmt.bind(idx++, password_md5.toUtf8().constData());
      stmt.bind(idx++, kL3SU1);

      stmt.bind(idx++, 2);
      stmt.bind(idx++, "engineer");
      stmt.bind(idx++, password_md5.toUtf8().constData());
      stmt.bind(idx++, kL2EG);

      stmt.bind(idx++, 3);
      stmt.bind(idx++, "operator");
      stmt.bind(idx++, password_md5.toUtf8().constData());
      stmt.bind(idx++, kL1OP);
      stmt.execDML();
    }
    return;
  } catch (CppSQLite3Exception& e) {
    LOG(WARNING) << "Init user error: " << e.errorMessage();
    return;
  }
}
