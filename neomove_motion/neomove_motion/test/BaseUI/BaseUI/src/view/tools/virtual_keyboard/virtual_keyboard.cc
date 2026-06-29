// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/04/23 14:59

#include "virtual_keyboard.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QColor>
#include <QDesktopWidget>
#include <QEvent>
#include <QFocusEvent>
#include <QFont>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QTextCursor>
#include <QTextEdit>

#include "view/components/ps_button/ps_button.h"
#include "view/components/ps_style/ps_surface_painter.h"

namespace {

constexpr int kKeyWidth = 72;
constexpr int kKeyHeight = 56;
constexpr int kKeyboardWidth = 420;
constexpr int kKeyboardHeight = 396;
constexpr int kSpacing = 6;
constexpr int kMargin = 8;
constexpr int kSurfaceMargin = 7;
constexpr int kKeyboardOffset = 4;
constexpr int kScreenBottomMargin = 8;
constexpr char kBackspaceKey[] = "BACKSPACE";
constexpr char kSwitchKey[] = "SWITCH";
constexpr char kEnterKey[] = "ENTER";
constexpr char kUpKey[] = "[UP]";
constexpr char kDownKey[] = "[DOWN]";
constexpr char kLeftKey[] = "[LEFT]";
constexpr char kRightKey[] = "[RIGHT]";

// 检查控件是否位于 QAbstractItemView（表格）内部，
// 用于区分表格编辑器和普通页面输入框，避免切页时误弹键盘
bool IsInsideTable(QWidget* widget) {
  QWidget* p = widget ? widget->parentWidget() : nullptr;
  while (p) {
    if (qobject_cast<QAbstractItemView*>(p)) {
      return true;
    }
    p = p->parentWidget();
  }
  return false;
}

bool IsArrowKey(const QString& key) {
  return key == kUpKey || key == kDownKey || key == kLeftKey ||
         key == kRightKey;
}

QString buttonText(const QString& key) {
  if (key == kBackspaceKey) return "⌫";
  if (key == kSwitchKey) return "⇄";
  if (key == kEnterKey) return "↵";
  if (key == kUpKey) return "↑";
  if (key == kDownKey) return "↓";
  if (key == kLeftKey) return "←";
  if (key == kRightKey) return "→";
  return key;
}

QString buttonStyle(const QString& key) {
  if (key == kSwitchKey) {
    return "QPushButton {"
           "  background-color: #F59E0B;"
           "  color: #FFFFFF;"
           "  border: none;"
           "  border-radius: 6px;"
           "  font-size: 20px;"
           "}"
           "QPushButton:hover {"
           "  background-color: #FBBF24;"
           "}"
           "QPushButton:pressed {"
           "  background-color: #D97706;"
           "}";
  }

  if (key == kBackspaceKey) {
    return "QPushButton {"
           "  background-color: #EF4444;"
           "  color: #FFFFFF;"
           "  border: none;"
           "  border-radius: 6px;"
           "  font-size: 20px;"
           "}"
           "QPushButton:hover {"
           "  background-color: #F87171;"
           "}"
           "QPushButton:pressed {"
           "  background-color: #DC2626;"
           "}";
  }

  if (key == kEnterKey) {
    return "QPushButton {"
           "  background-color: #10B981;"
           "  color: #FFFFFF;"
           "  border: none;"
           "  border-radius: 6px;"
           "  font-size: 24px;"
           "}"
           "QPushButton:hover {"
           "  background-color: #34D399;"
           "}"
           "QPushButton:pressed {"
           "  background-color: #059669;"
           "}";
  }

  if (IsArrowKey(key)) {
    return "QPushButton {"
           "  background-color: #6366F1;"
           "  color: #FFFFFF;"
           "  border: none;"
           "  border-radius: 6px;"
           "  font-size: 20px;"
           "}"
           "QPushButton:hover {"
           "  background-color: #818CF8;"
           "}"
           "QPushButton:pressed {"
           "  background-color: #4F46E5;"
           "}";
  }

  return "QPushButton {"
         "  background-color: #FFFFFF;"
         "  color: #1F2937;"
         "  border: 1px solid #D1D5DB;"
         "  border-radius: 6px;"
         "  font-size: 16px;"
         "  font-weight: 600;"
         "}"
         "QPushButton:hover {"
         "  background-color: #F9FAFB;"
         "  border-color: #9CA3AF;"
         "}"
         "QPushButton:pressed {"
         "  background-color: #E5E7EB;"
         "}";
}

#if PS_LEGACY_UI
QColor legacyButtonColor(const QString& key) {
  if (key == kSwitchKey) return QColor(0xFF, 0xD9, 0x66);
  if (key == kBackspaceKey) return QColor(0xD6, 0x45, 0x45);
  if (key == kEnterKey) return QColor(0x1F, 0xA4, 0x63);
  if (IsArrowKey(key)) return QColor(0x25, 0x0D, 0xFF);
  return QColor(0xF3, 0xF4, 0xF6);
}

void ApplyLegacyButtonStyle(PsButton* button, const QString& key) {
  button->Color().SetBaseColor(legacyButtonColor(key));
  button->SetBorderRadius(0.0);

  QFont f = button->font();
  f.setPixelSize(key == kEnterKey ? 24 : (key.length() == 1 ? 16 : 20));
  f.setBold(true);
  button->setFont(f);
}
#endif

}  // namespace

