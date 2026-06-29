// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/06/04

#include "ps_legacy_painter.h"

#include <QtMath>

namespace {

qreal Luma(const QColor& color) {
  return 0.2126 * color.redF() + 0.7152 * color.greenF() +
         0.0722 * color.blueF();
}

QColor WithAlpha(QColor color, int alpha) {
  color.setAlpha(qBound(0, alpha, 255));
  return color;
}

QColor DisabledBorder() { return QColor(0x93, 0x99, 0xA3); }

}  // namespace

namespace ps_style {

QColor LegacyTextColor(const QColor& base, bool enabled) {
  if (!enabled) return QColor(0x6B, 0x72, 0x80);
  return Luma(base) >= 0.62 ? QColor(0x11, 0x18, 0x27) : QColor(Qt::white);
}

QColor LegacyDisabledSurface() { return QColor(0xD7, 0xDC, 0xE2); }

void DrawLegacyBeveledPanel(QPainter* painter, const QRectF& rect,
                            const PsColor::Derived& cfg, bool pressed,
                            bool enabled, qreal edge) {
  if (!painter) return;

  QRectF r = rect.normalized();
  if (r.width() <= 0.0 || r.height() <= 0.0) return;

  const qreal auto_edge = qBound<qreal>(3.0, qMin(r.width(), r.height()) * 0.11,
                                        8.0);
  const qreal max_edge = qMax<qreal>(1.0, qMin(r.width(), r.height()) / 2.0);
  const qreal e = qMin(edge > 0.0 ? edge : auto_edge, max_edge);

  QColor base = enabled ? cfg.base : LegacyDisabledSurface();
  QColor center = pressed && enabled ? base.darker(128) : base;
  QColor light = enabled ? base.lighter(160) : QColor(0xF1, 0xF3, 0xF5);
  QColor mid_light = enabled ? base.lighter(128) : QColor(0xE5, 0xE7, 0xEB);
  QColor dark = enabled ? base.darker(165) : DisabledBorder();
  QColor deep = enabled ? base.darker(205) : QColor(0x7D, 0x86, 0x94);

  if (pressed && enabled) {
    qSwap(light, deep);
    qSwap(mid_light, dark);
  }

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, false);

  painter->setPen(Qt::NoPen);
  painter->setBrush(deep);
  painter->drawRect(r);

  const QRectF content = r.adjusted(e, e, -e, -e);
  const QPointF outer_tl = r.topLeft();
  const QPointF outer_tr = r.topRight();
  const QPointF outer_br = r.bottomRight();
  const QPointF outer_bl = r.bottomLeft();
  const QPointF inner_tl = content.topLeft();
  const QPointF inner_tr = content.topRight();
  const QPointF inner_br = content.bottomRight();
  const QPointF inner_bl = content.bottomLeft();

  painter->setBrush(light);
  painter->drawPolygon(QPolygonF({outer_tl, outer_tr, inner_tr, inner_tl}));
  painter->drawPolygon(QPolygonF({outer_tl, inner_tl, inner_bl, outer_bl}));

  painter->setBrush(dark);
  painter->drawPolygon(QPolygonF({outer_tr, outer_br, inner_br, inner_tr}));
  painter->drawPolygon(QPolygonF({outer_bl, inner_bl, inner_br, outer_br}));

  painter->setBrush(center);
  painter->drawRect(content);

  QColor inner_line = pressed && enabled ? WithAlpha(Qt::black, 34)
                                         : WithAlpha(Qt::white, 30);
  painter->setPen(QPen(inner_line, 1.0));
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(content.adjusted(0.5, 0.5, -0.5, -0.5));

  painter->setPen(QPen(enabled ? WithAlpha(deep, 190) : DisabledBorder(), 1.0));
  painter->drawRect(r.adjusted(0.5, 0.5, -0.5, -0.5));

  painter->restore();
}

void DrawLegacyBeveledPath(QPainter* painter, const QPainterPath& path,
                           const QRectF& bounds,
                           const PsColor::Derived& cfg, bool pressed,
                           bool enabled, qreal border_width) {
  if (!painter) return;

  QColor base = enabled ? cfg.base : LegacyDisabledSurface();
  QColor light = enabled ? base.lighter(155) : QColor(0xF1, 0xF3, 0xF5);
  QColor dark = enabled ? base.darker(180) : DisabledBorder();
  if (pressed && enabled) qSwap(light, dark);

  QLinearGradient fill(bounds.topLeft(), bounds.bottomRight());
  fill.setColorAt(0.0, light);
  fill.setColorAt(0.35, pressed && enabled ? base.darker(128) : base);
  fill.setColorAt(0.72, pressed && enabled ? base.darker(128) : base);
  fill.setColorAt(1.0, dark);

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(Qt::NoPen);
  painter->setBrush(fill);
  painter->drawPath(path);
  painter->setPen(QPen(dark, border_width, Qt::SolidLine, Qt::RoundCap,
                       Qt::RoundJoin));
  painter->setBrush(Qt::NoBrush);
  painter->drawPath(path);
  painter->setPen(QPen(WithAlpha(light, 170), qMax<qreal>(1.0, border_width / 2),
                       Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  painter->drawPath(path.translated(-0.8, -0.8));
  painter->restore();
}

void DrawLegacyInputPanel(QPainter* painter, const QRectF& rect,
                          const QColor& surface, bool active, bool enabled) {
  if (!painter) return;

  QRectF r = rect.normalized();
  QColor fill = surface;
  QColor border = active && enabled ? QColor(0x25, 0x0D, 0xFF)
                                    : QColor(0x8E, 0x97, 0xA4);

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, false);
  painter->setPen(Qt::NoPen);
  painter->setBrush(fill);
  painter->drawRect(r);

  painter->setPen(QPen(fill.lighter(125), 1.0));
  painter->drawLine(r.left() + 1, r.top() + 1, r.right() - 1, r.top() + 1);
  painter->drawLine(r.left() + 1, r.top() + 1, r.left() + 1, r.bottom() - 1);

  painter->setPen(QPen(fill.darker(125), 1.0));
  painter->drawLine(r.left() + 1, r.bottom() - 1, r.right() - 1,
                    r.bottom() - 1);
  painter->drawLine(r.right() - 1, r.top() + 1, r.right() - 1,
                    r.bottom() - 1);

  painter->setPen(QPen(border, active && enabled ? 2.0 : 1.0));
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(r.adjusted(0.5, 0.5, -0.5, -0.5));
  painter->restore();
}

}  // namespace ps_style
