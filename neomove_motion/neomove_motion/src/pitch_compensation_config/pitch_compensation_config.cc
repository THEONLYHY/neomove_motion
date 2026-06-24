#include "pitch_compensation_config.h"

#include <common/path/path_utils.h>
#include <glog/glog_helper.h>

PitchCompensationConfig::PitchCompensationConfig(int axis_id)
    : axis_id_(axis_id) {
  config_ = std::make_unique<NlohmanJsonConfig>();
  config_path_ = get_config_path();
}

bool PitchCompensationConfig::Init() { return config_->Init(config_path_); }

bool PitchCompensationConfig::Save() { return config_->SaveToFile(); }

std::string PitchCompensationConfig::get_type() const {
  bool success = false;
  std::string type =
      config_->GetString(get_axis_root_path() + "/type", "", &success);
  if (!success) {
    LOG(WARNING) << "Failed to get type for axis " << axis_id_;
  }
  return type;
}

void PitchCompensationConfig::set_type(const std::string& type) {
  if (!config_->SetString(get_axis_root_path() + "/type", type)) {
    LOG(WARNING) << "Failed to set type for axis " << axis_id_;
  }
}

double PitchCompensationConfig::get_origin_position() const {
  bool success = false;
  double value = config_->GetDouble(
      get_axis_root_path() + "/1d/origin_position", 0.0, &success);
  if (!success) {
    LOG(WARNING) << "Failed to get origin position for axis " << axis_id_;
  }
  return value;
}

void PitchCompensationConfig::set_origin_position(double value) {
  if (!config_->SetDouble(get_axis_root_path() + "/1d/origin_position",
                          value)) {
    LOG(WARNING) << "Failed to set origin position for axis " << axis_id_;
  }
}

double PitchCompensationConfig::get_interval() const {
  bool success = false;
  double value =
      config_->GetDouble(get_axis_root_path() + "/1d/interval", 0.0, &success);
  if (!success) {
    LOG(WARNING) << "Failed to get interval for axis " << axis_id_;
  }
  return value;
}

void PitchCompensationConfig::set_interval(double value) {
  if (!config_->SetDouble(get_axis_root_path() + "/1d/interval", value)) {
    LOG(WARNING) << "Failed to set interval for axis " << axis_id_;
  }
}

std::vector<double> PitchCompensationConfig::get_points() const {
  std::vector<double> points;
  bool success = false;
  int count =
      config_->GetArraySize(get_axis_root_path() + "/1d/points", &success);

  if (success && count > 0) {
    for (int i = 0; i < count; i++) {
      std::string path =
          get_axis_root_path() + "/1d/points[" + std::to_string(i) + "]";
      double value = config_->GetDouble(path, 0.0, &success);
      if (success) {
        points.push_back(value);
      }
    }
  }

  return points;
}

void PitchCompensationConfig::set_points(const std::vector<double>& points) {
  // 先清除现有数组
  bool success = false;
  int count =
      config_->GetArraySize(get_axis_root_path() + "/1d/points", &success);
  if (!success || count <= 0) {
    LOG(WARNING) << "Failed to get 1d/points count for axis" << axis_id_;
    return;
  }
  for (int i = 0; i < count; i++) {
    config_->SetDouble(
        get_axis_root_path() + "/1d/points[" + std::to_string(i) + "]", 0);
  }

  // 添加新点
  for (size_t i = 0; i < points.size(); i++) {
    if (!config_->SetDouble(
            get_axis_root_path() + "/1d/points[" + std::to_string(i) + "]",
            points[i])) {
      LOG(WARNING) << "Failed to set point " << i << " for axis " << axis_id_;
    }
  }
}

int PitchCompensationConfig::get_ref_axis_x() const {
  bool success = false;
  int value =
      config_->GetInt(get_axis_root_path() + "/2d/ref_axis_x", 0, &success);
  if (!success) {
    LOG(WARNING) << "Failed to get ref_axis_x for axis " << axis_id_;
  }
  return value;
}

void PitchCompensationConfig::set_ref_axis_x(int axis) {
  if (!config_->SetInt(get_axis_root_path() + "/2d/ref_axis_x", axis)) {
    LOG(WARNING) << "Failed to set ref_axis_x for axis " << axis_id_;
  }
}

int PitchCompensationConfig::get_ref_axis_y() const {
  bool success = false;
  int value =
      config_->GetInt(get_axis_root_path() + "/2d/ref_axis_y", 1, &success);
  if (!success) {
    LOG(WARNING) << "Failed to get ref_axis_y for axis " << axis_id_;
  }
  return value;
}

void PitchCompensationConfig::set_ref_axis_y(int axis) {
  if (!config_->SetInt(get_axis_root_path() + "/2d/ref_axis_y", axis)) {
    LOG(WARNING) << "Failed to set ref_axis_y for axis " << axis_id_;
  }
}

double PitchCompensationConfig::get_origin_position_x() const {
  bool success = false;
  double value = config_->GetDouble(
      get_axis_root_path() + "/2d/origin_position_x", 0.0, &success);
  if (!success) {
    LOG(WARNING) << "Failed to get origin_position_x for axis " << axis_id_;
  }
  return value;
}

void PitchCompensationConfig::set_origin_position_x(double value) {
  if (!config_->SetDouble(get_axis_root_path() + "/2d/origin_position_x",
                          value)) {
    LOG(WARNING) << "Failed to set origin_position_x for axis " << axis_id_;
  }
}

double PitchCompensationConfig::get_origin_position_y() const {
  bool success = false;
  double value = config_->GetDouble(
      get_axis_root_path() + "/2d/origin_position_y", 0.0, &success);
  if (!success) {
    LOG(WARNING) << "Failed to get origin_position_y for axis " << axis_id_;
  }
  return value;
}