const QList<QList<QString>> VirtualKeyboard::kLetterRows = {
    {"A", "B", "C", "D", kBackspaceKey},
    {"E", "F", "G", "H", "I"},
    {"J", "K", "L", "M", "N"},
    {"O", "P", "Q", "R", "S"},
    {"T", "U", "V", "W"},
    {"X", "Y", "Z", kSwitchKey},
};

const QList<QList<QString>> VirtualKeyboard::kNumberRows = {
    {"1", "2", "3", "-", kBackspaceKey},
    {"4", "5", "6", "'", "\""},
    {"7", "8", "9", "(", ")"},
    {"0", ".", "_", ",", ";"},
    {"/", kUpKey, "\\", "|"},
    {kLeftKey, kDownKey, kRightKey, kSwitchKey},
};

VirtualKeyboard::VirtualKeyboard() : QWidget(nullptr) {
  setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                 Qt::WindowDoesNotAcceptFocus | Qt::WindowStaysOnTopHint);
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_TranslucentBackground);
  setFocusPolicy(Qt::NoFocus);
  resize(kKeyboardWidth, kKeyboardHeight);
  SetupUi();
  SwitchToNumbers();
  hide();
}

VirtualKeyboard::~VirtualKeyboard() = default;

void VirtualKeyboard::InstallGlobalFilter() { qApp->installEventFilter(this); }

void VirtualKeyboard::SetTargetLineEdit(QLineEdit* line_edit) {
  target_line_edit_ = line_edit;
  target_text_edit_ = nullptr;
}

void VirtualKeyboard::SetTargetTextEdit(QTextEdit* text_edit) {
  target_text_edit_ = text_edit;
  target_line_edit_ = nullptr;
}

bool VirtualKeyboard::eventFilter(QObject* watched, QEvent* event) {
  // 点击键盘外部区域时隐藏键盘
  if (event->type() == QEvent::MouseButtonPress) {
    QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
    QWidget* widget = qobject_cast<QWidget*>(watched);
    if (widget != nullptr && widget != this && !isAncestorOf(widget)) {
      QPoint global_pos = mouse_event->globalPos();
      if (!rect().contains(mapFromGlobal(global_pos))) {
        hide();
      }
    }
  }

  // 通过qobject_cast判断是否是QLineEdit
  QLineEdit* line_edit = qobject_cast<QLineEdit*>(watched);
  if (line_edit == nullptr) {
    return QWidget::eventFilter(watched, event);
  }

  /* QLineEdit部分 */
  // 如果QLineEdit为只读属性，则不显示虚拟键盘
  if (line_edit->isReadOnly()) {
    return QWidget::eventFilter(watched, event);
  }

  // FocusIn 时判断是否弹出键盘：
  // - MouseFocusReason: 所有输入框都弹（用户主动点击）
  // - OtherFocusReason: 仅表格内的编辑器弹（表格 delegate 创建 editor 时
  //   的焦点变化不是 MouseFocusReason，需要兼容）
  //   其他场景（切页恢复焦点、代码 setFocus 等）不弹，避免误触发
  if (event->type() == QEvent::FocusIn) {
    QFocusEvent* focus_event = static_cast<QFocusEvent*>(event);
    bool should_show = focus_event->reason() == Qt::MouseFocusReason ||
                       (focus_event->reason() == Qt::OtherFocusReason &&
                        IsInsideTable(line_edit));
    if (should_show) {
      SetTargetLineEdit(line_edit);
      PositionBelowWidget(line_edit);
      show();
      raise();
    }
  }

  if (event->type() == QEvent::MouseButtonPress &&
      line_edit == target_line_edit_ && !isVisible()) {
    SetTargetLineEdit(line_edit);
    PositionBelowWidget(line_edit);
    show();
    raise();
  }

  return QWidget::eventFilter(watched, event);
}

