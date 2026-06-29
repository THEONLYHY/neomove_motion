// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/30 21:34

#include "popup_dialog.h"

#include <glog/glog_helper.h>
#include <main_process/task.h>

#include <QFont>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMetaObject>
#include <QPainter>
#include <QStackedWidget>
#include <QTableWidgetItem>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QThread>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>
#include <QtMath>

#include "controller/motion_control/motion_control.h"
#include "view/components/ps_button/ps_button.h"
#include "view/components/ps_label/ps_label.h"
#include "view/components/ps_loading_spinner/ps_loading_spinner.h"
#include "view/components/ps_progress_bar/ps_progress_bar.h"
#include "view/components/ps_style/ps_surface_painter.h"
#include "view/components/ps_table_widget/ps_table_widget.h"
#include "view/components/ps_text_edit/ps_text_edit.h"

namespace {

// 为 QTextEdit 设置纯文本，同时指定段落对齐方式（左对齐/居中等）
// QTextEdit 不能直接用 setAlignment() 控制文本对齐，必须通过 QTextBlockFormat
// 操作
void SetAlignedPlainText(QTextEdit* text_edit, const QString& text,
                         Qt::Alignment alignment) {
  text_edit->setPlainText(text);

  QTextCursor cursor(text_edit->document());
  cursor.select(QTextCursor::Document);

  QTextBlockFormat block_format;
  block_format.setAlignment(alignment);
  cursor.mergeBlockFormat(block_format);

  cursor.clearSelection();
  cursor.movePosition(QTextCursor::End);
  text_edit->setTextCursor(cursor);
}

// 创建居中对齐的表格单元格项，用于操作状态页的步骤表格
QTableWidgetItem* CreateCenteredItem(const QString& text) {
  QTableWidgetItem* item = new QTableWidgetItem(text);
  item->setTextAlignment(Qt::AlignCenter);
  return item;
}

}  // namespace

// ============================================================
// 构造函数
// ============================================================

PopupDialog::PopupDialog()
    : QDialog(nullptr),
      operation_status_close_timer_(nullptr),
      operation_status_close_after_seconds_(0) {
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint |
                 Qt::WindowStaysOnTopHint);
  setAttribute(Qt::WA_TranslucentBackground);

  auto* main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(20, 20, 20, 20);
  main_layout->setSpacing(8);

  // 标题栏：标题 + STOP 按钮
  auto* title_bar = new QHBoxLayout;
  title_bar->setSpacing(8);

  title_label_ = new PsLabel(this);
  title_label_->setAlignment(Qt::AlignCenter);
  title_label_->setMinimumHeight(56);
  title_label_->SetBorderRadius(8);
  title_label_->Color().SetBaseColor(QColor("#4A86E8"));
  title_label_->setFont([] {
    QFont f;
    f.setPixelSize(22);
    f.setBold(true);
    return f;
  }());
  title_label_->setText("操作状态");

  stop_button_ = new PsButton("停止", this);
  stop_button_->setFixedSize(170, 56);
  stop_button_->SetBorderRadius(8);
  stop_button_->Color().SetBaseColor(PsColor::Color::kRed);
  stop_button_->setFont([] {
    QFont f;
    f.setPixelSize(14);
    f.setBold(true);
    return f;
  }());
  stop_button_->hide();

  title_bar->addWidget(title_label_, 1);
  title_bar->addWidget(stop_button_, 0, Qt::AlignVCenter);

  connect(stop_button_, &QPushButton::clicked, this,
          &PopupDialog::OnStopButtonClicked);

  // 内容区：信息页 + 操作状态页
  content_stack_ = new QStackedWidget(this);
  content_stack_->addWidget(CreateInfoPage());
  content_stack_->addWidget(CreateOperationStatusPage());

  InitTimers();

  main_layout->addLayout(title_bar);
  main_layout->addWidget(content_stack_, 1);

  setFixedSize(600, 400);
  move(QPoint(800, 300));
  SwitchToErrorInfoPage();
  hide();
}

PopupDialog::~PopupDialog() = default;

// ============================================================
// 页面创建
// ============================================================

