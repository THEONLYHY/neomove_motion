// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#include "ps_line_edit.h"

#include <QFocusEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QShowEvent>
#include <QTimer>

#include "view/components/ps_style/ps_legacy_painter.h"

PsLineEdit::PsLineEdit(QWidget* parent) : QLineEdit(parent) {
  color_.SetOnChanged([this] { update(); });

  QPalette p = palette();
  p.setColor(QPalette::Base, Qt::transparent);
  p.setColor(QPalette::Text, PsColor::WidgetColor::kTextDefault);
  setPalette(p);
  setFrame(false);
  setAttribute(Qt::WA_TranslucentBackground);
  setProperty("ps_groupable", true);
  setTextMargins(10, 0, 10, 0);
}

PsLineEdit::PsLineEdit(const QString& text, QWidget* parent)
    : PsLineEdit(parent) {
  setText(text);
}

PsColor& PsLineEdit::Color() { return color_; }

const QColor PsLineEdit::kDefaultColor(0xFF, 0xFF, 0xFF);

void PsLineEdit::SetCustomBgColor(const QColor& color) {
  custom_bg_color_ = color;
  has_custom_bg_color_ = color.isValid();
  update();
}

void PsLineEdit::SetBorderRadius(qreal radius) {
  base_border_radius_ = radius;
  if (corner_top_left_.has_value()) corner_top_left_ = radius;
  if (corner_top_right_.has_value()) corner_top_right_ = radius;
  if (corner_bottom_right_.has_value()) corner_bottom_right_ = radius;
  if (corner_bottom_left_.has_value()) corner_bottom_left_ = radius;
  update();
}

void PsLineEdit::DetectNeighbors() {
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

QPainterPath PsLineEdit::BuildRoundedRectPath(const QRectF& rect) const {
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

void PsLineEdit::showEvent(QShowEvent* event) {
  QLineEdit::showEvent(event);
  DetectNeighbors();
  ScheduleRedetect();
}

void PsLineEdit::ScheduleRedetect() {
  if (redetect_scheduled_) return;
  redetect_scheduled_ = true;
  QTimer::singleShot(0, this, [this] {
    redetect_scheduled_ = false;
    neighbor_cache_valid_ = false;
    DetectNeighbors();
    update();
  });
}

void PsLineEdit::resizeEvent(QResizeEvent* event) {
  QLineEdit::resizeEvent(event);
  neighbor_cache_valid_ = false;
  DetectNeighbors();
}

void PsLineEdit::paintEvent(QPaintEvent* event) {
  if (!neighbor_cache_valid_ || property("ps_cache_dirty").toBool()) {
    setProperty("ps_cache_dirty", false);
    DetectNeighbors();
  }

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  PaintCard(&painter);
  painter.end();

  QLineEdit::paintEvent(event);
}

void PsLineEdit::PaintCard(QPainter* painter) {
  const QRectF r = rect().adjusted(3.0, 1.0, -3.0, -4.0);
  const auto& cfg = color_.Get();
  QPainterPath path = BuildRoundedRectPath(r);

#if PS_LEGACY_UI
  {
    const QRectF legacy_r = rect().adjusted(1.0, 1.0, -1.0, -1.0);
    const QColor surface = has_custom_bg_color_ ? custom_bg_color_ : cfg.base;
    ps_style::DrawLegacyInputPanel(painter, legacy_r, surface, is_focused_,
                                   isEnabled());
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::transparent);
    p.setColor(QPalette::Text, ps_style::LegacyTextColor(surface));
    setPalette(p);
  }
  return;
#endif

  painter->setPen(Qt::NoPen);

  // 1. 光晕（先画，后续背景会覆盖内部）
  if (!has_custom_bg_color_ && is_focused_) {
    QColor focus_color = PsColor::WidgetColor::kFocusBorder;
    for (int i = 3; i >= 1; --i) {
      qreal expand = i * 1.5;
      qreal alpha = 30.0 / i;
      painter->setPen(Qt::NoPen);
      painter->setBrush(QColor(focus_color.red(), focus_color.green(), focus_color.blue(),
                               static_cast<int>(alpha)));

      QRectF glow_r = r.adjusted(-expand, -expand, expand, expand);
      auto old_tl = corner_top_left_;
      auto old_tr = corner_top_right_;
      auto old_br = corner_bottom_right_;
      auto old_bl = corner_bottom_left_;

      auto& self = *const_cast<PsLineEdit*>(this);
      self.corner_top_left_ = old_tl.has_value() ? std::optional<qreal>(old_tl.value() + expand) : std::nullopt;
      self.corner_top_right_ = old_tr.has_value() ? std::optional<qreal>(old_tr.value() + expand) : std::nullopt;
      self.corner_bottom_right_ = old_br.has_value() ? std::optional<qreal>(old_br.value() + expand) : std::nullopt;
      self.corner_bottom_left_ = old_bl.has_value() ? std::optional<qreal>(old_bl.value() + expand) : std::nullopt;

      QPainterPath glow_path = BuildRoundedRectPath(glow_r);
      painter->drawPath(glow_path);

      const_cast<PsLineEdit*>(this)->corner_top_left_ = old_tl;
      const_cast<PsLineEdit*>(this)->corner_top_right_ = old_tr;
      const_cast<PsLineEdit*>(this)->corner_bottom_right_ = old_br;
      const_cast<PsLineEdit*>(this)->corner_bottom_left_ = old_bl;
    }
  }

  // 2. 背景（覆盖光晕内部）
  if (has_custom_bg_color_) {
    painter->setBrush(custom_bg_color_);
  } else {
    painter->setBrush(cfg.base);
  }
  painter->drawPath(path);

  // 3. 边框
  if (has_custom_bg_color_) {
    painter->setPen(QPen(PsColor::WidgetColor::kBorderDefault, 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);
  } else if (is_focused_) {
    painter->setPen(QPen(PsColor::WidgetColor::kFocusBorder, 1.8));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);
  } else {
    painter->setPen(QPen(cfg.border, 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);
  }

  QPalette p = palette();
  p.setColor(QPalette::Text, cfg.text);
  setPalette(p);
}

void PsLineEdit::focusInEvent(QFocusEvent* event) {
  is_focused_ = true;
  update();
  QLineEdit::focusInEvent(event);
}

void PsLineEdit::focusOutEvent(QFocusEvent* event) {
  is_focused_ = false;
  update();
  QLineEdit::focusOutEvent(event);
}
