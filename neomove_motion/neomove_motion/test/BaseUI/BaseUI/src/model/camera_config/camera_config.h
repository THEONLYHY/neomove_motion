// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/28 17:13

#ifndef BASE_UI_SRC_MODEL_CAMERA_CONFIG_CAMERA_CONFIG_H_
#define BASE_UI_SRC_MODEL_CAMERA_CONFIG_CAMERA_CONFIG_H_

#include <common/json_config/json_config_helper.h>

#include <atomic>
#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "base.h"

struct CameraParaConfig {
  std::string config_ids;
  int id = 0;
  double exposure_time = 500;
  double gain = 1;
  int frame_rate = 10;
  int frame_count = 1;
  yotta::RectInt roi;
  int reverse_image = 0;
  int trigger_type = 0;
  int image_type = 2;
  std::string high_low = "high";
};

std::string CameraConfigToJson(const CameraParaConfig& camera_config);

class CameraConfigMgr : boost::noncopyable,
                        public std::enable_shared_from_this<CameraConfigMgr> {
 public:
  CameraConfigMgr(){};
  ~CameraConfigMgr(){};

  bool Init(const std::wstring& file_path);
  int Save();
  bool IsReady(void) { return ready_; }

 public:
  int GetCameraInfoCount();
  void AddCameraInfo(CameraParaConfig);
  CameraParaConfig GetCameraInfo(int index);
  CameraParaConfig GetCameraInfoByIds(std::string config_ids);

 private:
  std::wstring file_path_;
  std::atomic_bool ready_ = false;
  std::mutex camera_info_list_mutex_;
  std::vector<CameraParaConfig> camera_info_list_;
  std::unique_ptr<JsonConfig> json_config_;
};

using CameraConfigMgrPtr = std::shared_ptr<CameraConfigMgr>;

#endif  // BASE_UI_SRC_MODEL_CAMERA_CONFIG_CAMERA_CONFIG_H_
