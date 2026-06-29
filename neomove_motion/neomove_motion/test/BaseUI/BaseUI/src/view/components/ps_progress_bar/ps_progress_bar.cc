// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#include "ps_progress_bar.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>

#include "view/components/ps_style/ps_legacy_painter.h"

PsProgressBar::PsProgressBar(QWidget* parent) : QProgressBar(parent) {
  color_.SetOnChanged([this] { update(); });

  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_OpaquePaintEvent, false);
  setAutoFillBackground(false);
  setTextVisible(false);
}

PsColor& PsProgressBar::Color() { return color_; }

void PsProgressBar::SetBorderRadius(qreal radius) {
  border_radius_ = radius;
  update();
}

void PsProgressBar::SetTrackVisible(bool visible) {
  track_visible_ = visible;
  update();
}

void PsProgressBar::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);
  const auto& cfg = color_.Get();
  const QRectF r = rect().adjusted(2.0, 2.0, -2.0, -2.0);
  const qreal radius = r.height() / 2.0;
  const int range = maximum() - minimum();
  const qreal progress =
      (range <= 0)
          ? 0.0
          : qBound(0.0, (value() - minimum()) / static_cast<qreal>(range), 1.0);
  const qreal fill_width = r.width() * progress;

  painter.setRenderHint(QPainter::Antialiasing);

#if PS_LEGACY_UI
  {
    const QRectF legacy_r = rect().adjusted(1.0, 1.0, -1.0, -1.0);
    if (track_visible_) {
      ps_style::DrawLegacyInputPanel(&painter, legacy_r,
                                     PsColor::WidgetColor::kTrackBg, false,
                                     isEnabled());
    }
    if (fill_width <= 0.0) return;
    QRectF fill_rect = legacy_r.adjusted(2.0, 2.0, -2.0, -2.0);
    fill_rect.setWidth(qMax<qreal>(4.0, fill_rect.width() * progress));
    ps_style::DrawLegacyBeveledPanel(&painter, fill_rect, cfg, false,
                                     isEnabled(), 2.0);
  }
  return;
#endif

  if (track_visible_) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(PsColor::WidgetColor::kTrackBg);
    painter.drawRoundedRect(r, radius, radius);
  }

  if (fill_width <= 0.0) return;

  QRectF fill_rect = r;
  fill_rect.setWidth(fill_width);

  painter.save();
  QPainterPath clip_path;
  clip_path.addRoundedRect(r, radius, radius);
  painter.setClipPath(clip_path);

  if (fill_rect.width() < radius * 2.0) {
    fill_rect.setWidth(radius * 2.0);
  }

  QLinearGradient fill_gradient(fill_rect.topLeft(), fill_rect.bottomLeft());
  fill_gradient.setColorAt(0.0, cfg.top);
  fill_gradient.setColorAt(1.0, cfg.base);

  painter.setPen(Qt::NoPen);
  painter.setBrush(fill_gradient);
  painter.drawRoundedRect(fill_rect, radius, radius);

  painter.restore();
}
