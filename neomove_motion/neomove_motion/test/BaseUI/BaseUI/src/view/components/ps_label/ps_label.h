// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#ifndef BASE_UI_SRC_VIEW_COMPONENTS_PS_LABEL_PS_LABEL_H_
#define BASE_UI_SRC_VIEW_COMPONENTS_PS_LABEL_PS_LABEL_H_

#include <QLabel>
#include <optional>

#include "view/components/ps_color/ps_color.h"

class PsLabel : public QLabel {
  Q_OBJECT

 public:
  explicit PsLabel(QWidget* parent = nullptr);
  PsLabel(const QString& text, QWidget* parent = nullptr);

  PsColor& Color();
  void SetBorderRadius(qreal radius);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void showEvent(QShowEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

 private:
  void PaintCard(QPainter* painter);
  void DetectNeighbors();
  void ScheduleRedetect();
  QPainterPath BuildRoundedRectPath(const QRectF& rect) const;

  PsColor color_ = PsColor(QColor(247, 249, 252));
  qreal base_border_radius_ = 10.0;
  std::optional<qreal> corner_top_left_;
  std::optional<qreal> corner_top_right_;
  std::optional<qreal> corner_bottom_right_;
  std::optional<qreal> corner_bottom_left_;
  bool neighbor_cache_valid_ = false;
  bool redetect_scheduled_ = false;
};

#endif  // BASE_UI_SRC_VIEW_COMPONENTS_PS_LABEL_PS_LABEL_H_
