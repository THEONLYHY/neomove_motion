// copyright 2025 YottaImage. All rights reserved.
// author panming
// date 2025/05/09 10:50
#include "model/model_mgr.h"

#include <common/path/path_utils.h>
#include <glog/glog_helper.h>

#include <filesystem>

bool ModelMgr::Init(ModelMgrReadyCallback callback) {
  return InitOnIOThread(callback);
}

bool ModelMgr::InitOnIOThread(ModelMgrReadyCallback callback) {
  if (!db_) {
    db_ = std::make_shared<CppSQLite3DB>();
    try {
      std::wstring db_file_name =
          path_utils::GetFullPathFromCurrentExe(L"config\\BaseUI.db");
      std::filesystem::create_directories(
          std::filesystem::path(db_file_name).parent_path());
      db_->open(db_file_name.c_str());
      db_->setBusyTimeout(5000);
      db_->execDML("PRAGMA foreign_keys = ON;");
      db_->execDML("PRAGMA journal_mode = WAL;");
      db_->execDML("PRAGMA synchronous = FULL;");
    } catch (CppSQLite3Exception& e) {
      LOG(ERROR) << "CppSQLite3Exception: " << e.errorMessage();
      return false;
    } catch (const std::exception& e) {
      LOG(ERROR) << "ModelMgr init error: " << e.what();
      return false;
    }
  }

  if (!user_mgr_->Init(db_)) {
    LOG(ERROR) << "初始化UserInfo失败!";
    if (callback) {
      callback(ModelType::kModelUserManage, false);
    }
  }
  if (!RealTimeDataSinglton::GetInstance()->Init(db_)) {
    LOG(ERROR) << "初始化RealTimeData失败!";
    if (callback) {
      callback(ModelType::kModelRealTimeData, false);
    }
  }
  if (!camera_config_->Init(path_utils::GetFullPathFromCurrentExe(
          L"config\\camera_config.json"))) {
    LOG(WARNING) << "初始化camera_config失败!";
  }
  if (!module_info_mgr_->Init(
          path_utils::GetFullPathFromCurrentExe(L"config\\module.json"))) {
    LOG(WARNING) << "初始化module_info_mgr失败!";
  }
  if (!unit_info_mgr_->Init(
          path_utils::GetFullPathFromCurrentExe(L"config\\unit.json"))) {
    LOG(WARNING) << "初始化unit_info_mgr失败!";
  }
  if (!unit_point_mgr_->Init(
          path_utils::GetFullPathFromCurrentExe(L"config\\unit_point.json"))) {
    LOG(WARNING) << "初始化unit_point_mgr失败!";
  }
  if (!calibration_mgr_->Init(path_utils::GetFullPathFromCurrentExe(
          L"config\\calibration.json"))) {
    LOG(WARNING) << "初始化calibration_info失败!";
  }

  if (callback) {
    callback(ModelType::kModelUndefine, true);
  }
  return true;
}
