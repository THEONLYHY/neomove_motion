// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/06/04

#include "view/components/ps_style/ps_surface_painter.h"

#include <QPainterPath>

#include "view/components/ps_color/ps_color.h"

namespace ps_style {
namespace {

void DrawModernSurface(QPainter* painter, const QRectF& card_rect,
                       const PsSurfaceOption& option) {
  painter->setRenderHint(QPainter::Antialiasing, true);

  for (int i = 5; i >= 1; --i) {
    const qreal spread = i * 1.5;
    QRectF shadow_rect =
        card_rect.adjusted(-spread, -spread * 0.25, spread, spread);
    shadow_rect.translate(0.0, i * 0.55);

    QPainterPath shadow_path;
    shadow_path.addRoundedRect(shadow_rect, option.radius + spread,
                               option.radius + spread);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(15, 23, 42, 18 / i));
    painter->drawPath(shadow_path);
  }

  QPainterPath path;
  path.addRoundedRect(card_rect, option.radius, option.radius);
  painter->setPen(Qt::NoPen);
  painter->setBrush(option.fill);
  painter->drawPath(path);

  painter->setPen(QPen(option.border, 1.0));
  painter->setBrush(Qt::NoBrush);
  painter->drawPath(path);
}

void DrawLegacySurface(QPainter* painter, const QRectF& card_rect,
                       const PsSurfaceOption& option) {
  painter->setRenderHint(QPainter::Antialiasing, false);

  QRectF shadow_rect = card_rect.translated(3.0, 3.0);
  painter->setPen(Qt::NoPen);
  painter->setBrush(QColor(0, 0, 0, 42));
  painter->drawRect(shadow_rect);

  QRectF r = card_rect.adjusted(0.5, 0.5, -0.5, -0.5);
  const QColor fill = option.fill.isValid() ? option.fill : QColor(0xF4, 0xF5, 0xF7);
  const QColor border =
      option.border.isValid() ? option.border : QColor(0x78, 0x80, 0x8C);

  painter->setPen(Qt::NoPen);
  painter->setBrush(fill);
  painter->drawRect(r);

  painter->setPen(QPen(QColor(0xFF, 0xFF, 0xFF), 1.0));
  painter->drawLine(r.topLeft(), r.topRight());
  painter->drawLine(r.topLeft(), r.bottomLeft());

  painter->setPen(QPen(border.darker(160), 1.0));
  painter->drawLine(r.topRight(), r.bottomRight());
  painter->drawLine(r.bottomLeft(), r.bottomRight());

  painter->setPen(QPen(border.darker(190), 1.0));
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(r);
}

}  // namespace

void PsDrawSurface(QPainter* painter, const QRectF& card_rect,
                   const PsSurfaceOption& option) {
  if (painter == nullptr || card_rect.width() <= 0.0 ||
      card_rect.height() <= 0.0) {
    return;
  }

  painter->save();
  if (PsColor::IsLegacyUi()) {
    DrawLegacySurface(painter, card_rect.normalized(), option);
  } else {
    DrawModernSurface(painter, card_rect.normalized(), option);
  }
  painter->restore();
}

}  // namespace ps_style
