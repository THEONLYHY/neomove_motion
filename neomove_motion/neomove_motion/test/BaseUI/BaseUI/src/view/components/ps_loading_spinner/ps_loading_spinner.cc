// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#include "ps_loading_spinner.h"

#include <QPainter>
#include <QtMath>

#include "view/components/ps_style/ps_legacy_painter.h"

PsLoadingSpinner::PsLoadingSpinner(QWidget* parent) : QWidget(parent) {
  color_.SetOnChanged([this] { update(); });

  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_OpaquePaintEvent, false);
  setAutoFillBackground(false);
  color_.SetBaseColor(PsColor::Color::kBlue);

  connect(&timer_, &QTimer::timeout, this, &PsLoadingSpinner::OnTimeout);
  timer_.setInterval(50);
}

QSize PsLoadingSpinner::sizeHint() const {
  qreal s = ring_diameter_ + dot_diameter_ * 2 + 4;
  return QSize(static_cast<int>(s), static_cast<int>(s));
}

void PsLoadingSpinner::SetDotCount(int count) {
  dot_count_ = qMax(2, count);
  update();
}

void PsLoadingSpinner::SetRingDiameter(qreal diameter) {
  ring_diameter_ = qMax(dot_diameter_ * 2, diameter);
  updateGeometry();
  update();
}

void PsLoadingSpinner::SetDotDiameter(qreal diameter) {
  dot_diameter_ = qMax(1.0, diameter);
  updateGeometry();
  update();
}

PsColor& PsLoadingSpinner::Color() { return color_; }

void PsLoadingSpinner::Start() {
  phase_ = 0.0;
  timer_.start();
}

void PsLoadingSpinner::Stop() {
  timer_.stop();
  phase_ = 0.0;
  update();
}

bool PsLoadingSpinner::IsRunning() const { return timer_.isActive(); }

void PsLoadingSpinner::OnTimeout() {
  phase_ = fmod(phase_ + 0.4, dot_count_);
  update();
}

void PsLoadingSpinner::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  const QColor dot_color = color_.Base().isValid() ? color_.Base() : PsColor::WidgetColor::kFocusBorder;

#if PS_LEGACY_UI
  {
    painter.setRenderHint(QPainter::Antialiasing, false);
    const qreal cx = width() / 2.0;
    const qreal cy = height() / 2.0;
    const qreal ring_radius = ring_diameter_ / 2.0;
    const qreal side = qMax<qreal>(2.0, dot_diameter_);
    const int wave_len = dot_count_ / 2;

    for (int i = 0; i < dot_count_; ++i) {
      const qreal angle = qDegreesToRadians(i * 360.0 / dot_count_ - 90.0);
      const qreal x = cx + ring_radius * qCos(angle);
      const qreal y = cy + ring_radius * qSin(angle);
      double dist = fmod(phase_ - i + dot_count_, dot_count_);
      qreal t = qMax(0.0, 1.0 - dist / wave_len);
      t = t * t;
      QColor c = dot_color.lighter(static_cast<int>(100 + 45 * t));
      c.setAlpha(static_cast<int>(70 + 185 * t));
      painter.setPen(QPen(dot_color.darker(160), 1.0));
      painter.setBrush(c);
      painter.drawRect(QRectF(x - side / 2, y - side / 2, side, side));
    }
  }
  return;
#endif

  const qreal cx = width() / 2.0;
  const qreal cy = height() / 2.0;
  const qreal ring_radius = ring_diameter_ / 2.0;
  const qreal base_r = dot_diameter_ / 2.0;
  const qreal max_r = base_r * 1.6;
  const int wave_len = dot_count_ / 2;

  for (int i = 0; i < dot_count_; ++i) {
    const qreal angle = qDegreesToRadians(i * 360.0 / dot_count_ - 90.0);
    const qreal x = cx + ring_radius * qCos(angle);
    const qreal y = cy + ring_radius * qSin(angle);

    double dist = fmod(phase_ - i + dot_count_, dot_count_);
    qreal t = qMax(0.0, 1.0 - dist / wave_len);
    t = t * t;

    const qreal r = base_r + (max_r - base_r) * t;
    const int alpha = static_cast<int>(60 + 195 * t);

    painter.setPen(Qt::NoPen);
    painter.setBrush(
        QColor(dot_color.red(), dot_color.green(), dot_color.blue(), alpha));
    painter.drawEllipse(QPointF(x, y), r, r);
  }
}
