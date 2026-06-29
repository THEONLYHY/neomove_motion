// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#include "ps_button.h"

#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>

#include "view/components/ps_style/ps_legacy_painter.h"

PsButton::PsButton(QWidget* parent) : QPushButton(parent) {
  color_.SetOnChanged([this] { update(); });
  checked_color_.SetOnChanged([this] { update(); });
  setAttribute(Qt::WA_TranslucentBackground);
}

PsButton::PsButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent) {
  color_.SetOnChanged([this] { update(); });
  checked_color_.SetOnChanged([this] { update(); });
  setAttribute(Qt::WA_TranslucentBackground);
}

PsColor& PsButton::Color() { return color_; }

PsColor& PsButton::CheckedColor() { return checked_color_; }

void PsButton::SetBorderRadius(qreal radius) {
  border_radius_ = radius;
  update();
}

void PsButton::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);

  const bool use_checked = isCheckable() && isChecked();
  const auto& cfg = use_checked ? checked_color_.Get() : color_.Get();
  const bool is_visually_pressed = is_pressed_ || use_checked;
  const bool should_show_checked_state = use_checked;

#if PS_LEGACY_UI
  QRectF legacy_rect = rect().adjusted(1.0, 1.0, -1.0, -1.0);
  ps_style::DrawLegacyBeveledPanel(&painter, legacy_rect, cfg,
                                   is_visually_pressed, isEnabled());
  painter.setPen((!isEnabled() && !should_show_checked_state)
                     ? PsColor::WidgetColor::kTextDisabled
                     : ps_style::LegacyTextColor(cfg.base, isEnabled()));
  painter.setFont(font());
  painter.drawText(legacy_rect.translated(0, is_visually_pressed ? 1.0 : 0.0),
                   Qt::AlignCenter, text());
  return;
#endif

  qreal margin_x = 4.0;
  qreal margin_top = 2.0;
  qreal margin_bottom = 4.0;
  QRectF r = rect().adjusted(margin_x, margin_top, -margin_x, -margin_bottom);
  const qreal radius = border_radius_;

  painter.setRenderHint(QPainter::Antialiasing);

  if (is_visually_pressed && isEnabled()) r.translate(0, 1.5);

  if (isEnabled()) {
    if (is_visually_pressed) {
      painter.setOpacity(0.06);
      painter.setBrush(Qt::black);
      painter.setPen(Qt::NoPen);
      painter.drawRoundedRect(r.translated(0, 0.5), radius, radius);
      painter.setOpacity(1.0);
    } else {
      painter.save();
      for (int i = 1; i <= 3; ++i) {
        painter.setOpacity((cfg.shadow_alpha / 255.0) / i);
        qreal spread = i * 1.8;
        painter.setBrush(cfg.shadow);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(r.adjusted(-spread, spread, spread, spread + i),
                                radius + spread, radius + spread);
      }
      painter.restore();
    }
  }

  QLinearGradient gradient(r.topLeft(), r.bottomLeft());
  if (!isEnabled() && !should_show_checked_state) {
    gradient.setColorAt(0.0, PsColor::WidgetColor::kBgDisabled);
    gradient.setColorAt(1.0, PsColor::WidgetColor::kBgDisabled);
  } else if (is_visually_pressed) {
    gradient.setColorAt(0.0, cfg.base.darker(112));
    gradient.setColorAt(1.0, cfg.base);
  } else if (is_hovered_) {
    gradient.setColorAt(0.0, cfg.top.lighter(105));
    gradient.setColorAt(1.0, cfg.base.lighter(105));
  } else {
    gradient.setColorAt(0.0, cfg.top);
    gradient.setColorAt(1.0, cfg.base);
  }

  painter.setPen(Qt::NoPen);
  painter.setBrush(gradient);
  painter.drawRoundedRect(r, radius, radius);

  if (isEnabled() || should_show_checked_state) {
    painter.setPen(QPen(QColor(255, 255, 255, cfg.highlight_alpha), 1.1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(r.adjusted(0.5, 0.5, -0.5, -0.5), radius, radius);

    const QRgb rgb = color_.Base().rgb();
    const bool is_light_style = (rgb == QColor(0xF3, 0xF4, 0xF6).rgb() ||
                                 rgb == QColor(0x64, 0x74, 0x8B).rgb());
    if (is_light_style) {
      painter.setPen(QPen(QColor(0, 0, 0, 25), 0.5));
      painter.drawRoundedRect(r.adjusted(-0.5, -0.5, 0.5, 0.5), radius, radius);
    }
  }

  painter.setPen((!isEnabled() && !should_show_checked_state)
                     ? PsColor::WidgetColor::kTextDisabled
                     : cfg.text);
  QFont f = font();
  painter.setFont(f);
  painter.drawText(r.translated(0, is_visually_pressed ? 0.5 : -0.5),
                   Qt::AlignCenter, text());
}

void PsButton::enterEvent(QEvent* event) {
  is_hovered_ = true;
  update();
  QPushButton::enterEvent(event);
}
void PsButton::leaveEvent(QEvent* event) {
  is_hovered_ = false;
  update();
  QPushButton::leaveEvent(event);
}
void PsButton::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    is_pressed_ = true;
    update();
  }
  QPushButton::mousePressEvent(event);
}
void PsButton::mouseReleaseEvent(QMouseEvent* event) {
  is_pressed_ = false;
  update();
  QPushButton::mouseReleaseEvent(event);
}
