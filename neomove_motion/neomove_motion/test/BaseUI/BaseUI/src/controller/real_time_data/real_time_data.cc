#include "controller/real_time_data/real_time_data.h"

#include <glog/glog_helper.h>

std::string RealTimeData::GetCreateTableSQL() {
  return R"(
    CREATE TABLE IF NOT EXISTS real_time_data (
      current_user TEXT,
      current_user_level INTEGER
    );
  )";
}

std::string RealTimeData::GetInsertSQL() {
  return R"(
    INSERT INTO real_time_data (
      current_user,
      current_user_level
    ) VALUES (
      ?,
      ?
    );
  )";
}

int RealTimeData::BindToStatement(CppSQLite3Statement& stmt) {
  try {
    std::lock_guard<std::mutex> lock(mutex_);
    int index = 1;
    stmt.bind(index++, current_user_.c_str());
    stmt.bind(index++, static_cast<int>(current_user_level_));
    return 0;
  } catch (CppSQLite3Exception& e) {
    LOG(ERROR) << "RealTimeData::BindToStatement: " << e.errorMessage();
    return -1;
  }
}

void RealTimeData::LoadFromQuery(CppSQLite3Query& query) {
  try {
    std::lock_guard<std::mutex> lock(mutex_);
    current_user_ = query.getStringField("current_user", "");
    current_user_level_ =
        static_cast<UserGroupLevel>(query.getIntField("current_user_level",
                                                      static_cast<int>(kL1OP)));
  } catch (CppSQLite3Exception& e) {
    LOG(ERROR) << "RealTimeData::LoadFromQuery: " << e.errorMessage();
  }
}

int RealTimeData::Save() {
  return model_common::ConfigDbManager<RealTimeData>::Save();
}
