#ifndef PITCH_COMPENSATION_CONFIG_H
#define PITCH_COMPENSATION_CONFIG_H

#include <common/json_config/json_config_helper.h>
#include <memory>
#include <vector>
#include <string>

// 螺距补偿json文件，放在exe目录/config/axis_compensation.json
class PitchCompensationConfig {
public:
	explicit PitchCompensationConfig(int axis_id);

	bool Init();
	bool Save();

	// 配置类型（"1D" 或 "2D"）
  std::string get_type() const;
  void set_type(const std::string& type);

  // 是否启用
  bool is_enabled() const;
  void set_enabled(bool enabled);

	// 1D补偿参数（axes/[axis_id]/1d/...）
  double get_origin_position() const;
  void set_origin_position(double value);
  double get_interval() const;
  void set_interval(double value);
  std::vector<double> get_points() const;
  void set_points(const std::vector<double>& points);

	// 2D补偿参数（axes/[axis_id]/2d/...）
  int get_ref_axis_x() const;
  void set_ref_axis_x(int axis);
  int get_ref_axis_y() const;
  void set_ref_axis_y(int axis);
  double get_origin_position_x() const;
  void set_origin_position_x(double value);
  double get_origin_position_y() const;
  void set_origin_position_y(double value);
  double get_interval_x() const;
  void set_interval_x(double value);
  double get_interval_y() const;
  void set_interval_y(double value);
  int get_points_width() const;
  void set_points_width(int width);
  int get_points_height() const;
  void set_points_height(int height);
  std::vector<std::vector<double>> get_points_2d() const;
  void set_points_2d(const std::vector<std::vector<double>>& points);

private:
	std::wstring get_config_path() const;
  std::string get_axis_root_path() const;

private:
	int axis_id_;
	std::unique_ptr<JsonConfig> config_;
	std::wstring config_path_;
};

#endif // PITCH_COMPENSATION_CONFIG_H