void PitchCompensationConfig::set_origin_position_y(double value) {
  if (!config_->SetDouble(get_axis_root_path() + "/2d/origin_position_y",
                          value)) {
    LOG(WARNING) << "Failed to set origin_position_y for axis " << axis_id_;
  }
}

double PitchCompensationConfig::get_interval_x() const {
  bool success = false;
  double value = config_->GetDouble(get_axis_root_path() + "/2d/interval_x",
                                    0.0, &success);
  if (!success) {
    LOG(WARNING) << "Failed to get interval_x for axis " << axis_id_;
  }
  return value;
}

void PitchCompensationConfig::set_interval_x(double value) {
  if (!config_->SetDouble(get_axis_root_path() + "/2d/interval_x", value)) {
    LOG(WARNING) << "Failed to set interval_x for axis " << axis_id_;
  }
}

double PitchCompensationConfig::get_interval_y() const {
  bool success = false;
  double value = config_->GetDouble(get_axis_root_path() + "/2d/interval_y",
                                    0.0, &success);
  if (!success) {
    LOG(WARNING) << "Failed to get interval_y for axis " << axis_id_;
  }
  return value;
}

void PitchCompensationConfig::set_interval_y(double value) {
  if (!config_->SetDouble(get_axis_root_path() + "/2d/interval_y", value)) {
    LOG(WARNING) << "Failed to set interval_y for axis " << axis_id_;
  }
}

int PitchCompensationConfig::get_points_width() const {
  bool success = false;
  int value =
      config_->GetInt(get_axis_root_path() + "/2d/points_width", 0, &success);
  if (!success) {
    LOG(WARNING) << "Failed to get points_width for axis " << axis_id_;
  }
  return value;
}

void PitchCompensationConfig::set_points_width(int width) {
  if (!config_->SetInt(get_axis_root_path() + "/2d/points_width", width)) {
    LOG(WARNING) << "Failed to set points_width for axis " << axis_id_;
  }
}

int PitchCompensationConfig::get_points_height() const {
  bool success = false;
  int value =
      config_->GetInt(get_axis_root_path() + "/2d/points_height", 0, &success);
  if (!success) {
    LOG(WARNING) << "Failed to get points_height for axis " << axis_id_;
  }
  return value;
}

void PitchCompensationConfig::set_points_height(int height) {
  if (!config_->SetInt(get_axis_root_path() + "/2d/points_height", height)) {
    LOG(WARNING) << "Failed to set points_height for axis " << axis_id_;
  }
}

std::vector<std::vector<double>> PitchCompensationConfig::get_points_2d()
    const {
  std::vector<std::vector<double>> points_2d;
  bool success = false;
  int height =
      config_->GetInt(get_axis_root_path() + "/2d/points_height", 0, &success);

  if (!success || height <= 0) return points_2d;

  int width =
      config_->GetInt(get_axis_root_path() + "/2d/points_width", 0, &success);
  if (!success || width <= 0) return points_2d;

  points_2d.reserve(height);

  for (int y = 0; y < height; y++) {
    std::vector<double> row;
    row.reserve(width);

    for (int x = 0; x < width; x++) {
      std::string path = get_axis_root_path() + "/2d/points[" +
                         std::to_string(y) + "][" + std::to_string(x) + "]";
      double value = config_->GetDouble(path, 0.0, &success);
      if (success) {
        row.push_back(value);
      }
    }
    points_2d.push_back(std::move(row));
  }

  return points_2d;
}

void PitchCompensationConfig::set_points_2d(
    const std::vector<std::vector<double>>& points) {
  if (points.empty()) {
    config_->SetInt(get_axis_root_path() + "/2d/points_height", 0);
    config_->SetInt(get_axis_root_path() + "/2d/points_width", 0);
    return;
  }

  const int height = static_cast<int>(points.size());
  const int width = static_cast<int>(points[0].size());

  // 检查行宽一致性
  for (int y = 1; y < height; ++y) {
    if (points[y].size() != static_cast<size_t>(width)) {
      LOG(ERROR) << "Invalid 2D points array: row " << y
                 << " has different size (" << points[y].size()
                 << ") than first row (" << width << ")";
      return;
    }
  }

  // 设置维度
  config_->SetInt(get_axis_root_path() + "/2d/points_height", height);
  config_->SetInt(get_axis_root_path() + "/2d/points_width", width);

  // 设置2D点数据
  for (int y = 0; y < height; ++y) {
    const std::string y_index = "[" + std::to_string(y) + "]";
    for (int x = 0; x < width; ++x) {
      const std::string path = get_axis_root_path() + "/2d/points" + y_index +
                               "[" + std::to_string(x) + "]";
      if (!config_->SetDouble(path, points[y][x])) {
        LOG(WARNING) << "Failed to set point (" << y << "," << x
                     << ") for axis " << axis_id_;
      }
    }
  }
}

bool PitchCompensationConfig::is_enabled() const {
  bool success = false;
  bool enabled =
      config_->GetBool(get_axis_root_path() + "/enabled", false, &success);
  if (!success) {
    LOG(WARNING) << "Failed to get enabled status for axis " << axis_id_;
  }
  return enabled;
}

void PitchCompensationConfig::set_enabled(bool enabled) {
  if (!config_->SetBool(get_axis_root_path() + "/enabled", enabled)) {
    LOG(WARNING) << "Failed to set enabled status for axis " << axis_id_;
  }
}

std::wstring PitchCompensationConfig::get_config_path() const {
  std::wstring config_dir = path_utils::GetCurrentModulePath();
  return config_dir + L"/config/axis_compensation.json";
}

std::string PitchCompensationConfig::get_axis_root_path() const {
  return "axis/" + std::to_string(axis_id_);
}