// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/28 17:13

#ifndef BASE_UI_SRC_MODEL_MODULE_INFO_MODULE_INFO_H_
#define BASE_UI_SRC_MODEL_MODULE_INFO_MODULE_INFO_H_

#include <common/json_config/json_config_helper.h>

#include <atomic>
#include <boost/noncopyable.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

struct ModuleInfo {
  std::string name;
  std::string ids;
  std::string description;
  std::vector<int> camera_id_list;
  std::vector<std::string> camera_name_list;
  std::vector<std::string> task_list;
};

class ModuleInfoMgr : boost::noncopyable,
                      public std::enable_shared_from_this<ModuleInfoMgr> {
 public:
  ModuleInfoMgr();
  ~ModuleInfoMgr();
  bool Init(const std::wstring& file_path);
  int Save();  //不用保存,只读
  bool IsReady() { return ready_; }

  // Getters
  int GetModuleCount();
  ModuleInfo GetModuleInfo(int index);
  ModuleInfo GetModuleInfo(const std::string& ids);

  // Setters
  void AddModuleInfo(ModuleInfo module_info);
  void DeleteModuleInfo(const std::string& ids);

 private:
  int SaveToJson();

  std::wstring file_path_;
  std::atomic_bool ready_ = false;

  std::mutex module_info_list_mutex_;
  std::vector<ModuleInfo> module_info_list_;
  std::unique_ptr<JsonConfig> json_config_;
};

using ModuleInfoMgrPtr = std::shared_ptr<ModuleInfoMgr>;

#endif  // BASE_UI_SRC_MODEL_MODULE_INFO_MODULE_INFO_H_