QWidget* PopupDialog::CreateInfoPage() {
  auto* page = new QWidget(this);
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(8);

  // 信息文本区
  info_text_edit_ = new PsTextEdit(page);
  info_text_edit_->setReadOnly(true);
  info_text_edit_->Color().SetBaseColor(Qt::white);
  info_text_edit_->SetBorderRadius(6);
  info_text_edit_->setFont([] {
    QFont f;
    f.setPixelSize(16);
    return f;
  }());

  // 解决方案标签
  solution_label_ = new PsLabel(page);
  solution_label_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  solution_label_->setWordWrap(true);
  solution_label_->setFont([] {
    QFont f;
    f.setPixelSize(16);
    return f;
  }());
  solution_label_->hide();

  // 按钮行
  button_widget_ = new QWidget(page);
  auto* button_layout = new QHBoxLayout(button_widget_);
  button_layout->setContentsMargins(0, 0, 0, 0);
  button_layout->setSpacing(24);

  close_buzzer_button_ = new PsButton(button_widget_);
  close_buzzer_button_->setMinimumSize(140, 48);
  close_buzzer_button_->SetBorderRadius(8);
  close_buzzer_button_->setFont([] {
    QFont f;
    f.setPixelSize(15);
    f.setBold(true);
    return f;
  }());
  close_buzzer_button_->Color().SetBaseColor(PsColor::Color::kBlue);

  action_button_ = new PsButton(button_widget_);
  action_button_->setMinimumSize(140, 48);
  action_button_->SetBorderRadius(8);
  action_button_->setFont([] {
    QFont f;
    f.setPixelSize(15);
    f.setBold(true);
    return f;
  }());
  action_button_->Color().SetBaseColor(PsColor::Color::kBlue);

  button_layout->addStretch();
  button_layout->addWidget(close_buzzer_button_);
  button_layout->addWidget(action_button_);
  button_layout->addStretch();
  button_widget_->hide();

  layout->addWidget(info_text_edit_, 2);
  layout->addWidget(solution_label_, 1);
  layout->addWidget(button_widget_);

  return page;
}

QWidget* PopupDialog::CreateOperationStatusPage() {
  operation_status_widget_ = new QWidget(this);
  // operation_status_widget_->setStyleSheet("background-color: #FFFFFF;");
  auto* operation_layout = new QVBoxLayout(operation_status_widget_);
  operation_layout->setContentsMargins(0, 0, 0, 0);
  operation_layout->setSpacing(16);

  // 顶部摘要：spinner + 任务名 + 提示 + 进度条
  auto* summary_layout = new QHBoxLayout;
  summary_layout->setSpacing(16);

  spinner_ = new PsLoadingSpinner(operation_status_widget_);
  spinner_->setFixedSize(spinner_->sizeHint());

  auto* center_layout = new QVBoxLayout;
  center_layout->setSpacing(10);

  task_name_label_ = new PsLabel(operation_status_widget_);
  task_name_label_->Color().SetBaseColor(PsColor::Color::kTransparent);
  task_name_label_->setFont([] {
    QFont f;
    f.setPixelSize(18);
    f.setBold(true);
    return f;
  }());
  task_name_label_->setText("-");

  auto* tip_row = new QHBoxLayout;
  tip_row->setSpacing(12);
  tip_label_ = new PsLabel(operation_status_widget_);
  tip_label_->Color().SetBaseColor(PsColor::Color::kTransparent);
  tip_label_->setFont([] {
    QFont f;
    f.setPixelSize(14);
    return f;
  }());
  tip_label_->setText("请等待设备完成当前动作");

  step_label_ = new PsLabel(operation_status_widget_);
  step_label_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  step_label_->Color().SetBaseColor(PsColor::Color::kTransparent);
  step_label_->setFont([] {
    QFont f;
    f.setPixelSize(14);
    f.setBold(true);
    return f;
  }());
  step_label_->setMinimumWidth(90);
  step_label_->setText("步骤0/0");

  tip_row->addWidget(tip_label_, 1);
  tip_row->addWidget(step_label_);

  auto* progress_row = new QHBoxLayout;
  progress_row->setSpacing(12);

  progress_bar_ = new PsProgressBar(operation_status_widget_);
  progress_bar_->setRange(0, 100);
  progress_bar_->setValue(0);
  progress_bar_->setFixedHeight(12);
  progress_bar_->setTextVisible(false);

  timer_label_ = new PsLabel(operation_status_widget_);
  timer_label_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  timer_label_->Color().SetBaseColor(PsColor::Color::kTransparent);
  timer_label_->setFont([] {
    QFont f;
    f.setPixelSize(16);
    f.setBold(true);
    return f;
  }());
  timer_label_->setMinimumWidth(80);
  timer_label_->setText("00:00:00");

  progress_row->addWidget(progress_bar_, 1);
  progress_row->addWidget(timer_label_);

  center_layout->addWidget(task_name_label_);
  center_layout->addLayout(tip_row);
  center_layout->addLayout(progress_row);

  summary_layout->addWidget(spinner_, 0, Qt::AlignTop);
  summary_layout->addLayout(center_layout, 1);

  // 步骤表格
  step_table_ = new PsTableWidget(operation_status_widget_);
  step_table_->setColumnCount(4);
  step_table_->setHorizontalHeaderLabels({"步骤", "动作内容", "状态", "时间"});
  step_table_->verticalHeader()->hide();
  step_table_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  step_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  step_table_->setSelectionMode(QAbstractItemView::NoSelection);
  step_table_->setFocusPolicy(Qt::NoFocus);
  step_table_->SetShowBlankRows(true);
  step_table_->horizontalHeader()->setStretchLastSection(false);
  step_table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
  step_table_->horizontalHeader()->setSectionResizeMode(1,
                                                        QHeaderView::Stretch);
  step_table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
  step_table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
  step_table_->setColumnWidth(0, 70);
  step_table_->setColumnWidth(2, 100);
  step_table_->setColumnWidth(3, 120);

  operation_layout->addLayout(summary_layout);
  operation_layout->addWidget(step_table_, 1);

  return operation_status_widget_;
}

