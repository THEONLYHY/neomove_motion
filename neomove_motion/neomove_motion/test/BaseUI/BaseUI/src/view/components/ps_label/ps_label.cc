// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#include "ps_label.h"

#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QShowEvent>
#include <QTimer>

#include "view/components/ps_style/ps_legacy_painter.h"

PsLabel::PsLabel(QWidget* parent) : QLabel(parent) {
  color_.SetOnChanged([this] { update(); });
  setAttribute(Qt::WA_TranslucentBackground);
  setProperty("ps_groupable", true);
}

PsLabel::PsLabel(const QString& text, QWidget* parent) : QLabel(text, parent) {
  color_.SetOnChanged([this] { update(); });
  setAttribute(Qt::WA_TranslucentBackground);
  setProperty("ps_groupable", true);
}

PsColor& PsLabel::Color() { return color_; }

void PsLabel::SetBorderRadius(qreal radius) {
  base_border_radius_ = radius;
  if (corner_top_left_.has_value()) corner_top_left_ = radius;
  if (corner_top_right_.has_value()) corner_top_right_ = radius;
  if (corner_bottom_right_.has_value()) corner_bottom_right_ = radius;
  if (corner_bottom_left_.has_value()) corner_bottom_left_ = radius;
  update();
}

void PsLabel::DetectNeighbors() {
  if (!parentWidget()) return;

  auto old_tl = corner_top_left_;
  auto old_tr = corner_top_right_;
  auto old_br = corner_bottom_right_;
  auto old_bl = corner_bottom_left_;

  corner_top_left_ = base_border_radius_;
  corner_top_right_ = base_border_radius_;
  corner_bottom_right_ = base_border_radius_;
  corner_bottom_left_ = base_border_radius_;

  bool has_left = false, has_right = false, has_top = false, has_bottom = false;

  const QRectF my_rect = QRectF(geometry());
  const qreal kTolerance = 0.5;

  auto yOverlap = [](const QRectF& a, const QRectF& b) -> qreal {
    qreal overlap = qMin(a.bottom(), b.bottom()) - qMax(a.top(), b.top());
    qreal shorter = qMin(a.height(), b.height());
    return (shorter > 0) ? (overlap / shorter) : 0.0;
  };
  auto xOverlap = [](const QRectF& a, const QRectF& b) -> qreal {
    qreal overlap = qMin(a.right(), b.right()) - qMax(a.left(), b.left());
    qreal shorter = qMin(a.width(), b.width());
    return (shorter > 0) ? (overlap / shorter) : 0.0;
  };

  const QObjectList& siblings = parentWidget()->children();
  for (QObject* obj : siblings) {
    QWidget* sibling = qobject_cast<QWidget*>(obj);
    if (!sibling || sibling == this || !sibling->isVisible())
      continue;
    if (!sibling->property("ps_groupable").toBool())
      continue;

    QRectF s_rect = QRectF(sibling->geometry());

    qreal gap_right = s_rect.left() - my_rect.right();
    if (qAbs(gap_right) < kTolerance && yOverlap(my_rect, s_rect) >= 0.5)
      has_right = true;

    qreal gap_left = my_rect.left() - s_rect.right();
    if (qAbs(gap_left) < kTolerance && yOverlap(my_rect, s_rect) >= 0.5)
      has_left = true;

    qreal gap_bottom = s_rect.top() - my_rect.bottom();
    if (qAbs(gap_bottom) < kTolerance && xOverlap(my_rect, s_rect) >= 0.5)
      has_bottom = true;

    qreal gap_top = my_rect.top() - s_rect.bottom();
    if (qAbs(gap_top) < kTolerance && xOverlap(my_rect, s_rect) >= 0.5)
      has_top = true;
  }

  if (has_left) {
    corner_top_left_ = std::nullopt;
    corner_bottom_left_ = std::nullopt;
  }
  if (has_right) {
    corner_top_right_ = std::nullopt;
    corner_bottom_right_ = std::nullopt;
  }
  if (has_top) {
    corner_top_left_ = std::nullopt;
    corner_top_right_ = std::nullopt;
  }
  if (has_bottom) {
    corner_bottom_left_ = std::nullopt;
    corner_bottom_right_ = std::nullopt;
  }

  neighbor_cache_valid_ = true;

  bool changed = (old_tl != corner_top_left_ || old_tr != corner_top_right_ ||
                  old_br != corner_bottom_right_ || old_bl != corner_bottom_left_);
  if (changed) {
    update();
    for (QObject* obj : siblings) {
      QWidget* sibling = qobject_cast<QWidget*>(obj);
      if (sibling && sibling != this && sibling->isVisible() &&
          sibling->property("ps_groupable").toBool()) {
        sibling->setProperty("ps_cache_dirty", true);
        sibling->update();
      }
    }
  }
}

