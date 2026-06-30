#include "view/main_window/base_main_window.h"

#include <action/impl/register_current_action_module.h>
#include <common/encode_helper.h>
#include <common/path/path_utils.h>
#include <glog/glog_helper.h>
#include <main_process/module_mgr.h>
#include <windows.h>

#include <QPushButton>

#include "config/config_factory.h"
#include "controller/camera_manager/camera_manager.h"
#include "controller/device_status_monitor/device_status_monitor.h"
#include "controller/log_manager/log_view_sink.h"
#include "controller/motion_control/motion_control.h"
#include "model/model_mgr.h"
#include "view/components/ps_button/ps_button.h"
#include "view/pages/maintenance_change_view/maintenance_change_view.h"
#include "view/pages/main_page/main_page.h"
#include "view/pages/start_page/start_page.h"
#include "view/pages/system_setting_page/system_setting_page.h"
#include "view/pages/title_bar_view/title_bar_view.h"
#include "view/tools/multi_image_show_view/multi_image_show_view.h"
#include "view/tools/popup_dialog/popup_dialog.h"
#include "view/tools/robot_xyrz_view/robot_xyrz.h"

namespace {

bool ConfigureMotionDllDirectory() {
  const std::wstring motion_dir =
      path_utils::GetFullPathFromCurrentExe(L"motion/");
  if (!SetDllDirectoryW(motion_dir.c_str())) {
    LOG(ERROR) << "SetDllDirectoryW failed, motion_dir="
               << encode_helper::Unicode2Utf8(motion_dir.c_str())
               << ", error=" << GetLastError();
    return false;
  }

  LOG(INFO) << "Motion DLL directory configured: "
            << encode_helper::Unicode2Utf8(motion_dir.c_str());
  return true;
}

}  // namespace

BaseMainWindow::BaseMainWindow(QWidget* parent)
    : QDialog(parent), ui_(new Ui::BaseMainWindowClass()) {
  ui_->setupUi(this);
  setWindowFlag(Qt::FramelessWindowHint);
  setFixedSize(1920, 1080);
  setWindowTitle("BaseUI");

  // 防止 Enter 键误触发按钮（QDialog 中 QPushButton 默认 autoDefault=true）
  for (auto* btn : findChildren<QPushButton*>()) btn->setAutoDefault(false);

  setWindowIcon(QIcon(":/icon/Yti_icon.ico"));
  // 收当前进程的所有模块日志
  glog_helper::InstallLogProxy(LogViewSinkSinglton::GetInstance());
  // 收算法进程的日志
  main_process::GetModuleMgr()->SetAlgLogCallback(
      LogViewSinkSinglton::GetInstancePtr());

  yotta::ConfigFactory::GetInstance()->Init(0);
  // 在action初始化之前注册当前exe模块里自定义的action
  //  注册当前exe里定义的Action
#ifndef USING_MODULE_MGR_LIB
  RegisterCurrentActionModule();
#endif

  // 工位数据第一时间初始化
  module_mgr_initialized_ = InitModuleMgr();
  // 尽快初始化DB
  ModelMgrSinglton::GetInstance()->Init(nullptr);
  // 开始监控设备状态
  if (module_mgr_initialized_) {
    DeviceStatusMonitorSinglton::GetInstance()->Start();
  }
  // 初始化弹窗
  PopupDialogSingleton::GetInstance()->HidePage();
  RobotXYZRSingleton::GetInstance()->HidePage();
  MultiImageShowViewSingleton::GetInstance()->HidePage();

  title_bar_ = ui_->title_bar;
  main_stack_ = ui_->main_stack_;
  system_setting_button_ = ui_->system_setting_button_;
  stop_action_ = ui_->stop_action_;
  back_page_ = ui_->back_page_;

  system_setting_button_->Color().SetBaseColor(PsColor::Color::kBlue);
  stop_action_->Color().SetBaseColor(PsColor::Color::kRed);
  back_page_->Color().SetBaseColor(PsColor::Color::kBlue);

  connect(stop_action_, &QPushButton::clicked, this,
          &BaseMainWindow::OnStopActionClicked);
  connect(back_page_, &QPushButton::clicked, this,
          &BaseMainWindow::OnBackPageClicked);
  connect(system_setting_button_, &QPushButton::clicked, this,
          &BaseMainWindow::OnSystemSettingClicked);
  connect(title_bar_, &TitleBarView::SigMinimizeRequested, this,
          &QWidget::showMinimized);

  InitPage();
  LOG_OPERATE("BaseUI started.");
}

