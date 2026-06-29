// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/29 10:05

#ifndef BASE_UI_SRC_VIEW_COMPONENTS_PS_ARROW_BUTTON_PS_ARROW_BUTTON_H_
#define BASE_UI_SRC_VIEW_COMPONENTS_PS_ARROW_BUTTON_PS_ARROW_BUTTON_H_

#include <QPushButton>
#include <QPainterPath>

#include "view/components/ps_color/ps_color.h"

class PsArrowButton : public QPushButton {
  Q_OBJECT

 public:
  enum class Direction { Up, Down, Left, Right, UpRight, UpLeft, DownRight, DownLeft };
  enum class ArrowStyle { Straight, Curved };

  explicit PsArrowButton(QWidget* parent = nullptr);
  PsArrowButton(const QString& text, QWidget* parent = nullptr);

  PsColor& Color();

  void SetDirection(Direction dir);
  void SetArrowStyle(ArrowStyle style);
  void SetBorderRadius(qreal radius);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void enterEvent(QEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

 private:
  QPainterPath BuildArrowPath(const QRectF& r);
  QPainterPath BuildLegacyArrowGlyphPath(const QRectF& r);
  QPainterPath BuildLegacyStraightArrowPath(const QRectF& r);

  PsColor color_ = PsColor(PsColor::Color::kGreen);
  qreal border_radius_ = 10.0;
  Direction direction_ = Direction::Right;
  ArrowStyle style_ = ArrowStyle::Straight;

  bool is_hovered_ = false;
  bool is_pressed_ = false;
};



#endif  // BASE_UI_SRC_VIEW_COMPONENTS_PS_ARROW_BUTTON_PS_ARROW_BUTTON_H_
