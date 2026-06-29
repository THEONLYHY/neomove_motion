#include "view/pages/system_setting_page/system_setting_page.h"

#include <glog/glog_helper.h>

#include <QAbstractButton>
#include <QApplication>

#include "controller/log_manager/log_view_sink.h"
#include "view/tools/multi_image_show_view/multi_image_show_view.h"
#include "view/tools/robot_xyrz_view/robot_xyrz.h"

SystemSettingPage::SystemSettingPage(QWidget* parent)
    : QWidget(parent), ui(new Ui::system_setting_pageClass()) {
  ui->setupUi(this);

  btn_group_.addButton(ui->point_setting, 0);
  btn_group_.addButton(ui->io_setting, 1);
  btn_group_.addButton(ui->axis_setting, 2);
  btn_group_.addButton(ui->error_list, 3);
  btn_group_.addButton(ui->export_config, 4);

  for (QAbstractButton* button : btn_group_.buttons()) {
    connect(button, &QAbstractButton::toggled, this,
            [this, button](bool checked) {
              if (!checked) {
                return;
              }
              OnSetButtonClicked(btn_group_.id(button));
            });
  }

  point_setting_view_ = new PointSettingView;
  io_setting_view_ = new IoSettingView;
  motion_axis_setting_ = new AxisSettingView;
  error_list_view_ = new ErrorListView;
  export_config_view_ = new ExportConfigView;

  ui->set_page->addWidget(point_setting_view_);
  ui->set_page->addWidget(io_setting_view_);
  ui->set_page->addWidget(motion_axis_setting_);
  ui->set_page->addWidget(error_list_view_);
  ui->set_page->addWidget(export_config_view_);

  ui->set_page->setCurrentWidget(point_setting_view_);
  ui->point_setting->setChecked(true);
}

SystemSettingPage::~SystemSettingPage() { delete ui; }

void SystemSettingPage::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  if (point_setting_view_) {
    QShowEvent point_setting_show_event;
    QApplication::sendEvent(point_setting_view_, &point_setting_show_event);
  }
  ui->point_setting->setChecked(true);
  RobotXYZRSingleton::GetInstance()->ShowPage(ui->set_page);
}

void SystemSettingPage::FloatDockWidget(bool imageview_float,
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

void SystemSettingPage::ReTranslate() { ui->retranslateUi(this); }

void SystemSettingPage::OnSetButtonClicked(int id) {
  switch (id) {
    case 0:
      LOG_OPERATE(tr("POINT SETTING").toStdString());
      ui->set_page->setCurrentWidget(point_setting_view_);
      if (point_setting_view_) {
        QShowEvent point_setting_show_event;
        QApplication::sendEvent(point_setting_view_, &point_setting_show_event);
      }
      RobotXYZRSingleton::GetInstance()->ShowPage(ui->set_page);
      break;
    case 1:
      LOG_OPERATE(tr("IO SETTING").toStdString());
      ui->set_page->setCurrentWidget(io_setting_view_);
      FloatDockWidget(false, false);
      break;
    case 2:
      LOG_OPERATE(tr("AXIS SETTING").toStdString());
      ui->set_page->setCurrentWidget(motion_axis_setting_);
      FloatDockWidget(false, false);
      break;
    case 3:
      LOG_OPERATE(tr("ERROR LIST").toStdString());
      ui->set_page->setCurrentWidget(error_list_view_);
      FloatDockWidget(false, false);
      break;
    case 4:
      LOG_OPERATE(tr("EXPORT CONFIG").toStdString());
      ui->set_page->setCurrentWidget(export_config_view_);
      FloatDockWidget(false, false);
      break;
    default:
      break;
  }
}
