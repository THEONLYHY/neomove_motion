// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#ifndef BASE_UI_SRC_VIEW_COMPONENTS_PS_BUTTON_PS_BUTTON_H_
#define BASE_UI_SRC_VIEW_COMPONENTS_PS_BUTTON_PS_BUTTON_H_

#include <QPushButton>

#include "view/components/ps_color/ps_color.h"

class PsButton : public QPushButton {
  Q_OBJECT

 public:
  explicit PsButton(QWidget* parent = nullptr);
  PsButton(const QString& text, QWidget* parent = nullptr);

  PsColor& Color();
  PsColor& CheckedColor();

  void SetBorderRadius(qreal radius);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void enterEvent(QEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

 private:
  PsColor color_ = PsColor(PsColor::Color::kBlue);
  PsColor checked_color_ = PsColor(PsColor::Color::kOrange);
  qreal border_radius_ = 10.0;
  bool is_hovered_ = false;
  bool is_pressed_ = false;
};



#endif  // BASE_UI_SRC_VIEW_COMPONENTS_PS_BUTTON_PS_BUTTON_H_
