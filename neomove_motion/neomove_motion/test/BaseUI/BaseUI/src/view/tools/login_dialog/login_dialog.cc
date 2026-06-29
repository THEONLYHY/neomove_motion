// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/04/23 10:00

#include "view/tools/login_dialog/login_dialog.h"

#include <QGridLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QVBoxLayout>

#include "model/user_manage/user_manage.h"
#include "view/components/ps_color/ps_color.h"
#include "view/components/ps_style/ps_surface_painter.h"

LoginDialog::LoginDialog(UserManager* user_mgr, QWidget* parent)
    : QDialog(parent), user_mgr_(user_mgr) {
  setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);

  setFixedSize(424, 318);

  QVBoxLayout* main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(42, 34, 42, 30);
  main_layout->setSpacing(0);

  QLabel* title_label = new QLabel("用户登录", this);
  QFont title_font = title_label->font();
  title_font.setPointSize(22);
  title_font.setBold(true);
  title_label->setFont(title_font);
  title_label->setStyleSheet("color: #111827;");
  title_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  title_label->setFixedHeight(32);
  main_layout->addWidget(title_label);

  main_layout->addSpacing(28);

  QGridLayout* form_layout = new QGridLayout();
  form_layout->setContentsMargins(0, 0, 0, 0);
  form_layout->setHorizontalSpacing(14);
  form_layout->setVerticalSpacing(12);
  form_layout->setColumnMinimumWidth(0, 44);
  form_layout->setColumnStretch(1, 1);

  QLabel* user_label = new QLabel("用户", this);
  user_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  user_label->setStyleSheet("color: #374151; font-size: 14px;");

  user_combo_ = new PsComboBox(this);
  user_combo_->setFixedHeight(42);
  user_combo_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  user_combo_->SetBorderRadius(10);
  QFont input_font = user_combo_->font();
  input_font.setPointSize(10);
  user_combo_->setFont(input_font);

  form_layout->addWidget(user_label, 0, 0);
  form_layout->addWidget(user_combo_, 0, 1);

  QLabel* password_label = new QLabel("密码", this);
  password_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  password_label->setStyleSheet("color: #374151; font-size: 14px;");

  password_edit_ = new PsLineEdit(this);
  password_edit_->setPlaceholderText("请输入密码");
  password_edit_->setEchoMode(QLineEdit::Password);
  password_edit_->setFixedHeight(42);
  password_edit_->SetBorderRadius(10);
  password_edit_->setFont(input_font);

  form_layout->addWidget(password_label, 1, 0);
  form_layout->addWidget(password_edit_, 1, 1);

  main_layout->addLayout(form_layout);
  main_layout->addSpacing(8);

  status_label_ = new QLabel(this);
  status_label_->setFixedHeight(24);
  status_label_->setStyleSheet("color: #EF4444; font-size: 13px;");
  status_label_->setAlignment(Qt::AlignCenter);
  status_label_->setText("");
  main_layout->addWidget(status_label_);
  main_layout->addStretch();

  QHBoxLayout* button_layout = new QHBoxLayout();
  button_layout->setContentsMargins(0, 4, 0, 0);
  button_layout->setSpacing(12);
  button_layout->addStretch();

  login_button_ = new PsButton("登录", this);
  login_button_->setFixedSize(104, 44);
  login_button_->SetBorderRadius(9);
  login_button_->Color().SetBaseColor(QColor("#2563EB"));
  connect(login_button_, &PsButton::clicked, this,
          &LoginDialog::OnLoginClicked);

  cancel_button_ = new PsButton("取消", this);
  cancel_button_->setFixedSize(104, 44);
  cancel_button_->SetBorderRadius(9);
  cancel_button_->Color().SetBaseColor(QColor("#EEF2F7"));
  connect(cancel_button_, &PsButton::clicked, this,
          &LoginDialog::OnCancelClicked);

  button_layout->addWidget(cancel_button_);
  button_layout->addWidget(login_button_);
  main_layout->addLayout(button_layout);

  connect(password_edit_, &PsLineEdit::returnPressed, this,
          &LoginDialog::OnLoginClicked);
}

