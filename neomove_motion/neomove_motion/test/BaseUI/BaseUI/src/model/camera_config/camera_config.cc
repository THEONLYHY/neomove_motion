// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/28 17:13

#include "camera_config.h"

#include <common/json_config/json_config_helper.h>
#include <common/message_loop.h>
#include <common/path/path_utils.h>
#include <glog/glog_helper.h>

#include <QFile>
#include <QString>
#include <fstream>
#include <regex>

namespace {

void FillCameraHighLow(CameraParaConfig* camera_info) {
  if (!camera_info) {
    return;
  }

  if (camera_info->config_ids.find("高倍") != std::string::npos) {
    camera_info->high_low = "high";
    return;
  }

  if (camera_info->config_ids.find("低倍") != std::string::npos) {
    camera_info->high_low = "low";
  }
}

}  // namespace

std::string CameraConfigToJson(const CameraParaConfig& camera_config) {
  std::string camera_config_str =
      R"({ "id":)" + std::to_string(camera_config.id) + R"(,"exposure_time":)" +
      std::to_string(camera_config.exposure_time) + R"(,"gain":)" +
      std::to_string(camera_config.gain) + R"(,"frame_rate":)" +
      std::to_string(camera_config.frame_rate) + R"(,"reverse_image":)" +
      std::to_string(camera_config.reverse_image) + R"(,"trigger_type":)" +
      std::to_string(camera_config.trigger_type) + R"(,"roi":{"bottom":)" +
      std::to_string(camera_config.roi.bottom) + R"(,"left":)" +
      std::to_string(camera_config.roi.left) + R"(,"right":)" +
      std::to_string(camera_config.roi.right) + R"(,"top":)" +
      std::to_string(camera_config.roi.top) + "}}";
  return camera_config_str;
}

