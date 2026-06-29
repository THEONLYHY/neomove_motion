#include "view/pages/maintenance_change_view/maintenance_change_view.h"

#include <glog/glog_helper.h>

#include <QAbstractButton>
#include <QPoint>
#include <QShowEvent>

#include "view/tools/multi_image_show_view/multi_image_show_view.h"
#include "view/tools/robot_xyrz_view/robot_xyrz.h"

namespace {
constexpr int kCameraCalibrationImageWidth = 900;
}

MaintenanceChangeView::MaintenanceChangeView(QWidget* parent)
    : QWidget(parent), ui(new Ui::maintenance_change_viewClass()) {
  ui->setupUi(this);

  sensor_status_ = new SensorStatusView(this);
  init_axis_page_ = new InitPageView(this);
  camera_calibration_ = new CameraCalibrationView(this);

  ui->set_page->addWidget(sensor_status_);
  ui->set_page->addWidget(init_axis_page_);
  ui->set_page->addWidget(camera_calibration_);
  ui->set_page->setCurrentWidget(sensor_status_);

  btn_group_.addButton(ui->sensor_control, 0);
  btn_group_.addButton(ui->manual_operation, 1);
  btn_group_.addButton(ui->camera_calibration, 2);
  btn_group_.setExclusive(true);

  for (QAbstractButton* button : btn_group_.buttons()) {
    connect(button, &QAbstractButton::toggled, this, [this, button](bool checked) {
      if (!checked) {
        return;
      }
      OnSetButtonClicked(btn_group_.id(button));
    });
  }
}

MaintenanceChangeView::~MaintenanceChangeView() { delete ui; }

void MaintenanceChangeView::FloatDockWidget(bool imageview_float,
                                            bool robotxyrz_float) {
  if (imageview_float) {
    MultiImageShowViewSingleton::GetInstance()->ShowPage(ui->set_page);
  } else {
    MultiImageShowViewSingleton::GetInstance()->HidePage();
  }

  if (robotxyrz_float) {
    RobotXYZRSingleton::GetInstance()->ShowPage(ui->set_page);
  } else {
    RobotXYZRSingleton::GetInstance()->HidePage();
  }
}

void MaintenanceChangeView::ReTranslate() {
  ui->retranslateUi(this);
  sensor_status_->ReTranslate();
}

void MaintenanceChangeView::OnSetButtonClicked(int id) {
  switch (id) {
    case 0:
      LOG(INFO) << "感应器状态";
      ui->set_page->setCurrentWidget(sensor_status_);
      FloatDockWidget(false, false);
      break;
    case 1:
      LOG(INFO) << "初始化页面";
      ui->set_page->setCurrentWidget(init_axis_page_);
      FloatDockWidget(false, true);
      break;
    case 2:
      LOG(INFO) << "相机标定";
      ui->set_page->setCurrentWidget(camera_calibration_);
      if (camera_calibration_) {
        const QPoint image_pos = camera_calibration_->mapToGlobal(QPoint(0, 0));
        int image_height = camera_calibration_->height();
        if (image_height <= 0) {
          image_height = ui->set_page->height();
        }
        MultiImageShowViewSingleton::GetInstance()->ShowPage(
            image_pos.x(), image_pos.y(), kCameraCalibrationImageWidth,
            image_height);
      }
      RobotXYZRSingleton::GetInstance()->ShowPage(ui->set_page);
      break;
    default:
      break;
  }
}

void MaintenanceChangeView::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  if (ui->sensor_control) {
    ui->sensor_control->setChecked(true);
  }
}
