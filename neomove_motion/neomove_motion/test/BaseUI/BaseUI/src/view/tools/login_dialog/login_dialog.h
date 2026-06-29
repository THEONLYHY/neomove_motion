// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/04/23 10:00

#ifndef BASE_UI_SRC_VIEW_TOOLS_LOGIN_DIALOG_LOGIN_DIALOG_H_
#define BASE_UI_SRC_VIEW_TOOLS_LOGIN_DIALOG_LOGIN_DIALOG_H_

#include <QDialog>
#include <QLabel>
#include <QPaintEvent>

#include "view/components/ps_button/ps_button.h"
#include "view/components/ps_combo_box/ps_combo_box.h"
#include "view/components/ps_line_edit/ps_line_edit.h"

class UserManager;

class LoginDialog : public QDialog {
  Q_OBJECT

 public:
  explicit LoginDialog(UserManager* user_mgr, QWidget* parent = nullptr);
  ~LoginDialog();

  void ShowDialog();

 signals:
  void LoginSuccess(class User* user);

 private:
  void OnLoginClicked();
  void OnCancelClicked();
  void RefreshUserList();
  void ShowError(const QString& msg);

 protected:
  void paintEvent(QPaintEvent* event) override;

  UserManager* user_mgr_ = nullptr;
  PsComboBox* user_combo_ = nullptr;
  PsLineEdit* password_edit_ = nullptr;
  PsButton* login_button_ = nullptr;
  PsButton* cancel_button_ = nullptr;
  QLabel* status_label_ = nullptr;
};

#endif  // BASE_UI_SRC_VIEW_TOOLS_LOGIN_DIALOG_LOGIN_DIALOG_H_
