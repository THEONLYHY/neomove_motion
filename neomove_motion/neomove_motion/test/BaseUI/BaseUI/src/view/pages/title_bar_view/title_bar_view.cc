#include "view/pages/title_bar_view/title_bar_view.h"

#include <Windows.h>
#include <common/message_loop.h>
#include <common/path/path_utils.h>

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLocale>
#include <QScreen>
#include <memory>
#include <sstream>

#include "controller/real_time_data/real_time_data.h"
#include "model/model_mgr.h"
#include "view/tools/popup_dialog/popup_dialog.h"
#pragma comment(lib, "Version.lib")

namespace {
std::wstring GetAppVersion() {
  wchar_t filename[MAX_PATH + 1];
  if (GetModuleFileName(nullptr, filename, MAX_PATH) == 0) {
    return L"";
  }

  DWORD dummy;
  DWORD size = GetFileVersionInfoSize(filename, &dummy);
  if (size == 0) {
    return L"";
  }

  auto data = std::make_unique<BYTE[]>(size);
  if (!GetFileVersionInfo(filename, 0, size, data.get())) {
    return L"";
  }

  UINT len = 0;
  VS_FIXEDFILEINFO* fixed_file_info = nullptr;
  if (!VerQueryValue(data.get(), TEXT("\\"),
                     reinterpret_cast<void**>(&fixed_file_info), &len)) {
    return L"";
  }

  std::wostringstream version_stream;
  version_stream << HIWORD(fixed_file_info->dwProductVersionMS) << L"."
                 << LOWORD(fixed_file_info->dwProductVersionMS) << L"."
                 << HIWORD(fixed_file_info->dwProductVersionLS) << L"."
                 << LOWORD(fixed_file_info->dwProductVersionLS);
  return version_stream.str();
}
}  // namespace

TitleBarView::TitleBarView(QWidget* parent)
    : QWidget(parent), ui(new Ui::title_bar_viewClass()) {
  ui->setupUi(this);
  ui->pushButton_logo->setPixmap(
      QPixmap(":/icon/Yti_icon.ico")
          .scaled(158, 45, Qt::KeepAspectRatio, Qt::SmoothTransformation));

  const std::wstring version_str = GetAppVersion();
  ui->version_number_label_->setText("Ver: " +
                                     QString::fromStdWString(version_str));
  ui->version_number_label_->Color().SetBaseColor(PsColor::Color::kWhite);

  connect(ui->print_screen_, &QPushButton::clicked, this,
          &TitleBarView::OnPrintScreenClicked);

  status_layout_ = ui->status_lay;
  custom_layout_ = ui->custom_lay;

  ui->user_login_->Color().SetBaseColor(PsColor::Color::kWhite);

  ui->current_time_label_->Color().SetBaseColor(PsColor::Color::kWhite);
  ui->current_time_label_->setCursor(Qt::PointingHandCursor);

  ui->pushButton_logo->installEventFilter(this);
  ui->pushButton_logo->setCursor(Qt::PointingHandCursor);
#ifdef _DEBUG
  ui->current_time_label_->installEventFilter(this);
#endif

  log_view_ = new LogView;
  alarm_lamp_view_ = new AlarmLampView;
  alarm_lamp_view_->SetAlarmType(3);
  ui->log_lay->addWidget(log_view_);
  ui->widget_alarm_lay->addWidget(alarm_lamp_view_);

  user_manager_ = ModelMgrSinglton::GetInstance()->user_mgr();
  login_dialog_ = new LoginDialog(user_manager_.get(), this);
  connect(ui->user_login_, &QPushButton::clicked, login_dialog_,
          &LoginDialog::ShowDialog);
  connect(login_dialog_, &LoginDialog::LoginSuccess, this,
          &TitleBarView::OnLoginSuccess);

  timer_ = new QTimer(this);
  connect(timer_, &QTimer::timeout, this, &TitleBarView::UpdateTime);
  timer_->start(200);
  UpdateTime();
}

TitleBarView::~TitleBarView() {
  if (timer_) {
    timer_->stop();
  }
  delete ui;
}

void TitleBarView::ReTranslate() {
  ui->retranslateUi(this);
  if (log_view_) {
    log_view_->ReTranslate();
  }
  if (alarm_lamp_view_) {
    alarm_lamp_view_->ReTranslate();
  }
}

void TitleBarView::SetStatusItems(const QVector<TitleBarStatusItem>& items) {
  QLayoutItem* item = nullptr;
  while ((item = status_layout_->takeAt(0)) != nullptr) {
    if (item->widget()) {
      item->widget()->deleteLater();
    }
    delete item;
  }
  status_labels_.clear();

  for (const auto& status : items) {
    if (!status.visible) {
      continue;
    }
    auto* label = CreateStatusLabel(status);
    status_layout_->addWidget(label);
    status_labels_[status.id] = label;
  }
}

