// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#include "log_view.h"

#include <glog/glog_helper.h>

#include <QDateTime>
#include <QScrollBar>
#include <QTimer>

#include "controller/log_manager/log_view_sink.h"
#include "controller/real_time_data/real_time_data.h"
#include "view/components/ps_button/ps_button.h"

LogView::LogView(QWidget* parent) : QWidget(parent) {
  ui.setupUi(this);

  // 按钮样式
  ui.msg_up->Color().SetBaseColor(PsColor::Color::kWhite);
  ui.msg_down->Color().SetBaseColor(PsColor::Color::kWhite);
  ui.clear_alarm->Color().SetBaseColor(PsColor::Color::kWhite);

  google::AddLogSink(LogViewSinkSinglton::GetInstance());
  event_log_fun_id_ = LogViewSinkSinglton::GetInstance()->AddEventLogFun(
      std::bind(&LogView::ShowLog, this, std::placeholders::_1,
                std::placeholders::_2));
  event_log_fun_registered_ = true;

  connect(this, &LogView::SigShowLog, this, &LogView::OnShowLog);
  connect(ui.clear_alarm, &QPushButton::clicked, this,
          &LogView::OnClearButtonCicked);
  // 上升下降按钮
  connect(ui.msg_up, &QPushButton::clicked, [this]() {
    int current_pos = ui.edit_msg_->verticalScrollBar()->value();
    ui.edit_msg_->verticalScrollBar()->setValue(current_pos -
                                                40);  // 每次滚动40个单位
  });
  connect(ui.msg_down, &QPushButton::clicked, [this]() {
    int current_pos = ui.edit_msg_->verticalScrollBar()->value();
    ui.edit_msg_->verticalScrollBar()->setValue(current_pos +
                                                40);  // 每次滚动40个单位
  });

}

LogView::~LogView() {
  if (event_log_fun_registered_) {
    LogViewSinkSinglton::GetInstance()->RemoveEventLogFun(event_log_fun_id_);
  }
  google::RemoveLogSink(LogViewSinkSinglton::GetInstance());
}
void LogView::ReTranslate() { ui.retranslateUi(this); }

void LogView::AppendLog(const QString& text) {
  OnShowLog(text.toStdString(), 6);
}

void LogView::ShowLog(std::string msg, int type) {
  // 界面只先显示等级大于WARNING的日志
  if (type > 1) {
    emit SigShowLog(msg, int(type));
  }
}

void LogView::OnShowLog(std::string msg, int type) {
  if (msg == "CLEAR") {
    ClearNonAlarmLogs();
    return;
  }
  if (type == 2 && Contains(QString::fromStdString(msg))) {
    return;  // 防止重复警告
  }
  // 根据日志级别设置颜色
  QString color;
  switch (type) {
    case 0:
      color = "black";
      break;  // INFO
    case 1:
      color = "yellow";
      break;  // WARNING
    case 2:
      color = "red";
      break;  // ERROR
    default:
      color = "black";
      break;
  }
  QDateTime current_time = QDateTime::currentDateTime();
  QString time =
      current_time.toString("yyyy-MM-dd HH:mm:ss.zzz");  // 获取当前时间
  QString show_time = "[" + time + "]";
  // 日志文本样式
  QString formatted_log =
      QString("<span style=\"color:%1;font-size:16pt;\">%2</span><br>")
          .arg(color)
          .arg(QString::fromStdString(msg));
  // 确定插入位置
  QTextCursor cursor = ui.edit_msg_->textCursor();
  // 警告
  if (type == 2) {
    cursor.movePosition(QTextCursor::Start);  // 移动到顶部输入
    // 计算错误日志最后的位置
    int insertPos = cursor.position();
    cursor.insertHtml(formatted_log);
    error_log_end_positon_ += cursor.position();

  } else {
    // 当前是否有警告信息
    if (error_log_end_positon_ == 0) {
      cursor.movePosition(QTextCursor::Start);  // 移动到顶部输入
    } else {
      cursor.setPosition(error_log_end_positon_);
    }
    cursor.insertHtml(formatted_log);  // 插入
  }

  if (type == 2) {
    current_log_mes_.push_back(QString::fromStdString(msg));
  }

  bool is_top =
      (ui.edit_msg_->verticalScrollBar()->value() == 0);  // 是否在顶部
  // 如果用户原本在顶部，确保更新后仍在顶部
  if (is_top) {
    ui.edit_msg_->moveCursor(QTextCursor::Start);
    ui.edit_msg_->ensureCursorVisible();
    ui.edit_msg_->verticalScrollBar()->setValue(0);
  }
}

void LogView::ClearNonAlarmLogs() {
  if (error_log_end_positon_ <= 0) {
    ui.edit_msg_->clear();
    return;
  }

  QTextCursor cursor(ui.edit_msg_->document());
  int log_start_position = error_log_end_positon_;
  int document_end_position = ui.edit_msg_->document()->characterCount() - 1;
  if (log_start_position > document_end_position) {
    log_start_position = document_end_position;
  }
  cursor.setPosition(log_start_position);
  cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
  cursor.removeSelectedText();
}

void LogView::OnClearButtonCicked() {
  if (RealTimeDataSinglton::GetInstance()->current_user_level() < kL2EG) {
    LOG(WARNING) << "清楚报警权限不够请与工程师联系或登录";
    return;
  }
  current_log_mes_.clear();
  ui.edit_msg_->clear();
  error_log_end_positon_ = 0;
}

bool LogView::Contains(QString msg) {
  for (const QString& log : current_log_mes_) {
    if (log == msg) {
      return true;
    }
  }
  return false;
}