void PopupDialog::InitTimers() {
  operation_status_close_timer_ = new QTimer(this);
  operation_status_close_timer_->setSingleShot(true);
  connect(operation_status_close_timer_, &QTimer::timeout, this,
          &PopupDialog::HidePage);

  elapsed_update_timer_ = new QTimer(this);
  elapsed_update_timer_->setInterval(200);
  connect(elapsed_update_timer_, &QTimer::timeout, this,
          &PopupDialog::UpdateElapsedTime);
}

// ============================================================
// paintEvent — 绘制对话框圆角边框
// ============================================================

void PopupDialog::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);

  QPainter painter(this);
  ps_style::PsDrawSurface(&painter, QRectF(rect()).adjusted(8, 8, -8, -8));
}

// ============================================================
// 页面切换
// ============================================================

void PopupDialog::SwitchToErrorInfoPage() {
  if (spinner_ != nullptr) {
    spinner_->Stop();
  }
  if (content_stack_ != nullptr) {
    content_stack_->setCurrentIndex(0);
  }
  if (stop_button_ != nullptr) {
    stop_button_->hide();
  }
}

void PopupDialog::SwitchToOperationStatusPage() {
  if (content_stack_ != nullptr) {
    content_stack_->setCurrentIndex(1);
  }
  if (spinner_ != nullptr && !spinner_->IsRunning()) {
    spinner_->Start();
  }
  if (stop_button_ != nullptr) {
    stop_button_->show();
  }
}

void PopupDialog::OnStopButtonClicked() {
  MotionControlSinglton::GetInstance()->Stop();
  HidePage();
}

// ============================================================
// 操作状态表格初始化
// ============================================================

void PopupDialog::InitOperationStatusTable(const QString& task_name,
                                           const QStringList& step_names,
                                           int close_after_seconds) {
  total_steps_ = step_names.size();
  operation_status_close_after_seconds_ = close_after_seconds;

  title_label_->setText("操作状态");
  title_label_->Color().SetBaseColor(QColor("#4A86E8"));
  task_name_label_->setText(task_name);
  tip_label_->setText("请等待设备完成当前动作");
  step_label_->setText(QString("步骤0/%1").arg(total_steps_));
  progress_bar_->setValue(0);
  timer_label_->setText("00:00:00");
  elapsed_timer_.start();
  elapsed_update_timer_->start();

  step_table_->setRowCount(0);
  for (int i = 0; i < step_names.size(); ++i) {
    step_table_->insertRow(i);

    QTableWidgetItem* step_no_item =
        CreateCenteredItem(QString("%1").arg(i + 1, 2, 10, QChar('0')));
    QTableWidgetItem* action_item = new QTableWidgetItem(step_names.at(i));
    QTableWidgetItem* status_item = CreateCenteredItem("待执行");
    QTableWidgetItem* time_item = CreateCenteredItem("--:--:--");

    status_item->setForeground(QBrush(QColor("#9CA3AF")));
    time_item->setForeground(QBrush(QColor("#9CA3AF")));

    step_table_->setItem(i, 0, step_no_item);
    step_table_->setItem(i, 1, action_item);
    step_table_->setItem(i, 2, status_item);
    step_table_->setItem(i, 3, time_item);
  }

  if (operation_status_close_after_seconds_ > 0) {
    operation_status_close_timer_->start(operation_status_close_after_seconds_ *
                                         1000);
  } else {
    operation_status_close_timer_->stop();
  }

  SwitchToOperationStatusPage();
  if (!isVisible()) {
    ShowPage();
  } else {
    raise();
    activateWindow();
  }
}

