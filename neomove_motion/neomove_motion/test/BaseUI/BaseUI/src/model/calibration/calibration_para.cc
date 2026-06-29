// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/07/17 16:15

#include "calibration_para.h"

#include <common/path/path_utils.h>
#include <glog/glog_helper.h>

#include <fstream>
#include <nlohmann/json.hpp>

bool CalibrationParaMgr::Init(
    const std::wstring& calibration_para_config_path) {
  std::lock_guard<std::mutex> lock(calibration_para_list_mutex_);
  calibration_para_list_.clear();

  nlohmann::json root_json;
  try {
    std::ifstream json_file(calibration_para_config_path);
    if (!json_file.is_open()) {
      std::ostringstream oss;
      LOG(ERROR) << "can not open calibration_para json file: "
                 << calibration_para_config_path;
      return false;
    }

    json_file >> root_json;
  } catch (const nlohmann::json::exception& e) {
    LOG(ERROR) << "JSON parse error: " << e.what();
    return false;
  } catch (const std::exception& e) {
    LOG(ERROR) << "read json error: " << e.what();
    return false;
  }

  try {
    nlohmann::json calibration_para_array = root_json["calibration_list"];
    for (nlohmann::json& calibration_para : calibration_para_array) {
      std::string calibration_para_ids = calibration_para["calibration_ids"];
      LOG(INFO) << "calibration_para_ids : " << calibration_para_ids;
      CalibrationParaPtr calibration_para_ptr =
          std::make_shared<CalibrationPara>();
      calibration_para_ptr->set_ids(calibration_para_ids);
      calibration_para_ptr->set_camera_id(calibration_para["camera_id"]);

      // axis ids 兼容：优先读取字符串 ids，不存在时兼容旧整型 id
      calibration_para_ptr->set_axis_x_ids(
          calibration_para["axis_x_ids"].get<std::string>());
      calibration_para_ptr->set_axis_y_ids(
          calibration_para["axis_y_ids"].get<std::string>());
      calibration_para_ptr->set_axis_r_ids(
          calibration_para["axis_r_ids"].get<std::string>());

      calibration_para_ptr->set_a(calibration_para["a"]);
      calibration_para_ptr->set_b(calibration_para["b"]);
      calibration_para_ptr->set_c(calibration_para["c"]);
      calibration_para_ptr->set_d(calibration_para["d"]);
      calibration_para_ptr->set_r_x(calibration_para["r_x"]);
      calibration_para_ptr->set_r_y(calibration_para["r_y"]);

      calibration_para_ptr->set_image_mark_x(calibration_para["image_mark_x"]);
      calibration_para_ptr->set_image_mark_y(calibration_para["image_mark_y"]);
      calibration_para_ptr->set_image_mark_r(calibration_para["image_mark_r"]);
      calibration_para_ptr->set_robot_mark_x(calibration_para["robot_mark_x"]);
      calibration_para_ptr->set_robot_mark_y(calibration_para["robot_mark_y"]);
      calibration_para_ptr->set_robot_mark_r(calibration_para["robot_mark_r"]);

      calibration_para_list_.push_back(calibration_para_ptr);
    }
  } catch (const nlohmann::json::exception& e) {
    LOG(ERROR) << "JSON parse error: " << e.what();
    return false;
  }
  return true;
}
int CalibrationParaMgr::Save() {
  std::wstring json_path =
      path_utils::GetFullPathFromCurrentExe(L"config\\calibration.json");
  std::ofstream file_stream(json_path, std::ios::out | std::ios::trunc);
  if (!file_stream.is_open()) {
    LOG(ERROR) << "打开文件失败 " << json_path;
    return -1;
  }
  file_stream << "{\n}";
  file_stream.close();

  json_config_.reset(new NlohmanJsonConfig);
  bool success = json_config_->Init(json_path);

  for (int i = 0; i < calibration_para_list_.size(); i++) {
    CalibrationParaPtr calibration_para = calibration_para_list_[i];
    std::string base_key = "calibration_list[" + std::to_string(i) + "]";

    json_config_->SetString(base_key + "/calibration_ids",
                            calibration_para->ids());
    json_config_->SetInt(base_key + "/camera_id",
                         calibration_para->camera_id());

    // 写入字符串 ids
    json_config_->SetString(base_key + "/axis_x_ids",
                            calibration_para->axis_x_ids());
    json_config_->SetString(base_key + "/axis_y_ids",
                            calibration_para->axis_y_ids());
    json_config_->SetString(base_key + "/axis_r_ids",
                            calibration_para->axis_r_ids());

    json_config_->SetDouble(base_key + "/a", calibration_para->a());
    json_config_->SetDouble(base_key + "/b", calibration_para->b());
    json_config_->SetDouble(base_key + "/c", calibration_para->c());
    json_config_->SetDouble(base_key + "/d", calibration_para->d());
    json_config_->SetDouble(base_key + "/r_x", calibration_para->r_x());
    json_config_->SetDouble(base_key + "/r_y", calibration_para->r_y());

    json_config_->SetDouble(base_key + "/image_mark_x",
                            calibration_para->image_mark_x());
    json_config_->SetDouble(base_key + "/image_mark_y",
                            calibration_para->image_mark_y());
    json_config_->SetDouble(base_key + "/image_mark_r",
                            calibration_para->image_mark_r());
    json_config_->SetDouble(base_key + "/robot_mark_x",
                            calibration_para->robot_mark_x());
    json_config_->SetDouble(base_key + "/robot_mark_y",
                            calibration_para->robot_mark_y());
    json_config_->SetDouble(base_key + "/robot_mark_r",
                            calibration_para->robot_mark_r());
    json_config_->SetDouble(base_key + "/robot2_mark_x",
                            calibration_para->robot2_mark_x());
    json_config_->SetDouble(base_key + "/robot2_mark_y",
                            calibration_para->robot2_mark_y());
    json_config_->SetDouble(base_key + "/robot2_mark_r",
                            calibration_para->robot2_mark_r());
  }

  json_config_->SaveToFile();
  return 0;
}