void VirtualKeyboard::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);

  QPainter painter(this);
  ps_style::PsDrawSurface(
      &painter, QRectF(rect()).adjusted(3.0, 3.0, -5.0, -5.0));
}

void VirtualKeyboard::SetupUi() {
  setStyleSheet(
      "QWidget { background-color: transparent; }"
      "QFrame#KeyboardPanel { background-color: transparent; border: none; }");

  main_layout_ = new QVBoxLayout(this);
  main_layout_->setContentsMargins(kSurfaceMargin, kSurfaceMargin,
                                   kSurfaceMargin, kSurfaceMargin);
  main_layout_->setSpacing(0);

  QFrame* panel = new QFrame(this);
  panel->setObjectName("KeyboardPanel");

  QVBoxLayout* panel_layout = new QVBoxLayout(panel);
  panel_layout->setContentsMargins(0, 0, 0, 0);
  panel_layout->setSpacing(0);

  keyboard_stack_ = new QStackedWidget(panel);
  keyboard_stack_->addWidget(CreateKeyboardPage(kLetterRows));
  keyboard_stack_->addWidget(CreateKeyboardPage(kNumberRows));

  panel_layout->addWidget(keyboard_stack_);
  main_layout_->addWidget(panel);
}

QPushButton* VirtualKeyboard::CreateKeyButton(const QString& text) {
#if PS_LEGACY_UI
  PsButton* button = new PsButton(buttonText(text), this);
  button->setFixedSize(kKeyWidth, kKeyHeight);
  button->setFocusPolicy(Qt::NoFocus);
  ApplyLegacyButtonStyle(button, text);
  connect(button, &QPushButton::clicked, this,
          [this, text]() { OnKeyClicked(text); });
  return button;
#else
  QPushButton* button = new QPushButton(buttonText(text), this);
  button->setFixedSize(kKeyWidth, kKeyHeight);
  button->setFocusPolicy(Qt::NoFocus);
  button->setStyleSheet(buttonStyle(text));
  connect(button, &QPushButton::clicked, this,
          [this, text]() { OnKeyClicked(text); });
  return button;
#endif
}

QPushButton* VirtualKeyboard::CreateEnterButton() {
#if PS_LEGACY_UI
  PsButton* button = new PsButton(buttonText(kEnterKey), this);
  button->setFixedSize(kKeyWidth, kKeyHeight * 2 + kSpacing);
  button->setFocusPolicy(Qt::NoFocus);
  ApplyLegacyButtonStyle(button, kEnterKey);
  connect(button, &QPushButton::clicked, this,
          [this]() { OnKeyClicked(kEnterKey); });
  return button;
#else
  QPushButton* button = new QPushButton(buttonText(kEnterKey), this);
  button->setFixedSize(kKeyWidth, kKeyHeight * 2 + kSpacing);
  button->setFocusPolicy(Qt::NoFocus);
  button->setStyleSheet(buttonStyle(kEnterKey));
  connect(button, &QPushButton::clicked, this,
          [this]() { OnKeyClicked(kEnterKey); });
  return button;
#endif
}

QFrame* VirtualKeyboard::CreateKeyboardPage(const QList<QList<QString>>& rows) {
  QFrame* frame = new QFrame(this);
  frame->setFrameShape(QFrame::NoFrame);

  QGridLayout* layout = new QGridLayout(frame);
  layout->setContentsMargins(kMargin, kMargin, kMargin, kMargin);
  layout->setHorizontalSpacing(kSpacing);
  layout->setVerticalSpacing(kSpacing);

  for (int row = 0; row < rows.size(); ++row) {
    const QList<QString>& columns = rows.at(row);
    for (int col = 0; col < columns.size(); ++col) {
      const QString& key = columns.at(col);
      layout->addWidget(CreateKeyButton(key), row, col, Qt::AlignCenter);
    }
  }

  layout->addWidget(CreateEnterButton(), 4, 4, 2, 1, Qt::AlignCenter);
  return frame;
}