LoginDialog::~LoginDialog() = default;

void LoginDialog::RefreshUserList() {
  user_combo_->clear();
  if (user_mgr_ == nullptr) return;

  auto users = user_mgr_->user_list();
  for (auto* user : users) {
    user_combo_->addItem(user->user_name(), QVariant(user->user_id()));
  }
}

void LoginDialog::ShowDialog() {
  RefreshUserList();
  password_edit_->clear();
  status_label_->setText("");

  if (layout() != nullptr) {
    layout()->activate();
  }
  adjustSize();

  if (parentWidget() != nullptr) {
    QWidget* top = parentWidget()->window();
    QRect geo = top->geometry();
    move(geo.center().x() - width() / 2, geo.center().y() - height() / 2);
  } else if (QScreen* screen = QGuiApplication::primaryScreen();
             screen != nullptr) {
    QRect geo = screen->availableGeometry();
    move(geo.center().x() - width() / 2, geo.center().y() - height() / 2);
  }

  show();
  raise();
  activateWindow();
}

void LoginDialog::ShowError(const QString& msg) { status_label_->setText(msg); }

void LoginDialog::OnLoginClicked() {
  QString username = user_combo_->currentText();
  QString password = password_edit_->text();

  if (username.isEmpty()) {
    ShowError("请选择用户");
    return;
  }

  if (password.isEmpty()) {
    ShowError("请输入密码");
    return;
  }

  if (user_mgr_ == nullptr) return;

  User* user = user_mgr_->GetUser(username);
  if (user == nullptr) {
    ShowError("用户不存在");
    return;
  }

  if (user->check_password(password) != 0) {
    ShowError("密码错误");
    return;
  }

  emit LoginSuccess(user);
  hide();
}

void LoginDialog::OnCancelClicked() { hide(); }

void LoginDialog::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  const QRectF card_rect(10, 8, width() - 20, height() - 18);

  if (PsColor::IsLegacyUi()) {
    ps_style::PsSurfaceOption option;
    option.fill = QColor(0xF3, 0xF4, 0xF6);
    option.border = QColor(0x7A, 0x86, 0x9A);
    option.radius = 0.0;
    ps_style::PsDrawSurface(&painter, card_rect, option);

    const QRectF header_rect(card_rect.left() + 1.0, card_rect.top() + 1.0,
                             card_rect.width() - 2.0, 70.0);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0xE3, 0xE7, 0xEF));
    painter.drawRect(header_rect);
    painter.fillRect(QRectF(header_rect.left(), header_rect.top(),
                            header_rect.width(), 4.0),
                     QColor(0x1F, 0xA4, 0x63));
    painter.setPen(QPen(QColor(0x7A, 0x86, 0x9A), 1.0));
    painter.drawLine(header_rect.bottomLeft(), header_rect.bottomRight());
    return;
  }

  for (int i = 5; i >= 1; --i) {
    const qreal spread = i * 1.4;
    QRectF shadow_rect =
        card_rect.adjusted(-spread, -spread * 0.25, spread, spread);
    shadow_rect.translate(0, i * 0.5);

    QPainterPath shadow_path;
    shadow_path.addRoundedRect(shadow_rect, 14 + spread, 14 + spread);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(15, 23, 42, 18 / i));
    painter.drawPath(shadow_path);
  }

  QPainterPath path;
  path.addRoundedRect(card_rect, 14, 14);

  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor("#FFFFFF"));
  painter.drawPath(path);

  painter.save();
  painter.setClipPath(path);
  painter.fillRect(QRectF(card_rect.left(), card_rect.top(), card_rect.width(),
                          82),
                   QColor("#F8FAFC"));
  painter.fillRect(QRectF(card_rect.left(), card_rect.top(), card_rect.width(),
                          4),
                   QColor("#10B981"));
  painter.restore();

  painter.setPen(QPen(QColor("#E5E7EB"), 1));
  painter.drawPath(path);
}
