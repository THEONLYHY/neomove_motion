// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/06/04

#ifndef BASE_UI_SRC_VIEW_COMPONENTS_PS_STYLE_PS_LEGACY_PAINTER_H_
#define BASE_UI_SRC_VIEW_COMPONENTS_PS_STYLE_PS_LEGACY_PAINTER_H_

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QRectF>

#include "view/components/ps_color/ps_color.h"

namespace ps_style {

QColor LegacyTextColor(const QColor& base, bool enabled = true);
QColor LegacyDisabledSurface();

void DrawLegacyBeveledPanel(QPainter* painter, const QRectF& rect,
                            const PsColor::Derived& cfg, bool pressed,
                            bool enabled, qreal edge = -1.0);

void DrawLegacyBeveledPath(QPainter* painter, const QPainterPath& path,
                           const QRectF& bounds,
                           const PsColor::Derived& cfg, bool pressed,
                           bool enabled, qreal border_width = 3.0);

void DrawLegacyInputPanel(QPainter* painter, const QRectF& rect,
                          const QColor& surface, bool active, bool enabled);

}  // namespace ps_style

#endif  // BASE_UI_SRC_VIEW_COMPONENTS_PS_STYLE_PS_LEGACY_PAINTER_H_