void VirtualKeyboard::OnKeyClicked(const QString& key) {
  emit KeyPressed(key);

  if (target_line_edit_ != nullptr) {
    target_line_edit_->setFocus(Qt::OtherFocusReason);
  } else if (target_text_edit_ != nullptr) {
    target_text_edit_->setFocus(Qt::OtherFocusReason);
  }

  if (key == kSwitchKey) {
    if (keyboard_stack_->currentIndex() == 0) {
      SwitchToNumbers();
    } else {
      SwitchToLetters();
    }
    return;
  }

  if (key == kBackspaceKey) {
    DeleteBackward();
    emit BackspacePressed();
    return;
  }

  if (key == kEnterKey) {
    if (target_text_edit_ != nullptr) {
      InsertText("\n");
    } else if (target_line_edit_ != nullptr) {
      QKeyEvent key_press(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
      QKeyEvent key_release(QEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier);
      qApp->sendEvent(target_line_edit_, &key_press);
      qApp->sendEvent(target_line_edit_, &key_release);
    }
    emit EnterPressed();
    hide();
    return;
  }

  if (key == kUpKey) {
    MoveCursor(QTextCursor::Up);
    emit CursorUp();
    return;
  }

  if (key == kDownKey) {
    MoveCursor(QTextCursor::Down);
    emit CursorDown();
    return;
  }

  if (key == kLeftKey) {
    MoveCursor(QTextCursor::Left);
    emit CursorLeft();
    return;
  }

  if (key == kRightKey) {
    MoveCursor(QTextCursor::Right);
    emit CursorRight();
    return;
  }

  InsertText(key);
}

void VirtualKeyboard::SwitchToLetters() { keyboard_stack_->setCurrentIndex(0); }

void VirtualKeyboard::SwitchToNumbers() { keyboard_stack_->setCurrentIndex(1); }

void VirtualKeyboard::InsertText(const QString& text) {
  if (target_line_edit_ != nullptr) {
    target_line_edit_->insert(text);
    return;
  }

  if (target_text_edit_ != nullptr) {
    QTextCursor cursor = target_text_edit_->textCursor();
    cursor.insertText(text);
    target_text_edit_->setTextCursor(cursor);
  }
}

void VirtualKeyboard::DeleteBackward() {
  if (target_line_edit_ != nullptr) {
    QString text = target_line_edit_->text();
    int cursor_pos = target_line_edit_->cursorPosition();
    if (cursor_pos > 0) {
      text.remove(cursor_pos - 1, 1);
      target_line_edit_->setText(text);
      target_line_edit_->setCursorPosition(cursor_pos - 1);
    }
    return;
  }

  if (target_text_edit_ != nullptr) {
    QTextCursor cursor = target_text_edit_->textCursor();
    if (!cursor.atStart()) {
      cursor.deletePreviousChar();
      target_text_edit_->setTextCursor(cursor);
    }
  }
}

void VirtualKeyboard::MoveCursor(int direction) {
  if (target_text_edit_ != nullptr) {
    QTextCursor cursor = target_text_edit_->textCursor();
    cursor.movePosition(static_cast<QTextCursor::MoveOperation>(direction));
    target_text_edit_->setTextCursor(cursor);
    return;
  }

  if (target_line_edit_ != nullptr) {
    int pos = target_line_edit_->cursorPosition();
    if (direction == QTextCursor::Left && pos > 0) {
      target_line_edit_->setCursorPosition(pos - 1);
    } else if (direction == QTextCursor::Right &&
               pos < target_line_edit_->text().length()) {
      target_line_edit_->setCursorPosition(pos + 1);
    }
  }
}

void VirtualKeyboard::PositionBelowWidget(QWidget* widget) {
  if (widget == nullptr) {
    return;
  }

  QPoint pos =
      widget->mapToGlobal(QPoint(0, widget->height() + kKeyboardOffset));
  QRect screen_rect = QApplication::desktop()->availableGeometry(widget);

  int x = pos.x();
  int y = pos.y();

  if (x + width() > screen_rect.right()) {
    x = screen_rect.right() - width();
  }
  if (x < screen_rect.left()) {
    x = screen_rect.left();
  }

  if (y + height() > screen_rect.bottom()) {
    y = widget->mapToGlobal(QPoint(0, -height() - kKeyboardOffset)).y();
  }

  if (y < screen_rect.top() || y + height() > screen_rect.bottom()) {
    x = screen_rect.left() + (screen_rect.width() - width()) / 2;
    y = screen_rect.bottom() - height() - kScreenBottomMargin;
  }

  move(x, y);
}
