// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#include "unit_point_mgr.h"

#include <common/message_loop.h>
#include <glog/glog_helper.h>

#include <QFile>
#include <QString>
#include <algorithm>
#include <fstream>
#include <regex>
#include <unordered_map>

// #include <common/message_loop.h>
#include "model/model_mgr.h"

UnitPointMgr::UnitPointMgr() : ready_(false) {}

bool UnitPointMgr::Init(const std::wstring& file_path) {
  file_path_ = file_path;

  json_config_.reset(new NlohmanJsonConfig);
  bool success = json_config_->Init(file_path);
  if (!success) {
    LOG(ERROR) << L"打开point_info文件失败: " << file_path;
    ready_ = false;
    return ready_;
  }

  std::lock_guard<std::mutex> lock(point_list_mutex_);
  vec_point_info_.clear();  // 清空原有数据
  // 读取其他点信息
  int module_count =
      ModelMgrSinglton::GetInstance()->unit_info_mgr()->GetUnitCount();
  for (int j = 0; j < module_count; j++) {
    std::string unit_ids = ModelMgrSinglton::GetInstance()
                               ->unit_info_mgr()
                               ->GetUnitInfo(j)
                               ->unit_ids;

    int point_size = json_config_->GetArraySize(unit_ids, nullptr);

    for (int i = 0; i < point_size; ++i) {
      UnitPoint point;
      bool point_success = true;
      std::string prefix_key =
          unit_ids + "[" + std::to_string(i) + "]/points_ids";
      // 模块名称
      point.unit_ids = unit_ids;
      // 点名称
      point.point_ids = json_config_->GetString(prefix_key, "", &point_success);
      if (!point_success || point.point_ids.empty()) {
        LOG(WARNING) << "解析点位信息[" << i << "]失败";
        continue;
      }
      const auto duplicated =
          std::any_of(vec_point_info_.begin(), vec_point_info_.end(),
                      [&point](const UnitPoint& item) {
                        return item.point_ids == point.point_ids;
                      });
      if (!duplicated) {
        vec_point_info_.push_back(point);
      } else {
        LOG(WARNING) << "重复点位ID已跳过: " << point.point_ids;
      }
    }
  }
  ready_ = true;
  return true;
}

int UnitPointMgr::SaveToJson() {
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

  std::unordered_map<std::string, int> idx_map;
  // 保存点位信息数组
  for (size_t i = 0; i < vec_point_info_.size(); ++i) {
    const UnitPoint& point = vec_point_info_[i];
    if (point.unit_ids.empty() || point.point_ids.empty()) {
      continue;
    }
    int index = idx_map[point.unit_ids]++;
    std::string prefix_key =
        point.unit_ids + "[" + std::to_string(index) + "]/points_ids";
    // 设置点位名称
    json_config_->SetString(prefix_key, point.point_ids);
  }
  return 0;
}

int UnitPointMgr::GetUnitPointCount(const std::string& unit_ids) {
  std::lock_guard<std::mutex> lock(point_list_mutex_);
  return static_cast<int>(std::count_if(vec_point_info_.begin(),
                                        vec_point_info_.end(),
                                        [&unit_ids](const UnitPoint& point) {
                                          return point.unit_ids == unit_ids;
                                        }));
}

UnitPoint UnitPointMgr::GetUnitPoint(const std::string& unit_ids, int index) {
  std::lock_guard<std::mutex> lock(point_list_mutex_);
  if (index < 0) {
    LOG(ERROR) << "获取点位失败";
    return UnitPoint();
  }
  int unit_index = 0;
  for (const UnitPoint& point : vec_point_info_) {
    if (point.unit_ids != unit_ids) {
      continue;
    }
    if (unit_index == index) {
      return point;
    }
    ++unit_index;
  }

  LOG(ERROR) << "获取点位失败";
  return UnitPoint();
}

bool UnitPointMgr::HasPointIds(const std::string& point_ids) {
  std::lock_guard<std::mutex> lock(point_list_mutex_);
  return std::any_of(vec_point_info_.begin(), vec_point_info_.end(),
                     [&point_ids](const UnitPoint& point) {
                       return point.point_ids == point_ids;
                     });
}

bool UnitPointMgr::HasUnitPoint(const std::string& unit_ids,
                                const std::string& point_ids) {
  std::lock_guard<std::mutex> lock(point_list_mutex_);
  return std::any_of(vec_point_info_.begin(), vec_point_info_.end(),
                     [&unit_ids, &point_ids](const UnitPoint& point) {
                       return point.unit_ids == unit_ids &&
                              point.point_ids == point_ids;
                     });
}

bool UnitPointMgr::AddUnitPoint(const UnitPoint& value) {
  if (value.unit_ids.empty() || value.point_ids.empty()) {
    LOG(WARNING) << "添加点位失败，工位或点位名为空";
    return false;
  }

  std::lock_guard<std::mutex> lock(point_list_mutex_);
  for (UnitPoint& point : vec_point_info_) {
    if (point.point_ids != value.point_ids) {
      continue;
    }

    if (point.unit_ids == value.unit_ids) {
      point = value;
      return SaveToJson() == 0;
    }

    LOG(WARNING) << "添加点位失败，点位ID已存在: " << value.point_ids;
    return false;
  }

  vec_point_info_.push_back(value);
  return SaveToJson() == 0;
}

bool UnitPointMgr::DelUnitPoint(const std::string& point_ids) {
  std::lock_guard<std::mutex> lock(point_list_mutex_);
  for (auto iter = vec_point_info_.begin(); iter != vec_point_info_.end();
       ++iter) {
    if (iter->point_ids == point_ids) {
      iter = vec_point_info_.erase(iter);
      return SaveToJson() == 0;
    }
  }
  return false;
}

// void UnitPointMgr::ModifyUnitPoint(int index, const UnitPoint& value) {
//  std::lock_guard<std::mutex> lock(point_list_mutex_);
//  // 更新vector
//  vec_point_info_[index] = value;
//  // 清空并重写json文件达到删除目的
//  SaveToJson();
//}