QPainterPath PsLabel::BuildRoundedRectPath(const QRectF& rect) const {
  QPainterPath path;

  qreal tl = corner_top_left_.value_or(0);
  qreal tr = corner_top_right_.value_or(0);
  qreal br = corner_bottom_right_.value_or(0);
  qreal bl = corner_bottom_left_.value_or(0);

  qreal x = rect.left();
  qreal y = rect.top();
  qreal w = rect.width();
  qreal h = rect.height();

  path.moveTo(x + tl, y);
  path.lineTo(x + w - tr, y);
  if (tr > 0)
    path.arcTo(x + w - 2 * tr, y, 2 * tr, 2 * tr, 90, -90);
  else
    path.lineTo(x + w, y);

  path.lineTo(x + w, y + h - br);
  if (br > 0)
    path.arcTo(x + w - 2 * br, y + h - 2 * br, 2 * br, 2 * br, 0, -90);
  else
    path.lineTo(x + w, y + h);

  path.lineTo(x + bl, y + h);
  if (bl > 0)
    path.arcTo(x, y + h - 2 * bl, 2 * bl, 2 * bl, 270, -90);
  else
    path.lineTo(x, y + h);

  path.lineTo(x, y + tl);
  if (tl > 0)
    path.arcTo(x, y, 2 * tl, 2 * tl, 180, -90);
  else
    path.lineTo(x, y);

  path.closeSubpath();
  return path;
}

void PsLabel::showEvent(QShowEvent* event) {
  QLabel::showEvent(event);
  DetectNeighbors();
  ScheduleRedetect();
}

void PsLabel::ScheduleRedetect() {
  if (redetect_scheduled_) return;
  redetect_scheduled_ = true;
  QTimer::singleShot(0, this, [this] {
    redetect_scheduled_ = false;
    neighbor_cache_valid_ = false;
    DetectNeighbors();
    update();
  });
}

void PsLabel::resizeEvent(QResizeEvent* event) {
  QLabel::resizeEvent(event);
  neighbor_cache_valid_ = false;
  DetectNeighbors();
}

void PsLabel::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);

  if (!neighbor_cache_valid_ || property("ps_cache_dirty").toBool()) {
    setProperty("ps_cache_dirty", false);
    DetectNeighbors();
  }

  QPainter painter(this);
  PaintCard(&painter);
}

void PsLabel::PaintCard(QPainter* painter) {
  const auto& cfg = color_.Get();
  qreal margin_x = 3.0;
  qreal margin_top = 1.0;
  qreal margin_bottom = 4.0;
  QRectF r = rect().adjusted(margin_x, margin_top, -margin_x, -margin_bottom);

#if PS_LEGACY_UI
  {
    r = rect().adjusted(1.0, 1.0, -1.0, -1.0);
    painter->setRenderHint(QPainter::Antialiasing, false);
    const bool transparent = cfg.base.alpha() == 0;
    if (!transparent) {
      ps_style::DrawLegacyBeveledPanel(painter, r, cfg, false, isEnabled());
    }

    const QPixmap* pix_ptr = this->pixmap();
    if (pix_ptr && !pix_ptr->isNull()) {
      QPixmap pix = pix_ptr->scaled(r.size().toSize(), Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);
      QRectF pix_r = QRectF(r.topLeft(), pix.size() / pix.devicePixelRatioF());
      pix_r.moveCenter(r.center());
      painter->drawPixmap(pix_r, pix, QRectF(pix.rect()));
    } else {
      painter->setPen(transparent ? PsColor::WidgetColor::kTextDefault
                                  : ps_style::LegacyTextColor(cfg.base,
                                                              isEnabled()));
      painter->setFont(font());
      painter->drawText(r, alignment(), text());
    }
  }
  return;
#endif

  painter->setRenderHint(QPainter::Antialiasing);

  QPainterPath path = BuildRoundedRectPath(r);

  painter->setPen(Qt::NoPen);
  painter->setBrush(QColor(cfg.shadow.red(), cfg.shadow.green(), cfg.shadow.blue(),
                           cfg.shadow_alpha));

  bool has_h = (!corner_top_left_.has_value() && !corner_bottom_left_.has_value()) ||
               (!corner_top_right_.has_value() && !corner_bottom_right_.has_value());
  bool has_v = (!corner_top_left_.has_value() && !corner_top_right_.has_value()) ||
               (!corner_bottom_left_.has_value() && !corner_bottom_right_.has_value());

  qreal shadow_dy = 2.0;
  if (has_h) shadow_dy = 1.0;
  if (has_v) shadow_dy = 0.0;

  painter->drawPath(BuildRoundedRectPath(r.translated(0.0, shadow_dy)));

  painter->setBrush(cfg.base);
  painter->drawPath(path);

  painter->setPen(QPen(cfg.border, 0.5));
  painter->setBrush(Qt::NoBrush);
  painter->drawPath(path);

  const QPixmap* pix_ptr = this->pixmap();
  if (pix_ptr && !pix_ptr->isNull()) {
    QPixmap pix = pix_ptr->scaled(r.size().toSize(), Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);
    QRectF pix_r = QRectF(r.topLeft(), pix.size() / pix.devicePixelRatioF());
    pix_r.moveCenter(r.center());
    painter->save();
    painter->setClipPath(path);
    painter->drawPixmap(pix_r, pix, QRectF(pix.rect()));
    painter->restore();
  } else {
    painter->setPen(cfg.text);
    QFont f = font();
    painter->setFont(f);
    painter->drawText(r, alignment(), text());
  }
}