size_t CalibrationParaMgr::GetCalibrationParaCount() {
  std::lock_guard<std::mutex> lock(calibration_para_list_mutex_);
  return calibration_para_list_.size();
}
CalibrationParaPtr CalibrationParaMgr::GetCalibrationPara(size_t idx) {
  std::lock_guard<std::mutex> lock(calibration_para_list_mutex_);
  if (idx >= calibration_para_list_.size()) {
    return nullptr;
  }
  return calibration_para_list_[idx];
}
CalibrationParaPtr CalibrationParaMgr::GetCalibrationPara(
    const std::string& ids) {
  std::lock_guard<std::mutex> lock(calibration_para_list_mutex_);
  for (CalibrationParaPtr& task_info : calibration_para_list_) {
    if (task_info->ids() == ids) {
      return task_info;
    }
  }
  return nullptr;
}

void CalibrationParaMgr::DelCalibrationPara(const std::string& ids) {
  std::lock_guard<std::mutex> lock(calibration_para_list_mutex_);
  for (auto it = calibration_para_list_.begin();
       it != calibration_para_list_.end(); ++it) {
    if ((*it)->ids() == ids) {
      calibration_para_list_.erase(it);
      LOG(INFO) << "delete calibration_para: id=" << ids;
      break;
    }
  }
}
int CalibrationParaMgr::AddCalibrationPara(
    CalibrationParaPtr calibration_para) {
  std::lock_guard<std::mutex> lock(calibration_para_list_mutex_);
  for (int i = 0; i < calibration_para_list_.size(); i++) {
    if (calibration_para_list_[i]->ids() == calibration_para->ids()) {
      calibration_para_list_[i] = calibration_para;
      return 0;
    }
  }
  // if (GetCalibrationPara(calibration_para->ids()) != nullptr) {
  //  DelCalibrationPara(calibration_para->ids());
  //  AddCalibrationPara(calibration_para);
  //  //LOG(ERROR) << "calibration_para already exist: id="
  //  //           << calibration_para->ids();
  //  return 1;
  //}
  calibration_para_list_.push_back(calibration_para);
  return 0;
}