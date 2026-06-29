// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#ifndef BASE_UI_SRC_VIEW_COMPONENTS_PS_TEXT_EDIT_PS_TEXT_EDIT_H_
#define BASE_UI_SRC_VIEW_COMPONENTS_PS_TEXT_EDIT_PS_TEXT_EDIT_H_

#include <QTextEdit>

#include "view/components/ps_color/ps_color.h"

class PsTextEdit : public QTextEdit {
  Q_OBJECT

 public:
  explicit PsTextEdit(QWidget* parent = nullptr);

  PsColor& Color();
  void SetBorderRadius(qreal radius);

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  PsColor color_;
  qreal border_radius_ = 8.0;
};



#endif  // BASE_UI_SRC_VIEW_COMPONENTS_PS_TEXT_EDIT_PS_TEXT_EDIT_H_
