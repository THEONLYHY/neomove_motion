// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/05/07 14:46

#ifndef PROBE_STATION8_SRC_CONTROLLER_ROBOT_MANUAL_ROBOT_MANUAL_H_
#define PROBE_STATION8_SRC_CONTROLLER_ROBOT_MANUAL_ROBOT_MANUAL_H_

#include <config/axis/axis_config.h>

#include <QtWidgets>
#include <map>

#include "robot_define.h"

class RobotManual : public QDialog {
  Q_OBJECT

 public:
  explicit RobotManual(QWidget* parent = nullptr);
  ~RobotManual() override;

 public slots:
  void SetIndexStep(const std::string& axis_ids, double index_step);
  void OnMovePropertyChanged(MoveProperty move_property);
  void OnButtonClicked(int device_index, std::string axis_ids, int direction);
  void OnButtonLongPressed(int device_index, std::string axis_ids,
                           int direction);
  void OnButtonLongPressReleased(int device_index, std::string axis_ids,
                                 int direction);

 private:
  MoveProperty move_property_;
  yotta::AxisConfigPtr axis_config_;
  std::map<std::string, double> index_steps_;
};

#endif  // PROBE_STATION8_SRC_CONTROLLER_ROBOT_MANUAL_ROBOT_MANUAL_H_
