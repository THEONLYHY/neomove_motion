// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#include "ps_text_edit.h"

#include <QFrame>
#include <QPainter>

#include "view/components/ps_style/ps_legacy_painter.h"

PsTextEdit::PsTextEdit(QWidget* parent) : QTextEdit(parent) {
  color_.SetOnChanged([this] { update(); });

  QPalette p = palette();
  p.setColor(QPalette::Base, Qt::transparent);
  p.setColor(QPalette::Text, PsColor::WidgetColor::kTextDefault);
  setPalette(p);
  setFrameShape(QFrame::NoFrame);
  setAttribute(Qt::WA_TranslucentBackground);
  color_.SetBaseColor(QColor(0xFF, 0xFF, 0xFF));
}

PsColor& PsTextEdit::Color() { return color_; }

void PsTextEdit::SetBorderRadius(qreal radius) {
  border_radius_ = radius;
  update();
}

void PsTextEdit::paintEvent(QPaintEvent* event) {
#if PS_LEGACY_UI
  {
    QPainter painter(viewport());
    QRectF r = viewport()->rect().adjusted(1.0, 1.0, -1.0, -1.0);
    const QColor surface = color_.Base().isValid() ? color_.Base()
                                                   : QColor(0xFF, 0xFF, 0xFF);
    ps_style::DrawLegacyInputPanel(&painter, r, surface, hasFocus(),
                                   isEnabled());
  }
#endif
  QTextEdit::paintEvent(event);
}
