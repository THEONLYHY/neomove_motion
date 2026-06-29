// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/05/30 11:24

#ifndef BASE_UI_SRC_MODEL_USER_MANAGE_USER_MANAGE_H_
#define BASE_UI_SRC_MODEL_USER_MANAGE_USER_MANAGE_H_

#include <sqlite3/CppSQLite3.h>
#include <sqlite3/sqlite3_helper.h>

#include <QCryptographicHash>
#include <list>
#include <memory>
#include <mutex>
#include <vector>

#include "model/db.h"
#include "yotta_qt_plugin/user_manager/user_manager.h"
#include "yotta_qt_plugin/yotta_qt_plugin_define.h"

class UserInfo : public User {
 public:
  UserInfo(int id, QString name, QString password, UserGroupLevel level)
      : id_(id), name_(name), password_(password), level_(level) {}

  int user_id() const override { return id_; }
  QString user_name() const override { return name_; }
  UserGroupLevel user_group_level() const override { return level_; }
  bool check_group_level(UserGroupLevel level) override {
    return level_ >= level;
  }  // 判断权限是否可以

  int check_password(QString password) override;  //判断密码是否正确
  int ChangeUserName(QString new_user_name) override {
    name_ = new_user_name;
    return 0;
  }
  int ChangePassword(QString new_password) override;

 private:
  int id_;
  QString name_;
  QString password_;
  UserGroupLevel level_;
};

using UserInfoPtr = std::shared_ptr<UserInfo>;

class UserManager : public UserDataMgr,
                    public std::enable_shared_from_this<UserManager> {
 public:
  UserManager() = default;

  std::list<User*> user_list() override;
  User* AddUser(const QString& new_user_name, const QString& new_user_password,
                const UserGroupLevel& new_user_level) override;
  int ModifyUser(User* user, const QString& new_user_name,
                 const QString& new_password) override;
  User* GetUser(const QString& user_name) override;
  User* GetUser(int user_id) override;

  // 设置/获取当前用户
  User* GetCurrentUser();
  void SetCurrentUser(const QString& user_name);
  int GetUserCount();
  int DeleteUser(int user_id) override;
  void LoadUsersFromDatabase();
  bool Init(DBWeakPtr db);

 private:
  int DeleteUserInDB(const int& user_id);
  int AddUserInDB(const QString& new_user_name,
                  const QString& new_user_password,
                  const UserGroupLevel& new_user_level, int id);
  int ModifyUserInDB(User* user, const QString& new_user_name,
                     const QString& new_password);
  void InitUser();  // 初始化用户

 private:
  DBWeakPtr weak_db_;
  std::vector<UserInfoPtr> users_;
  std::mutex users_mutex_;
  User* current_user_ = nullptr;
};

using UserManagerPtr = std::shared_ptr<UserManager>;

#endif  // BASE_UI_SRC_MODEL_USER_MANAGE_USER_MANAGE_H_
