// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/18 11:05

#ifndef BASE_UI_SRC_VIEW_PAGES_INIT_PAGE_VIEW_INIT_PAGE_VIEW_H_
#define BASE_UI_SRC_VIEW_PAGES_INIT_PAGE_VIEW_INIT_PAGE_VIEW_H_

#include <config/axis/axis_config.h>
#include <motion/axis.h>

#include <QButtonGroup>
#include <QPointer>
#include <QSet>
#include <QTimer>
#include <QWidget>

#include "view/components/ps_button/ps_button.h"

#include "ui_init_page_view.h"
#include "view/tools/robot_xyrz_view/robot_xyrz.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class init_page_viewClass;
};
QT_END_NAMESPACE

class AxisAlarmWatcher : public QObject, public yotta::Axis::Watcher {
  Q_OBJECT
 public:
  explicit AxisAlarmWatcher(int axis_index, QObject *parent = nullptr)
      : QObject(parent), axis_index_(axis_index) {}

  void YOTTA_API_CALL OnAlarm(int axis_id, bool amp_alarm,
                              int amp_alarm_code) override {
    if (amp_alarm) {
      emit SigAlarmTriggered(axis_index_);
    }
  }

  void YOTTA_API_CALL OnAxisOperationState(int axis_id, int state) override {}
  void YOTTA_API_CALL OnAxisEvent(int axis_id, int event_id,
                                  yotta::Axis::AxisEvent *event) override {}

 signals:
  void SigAlarmTriggered(int axis_index);  // 报警信号

 private:
  int axis_index_;
};

class InitPageView : public QWidget {
  Q_OBJECT

 public:
  InitPageView(QWidget *parent = nullptr);
  ~InitPageView();

  void showEvent(QShowEvent *event) override;
  void hideEvent(QHideEvent *event) override;
 signals:
  void SigInitEnd();

  // 回零结果
  void SigHomeResult(int index, int result);
  // 使能结果
  void SigServoOnResult(int index, int result);
  // 清除报警结果
  void SigClearWarningResult(int index, int result);

 private slots:
  void OnEquipmentInitClicked();
  void OnEquipmentInitEndClicked();
  void UpdateTimer();  // 定时器执行函数

  void OnUnitClicked(int);
  void OnButtonClickedHome(int);
  void OnButtonClickedClearWarning(int);
  void OnButtonClickedServoOn(int);
  void OnButtonClickedLimitMin(int);
  void OnButtonClickedLimitMax(int);
  void OnAxisAlarm(int index);

  void OnButtonClickedServoOnOff();     // 所有轴全使能/全不使能切换
  void OnButtonClickedClearAllAlarm();  // 清除所有轴警报

 private:
  void ClearLayout(QLayout *layout);
  void ClearWatchers();
  void ShowUnit();
  void ApplyUserLevelVisibility();
  void AxisHome();
  void AxisClearWarning();
  void AxisServoOn();

 private slots:
  void OnHomeResult(int index, int result);
  void OnServoOnResult(int index, int result);
  void OnClearWarningResult(int index, int result);

 private:
  // QPointer<RobotXYZR> robot_xyrz_;
  QTimer *time_monitor_ = nullptr;  //计时器,用来监控IO状态
  std::string module_id_;
  QButtonGroup btn_group_module_;
  yotta::AxisConfigPtr axis_config_;
  int axis_index_ = 0;
  QVector<QPointer<PsButton>> home_buttons_;
  QVector<QPointer<PsButton>> servo_on_buttons_;
  QVector<QPointer<PsButton>> clear_warning_buttons_;
  std::vector<AxisAlarmWatcher *> axis_watchers_;

  // Cache for UI state to avoid unnecessary updates
  std::vector<int> cached_home_states_;   // Cached home state for each axis
  std::vector<int> cached_servo_states_;  // Cached servo state for each axis
  QSet<int> alarmed_axes_;                // 当前有报警的轴索引集合

 private:
  Ui::init_page_viewClass *ui;
};

#endif  // BASE_UI_SRC_VIEW_PAGES_INIT_PAGE_VIEW_INIT_PAGE_VIEW_H_
