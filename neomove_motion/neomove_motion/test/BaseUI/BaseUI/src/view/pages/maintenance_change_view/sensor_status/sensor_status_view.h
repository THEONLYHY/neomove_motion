// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/07/15 19:50

#ifndef BASE_UI_SRC_VIEW_PAGES_MAINTENANCE_CHANGE_VIEW_SENSOR_STATUS_SENSOR_STATUS_VIEW_H_
#define BASE_UI_SRC_VIEW_PAGES_MAINTENANCE_CHANGE_VIEW_SENSOR_STATUS_SENSOR_STATUS_VIEW_H_

#include <QMap>
#include <QStringList>
#include <QTimer>
#include <QWidget>

#include "config/config_factory.h"
#include "model/model_mgr.h"
#include "ui_sensor_status_view.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class sensor_status_viewClass;
};
QT_END_NAMESPACE

class SensorStatusView : public QWidget {
  Q_OBJECT

 public:
  SensorStatusView(QWidget* parent = nullptr);
  ~SensorStatusView();
  void ReTranslate();

 private:
  void InitSensor();
  void ShowModule();
  void TimerSensor();
  void UpdateSensorValues();

 private:
  Ui::sensor_status_viewClass* ui;
  QTimer timer_sensor_;

  yotta::IoConfigPtr io_config_;
  std::string module_id_;  // 当前模块
  QMap<QString, QLineEdit*> input_line_edits_;
  QMap<QString, QLineEdit*> axis_line_edits_;
  QMap<QString, QLineEdit*> analog_line_edits_;
  QStringList input_sensor_ids_;
  QStringList axis_sensor_ids_;
  QStringList analog_sensor_ids_;
};

#endif  // BASE_UI_SRC_VIEW_PAGES_MAINTENANCE_CHANGE_VIEW_SENSOR_STATUS_SENSOR_STATUS_VIEW_H_
