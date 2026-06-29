// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/30 15:28

#include "alg_calibration.h"
#define RgbImageDepth 3
#define MaxGrayValue 255
#define bDebug true
#define circleAngle 360
#include <QFile>
#include <QTextStream>

const QString RectSeparate = QString(" ");

AlgCalibration::AlgCalibration(QString strPath) {
  point_count_ = 0;

  calibration_matrix_.SetM(2);
  calibration_matrix_.SetN(2);
  calibration_matrix_.InitMatrix();

  full_path_ = strPath;
}
AlgCalibration::~AlgCalibration() { calibration_matrix_.FreeMatrix(); }

void AlgCalibration::AddPoint(const double dXUm, const double dYUm,
                              const double dCircleXIndex,
                              const double dCircleYIndex) {
  calibration_points_.push_back(dXUm);
  calibration_points_.push_back(dYUm);
  calibration_points_.push_back(dCircleXIndex);
  calibration_points_.push_back(dCircleYIndex);
  ++point_count_;
}
void AlgCalibration::ClearPoint() {
  if (!calibration_points_.empty()) {
    calibration_points_.erase(calibration_points_.begin(),
                                calibration_points_.end());
  }
  point_count_ = 0;
}
int AlgCalibration::CalibrationData(double* pdA, double* pdB, double* pdC,
                                    double* pdD) {
  // if (point_count_ < nMinMark)
  //{
  //  return 1;
  //}

  int nTempWidth = 4;
  SimpleMatrix matrixA(point_count_ * 2, nTempWidth);
  matrixA.InitMatrix();
  SimpleMatrix matrixB(point_count_ * 2, 1);
  matrixB.InitMatrix();
  for (int i = 0; i < point_count_; ++i) {
    double dXUm = calibration_points_[i * 4] - calibration_points_[0];
    double dYUm = calibration_points_[i * 4 + 1] - calibration_points_[1];
    double dCircleXIndex =
        calibration_points_[i * 4 + 2] - calibration_points_[2];
    double dCircleYIndex =
        calibration_points_[i * 4 + 3] - calibration_points_[3];

    int nTempIndex = -1;
    matrixA.Write(2 * i, ++nTempIndex, dCircleXIndex);
    matrixA.Write(2 * i + 1, nTempIndex, 0);

    matrixA.Write(2 * i, ++nTempIndex, dCircleYIndex);
    matrixA.Write(2 * i + 1, nTempIndex, 0);

    matrixA.Write(2 * i, ++nTempIndex, 0);
    matrixA.Write(2 * i + 1, nTempIndex, dCircleXIndex);

    matrixA.Write(2 * i, ++nTempIndex, 0);
    matrixA.Write(2 * i + 1, nTempIndex, dCircleYIndex);

    matrixB.Write(2 * i, 0, dXUm);
    matrixB.Write(2 * i + 1, 0, dYUm);
  }
  // PrintMatrix(&matrixA);
  // PrintMatrix(&matrixB);
  SimpleMatrixCalc matrix_calc;
  SimpleMatrix matrixAT(nTempWidth, point_count_ * 2);
  matrixAT.InitMatrix();
  matrix_calc.Transpose(&matrixA, &matrixAT);

  SimpleMatrix matrixLeft(nTempWidth, nTempWidth);
  matrixLeft.InitMatrix();
  matrix_calc.Multiply(&matrixAT, &matrixA, &matrixLeft);

  SimpleMatrix matrixRight(nTempWidth, 1);
  matrixRight.InitMatrix();
  matrix_calc.Multiply(&matrixAT, &matrixB, &matrixRight);

  SimpleMatrix matrixInvLeft(nTempWidth, nTempWidth);
  matrixInvLeft.InitMatrix();
  matrix_calc.Inverse(&matrixLeft, &matrixInvLeft);

  SimpleMatrix matrixMid(nTempWidth, 1);
  matrixMid.InitMatrix();
  matrix_calc.Multiply(&matrixInvLeft, &matrixRight, &matrixMid);

  if (!calibration_params_.empty()) {
    calibration_params_.erase(calibration_params_.begin(),
                               calibration_params_.end());
  }
  QString matrix_data_str = QString();
  for (int i = 0; i < nTempWidth; ++i) {
    double dTemp = matrixMid.Read(i, 0);
    matrix_data_str += (QString::number(dTemp) + RectSeparate);
    calibration_params_.push_back(dTemp);
  }
  matrix_data_str += ("\r\n");

  matrixA.FreeMatrix();
  matrixB.FreeMatrix();
  matrixLeft.FreeMatrix();
  matrixRight.FreeMatrix();
  matrixInvLeft.FreeMatrix();
  matrixMid.FreeMatrix();

  int nIndex = 0;
  *pdA = calibration_params_[nIndex++];
  *pdB = calibration_params_[nIndex++];
  *pdC = calibration_params_[nIndex++];
  *pdD = calibration_params_[nIndex++];

  nIndex = 0;
  calibration_matrix_.Write(0, 0, calibration_params_[nIndex++]);
  calibration_matrix_.Write(0, 1, calibration_params_[nIndex++]);
  calibration_matrix_.Write(1, 0, calibration_params_[nIndex++]);
  calibration_matrix_.Write(1, 1, calibration_params_[nIndex++]);

  int nResult = SaveMatrixToTxt();
  if (nResult) {
    return nResult;
  }
  is_calibrated_ = true;
  return 0;
}
int AlgCalibration::SetCalibrationData(const double dA, const double dB,
                                       const double dC, const double dD) {
  QString matrix_data_str = QString();
  matrix_data_str +=
      (QString::number(dA) + RectSeparate + QString::number(dB) + RectSeparate +
       QString::number(dC) + RectSeparate + QString::number(dD) + RectSeparate);
  matrix_data_str += ("\r\n");

  int nIndex = 0;
  int nTempWidth = 4;
  if (!calibration_params_.empty()) {
    calibration_params_.erase(calibration_params_.begin(),
                               calibration_params_.end());
  }
  for (int i = 0; i < nTempWidth; ++i) {
    calibration_params_.push_back(0);
  }

  calibration_params_[nIndex++] = dA;
  calibration_params_[nIndex++] = dB;
  calibration_params_[nIndex++] = dC;
  calibration_params_[nIndex++] = dD;

  nIndex = 0;
  calibration_matrix_.Write(0, 0, calibration_params_[nIndex++]);
  calibration_matrix_.Write(0, 1, calibration_params_[nIndex++]);
  calibration_matrix_.Write(1, 0, calibration_params_[nIndex++]);
  calibration_matrix_.Write(1, 1, calibration_params_[nIndex++]);

  QFile SaveSetData(full_path_);
  if (!SaveSetData.open(QIODevice::WriteOnly)) {
    return 1;
  }
  QTextStream SaveSetVal(&SaveSetData);
  SaveSetVal << matrix_data_str;
  SaveSetData.close();
  is_calibrated_ = true;
  return 0;
}
int AlgCalibration::SetCalibrationData(QString strFullPath) {
  full_path_ = strFullPath;

  if (!calibration_params_.empty()) {
    calibration_params_.erase(calibration_params_.begin(),
                               calibration_params_.end());
  }
  QFile SaveSetData(full_path_);
  if (!SaveSetData.open((QFile::ReadOnly))) {
    return 1;
  }
  QTextStream SaveSetVal(&SaveSetData);
  QString strLine = SaveSetVal.readLine();
  SaveSetData.close();
  if (strLine.isNull()) {
    return 2;
  }
  int LeftIndex = 0;
  int RightIndex = 0;
  int nDataNum = 4;
  for (int i = 0; i < nDataNum; ++i) {
    RightIndex = strLine.indexOf(RectSeparate, LeftIndex);
    double dTemp = strLine.mid(LeftIndex, RightIndex - LeftIndex).toDouble();
    calibration_params_.push_back(dTemp);
    LeftIndex = RightIndex + 1;
    if (LeftIndex > strLine.size()) {
      break;
    }
  }
  if (calibration_params_.size() < nDataNum) {
    return 3;
  }
  int nIndex = 0;

  nIndex = 0;
  calibration_matrix_.Write(0, 0, calibration_params_[nIndex++]);
  calibration_matrix_.Write(0, 1, calibration_params_[nIndex++]);
  calibration_matrix_.Write(1, 0, calibration_params_[nIndex++]);
  calibration_matrix_.Write(1, 1, calibration_params_[nIndex++]);

  is_calibrated_ = true;
  return 0;
}