// ============================================================
// 公共接口
// ============================================================

void PopupDialog::ShowPage() {
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(
        this, [this]() { ShowPage(); }, Qt::QueuedConnection);
    return;
  }

  setWindowModality(Qt::ApplicationModal);
  show();
  raise();
  activateWindow();
}

void PopupDialog::HidePage() {
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(
        this, [this]() { HidePage(); }, Qt::QueuedConnection);
    return;
  }

  operation_status_close_timer_->stop();
  elapsed_update_timer_->stop();
  if (spinner_ != nullptr) {
    spinner_->Stop();
  }
  step_table_->setRowCount(0);
  total_steps_ = 0;
  hide();
}

void PopupDialog::UpdateElapsedTime() {
  qint64 secs = elapsed_timer_.elapsed() / 1000;
  qint64 h = secs / 3600;
  qint64 m = (secs % 3600) / 60;
  qint64 s = secs % 60;
  timer_label_->setText(QString("%1:%2:%3")
                            .arg(h, 2, 10, QChar('0'))
                            .arg(m, 2, 10, QChar('0'))
                            .arg(s, 2, 10, QChar('0')));
}

int PopupDialog::ExecBlockingChoice() {
  setModal(true);
  show();
  raise();
  activateWindow();

  const int result = exec();
  setModal(false);
  return result == QDialog::Accepted ? 1 : 0;
}

int PopupDialog::PopupError(int error_code, const QString& error_info,
                            const QString& solution) {
  if (QThread::currentThread() != thread()) {
    int result = 0;
    QMetaObject::invokeMethod(
        this,
        [this, error_code, error_info, solution, &result]() {
          result = PopupError(error_code, error_info, solution);
        },
        Qt::BlockingQueuedConnection);
    return result;
  }
  operation_status_close_timer_->stop();
  SwitchToErrorInfoPage();

  // 标题：红色报警
  title_label_->setText(QString("报警代码: %1").arg(error_code));
  title_label_->Color().SetBaseColor(QColor("#D64545"));

  // 信息文本
  QString error_text;
  if (!error_info.trimmed().isEmpty()) {
    error_text = QString("错误信息: %1").arg(error_info);
  }
  SetAlignedPlainText(info_text_edit_, error_text, Qt::AlignLeft);

  // 解决方案
  if (solution.trimmed().isEmpty()) {
    solution_label_->clear();
    solution_label_->hide();
  } else {
    solution_label_->setText(QString("解决方案: %1").arg(solution));
    solution_label_->show();
  }

  // 按钮：OK
  disconnect(close_buzzer_button_, nullptr, nullptr, nullptr);
  disconnect(action_button_, nullptr, nullptr, nullptr);
  button_widget_->show();
  close_buzzer_button_->hide();
  action_button_->show();
  action_button_->setText("OK");
  action_button_->Color().SetBaseColor(PsColor::Color::kBlue);
  action_button_->setEnabled(true);

  connect(action_button_, &QPushButton::clicked, this, &QDialog::accept);

  return ExecBlockingChoice();
}

void PopupDialog::PopupOperationStatus(const QString& task_name,
                                       const QStringList& step_names,
                                       int close_after_seconds) {
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(
        this,
        [this, task_name, step_names, close_after_seconds]() {
          PopupOperationStatus(task_name, step_names, close_after_seconds);
        },
        Qt::QueuedConnection);
    return;
  }

  InitOperationStatusTable(task_name, step_names, close_after_seconds);
}

