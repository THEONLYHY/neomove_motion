// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#ifndef PROBE_STATION8_SRC_VIEW_PAGES_MAINTENANCE_CHANGE_VIEW_CAMERA_CALIBRATION_VIEW_CAMERA_CALIBRATION_VIEW_H_
#define PROBE_STATION8_SRC_VIEW_PAGES_MAINTENANCE_CHANGE_VIEW_CAMERA_CALIBRATION_VIEW_CAMERA_CALIBRATION_VIEW_H_

#include <QPointer>
#include <QWidget>

#include "model/calibration/calibration_para.h"
#include "ui_camera_calibration_view.h"
#include "view/tools/multi_image_show_view/multi_image_show_view.h"
#include "view/tools/robot_xyrz_view/robot_xyrz.h"

class CameraCalibrationView : public QWidget {
  Q_OBJECT

 public:
  CameraCalibrationView(QWidget* parent = nullptr);
  ~CameraCalibrationView();

 private slots:
  void OnCameraCalibrationlListItemClicked(int row, int column);
  void onClickedWAdd();
  void onClickedWSub();
  void onClickedHAdd();
  void onClickedHSub();
  void onClickedClear();
  void onClickedCreacTemplate();
  void onClickedCalibreation();
  void OnBoxChanged(QRect box);
  // void onClickedCameraMark();
  // void onClickedRobotMark();
  // void onClickedRobot2Mark();
 signals:
  void BoxChanged(QRect box);

 protected:
  void showEvent(QShowEvent *event) override;

 private:
  CalibrationParaMgrPtr camera_calibration_para_mgr_;  //所有标定数据
  CalibrationParaPtr camera_calibration_para_;         //当前标定数据

  int image_w_ = 1024;
  int image_h_ = 960;
  int box_width_ = 100;
  int box_height_ = 100;
  int box_step_ = 5;

 private:
  Ui::camera_calibration_viewClass ui;
};

#endif  // PROBE_STATION8_SRC_VIEW_PAGES_MAINTENANCE_CHANGE_VIEW_CAMERA_CALIBRATION_VIEW_CAMERA_CALIBRATION_VIEW_H_
