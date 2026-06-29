// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/28 17:13

#include "module_info.h"

#include <glog/glog_helper.h>

#include <QFile>
#include <QString>
#include <fstream>

ModuleInfoMgr::ModuleInfoMgr() : ready_(false) {}

ModuleInfoMgr::~ModuleInfoMgr() {}

bool ModuleInfoMgr::Init(const std::wstring& file_path) {
  file_path_ = file_path;

  json_config_.reset(new NlohmanJsonConfig);
  bool success = json_config_->Init(file_path);
  if (!success) {
    LOG(ERROR) << L"打开module文件失败: " << file_path;
    ready_ = false;
    return ready_;
  }

  std::lock_guard<std::mutex> lock(module_info_list_mutex_);
  module_info_list_.clear();  // 清空原有数据

  // 读取模块信息
  int module_size = json_config_->GetArraySize("modules", nullptr);
  for (int i = 0; i < module_size; ++i) {
    bool module_success = true;
    std::string prefix_key = "modules[" + std::to_string(i) + "]";

    ModuleInfo module_info;

    // 读取模块基本信息
    module_info.name =
        json_config_->GetString(prefix_key + "/name", "", &module_success);
    module_info.ids =
        json_config_->GetString(prefix_key + "/ids", "", &module_success);
    module_info.description =
        json_config_->GetString(prefix_key + "/description", "", nullptr);

    // 读取相机名称列表
    int camera_size =
        json_config_->GetArraySize(prefix_key + "/init/camera", nullptr);
    for (int j = 0; j < camera_size; ++j) {
      std::string camera_name_key =
          prefix_key + "/init/camera[" + std::to_string(j) + "]/name";
      std::string camera_name =
          json_config_->GetString(camera_name_key, "", nullptr);
      if (!camera_name.empty()) {
        module_info.camera_name_list.push_back(camera_name);
      }

      std::string camera_id_key =
          prefix_key + "/init/camera[" + std::to_string(j) + "]/id";
      int camera_id = json_config_->GetInt(camera_id_key, 0, nullptr);
      module_info.camera_id_list.push_back(camera_id);
    }

    // 读取任务列表
    int task_size = json_config_->GetArraySize(prefix_key + "/tasks", nullptr);
    for (int k = 0; k < task_size; ++k) {
      std::string task_key =
          prefix_key + "/tasks[" + std::to_string(k) + "]/ids";
      std::string task_ids = json_config_->GetString(task_key, "", nullptr);
      if (!task_ids.empty()) {
        module_info.task_list.push_back(task_ids);
      }
    }

    if (!module_success) {
      LOG(WARNING) << "解析模块信息[" << i << "]失败";
      continue;
    }

    module_info_list_.push_back(module_info);
  }

  ready_ = true;
  return true;
}

int ModuleInfoMgr::Save() {
  //不需要保存
  // return SaveToJson();
  return 0;
}

int ModuleInfoMgr::GetModuleCount() {
  std::lock_guard<std::mutex> lock(module_info_list_mutex_);
  return static_cast<int>(module_info_list_.size());
}

ModuleInfo ModuleInfoMgr::GetModuleInfo(int index) {
  std::lock_guard<std::mutex> lock(module_info_list_mutex_);
  if (index < 0 || index >= static_cast<int>(module_info_list_.size())) {
    LOG(ERROR) << "GetModuleInfo error : An out-of-bounds error has occurred";
    return ModuleInfo();
  }
  return module_info_list_[index];
}

ModuleInfo ModuleInfoMgr::GetModuleInfo(const std::string& ids) {
  std::lock_guard<std::mutex> lock(module_info_list_mutex_);
  for (size_t i = 0; i < module_info_list_.size(); i++) {
    if (ids == module_info_list_[i].ids) {
      return module_info_list_[i];
    }
  }
  LOG(WARNING) << "GetModuleInfo error : No this module :" << ids;
  return ModuleInfo();
}

int ModuleInfoMgr::SaveToJson() {
  // 打开文件并清空内容
  std::ofstream file_stream(file_path_, std::ios::out | std::ios::trunc);
  if (!file_stream.is_open()) {
    LOG(ERROR) << "打开文件失败 " << file_path_;
    return -1;
  }
  file_stream << "{\n}";
  file_stream.close();

  // 清空json
  json_config_.reset(new NlohmanJsonConfig());
  bool success = json_config_->Init(file_path_);
  if (!success) {
    LOG(ERROR) << "重新初始化JSON配置失败";
    return -1;
  }

  // 保存模块信息数组
  std::lock_guard<std::mutex> lock(module_info_list_mutex_);
  for (size_t i = 0; i < module_info_list_.size(); ++i) {
    const ModuleInfo& module_info = module_info_list_[i];
    std::string prefix_key = "modules[" + std::to_string(i) + "]";

    // 保存基本信息
    json_config_->SetString(prefix_key + "/name", module_info.name);
    json_config_->SetString(prefix_key + "/ids", module_info.ids);
    json_config_->SetString(prefix_key + "/description",
                            module_info.description);

    // 保存相机名称列表
    size_t camera_count = module_info.camera_name_list.size() <
                                  module_info.camera_id_list.size()
                              ? module_info.camera_name_list.size()
                              : module_info.camera_id_list.size();
    for (size_t j = 0; j < camera_count; ++j) {
      std::string camera_key =
          prefix_key + "/init/camera[" + std::to_string(j) + "]/name";
      json_config_->SetString(camera_key, module_info.camera_name_list[j]);
      camera_key = prefix_key + "/init/camera[" + std::to_string(j) + "]/id";
      json_config_->SetInt(camera_key, module_info.camera_id_list[j]);
    }

    // 保存任务列表
    for (size_t k = 0; k < module_info.task_list.size(); ++k) {
      std::string task_key =
          prefix_key + "/tasks[" + std::to_string(k) + "]/ids";
      json_config_->SetString(task_key, module_info.task_list[k]);
    }
  }

  json_config_->SaveToFile();
  return 0;
}

void ModuleInfoMgr::AddModuleInfo(ModuleInfo module_info) {
  {
    std::lock_guard<std::mutex> lock(module_info_list_mutex_);
    bool updated = false;
    for (size_t i = 0; i < module_info_list_.size(); ++i) {
      if (module_info_list_[i].ids == module_info.ids) {
        module_info_list_[i] = module_info;
        updated = true;
        break;
      }
    }
    if (!updated) {
      module_info_list_.push_back(module_info);
    }
  }
  SaveToJson();
}
void ModuleInfoMgr::DeleteModuleInfo(const std::string& ids) {
  bool removed = false;
  {
    std::lock_guard<std::mutex> lock(module_info_list_mutex_);
    for (auto iter = module_info_list_.begin();
         iter != module_info_list_.end();) {
      if ((*iter).ids == ids) {
        iter = module_info_list_.erase(iter);
        removed = true;
        break;
      } else {
        ++iter;
      }
    }
  }
  if (removed) {
    SaveToJson();
  }
}
