// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#ifndef BASE_UI_SRC_VIEW_TOOLS_LOG_VIEW_LOG_VIEW_H_
#define BASE_UI_SRC_VIEW_TOOLS_LOG_VIEW_LOG_VIEW_H_

#include <glog/glog_helper.h>

#include <QString>
#include <QWidget>
#include <functional>
#include <memory>
#include <mutex>

#include "controller/log_manager/log_view_sink.h"
#include "ui_log_view.h"
class LogView : public QWidget {
  Q_OBJECT

 public:
  LogView(QWidget *parent = nullptr);
  ~LogView();
  void ReTranslate();
  void AppendLog(const QString& text);
signals:
  void SigShowLog(std::string msg, int type);

 private slots:
  void OnClearButtonCicked();
  void OnShowLog(std::string msg, int type);

 private:
  void ShowLog(std::string msg, int type);
  void ClearNonAlarmLogs();
  bool Contains(QString msg);  //判断NG信息是否已存在

 private:
  Ui::log_viewClass ui;
  EventLogFunId event_log_fun_id_ = 0;
  bool event_log_fun_registered_ = false;
  // std::shared_ptr<LogViewSink> my_log_{new LogViewSink};
  QVector<QString> current_log_mes_;  // 所有NG信息
  int error_log_end_positon_ = 0;  // 维护变量  警告日志的下方位置
};

#endif  // BASE_UI_SRC_VIEW_TOOLS_LOG_VIEW_LOG_VIEW_H_
