// copyright 2025 YottaImage. All rights reserved.
// BaseUI generic runtime state.
#ifndef BASE_UI_SRC_CONTROLLER_REAL_TIME_DATA_REAL_TIME_DATA_H_
#define BASE_UI_SRC_CONTROLLER_REAL_TIME_DATA_REAL_TIME_DATA_H_

#include <singleton.h>

#include <map>
#include <mutex>
#include <string>

#include "model/common/config_db_manager.h"
#include "model/user_manage/user_manage.h"

class RealTimeData : public model_common::ConfigDbManager<RealTimeData> {
  SINGLETON(RealTimeData);

 public:
  RealTimeData() : model_common::ConfigDbManager<RealTimeData>("real_time_data") {}
  ~RealTimeData() = default;

  int Save();

  int camera_index() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return camera_index_;
  }

  void set_camera_index(int camera_index) {
    std::lock_guard<std::mutex> lock(mutex_);
    camera_index_ = camera_index;
  }

  int high_low() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return high_low_;
  }

  void set_high_low(int high_low) {
    std::lock_guard<std::mutex> lock(mutex_);
    high_low_ = high_low;
  }

  int is_first_page() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return is_first_page_;
  }

  void set_is_first_page(int is_first_page) {
    std::lock_guard<std::mutex> lock(mutex_);
    is_first_page_ = is_first_page;
  }

  std::string get_calibration_para_ids() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return calibration_para_ids_;
  }

  void set_calibration_para_ids(const std::string& calibration_para_ids) {
    std::lock_guard<std::mutex> lock(mutex_);
    calibration_para_ids_ = calibration_para_ids;
  }

  std::string current_user() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_user_;
  }

  void set_current_user(const std::string& current_user) {
    std::lock_guard<std::mutex> lock(mutex_);
    current_user_ = current_user;
  }

  UserGroupLevel current_user_level() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_user_level_;
  }

  void set_current_user_level(UserGroupLevel current_user_level) {
    std::lock_guard<std::mutex> lock(mutex_);
    current_user_level_ = current_user_level;
  }

  bool get_condition_value(const std::string& condition_ids) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = condition_value_map_.find(condition_ids);
    if (it == condition_value_map_.end()) {
      return false;
    }
    return it->second;
  }

  bool set_condition_value(const std::string& condition_ids, bool value) {
    std::lock_guard<std::mutex> lock(mutex_);
    condition_value_map_[condition_ids] = value;
    return true;
  }

  void clear_condition_values() {
    std::lock_guard<std::mutex> lock(mutex_);
    condition_value_map_.clear();
  }

 protected:
  std::string GetCreateTableSQL() override;
  std::string GetInsertSQL() override;
  int BindToStatement(CppSQLite3Statement& stmt) override;
  void LoadFromQuery(CppSQLite3Query& query) override;

 private:
  mutable std::mutex mutex_;
  int camera_index_ = 0;
  int high_low_ = 0;
  int is_first_page_ = 1;
  std::string calibration_para_ids_;
  std::string current_user_;
  UserGroupLevel current_user_level_ = kL1OP;
  std::map<std::string, bool> condition_value_map_;
};

using RealTimeDataSinglton = yotta::Singleton<RealTimeData>;

#endif  // BASE_UI_SRC_CONTROLLER_REAL_TIME_DATA_REAL_TIME_DATA_H_