BaseMainWindow::~BaseMainWindow() {
  MotionControlSinglton::GetInstance()->Stop();
  DeviceStatusMonitorSinglton::GetInstance()->Stop();
  MultiImageShowViewSingleton::GetInstance()->HidePage();
  RobotXYZRSingleton::GetInstance()->HidePage();
  if (alg_process_callback_) {
    alg_process_callback_->Quit();
  }
  CameraManagerSinglton::GetInstance()->Stop();
  main_process::GetModuleMgr()->Uninit();
  delete ui_;
}

bool BaseMainWindow::InitModuleMgr() {
  if (!ConfigureMotionDllDirectory()) {
    return false;
  }

  // 初始化运动控制框架，使用 exe 目录下的 NeoMove 配置文件。
  const std::wstring config_dir_w =
      path_utils::GetFullPathFromCurrentExe(L"config/");
  const std::string config_dir =
      encode_helper::Unicode2Utf8(config_dir_w.c_str());
  const std::string neomove_config_file = "neomove.json";

  int ret = main_process::GetModuleMgr()->Init(
      config_dir.c_str(), static_cast<int>(config_dir.size()),
      neomove_config_file.c_str(),
      static_cast<int>(neomove_config_file.size()), alg_process_callback_);
  if (ret != 0) {
    LOG(ERROR) << "Init ModuleMgr failed, ret=" << ret;
    return false;
  }

  // 启动阶段只加载运动模块和配置，不直接清报警或上使能。
  // 真实硬件动作由 InitPageView::OnEquipmentInitClicked() 排到运动线程执行。
  return true;
}

void BaseMainWindow::InitPage() {
  system_setting_page_ = new SystemSettingPage(this);
  calibration_view_ = new MaintenanceChangeView(this);
  main_page_ = new MainPage(this);
  start_page_ = new StartPageView(this);

  main_stack_->addWidget(system_setting_page_);
  main_stack_->addWidget(calibration_view_);
  main_stack_->addWidget(main_page_);
  main_stack_->addWidget(start_page_);
  main_stack_->setCurrentWidget(start_page_);
  last_widget_ = start_page_;

  connect(start_page_, &StartPageView::SystemSettingRequested, this,
          &BaseMainWindow::OnSystemSettingClicked);
  connect(start_page_, &StartPageView::CalibrationRequested, this,
          &BaseMainWindow::OnCalibrationClicked);
  connect(start_page_, &StartPageView::InitSucceeded, this,
          &BaseMainWindow::OnProbeInitEnd);
  connect(title_bar_, &TitleBarView::SigLoginSuccess, start_page_,
          &StartPageView::OnLoginSuccess);
}

void BaseMainWindow::OnSystemSettingClicked() {
  last_widget_ = main_stack_->currentWidget();
  FloatImageView(false);
  FloatRobotXYRZView(false);
  if (system_setting_page_) {
    main_stack_->setCurrentWidget(system_setting_page_);
  }
}

void BaseMainWindow::OnCalibrationClicked() {
  last_widget_ = main_stack_->currentWidget();
  if (calibration_view_) {
    main_stack_->setCurrentWidget(calibration_view_);
  }
  FloatImageView(false);
  FloatRobotXYRZView(false);
}

void BaseMainWindow::OnProbeInitEnd() {
  if (!module_mgr_initialized_) {
    LOG(ERROR) << "Probe init ignored because ModuleMgr is not initialized.";
    return;
  }

  motion_init_complete_ = true;
  last_widget_ = start_page_;
  if (main_page_) {
    main_stack_->setCurrentWidget(main_page_);
  }
  FloatImageView(false);
  FloatRobotXYRZView(false);
}

void BaseMainWindow::OnStopActionClicked() {
  MotionControlSinglton::GetInstance()->Stop();
}

void BaseMainWindow::OnBackPageClicked() {
  QWidget* current_widget = main_stack_->currentWidget();
  if (current_widget == start_page_) {
    return;
  }

  FloatImageView(false);
  FloatRobotXYRZView(false);

  QWidget* target_widget = last_widget_ ? last_widget_ : start_page_.data();
  if (!motion_init_complete_ && current_widget == main_page_) {
    target_widget = start_page_;
  }
  if (target_widget) {
    main_stack_->setCurrentWidget(target_widget);
  }
  if (main_stack_->currentWidget() == start_page_) {
    last_widget_ = start_page_;
  }
}

void BaseMainWindow::FloatImageView(bool is_float, bool show) {
  if (!is_float || !show) {
    MultiImageShowViewSingleton::GetInstance()->HidePage();
    return;
  }
  MultiImageShowViewSingleton::GetInstance()->ShowPage(main_stack_);
}

void BaseMainWindow::FloatRobotXYRZView(bool is_float, bool show) {
  if (!is_float || !show) {
    RobotXYZRSingleton::GetInstance()->HidePage();
    return;
  }
  RobotXYZRSingleton::GetInstance()->ShowPage(main_stack_);
}
