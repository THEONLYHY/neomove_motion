// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/04/23 14:59

#ifndef BASE_UI_SRC_VIEW_TOOLS_VIRTUAL_KEYBOARD_VIRTUAL_KEYBOARD_H_
#define BASE_UI_SRC_VIEW_TOOLS_VIRTUAL_KEYBOARD_VIRTUAL_KEYBOARD_H_

#include <singleton.h>

#include <QFrame>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>


class QLineEdit;
class QPaintEvent;
class QTextEdit;

class VirtualKeyboard : public QWidget {
  Q_OBJECT
  SINGLETON(VirtualKeyboard);

 public:
  ~VirtualKeyboard();

  void SetTargetLineEdit(QLineEdit* line_edit);
  void SetTargetTextEdit(QTextEdit* text_edit);

  void InstallGlobalFilter();

 signals:
  void KeyPressed(const QString& key);
  void EnterPressed();
  void BackspacePressed();
  void CursorUp();
  void CursorDown();
  void CursorLeft();
  void CursorRight();

 protected:
  bool eventFilter(QObject* watched, QEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

 private:
  VirtualKeyboard();

  void SetupUi();
  QPushButton* CreateKeyButton(const QString& text);
  QPushButton* CreateEnterButton();
  QFrame* CreateKeyboardPage(const QList<QList<QString>>& rows);
  void OnKeyClicked(const QString& key);
  void SwitchToLetters();
  void SwitchToNumbers();
  void InsertText(const QString& text);
  void DeleteBackward();
  void MoveCursor(int direction);
  void PositionBelowWidget(QWidget* widget);

  QVBoxLayout* main_layout_ = nullptr;
  QStackedWidget* keyboard_stack_ = nullptr;
  QLineEdit* target_line_edit_ = nullptr;
  QTextEdit* target_text_edit_ = nullptr;

  static const QList<QList<QString>> kLetterRows;
  static const QList<QList<QString>> kNumberRows;
};

using VirtualKeyboardSingleton = yotta::Singleton<VirtualKeyboard>;

#endif  // BASE_UI_SRC_VIEW_TOOLS_VIRTUAL_KEYBOARD_VIRTUAL_KEYBOARD_H_
