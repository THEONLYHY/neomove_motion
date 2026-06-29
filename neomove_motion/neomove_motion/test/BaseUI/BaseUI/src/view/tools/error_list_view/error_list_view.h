// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/05/22 14:54

#ifndef BASE_UI_SRC_VIEW_TOOLS_ERROR_LIST_VIEW_ERROR_LIST_VIEW_H_
#define BASE_UI_SRC_VIEW_TOOLS_ERROR_LIST_VIEW_ERROR_LIST_VIEW_H_

#include <QDateTime>
#include <QVector>
#include <QWidget>

#include "ui_error_list_view.h"
#include "controller/log_manager/log_view_sink.h"

struct ErrorEntry {
  QString message;
  QDateTime first_time;
};

class ErrorListView : public QWidget {
  Q_OBJECT

 public:
  explicit ErrorListView(QWidget* parent = nullptr);
  ~ErrorListView() override;

 signals:
  // 跨线程安全：回调线程通过信号切到 UI 线程更新表格
  void SigErrorLog(QString msg, int type);

 private slots:
  void OnErrorLog(QString msg, int type);
  void OnClearButtonClicked();
  void OnUpButtonClicked();
  void OnDownButtonClicked();

 private:
  // 由 LogViewSink 回调触发，运行在调用 LOG(ERROR) 的线程
  void OnSinkLog(const std::string& msg, int type);

  Ui::ErrorListViewClass* ui;
  EventLogFunId event_log_fun_id_ = 0;
  bool event_log_fun_registered_ = false;
  QVector<ErrorEntry> error_entries_;
};

#endif  // BASE_UI_SRC_VIEW_TOOLS_ERROR_LIST_VIEW_ERROR_LIST_VIEW_H_