void TitleBarView::UpdateStatusValue(const QString& id, const QString& value) {
  auto it = status_labels_.find(id);
  if (it == status_labels_.end()) {
    return;
  }
  auto* label = it.value();
  label->setProperty("status_value", value);
  const QString name = label->property("status_name").toString();
  label->setText(name.isEmpty() ? value : name + "\n" + value);
}

void TitleBarView::UpdateStatusColor(const QString& id, const QColor& color) {
  auto it = status_labels_.find(id);
  if (it == status_labels_.end()) {
    return;
  }
  it.value()->Color().SetBaseColor(color);
}

void TitleBarView::SetCustomWidget(QWidget* widget) {
  ReplaceLayoutWidget(custom_layout_, &custom_widget_, widget);
}

bool TitleBarView::eventFilter(QObject* watched, QEvent* event) {
  if (event->type() == QEvent::MouseButtonRelease) {
    if (watched == ui->pushButton_logo) {
      OnClickedLogo();
      return true;
    }
    if (watched == ui->current_time_label_) {
      emit SigMinimizeRequested();
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TitleBarView::OnClickedLogo() {
  PopupDialogSingleton::GetInstance()->ShowPage();
  if (PopupDialogSingleton::GetInstance()->PopupInfo("确定要退出程序吗 ?")) {
    qApp->quit();
  }
}

void TitleBarView::UpdateTime() {
  QDateTime current_date_time = QDateTime::currentDateTime();
  QLocale c_locale = QLocale::c();
  QString month = c_locale.toString(current_date_time.date(), "MMM").toUpper();
  int day = current_date_time.date().day();
  int year = current_date_time.date().year();
  QString time = current_date_time.toString("HH:mm:ss");

  QString formatted_date_time =
      QString("%1/%2/%3\n%4")
          .arg(month)
          .arg(day, 2, 10, QChar('0'))
          .arg(year)
          .arg(time);
  ui->current_time_label_->setText(formatted_date_time);
}

void TitleBarView::OnPrintScreenClicked() {
  QScreen* screen = QGuiApplication::primaryScreen();
  if (!screen) {
    return;
  }
  const QPixmap screenshot = screen->grabWindow(0);
  common::MessageLoop::GetMessageLoop(common::kIo)
      ->PostTask(std::bind(&TitleBarView::SaveScreenShot, this, screenshot));
}

void TitleBarView::SaveScreenShot(const QPixmap& screen_shot) {
  QDateTime current_time = QDateTime::currentDateTime();
  QString time = current_time.toString("yyyy-MM-dd_HH-mm-ss.zzz");

  QString path =
      QString::fromStdWString(path_utils::GetFullPathFromCurrentModule(
                                  L"ImageSave\\ScreenShot\\%1.png"))
          .arg(time);
  QFileInfo file(path);
  QDir().mkpath(file.absolutePath());
  if (screen_shot.save(path)) {
    LOG(INFO) << "截图保存成功: " << path.toStdString();
  } else {
    LOG(ERROR) << "截图保存失败，路径可能无效: " << path.toStdString();
  }
}

void TitleBarView::OnLoginSuccess(User* user) {
  if (user == nullptr) {
    return;
  }
  ui->user_login_->setText("用户: " + user->user_name());
  if (user_manager_) {
    user_manager_->SetCurrentUser(user->user_name());
  }
  RealTimeDataSinglton::GetInstance()->set_current_user(
      user->user_name().toStdString());
  RealTimeDataSinglton::GetInstance()->set_current_user_level(
      user->user_group_level());
  emit SigLoginSuccess();
}

PsLabel* TitleBarView::CreateStatusLabel(const TitleBarStatusItem& item) {
  auto* label = new PsLabel(this);
  label->setFixedSize(item.width, 128);
  label->setAlignment(Qt::AlignCenter);
  label->setWordWrap(true);
  label->setProperty("status_name", item.name);
  label->setProperty("status_value", item.value);
  label->setText(item.name.isEmpty() ? item.value
                                      : item.name + "\n" + item.value);
  label->Color().SetBaseColor(item.color);
  QFont f = label->font();
  f.setPointSize(16);
  f.setBold(true);
  label->setFont(f);
  return label;
}

void TitleBarView::ReplaceLayoutWidget(QHBoxLayout* layout, QWidget** holder,
                                       QWidget* widget) {
  if (*holder) {
    layout->removeWidget(*holder);
    if ((*holder)->parent() == this) {
      (*holder)->hide();
    }
  }
  *holder = widget;
  if (!widget) {
    return;
  }
  widget->setParent(this);
  layout->addWidget(widget);
  widget->show();
}
