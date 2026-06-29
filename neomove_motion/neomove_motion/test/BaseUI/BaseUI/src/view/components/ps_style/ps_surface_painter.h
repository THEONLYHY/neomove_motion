// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/06/04

#ifndef BASE_UI_SRC_VIEW_COMPONENTS_PS_STYLE_PS_SURFACE_PAINTER_H_
#define BASE_UI_SRC_VIEW_COMPONENTS_PS_STYLE_PS_SURFACE_PAINTER_H_

#include <QColor>
#include <QPainter>
#include <QRectF>

namespace ps_style {

struct PsSurfaceOption {
  QColor fill = QColor(0xFF, 0xFF, 0xFF);
  QColor border = QColor(0xE5, 0xE7, 0xEB);
  qreal radius = 12.0;
};

void PsDrawSurface(QPainter* painter, const QRectF& card_rect,
                   const PsSurfaceOption& option = PsSurfaceOption());

}  // namespace ps_style

#endif  // BASE_UI_SRC_VIEW_COMPONENTS_PS_STYLE_PS_SURFACE_PAINTER_H_