void PopupDialog::PopupOperationStatusAppend(const QString& step_name,
                                             int status) {
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(
        this,
        [this, step_name, status]() {
          PopupOperationStatusAppend(step_name, status);
        },
        Qt::QueuedConnection);
    return;
  }

  int row = -1;
  for (int i = 0; i < step_table_->rowCount(); ++i) {
    QTableWidgetItem* item = step_table_->item(i, 1);
    QTableWidgetItem* status_item = step_table_->item(i, 2);
    if (item != nullptr && item->text() == step_name) {
      if (status_item != nullptr && status_item->text() == "完成") {
        continue;
      }
      row = i;
      break;
    }
  }

  if (row < 0 && !step_name.trimmed().isEmpty()) {
    row = step_table_->rowCount();
    step_table_->insertRow(row);
    total_steps_ = step_table_->rowCount();

    QTableWidgetItem* step_no_item =
        CreateCenteredItem(QString("%1").arg(row + 1, 2, 10, QChar('0')));
    QTableWidgetItem* action_item = new QTableWidgetItem(step_name);
    QTableWidgetItem* status_item = CreateCenteredItem("待执行");
    QTableWidgetItem* time_item = CreateCenteredItem("--:--:--");

    status_item->setForeground(QBrush(QColor("#9CA3AF")));
    time_item->setForeground(QBrush(QColor("#9CA3AF")));

    step_table_->setItem(row, 0, step_no_item);
    step_table_->setItem(row, 1, action_item);
    step_table_->setItem(row, 2, status_item);
    step_table_->setItem(row, 3, time_item);
  }

  if (step_table_->rowCount() == 0) {
    return;
  }

  if (row < 0) {
    row = step_table_->rowCount() - 1;
  }
  if (row < 0) {
    return;
  }

  total_steps_ = step_table_->rowCount();

  QTableWidgetItem* status_item = step_table_->item(row, 2);
  QTableWidgetItem* time_item = step_table_->item(row, 3);

  if (status == 2) {
    if (status_item != nullptr) {
      status_item->setText("进行中");
      status_item->setForeground(QBrush(QColor("#3B82F6")));
    }
    if (time_item != nullptr) {
      time_item->setText("--:--:--");
      time_item->setForeground(QBrush(QColor("#9CA3AF")));
    }
    if (total_steps_ > 0) {
      step_label_->setText(QString("步骤%1/%2").arg(row + 1).arg(total_steps_));
      const int percent = (row * 100) / total_steps_;
      progress_bar_->setValue(percent);
    }
  } else if (status == 1) {
    if (status_item != nullptr) {
      status_item->setText("完成");
      status_item->setForeground(QBrush(QColor("#10B981")));
    }
    if (time_item != nullptr) {
      time_item->setText(QTime::currentTime().toString("HH:mm:ss"));
      time_item->setForeground(QBrush(QColor("#374151")));
    }
    if (total_steps_ > 0) {
      step_label_->setText(QString("步骤%1/%2").arg(row + 1).arg(total_steps_));
      const int percent = ((row + 1) * 100) / total_steps_;
      progress_bar_->setValue(percent);
    }
  }

  QTableWidgetItem* action_item = step_table_->item(row, 1);
  if (action_item != nullptr) {
    QRect item_rect = step_table_->visualItemRect(action_item);
    if (item_rect.top() < 0 ||
        item_rect.bottom() > step_table_->viewport()->height()) {
      step_table_->scrollToItem(action_item, QAbstractItemView::EnsureVisible);
    }
  }
}

int PopupDialog::PopupInfo(const QString& info) {
  if (QThread::currentThread() != thread()) {
    int result = 0;
    QMetaObject::invokeMethod(
        this, [this, info, &result]() { result = PopupInfo(info); },
        Qt::BlockingQueuedConnection);
    return result;
  }

  operation_status_close_timer_->stop();
  SwitchToErrorInfoPage();

  // 标题：灰色信息
  title_label_->setText("信息显示");
  title_label_->Color().SetBaseColor(QColor("#7A869A"));

  SetAlignedPlainText(info_text_edit_, info, Qt::AlignCenter);
  solution_label_->clear();
  solution_label_->hide();

  // 按钮：OK + CANCEL
  disconnect(close_buzzer_button_, nullptr, nullptr, nullptr);
  disconnect(action_button_, nullptr, nullptr, nullptr);
  button_widget_->show();
  close_buzzer_button_->show();
  action_button_->show();
  close_buzzer_button_->setEnabled(true);
  action_button_->setEnabled(true);
  close_buzzer_button_->setText("OK");
  close_buzzer_button_->Color().SetBaseColor(PsColor::Color::kBlue);
  action_button_->setText("CANCEL");
  action_button_->Color().SetBaseColor(PsColor::Color::kBlue);

  connect(close_buzzer_button_, &QPushButton::clicked, this, &QDialog::accept);
  connect(action_button_, &QPushButton::clicked, this, &QDialog::reject);

  return ExecBlockingChoice();
}
