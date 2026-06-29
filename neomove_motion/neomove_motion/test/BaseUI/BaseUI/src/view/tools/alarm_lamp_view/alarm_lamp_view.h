// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#ifndef PROBE_STATION8_SRC_VIEW_TOOLS_ALARM_LAMP_VIEW_ALARM_LAMP_VIEW_H_
#define PROBE_STATION8_SRC_VIEW_TOOLS_ALARM_LAMP_VIEW_ALARM_LAMP_VIEW_H_

#include <QTimer>
#include <QWidget>

#include "ui_alarm_lamp_view.h"

class QPaintEvent;

class AlarmLampView : public QWidget {
  Q_OBJECT

 public:
  AlarmLampView(QWidget *parent = nullptr);
  ~AlarmLampView();
  void SetAlarmType(int type);
  void ReTranslate();

 private slots:
  void OnFlashTimeout();  // Flash timer callback

 private:
  void paintEvent(QPaintEvent *event) override;
  bool LampIsOn(int lamp_type) const;

  Ui::AlarmLampViewClass ui;
  int current_alarm_type_ = 0;       // 0=off, 1=red flash, 2=yellow flash, 3=green on
  QTimer *flash_timer_ = nullptr;  // Flash timer
  bool flash_state_ = false;       // true=on, false=off
};

#endif  // PROBE_STATION8_SRC_VIEW_TOOLS_ALARM_LAMP_VIEW_ALARM_LAMP_VIEW_H_
