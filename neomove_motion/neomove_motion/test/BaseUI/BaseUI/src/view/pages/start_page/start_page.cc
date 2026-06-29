#include "view/pages/start_page/start_page.h"

#include <common/message_loop.h>
#include <glog/glog_helper.h>

#include <QPointer>
#include <QPushButton>
#include <QShowEvent>

#include "controller/log_manager/log_view_sink.h"
#include "controller/motion_control/motion_control.h"
#include "model/error_define.h"
#include "view/components/ps_button/ps_button.h"
#include "view/tools/popup_dialog/popup_dialog.h"

StartPageView::StartPageView(QWidget* parent)
    : QWidget(parent), ui(new Ui::StartPageClass()) {
  ui->setupUi(this);

  ui->probe_config->Color().SetBaseColor(PsColor::Color::kWhite);
  ui->maintenance_change->Color().SetBaseColor(PsColor::Color::kWhite);
  ui->motion_zero->Color().SetBaseColor(PsColor::Color::kBlue);

  ui->stackedWidget->setCurrentWidget(ui->first_page);

  connect(ui->probe_config, &QPushButton::clicked, this,
          &StartPageView::OnProbeConfigClicked);
  connect(ui->maintenance_change, &QPushButton::clicked, this,
          &StartPageView::OnMaintenanceClicked);
  connect(ui->motion_zero, &QPushButton::clicked, this,
          &StartPageView::OnMotionInitClicked);
  connect(this, &StartPageView::InitTaskFinished, this,
          &StartPageView::OnInitTaskFinished);

  ui->probe_config->setEnabled(false);
  ui->maintenance_change->setEnabled(false);
  ui->motion_zero->setEnabled(false);
}

StartPageView::~StartPageView() { delete ui; }

void StartPageView::ReTranslate() { ui->retranslateUi(this); }

void StartPageView::OnLoginSuccess() {
  ui->probe_config->setEnabled(true);
  ui->maintenance_change->setEnabled(true);
  ui->motion_zero->setEnabled(true);
}

void StartPageView::OnProbeConfigClicked() {
  LOG_OPERATE(tr("SystemSetting").toStdString());
  emit SystemSettingRequested();
}

void StartPageView::OnMaintenanceClicked() {
  LOG_OPERATE(tr("MaintenanceChange").toStdString());
  emit CalibrationRequested();
}

void StartPageView::OnMotionInitClicked() {
  ui->motion_zero->setEnabled(false);

  QPointer<StartPageView> self(this);
  auto run_init = [self]() {
    PopupDialogSingleton::GetInstance()->PopupOperationStatus(
        "设备开始初始化......", {"轴初始化"}, 600);
    PopupDialogSingleton::GetInstance()->PopupOperationStatusAppend("轴初始化",
                                                                    2);

    const int result =
        MotionControlSinglton::GetInstance()->DoTaskSync("轴初始化", 600);
    if (self) {
      emit self->InitTaskFinished(result);
    }
  };

  common::MessageLoopPtr motion_loop =
      common::MessageLoop::GetMessageLoop(common::kMotion);
  if (motion_loop) {
    motion_loop->PostTask(run_init);
  } else {
    run_init();
  }
}

void StartPageView::OnInitTaskFinished(int result) {
  ui->motion_zero->setEnabled(true);
  if (result == kMotionOk) {
    PopupDialogSingleton::GetInstance()->HidePage();
    emit SigInitEnd();
    emit InitSucceeded();
    emit SigGoSecondPage();
  } else {
    PopupDialogSingleton::GetInstance()->PopupError(result, "轴初始化错误", "");
    emit InitFailed(result);
  }
}

void StartPageView::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  ui->stackedWidget->setCurrentWidget(ui->first_page);
}
