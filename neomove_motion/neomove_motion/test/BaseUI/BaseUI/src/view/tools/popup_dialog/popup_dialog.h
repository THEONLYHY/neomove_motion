// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/30 21:34

#ifndef BASE_UI_SRC_VIEW_TOOLS_POPUP_DIALOG_POPUP_DIALOG_H_
#define BASE_UI_SRC_VIEW_TOOLS_POPUP_DIALOG_POPUP_DIALOG_H_

#include <singleton.h>

#include <QDialog>
#include <QElapsedTimer>
#include <QList>
#include <QPaintEvent>
#include <QPainterPath>
#include <QProgressBar>
#include <QStackedWidget>
#include <QStringList>
#include <QTableWidget>
#include <QTimer>
#include <QVector>
#include <QWidget>

#include "view/components/ps_button/ps_button.h"
#include "view/components/ps_label/ps_label.h"
#include "view/components/ps_loading_spinner/ps_loading_spinner.h"
#include "view/components/ps_progress_bar/ps_progress_bar.h"
#include "view/components/ps_table_widget/ps_table_widget.h"
#include "view/components/ps_text_edit/ps_text_edit.h"

class PopupDialog : public QDialog {
  Q_OBJECT
  SINGLETON(PopupDialog);

 public:
  ~PopupDialog();

  void ShowPage();
  void HidePage();

  // 错误报警弹窗，返回 1=Accepted(OK), 0=Rejected
  int PopupError(int error_code, const QString& error_info,
                 const QString& solution);

  // 操作状态：初始化任务表格（step_names 为空时支持动态补行）
  void PopupOperationStatus(const QString& task_name,
                            const QStringList& step_names = QStringList(),
                            int close_after_seconds = 300);

  // 步骤状态更新：status 1=完成 2=进行中
  // 当 step_name 不在表中时自动追加新行（动态补行）
  void PopupOperationStatusAppend(const QString& step_name, int status = 0);

  // 信息提示
  int PopupInfo(const QString& info);

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  PopupDialog();

  // 页面创建（从构造函数中提取）
  QWidget* CreateInfoPage();
  QWidget* CreateOperationStatusPage();
  void InitTimers();

  int ExecBlockingChoice();
  void SwitchToErrorInfoPage();
  void SwitchToOperationStatusPage();
  void OnStopButtonClicked();
  void InitOperationStatusTable(const QString& task_name,
                                const QStringList& step_names,
                                int close_after_seconds);
  void UpdateElapsedTime();

  // 公共控件
  PsLabel* title_label_ = nullptr;
  PsButton* stop_button_ = nullptr;
  QStackedWidget* content_stack_ = nullptr;
  QTimer* operation_status_close_timer_ = nullptr;
  int operation_status_close_after_seconds_ = 300;

  // 错误/信息页控件
  PsTextEdit* info_text_edit_ = nullptr;
  PsLabel* solution_label_ = nullptr;
  PsButton* close_buzzer_button_ = nullptr;
  PsButton* action_button_ = nullptr;
  QWidget* button_widget_ = nullptr;

  // 操作状态页控件
  QWidget* operation_status_widget_ = nullptr;
  PsLoadingSpinner* spinner_ = nullptr;
  PsLabel* task_name_label_ = nullptr;
  PsLabel* tip_label_ = nullptr;
  PsLabel* step_label_ = nullptr;
  PsLabel* timer_label_ = nullptr;
  QElapsedTimer elapsed_timer_;
  QTimer* elapsed_update_timer_ = nullptr;
  PsProgressBar* progress_bar_ = nullptr;
  PsTableWidget* step_table_ = nullptr;

  int total_steps_ = 0;
};

using PopupDialogSingleton = yotta::Singleton<PopupDialog>;

#endif  // BASE_UI_SRC_VIEW_TOOLS_POPUP_DIALOG_POPUP_DIALOG_H_
