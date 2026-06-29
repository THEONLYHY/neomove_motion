// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#ifndef BASE_UI_SRC_VIEW_COMPONENTS_PS_LINE_EDIT_PS_LINE_EDIT_H_
#define BASE_UI_SRC_VIEW_COMPONENTS_PS_LINE_EDIT_PS_LINE_EDIT_H_

#include <QLineEdit>
#include <optional>

#include "view/components/ps_color/ps_color.h"

class PsLineEdit : public QLineEdit {
  Q_OBJECT

 public:
  explicit PsLineEdit(QWidget* parent = nullptr);
  PsLineEdit(const QString& text, QWidget* parent = nullptr);

  PsColor& Color();
  void SetBorderRadius(qreal radius);
  void SetCustomBgColor(const QColor& color);
  QColor CustomBgColor() const { return custom_bg_color_; }

  static const QColor kDefaultColor;

 protected:
  void paintEvent(QPaintEvent* event) override;
  void focusInEvent(QFocusEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;
  void showEvent(QShowEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

 private:
  void PaintCard(QPainter* painter);
  void DetectNeighbors();
  void ScheduleRedetect();
  QPainterPath BuildRoundedRectPath(const QRectF& rect) const;

  PsColor color_ = PsColor(QColor(0xFF, 0xFF, 0xFF));
  qreal base_border_radius_ = 8.0;
  std::optional<qreal> corner_top_left_;
  std::optional<qreal> corner_top_right_;
  std::optional<qreal> corner_bottom_right_;
  std::optional<qreal> corner_bottom_left_;
  bool neighbor_cache_valid_ = false;
  bool redetect_scheduled_ = false;

  bool is_focused_ = false;

  QColor custom_bg_color_;
  bool has_custom_bg_color_ = false;
};

#endif  // BASE_UI_SRC_VIEW_COMPONENTS_PS_LINE_EDIT_PS_LINE_EDIT_H_
