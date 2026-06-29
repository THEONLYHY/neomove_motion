// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/07/17 16:15

#ifndef PROBE_STATION8_SRC_MODEL_CALIBRATION_CALIBRATION_PARA_H_
#define PROBE_STATION8_SRC_MODEL_CALIBRATION_CALIBRATION_PARA_H_

#include <common/json_config/json_config_helper.h>
#include <singleton.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

//标定参数
class CalibrationPara {
 public:
  ~CalibrationPara() = default;
  // Getters
  const std::string& ids() const { return ids_; }
  int camera_id() const { return camera_id_; }
  const std::string& axis_x_ids() const { return axis_x_ids_; }
  const std::string& axis_y_ids() const { return axis_y_ids_; }
  const std::string& axis_r_ids() const { return axis_r_ids_; }
  double a() const { return a_; }
  double b() const { return b_; }
  double c() const { return c_; }
  double d() const { return d_; }
  double r_x() const { return r_x_; }
  double r_y() const { return r_y_; }
  double image_mark_x() const { return image_mark_x_; }
  double image_mark_y() const { return image_mark_y_; }
  double image_mark_r() const { return image_mark_r_; }
  double robot_mark_x() const { return robot_mark_x_; }
  double robot_mark_y() const { return robot_mark_y_; }
  double robot_mark_r() const { return robot_mark_r_; }
  double robot2_mark_x() const { return robot2_mark_x_; }
  double robot2_mark_y() const { return robot2_mark_y_; }
  double robot2_mark_r() const { return robot2_mark_r_; }

  // Setter
  void set_ids(const std::string& ids) { ids_ = ids; }
  void set_camera_id(int camera_id) { camera_id_ = camera_id; }
  void set_axis_x_ids(const std::string& axis_x_ids) {
    axis_x_ids_ = axis_x_ids;
  }
  void set_axis_y_ids(const std::string& axis_y_ids) {
    axis_y_ids_ = axis_y_ids;
  }
  void set_axis_r_ids(const std::string& axis_r_ids) {
    axis_r_ids_ = axis_r_ids;
  }
  void set_a(double a) { a_ = a; }
  void set_b(double b) { b_ = b; }
  void set_c(double c) { c_ = c; }
  void set_d(double d) { d_ = d; }
  void set_r_x(double r_x) { r_x_ = r_x; }
  void set_r_y(double r_y) { r_y_ = r_y; }

  void set_image_mark_x(double image_mark_x) { image_mark_x_ = image_mark_x; }
  void set_image_mark_y(double image_mark_y) { image_mark_y_ = image_mark_y; }
  void set_image_mark_r(double image_mark_r) { image_mark_r_ = image_mark_r; }
  void set_robot_mark_x(double robot_mark_x) { robot_mark_x_ = robot_mark_x; }
  void set_robot_mark_y(double robot_mark_y) { robot_mark_y_ = robot_mark_y; }
  void set_robot_mark_r(double robot_mark_r) { robot_mark_r_ = robot_mark_r; }
  void set_robot2_mark_x(double robot2_mark_x) {
    robot2_mark_x_ = robot2_mark_x;
  }
  void set_robot2_mark_y(double robot2_mark_y) {
    robot2_mark_y_ = robot2_mark_y;
  }
  void set_robot2_mark_r(double robot2_mark_r) {
    robot2_mark_r_ = robot2_mark_r;
  }

  // save&write

 private:
  std::string ids_;
  int camera_id_ = 0;

  std::string axis_x_ids_ = "";
  std::string axis_y_ids_ = "";
  std::string axis_r_ids_ = "";

  double a_ = 0;
  double b_ = 0;
  double c_ = 0;
  double d_ = 0;
  double r_x_ = 0;
  double r_y_ = 0;

  double image_mark_x_ = 0;  // mark在图像中心时坐标
  double image_mark_y_ = 0;
  double image_mark_r_ = 0;
  double robot_mark_x_ = 0;  //吸嘴  示教到mark时坐标
  double robot_mark_y_ = 0;
  double robot_mark_r_ = 0;
  double robot2_mark_x_ = 0;  //点胶头 示教到mark时坐标
  double robot2_mark_y_ = 0;
  double robot2_mark_r_ = 0;
};
using CalibrationParaPtr = std::shared_ptr<CalibrationPara>;

//所有标定参数管理
class CalibrationParaMgr {
  SINGLETON(CalibrationParaMgr);

 public:
  CalibrationParaMgr(){};
  ~CalibrationParaMgr(){};
  bool Init(const std::wstring&);
  int Save();

  // Getters
  size_t GetCalibrationParaCount();
  CalibrationParaPtr GetCalibrationPara(size_t idx);
  CalibrationParaPtr GetCalibrationPara(const std::string& ids);

  // Setter
  int AddCalibrationPara(CalibrationParaPtr);
  void DelCalibrationPara(const std::string& ids);

 private:
  std::unique_ptr<JsonConfig> json_config_;
  std::mutex calibration_para_list_mutex_;
  std::vector<CalibrationParaPtr> calibration_para_list_;
};

// 对外都使用shared_ptr
using CalibrationParaMgrPtr = std::shared_ptr<CalibrationParaMgr>;
// using CalibrationParaMgrSinglton = yotta::Singleton<CalibrationParaMgr>;

#endif  // PROBE_STATION8_SRC_MODEL_CALIBRATION_CALIBRATION_PARA_H_
