// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/30 15:28

#ifndef PROBE_STATION8_SRC_ALG_ALG_CALIBRATION_H_
#define PROBE_STATION8_SRC_ALG_ALG_CALIBRATION_H_

#include <math.h>
#include <memory.h>
#include <stdio.h>

#include <QString>
#include <algorithm>
#include <thread>
#include <vector>

#include "simple_matrix.h"

#define DiamondNum 5
#define EdgeNum 4
#define GrayImageDepth 1
#define RbgImageDepth 4
#define maxContrast(a, b) ((a) > (b) ? (a) : (b))

#define nMinMark 5

class AlgCalibration {
 public:
  AlgCalibration(QString strPath = QString::fromLocal8Bit("标定数据.txt"));
  ~AlgCalibration();
  void AddPoint(const double, const double, const double, const double);
  void ClearPoint();
  int CalibrationData(double*, double*, double*, double*);
  int SetCalibrationData(const double, const double, const double,
                         const double);
  int SetCalibrationData(QString);
  int RobotCoordinateOffset(double, double, double*, double*);
  int CalibrationMatrixFromTxt(QString, double*, double*, double*, double*);

 private:
  int SaveMatrixToTxt();

 private:
  bool is_calibrated_{false};
  int point_count_{0};

  std::vector<double> calibration_points_;
  std::vector<double> calibration_params_;

  SimpleMatrixCalc matrix_calc_;
  SimpleMatrix calibration_matrix_;

  QString full_path_;

  double offset_um_x_{0};
  double offset_um_y_{0};
};

#endif  // PROBE_STATION8_SRC_ALG_ALG_CALIBRATION_H_