int AlgCalibration::RobotCoordinateOffset(double dInputX, double dInputY,
                                          double* pdOffsetX,
                                          double* pdOffsetY) {
  if (!is_calibrated_) {
    return 1;
  }
  SimpleMatrix matrixRotateCenter(2, 1);
  matrixRotateCenter.InitMatrix();
  matrixRotateCenter.Write(0, 0, dInputX);
  matrixRotateCenter.Write(1, 0, dInputY);
  SimpleMatrix matrixRotateCenterRobot(2, 1);
  matrixRotateCenterRobot.InitMatrix();
  matrix_calc_.Multiply(&calibration_matrix_, &matrixRotateCenter,
               &matrixRotateCenterRobot);
  *pdOffsetX = matrixRotateCenterRobot.Read(0, 0);
  *pdOffsetY = matrixRotateCenterRobot.Read(1, 0);
  matrixRotateCenter.FreeMatrix();
  matrixRotateCenterRobot.FreeMatrix();
  return 0;
}
int AlgCalibration::CalibrationMatrixFromTxt(QString strPath, double* pdA,
                                             double* pdB, double* pdC,
                                             double* pdD) {
  QString strTemp = full_path_;
  int nResult = SetCalibrationData(strPath);

  full_path_ = strTemp;
  if (nResult) {
    return nResult;
  }

  if (!is_calibrated_) {
    return 1;
  }
  int nIndex = 0;
  *pdA = calibration_params_[nIndex++];
  *pdB = calibration_params_[nIndex++];
  *pdC = calibration_params_[nIndex++];
  *pdD = calibration_params_[nIndex++];

  nResult = SaveMatrixToTxt();
  return nResult;
}
int AlgCalibration::SaveMatrixToTxt() {
  int nTempWidth = 4;
  QString matrix_data_str = QString();
  for (int i = 0; i < nTempWidth; ++i) {
    double dTemp = calibration_params_[i];
    matrix_data_str += (QString::number(dTemp) + RectSeparate);
  }
  matrix_data_str += ("\r\n");

  QFile SaveSetData(full_path_);
  if (!SaveSetData.open(QIODevice::WriteOnly)) {
    return 1;
  }
  QTextStream SaveSetVal(&SaveSetData);
  SaveSetVal << matrix_data_str;
  SaveSetData.close();
  return 0;
}
