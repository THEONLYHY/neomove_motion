// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/05/22 14:54

#include "error_list_view.h"

#include <QDateTime>
#include <QFont>
#include <QHeaderView>

#include "controller/log_manager/log_view_sink.h"
#include "view/components/ps_button/ps_button.h"

ErrorListView::ErrorListView(QWidget* parent) : QWidget(parent) {
  ui = new Ui::ErrorListViewClass();
  ui->setupUi(this);

  // 初始化表格：时间 + 错误信息
  ui->error_table_->setColumnCount(2);
  ui->error_table_->setHorizontalHeaderLabels({tr("时间"), tr("错误信息")});
  ui->error_table_->verticalHeader()->hide();
  ui->error_table_->verticalHeader()->setDefaultSectionSize(48);
  ui->error_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->error_table_->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->error_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->error_table_->SetShowBlankRows(true);
  ui->error_table_->horizontalHeader()->setStretchLastSection(true);
  ui->error_table_->horizontalHeader()->setSectionResizeMode(
      0, QHeaderView::Fixed);
  ui->error_table_->setColumnWidth(0, 240);

  // 清除按钮样式与信号
  ui->clear_button_->Color().SetBaseColor(PsColor::Color::kRed);
  connect(ui->clear_button_, &QPushButton::clicked, this,
          &ErrorListView::OnClearButtonClicked);
  connect(ui->up_button, &QPushButton::clicked, this,
          &ErrorListView::OnUpButtonClicked);
  connect(ui->down_button, &QPushButton::clicked, this,
          &ErrorListView::OnDownButtonClicked);

  // 跨线程安全：信号从回调线程切回 UI 线程
  connect(this, &ErrorListView::SigErrorLog, this, &ErrorListView::OnErrorLog,
          Qt::QueuedConnection);

  // 注册到 LogViewSink 接收 ERROR 日志
  event_log_fun_id_ = LogViewSinkSinglton::GetInstance()->AddEventLogFun(
      [this](const std::string& msg, int type) { OnSinkLog(msg, type); });
  event_log_fun_registered_ = true;
}

ErrorListView::~ErrorListView() {
  if (event_log_fun_registered_) {
    LogViewSinkSinglton::GetInstance()->RemoveEventLogFun(event_log_fun_id_);
  }
  delete ui;
}

void ErrorListView::OnSinkLog(const std::string& msg, int type) {
  if (type != 2) return;
  emit SigErrorLog(QString::fromStdString(msg), type);
}

void ErrorListView::OnErrorLog(QString msg, int type) {
  Q_UNUSED(type);

  // glog 日志尾部常带 \n，导致 drawText 居中时文字偏上
  msg = msg.trimmed();

  QFont item_font;
  item_font.setPointSize(16);

  QDateTime now = QDateTime::currentDateTime();

  // 检查是否已存在相同消息，有则提到最前面并更新时间
  for (int i = 0; i < error_entries_.size(); ++i) {
    if (error_entries_[i].message == msg) {
      error_entries_[i].first_time = now;
      // 移到最前面
      if (i != 0) {
        error_entries_.move(i, 0);
        ui->error_table_->removeRow(i);
        ui->error_table_->insertRow(0);
        ui->error_table_->setRowHeight(0, 48);
      }
      auto* time_item =
          new QTableWidgetItem(now.toString("yyyy-MM-dd HH:mm:ss"));
      time_item->setTextAlignment(Qt::AlignCenter);
      time_item->setFont(item_font);

      auto* msg_item = new QTableWidgetItem(msg);
      msg_item->setForeground(QColor("#EF4444"));
      msg_item->setFont(item_font);

      ui->error_table_->setItem(0, 0, time_item);
      ui->error_table_->setItem(0, 1, msg_item);
      return;
    }
  }

  // 新消息，添加新条目
  ErrorEntry entry;
  entry.message = msg;
  entry.first_time = now;
  error_entries_.prepend(entry);

  ui->error_table_->insertRow(0);
  ui->error_table_->setRowHeight(0, 48);

  auto* time_item =
      new QTableWidgetItem(entry.first_time.toString("yyyy-MM-dd HH:mm:ss"));
  time_item->setTextAlignment(Qt::AlignCenter);
  time_item->setFont(item_font);

  auto* msg_item = new QTableWidgetItem(msg);
  msg_item->setForeground(QColor("#EF4444"));
  msg_item->setFont(item_font);

  ui->error_table_->setItem(0, 0, time_item);
  ui->error_table_->setItem(0, 1, msg_item);

  while (error_entries_.size() > 500) {
    error_entries_.removeLast();
    ui->error_table_->removeRow(ui->error_table_->rowCount() - 1);
  }

  ui->error_count_label_->setText(tr("共 %1 条").arg(error_entries_.size()));
}

void ErrorListView::OnClearButtonClicked() {
  error_entries_.clear();
  ui->error_table_->setRowCount(0);
  ui->error_count_label_->setText(tr("共 0 条"));
}

void ErrorListView::OnUpButtonClicked() {
  int row_count = ui->error_table_->rowCount();
  if (row_count == 0) return;

  int current = ui->error_table_->currentRow();
  int target = (current < 0) ? 0 : current - 1;
  if (target < 0) return;

  ui->error_table_->selectRow(target);
  ui->error_table_->scrollToItem(ui->error_table_->item(target, 0),
                                 QAbstractItemView::EnsureVisible);
}

void ErrorListView::OnDownButtonClicked() {
  int row_count = ui->error_table_->rowCount();
  if (row_count == 0) return;

  int current = ui->error_table_->currentRow();
  int target = (current < 0) ? 0 : current + 1;
  if (target >= row_count) return;

  ui->error_table_->selectRow(target);
  ui->error_table_->scrollToItem(ui->error_table_->item(target, 0),
                                 QAbstractItemView::EnsureVisible);
}
