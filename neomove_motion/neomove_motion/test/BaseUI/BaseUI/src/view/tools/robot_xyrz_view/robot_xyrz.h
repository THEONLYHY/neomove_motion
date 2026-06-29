// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#ifndef PROBE_STATION8_SRC_VIEW_TOOLS_ROBOT_XYRZ_VIEW_ROBOT_XYRZ_H_
#define PROBE_STATION8_SRC_VIEW_TOOLS_ROBOT_XYRZ_VIEW_ROBOT_XYRZ_H_

#include <config/axis/axis_config.h>
#include <singleton.h>

#include <QDialog>
#include <QTimer>
#include <vector>

#include "controller/robot_manual/robot_define.h"
#include "controller/robot_manual/robot_manual.h"
#include "view/components/ps_arrow_button/ps_arrow_button.h"
#include "yotta_qt_plugin/coordinate_display/coordinate_display_widget.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class RobotXYZRClass;
}
QT_END_NAMESPACE

class RobotXYZR : public QDialog {
  Q_OBJECT
  SINGLETON(RobotXYZR);

 public:
  ~RobotXYZR() override;

  void SetStepInfo(const std::vector<double>& step_values,
                   const std::vector<int>& steps);
  void SetIndexStep(const std::string& axis_ids, double index_step);
  void SetUnit(const std::string& unit_ids);
  // double GetAxisPos(int axis_index);
  void ShowPage();
  void ShowPage(QWidget* anchor_widget);
  void ShowPage(int x, int y, int width, int height);
  void HidePage();
  void ReTranslate();
 signals:
  void MovePropertyChanged(const MoveProperty& property);
  void ButtonClicked(int unit_index, std::string axis_ids, int direction);
  void ButtonLongPressed(int unit_index, std::string axis_ids, int direction);
  void ButtonLongPressReleased(int unit_index, std::string axis_ids,
                               int direction);
  void UnitButtonClicked(int unit_index);

 public slots:
  void OnUnitChanged();
  void OnStepValChanged();
  void OnStepChanged();
  void OnTypeChanged();
  void OnButtonClicked();
  void OnButtonPressed();
  void OnButtonReleased();
  void OnDiagonalButtonClicked();
  void OnDiagonalButtonPressed();
  void OnDiagonalButtonReleased();
  void ShowAxisPosWidget(int unit_index);
  void UpdateTimer();  // 定时器执行函数

 private:
  RobotXYZR();

  void InitializeButtons();
  void UpdateMoveProperty();
  void SetAxisInfo(const std::vector<UnitInfo>& unit);
  void SetAxisButton(int button_index, const QString& text,
                     std::string axis_ids);
  void ConfigureArrowButton(
      int button_index, PsArrowButton::Direction direction,
      PsArrowButton::ArrowStyle style = PsArrowButton::ArrowStyle::Straight);
  void ProcessAxisButtonEvent(void (RobotXYZR::*signal)(int, std::string, int));

  void RefreshDevice(int unit_index);
  void RefreshStep(int step_index);
  void RefreshType(int type_index);
  void ClearLayout(QLayout* layout);
  QSize PreferredWindowSize();
  // Lifecycle management
  void showEvent(QShowEvent* event) override;
  void hideEvent(QHideEvent* event) override;

 private:
  QPointer<RobotManual> robot_manual_;

  std::vector<QPushButton*> unit_buttons_;
  std::vector<QPushButton*> step_buttons_;
  std::vector<QPushButton*> type_buttons_;
  std::vector<QPushButton*> axis_buttons_;

  std::vector<UnitInfo> unit_;
  int current_module_index_ = 0;
  std::vector<std::string> current_unit_axis_ids_;

  int current_step_value_index_ = 0;
  std::vector<double> step_values_ = {1, 10, 1000};
  int current_step_index_ = 0;
  std::vector<int> steps_ = {1, 2, 3, 5, 10};
  int current_type_index_ = 1;
  MoveProperty move_property_;

  yotta::AxisConfigPtr axis_config_;
  QTimer* time_monitor_ = nullptr;  // 计时器,用来监控轴坐标
  std::vector<CoordinateDisplayWidget*> axis_pos_display_widget_;
  // std::vector<double> axis_pos_;  //轴坐标

  // Cache for axis states to avoid unnecessary UI updates
  std::vector<int> cached_axis_states_;  // Cached axis state for each axis
  double cached_axis_pos_ = 0.;

  Ui::RobotXYZRClass* ui_;
};

using RobotXYZRSingleton = yotta::Singleton<RobotXYZR>;

#endif  // PROBE_STATION8_SRC_VIEW_TOOLS_ROBOT_XYRZ_VIEW_ROBOT_XYRZ_H_
