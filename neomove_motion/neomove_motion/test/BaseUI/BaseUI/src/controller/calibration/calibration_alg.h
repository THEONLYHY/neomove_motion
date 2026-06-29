// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/05/26 14:09

#ifndef PROBE_STATION8_SRC_CONTROLLER_CALIBRATION_CALIBRATION_ALG_H_
#define PROBE_STATION8_SRC_CONTROLLER_CALIBRATION_CALIBRATION_ALG_H_

#include <singleton.h>

#include <memory>

#include "alg/alg_calibration.h"
#include "model/calibration/calibration_para.h"

struct Point4 {
  double dXUm = 0;  //机械坐标
  double dYUm = 0;
  double dCircleXIndex = 0;  //图像坐标
  double dCircleYIndex = 0;
};

//标定算法
class CalibrationAlg {
  SINGLETON(CalibrationAlg);

 public:
  CalibrationAlg();
  ~CalibrationAlg() = default;

  //标定
  void SetPointData(std::vector<Point4>);  //一次设置图像机械点对
  void AddPoint(Point4);
  void CreactCalibrationPara();  //创建标定数据
  CalibrationParaPtr GetCalibrationPara();

  //执行
  void SetCalibrationPara(CalibrationParaPtr);  //设置标定数据
  void Calibration(int type, double x, double y, double r, double &, double &,
                   double &);  //执行标定转换

 private:
  std::mutex point_list_mutex_;
  std::vector<Point4> point_list_;                       //坐标数据
  CalibrationParaPtr calibration_para_ptr_;              //标定数据
  std::shared_ptr<AlgCalibration> calibration_alg_ptr_;  //标定算法类
};
// using CalibrationAlgPtr = std::shared_ptr<CalibrationAlg>;
using CalibrationAlgSinglton = yotta::Singleton<CalibrationAlg>;

#endif  // PROBE_STATION8_SRC_CONTROLLER_CALIBRATION_CALIBRATION_ALG_H_
