// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/07/14 20:45

#ifndef BASE_UI_SRC_VIEW_PAGES_MAINTENANCE_CHANGE_VIEW_MAINTENANCE_CHANGE_VIEW_H_
#define BASE_UI_SRC_VIEW_PAGES_MAINTENANCE_CHANGE_VIEW_MAINTENANCE_CHANGE_VIEW_H_

#include <QButtonGroup>
#include <QPointer>
#include <QWidget>

#include "ui_maintenance_change_view.h"
#include "view/pages/init_page_view/init_page_view.h"
#include "view/pages/maintenance_change_view/camera_calibration_view/camera_calibration_view.h"
#include "view/pages/maintenance_change_view/sensor_status/sensor_status_view.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class maintenance_change_viewClass;
};
QT_END_NAMESPACE

class MaintenanceChangeView : public QWidget {
  Q_OBJECT

 public:
  MaintenanceChangeView(QWidget *parent = nullptr);
  ~MaintenanceChangeView();
  void ReTranslate();

 private slots:
  void OnSetButtonClicked(int id);

 private:
  void FloatDockWidget(bool imageview_float, bool robotxyrz_float);

 protected:
  void showEvent(QShowEvent *event) override;

 private:
  QPointer<SensorStatusView> sensor_status_;
  QPointer<InitPageView> init_axis_page_;
  QPointer<CameraCalibrationView> camera_calibration_;

 private:
  Ui::maintenance_change_viewClass *ui;
  QButtonGroup btn_group_;
};

#endif  // BASE_UI_SRC_VIEW_PAGES_MAINTENANCE_CHANGE_VIEW_MAINTENANCE_CHANGE_VIEW_H_