bool CameraConfigMgr::Init(const std::wstring& file_path) {
  file_path_ = file_path;
  // 初始化配置文件
  QFile file(QString::fromStdWString(file_path));
  if (!file.exists()) {
    // 文件不存在
    // 创建文件并初始化数据
    std::ofstream camera_info_json(file_path);
    camera_info_json.close();

    std::lock_guard<std::mutex> lock(camera_info_list_mutex_);
    camera_info_list_.clear();
    ready_ = false;
    return ready_;
  }
  // 文件存在
  json_config_.reset(new NlohmanJsonConfig());
  bool json_open = json_config_->Init(file_path);  // 初始化配置文件
  if (!json_open) {
    LOG(ERROR) << L"打开camera_config文件失败: " << file_path;
    std::lock_guard<std::mutex> lock(camera_info_list_mutex_);
    camera_info_list_.clear();
    ready_ = false;
    return ready_;
  }
  bool success = false;
  // 读取数据
  int camera_size = json_config_->GetArraySize("camera_configs", &success);
  if (success) {
    // 加锁
    std::lock_guard<std::mutex> lock(camera_info_list_mutex_);
    camera_info_list_.clear();
    camera_info_list_.reserve(camera_size);
    for (int i = 0; i < camera_size; ++i) {
      CameraParaConfig tmp_camera_info;
      std::string base_key = "camera_configs[" + std::to_string(i) + "]";
      // 读取基本数据
      tmp_camera_info.config_ids =
          json_config_->GetString(base_key + "/config_ids", "", nullptr);
      tmp_camera_info.id = json_config_->GetInt(base_key + "/id", -1, nullptr);
      // tmp_camera_info.serial_number =
      //    json_config_->GetString(base_key + "/serial_number", "", nullptr);
      // tmp_camera_info.name = json_config_->GetString(base_key + "/name", "",
      // nullptr); tmp_camera_info.type = json_config_->GetString(base_key +
      // "/type", "", nullptr);
      tmp_camera_info.exposure_time =
          json_config_->GetDouble(base_key + "/exposure_time", -1.0, nullptr);
      tmp_camera_info.gain =
          json_config_->GetDouble(base_key + "/gain", -1.0, nullptr);
      tmp_camera_info.frame_rate =
          json_config_->GetInt(base_key + "/frame_rate", -1, nullptr);
      tmp_camera_info.frame_count =
          json_config_->GetInt(base_key + "/frame_count", -1, nullptr);
      // tmp_camera_info.image_node_number =
      //    json_config_->GetInt(base_key + "/image_node_number", -1, nullptr);
      tmp_camera_info.reverse_image =
          json_config_->GetInt(base_key + "/reverse_image", -1, nullptr);
      tmp_camera_info.trigger_type =
          json_config_->GetInt(base_key + "/trigger_type", -1, nullptr);
      tmp_camera_info.image_type =
          json_config_->GetInt(base_key + "/image_type", -1, nullptr);

      tmp_camera_info.roi.left =
          json_config_->GetInt(base_key + "/roi/left", -1, nullptr);
      tmp_camera_info.roi.top =
          json_config_->GetInt(base_key + "/roi/top", -1, nullptr);
      tmp_camera_info.roi.right =
          json_config_->GetInt(base_key + "/roi/right", -1, nullptr);
      tmp_camera_info.roi.bottom =
          json_config_->GetInt(base_key + "/roi/bottom", -1, nullptr);
      FillCameraHighLow(&tmp_camera_info);

      // if (tmp_camera_info.type == "virtual_folder") {
      //  tmp_camera_info.virtual_folder.directory = json_config_->GetString(
      //      base_key + "/virtual_folder/directory", "", nullptr);
      //  tmp_camera_info.virtual_folder.image_extension =
      //  json_config_->GetString(
      //      base_key + "/virtual_folder/image_extension", "", nullptr);
      //  tmp_camera_info.virtual_folder.loop =
      //      json_config_->GetBool(base_key + "/virtual_folder/loop", true,
      //      nullptr);
      //}
      camera_info_list_.push_back(tmp_camera_info);
    }
  }
  ready_ = success;
  return ready_;
}
int CameraConfigMgr::Save() {
  std::wstring json_path = file_path_.empty()
                               ? path_utils::GetFullPathFromCurrentExe(
                                     L"config\\camera_config.json")
                               : file_path_;

  std::ofstream file_stream(json_path, std::ios::out | std::ios::trunc);
  if (!file_stream.is_open()) {
    LOG(ERROR) << "打开文件失败 " << json_path;
    return -1;
  }
  file_stream << "{\n}";
  file_stream.close();

  json_config_.reset(new NlohmanJsonConfig);
  bool success = json_config_->Init(json_path);
  if (!success) {
    LOG(ERROR) << "重新初始化camera_config失败";
    return -1;
  }

  std::lock_guard<std::mutex> lock(camera_info_list_mutex_);
  for (size_t i = 0; i < camera_info_list_.size(); i++) {
    std::string title = "camera_configs[" + std::to_string(i) + "]/config_ids";
    json_config_->SetString(title, camera_info_list_[i].config_ids);
    title = "camera_configs[" + std::to_string(i) + "]/id";
    json_config_->SetInt(title, camera_info_list_[i].id);
    title = "camera_configs[" + std::to_string(i) + "]/serial_number";
    // json_config_->SetString(title, camera_info_list_[i].serial_number);
    // title = "camera_configs[" + std::to_string(i) + "]/name";
    // json_config_->SetString(title, camera_info_list_[i].name);
    // title = "camera_configs[" + std::to_string(i) + "]/type";
    // json_config_->SetString(title, camera_info_list_[i].type);
    title = "camera_configs[" + std::to_string(i) + "]/exposure_time";
    json_config_->SetDouble(title, camera_info_list_[i].exposure_time);
    title = "camera_configs[" + std::to_string(i) + "]/gain";
    json_config_->SetDouble(title, camera_info_list_[i].gain);
    title = "camera_configs[" + std::to_string(i) + "]/frame_rate";
    json_config_->SetInt(title, camera_info_list_[i].frame_rate);
    title = "camera_configs[" + std::to_string(i) + "]/frame_count";
    json_config_->SetInt(title, camera_info_list_[i].frame_count);
    // title = "camera_configs[" + std::to_string(i) + "]/image_node_number";
    // json_config_->SetInt(title, camera_info_list_[i].image_node_number);
    title = "camera_configs[" + std::to_string(i) + "]/reverse_image";
    json_config_->SetInt(title, camera_info_list_[i].reverse_image);
    title = "camera_configs[" + std::to_string(i) + "]/trigger_type";
    json_config_->SetInt(title, camera_info_list_[i].trigger_type);
    title = "camera_configs[" + std::to_string(i) + "]/image_type";
    json_config_->SetInt(title, camera_info_list_[i].image_type);

    title = "camera_configs[" + std::to_string(i) + "]/roi/left";
    json_config_->SetInt(title, camera_info_list_[i].roi.left);
    title = "camera_configs[" + std::to_string(i) + "]/roi/top";
    json_config_->SetInt(title, camera_info_list_[i].roi.top);
    title = "camera_configs[" + std::to_string(i) + "]/roi/right";
    json_config_->SetInt(title, camera_info_list_[i].roi.right);
    title = "camera_configs[" + std::to_string(i) + "]/roi/bottom";
    json_config_->SetInt(title, camera_info_list_[i].roi.bottom);
  }

  json_config_->SaveToFile();
  return 0;
}

int CameraConfigMgr::GetCameraInfoCount() {
  std::lock_guard<std::mutex> lock(camera_info_list_mutex_);
  return static_cast<int>(camera_info_list_.size());
}
CameraParaConfig CameraConfigMgr::GetCameraInfo(int index) {
  std::lock_guard<std::mutex> lock(camera_info_list_mutex_);
  if (index < 0 || index >= static_cast<int>(camera_info_list_.size())) {
    LOG(ERROR) << "GetCameraInfo Error index = " << index;
    return CameraParaConfig();
  }
  CameraParaConfig camera_info = camera_info_list_[index];
  FillCameraHighLow(&camera_info);
  return camera_info;
}
CameraParaConfig CameraConfigMgr::GetCameraInfoByIds(std::string config_ids) {
  std::lock_guard<std::mutex> lock(camera_info_list_mutex_);
  for (size_t i = 0; i < camera_info_list_.size(); i++) {
    if (config_ids == camera_info_list_[i].config_ids) {
      CameraParaConfig camera_info = camera_info_list_[i];
      FillCameraHighLow(&camera_info);
      return camera_info;
    }
  }
  return CameraParaConfig();
}

void CameraConfigMgr::AddCameraInfo(CameraParaConfig camera_info) {
  FillCameraHighLow(&camera_info);
  std::lock_guard<std::mutex> lock(camera_info_list_mutex_);
  for (size_t i = 0; i < camera_info_list_.size(); i++) {
    if (camera_info_list_[i].config_ids == camera_info.config_ids) {
      camera_info_list_[i] = camera_info;
      // Save();
      return;
    }
  }
  camera_info_list_.push_back(camera_info);
  // Save();
}
