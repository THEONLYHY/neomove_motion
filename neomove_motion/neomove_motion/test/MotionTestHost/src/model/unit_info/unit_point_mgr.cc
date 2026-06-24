#include "unit_point_mgr.h"

#include <common/message_loop.h>
#include <glog/glog_helper.h>

#include <QFile>
#include <QString>
#include <fstream>
#include <regex>

//#include <common/message_loop.h>
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
      if (!point_success) {
        LOG(WARNING) << "解析点位信息[" << i << "]失败";
        continue;
      }
      // std::string point_key = prefix_key + "/point";
      // 轴信息
      // int axis_size = json_config_->GetArraySize(point_key, nullptr);
      // point.axis_ids.reserve(axis_size);
      // for (int j = 0; j < axis_size; ++j) {
      //  std::string item_key = point_key + "[" + std::to_string(j) + "]";
      //  // int axis_id =
      //  //    json_config_->GetInt(item_key + "/axis_index", 0, nullptr);
      //  // point.axis_index.push_back(axis_id);
      //  double axis_pos =
      //      json_config_->GetDouble(item_key + "/position", 0.0, nullptr);
      //  point.axis_pos.push_back(axis_pos);
      //  std::string axis_name =
      //      json_config_->GetString(item_key + "/ids", "", nullptr);
      //  point.axis_ids.push_back(axis_name);
      //}
      // 将点位信息添加到列表
      vec_point_info_.push_back(point);
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
    int index = idx_map[point.unit_ids]++;
    std::string prefix_key =
        point.unit_ids + "[" + std::to_string(index) + "]/points_ids";
    // 设置点位名称
    json_config_->SetString(prefix_key, point.point_ids);
    // 拼接点位信息数组的key
    // std::string point_key = prefix_key + "/point";
    // for (size_t j = 0; j < point.axis_ids.size(); ++j) {
    //  // 保存轴id、轴名称、轴位置
    //  std::string item_key = point_key + "[" + std::to_string(j) + "]";
    //  json_config_->SetString(item_key + "/ids", point.axis_ids[j]);
    //  json_config_->SetDouble(item_key + "/position", point.axis_pos[j]);
    //}
  }
  return 0;
}

int UnitPointMgr::GetUnitPointCount() {
  std::lock_guard<std::mutex> lock(point_list_mutex_);
  return static_cast<int>(vec_point_info_.size());
}

UnitPoint UnitPointMgr::GetUnitPoint(int index) {
  std::lock_guard<std::mutex> lock(point_list_mutex_);
  if (index < 0 || index > vec_point_info_.size()) {
    LOG(ERROR) << "获取点位失败";
    return UnitPoint();
  }
  return vec_point_info_[index];
}

UnitPoint UnitPointMgr::GetUnitPointByIds(std::string point_ids) {
  std::lock_guard<std::mutex> lock(point_list_mutex_);
  for (int i = 0; i < vec_point_info_.size(); ++i) {
    if (point_ids == vec_point_info_[i].point_ids) {
      return vec_point_info_[i];
    }
  }
  //LOG(ERROR) << "获取点位信息失败，点位名不存在";
  return UnitPoint();
}

void UnitPointMgr::AddUnitPoint(const UnitPoint& value) {
  int point_exist = 0;
  std::lock_guard<std::mutex> lock(point_list_mutex_);
  for (int i = 0; i < vec_point_info_.size(); i++) {
    if (vec_point_info_[i].point_ids == value.point_ids) {
      vec_point_info_[i] = value;
      point_exist = 1;
    }
  }
  if (!point_exist) {
    vec_point_info_.push_back(value);
  }
  SaveToJson();
}

// void UnitPointMgr::AddUnitPoint(int index, const UnitPoint& value) {
//  std::lock_guard<std::mutex> lock(point_list_mutex_);
//  vec_point_info_.insert(vec_point_info_.begin() + index, value);
//  SaveToJson();
//}

void UnitPointMgr::DelUnitPoint(const std::string& point_ids) {
  std::lock_guard<std::mutex> lock(point_list_mutex_);
  for (auto iter = vec_point_info_.begin(); iter != vec_point_info_.end();
       ++iter) {
    if (iter->point_ids == point_ids) {
      iter = vec_point_info_.erase(iter);
      break;
    }
  }
  // 清空并重写json文件达到删除目的
  SaveToJson();
}

// void UnitPointMgr::ModifyUnitPoint(int index, const UnitPoint& value) {
//  std::lock_guard<std::mutex> lock(point_list_mutex_);
//  // 更新vector
//  vec_point_info_[index] = value;
//  // 清空并重写json文件达到删除目的
//  SaveToJson();
//}