// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/05/26 14:09

#include "calibration_alg.h"

#include <glog/glog_helper.h>

CalibrationAlg::CalibrationAlg() {
  calibration_para_ptr_ = std::make_shared<CalibrationPara>();
  calibration_alg_ptr_ = std::make_shared<AlgCalibration>();
}
//标定
void CalibrationAlg::SetPointData(std::vector<Point4> points) {
  std::lock_guard<std::mutex> lock(point_list_mutex_);
  point_list_.clear();
  point_list_ = points;
}
void CalibrationAlg::AddPoint(Point4 point) {
  std::lock_guard<std::mutex> lock(point_list_mutex_);
  point_list_.push_back(point);
}
void CalibrationAlg::CreactCalibrationPara() {
  calibration_alg_ptr_->ClearPoint();
  for (int i = 0; i < point_list_.size(); i++) {
    calibration_alg_ptr_->AddPoint(point_list_[i].dXUm, point_list_[i].dYUm,
                                   point_list_[i].dCircleXIndex,
                                   point_list_[i].dCircleYIndex);
  }
  double a = 0;
  double b = 0;
  double c = 0;
  double d = 0;
  calibration_alg_ptr_->CalibrationData(&a, &b, &c, &d);
  calibration_para_ptr_->set_a(a);
  calibration_para_ptr_->set_b(b);
  calibration_para_ptr_->set_c(c);
  calibration_para_ptr_->set_d(d);
}
CalibrationParaPtr CalibrationAlg::GetCalibrationPara() {
  return calibration_para_ptr_;
}
//执行
void CalibrationAlg::SetCalibrationPara(
    CalibrationParaPtr calibration_para_ptr) {
  calibration_para_ptr_ = calibration_para_ptr;
  if (!calibration_para_ptr_) {
    LOG(ERROR) << "标定数据为空";
    return;
  }

  calibration_alg_ptr_->SetCalibrationData(
      calibration_para_ptr_->a(), calibration_para_ptr_->b(),
      calibration_para_ptr_->c(), calibration_para_ptr_->d());
}
void CalibrationAlg::Calibration(int type, double m_dMarkX, double m_dMarkY,
                                 double m_dMarkR, double& dOffsetX,
                                 double& dOffsetY, double& dOffsetR) {
  if (!calibration_para_ptr_) {
    LOG(ERROR) << "标定数据为空";
    return;
  }
  if (type == 0) {
    double dImageOffsetX = /*calibration_para_ptr_->image_mark_x() -*/ m_dMarkX;
    double dImageOffsetY = /*calibration_para_ptr_->image_mark_y() -*/ m_dMarkY;
    double dImageOffsetR = /*calibration_para_ptr_->image_mark_r() -*/ m_dMarkR;
    calibration_alg_ptr_->RobotCoordinateOffset(dImageOffsetX, dImageOffsetY,
                                                &dOffsetX, &dOffsetY);
    dOffsetR = dImageOffsetR;
  }
}
