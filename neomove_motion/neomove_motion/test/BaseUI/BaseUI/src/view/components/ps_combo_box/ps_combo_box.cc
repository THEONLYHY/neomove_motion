// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#include "ps_combo_box.h"

#include <QFocusEvent>
#include <QListView>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QShowEvent>
#include <QStyledItemDelegate>

#include "view/components/ps_style/ps_legacy_painter.h"

PsComboBox::PsComboBox(QWidget* parent) : QComboBox(parent) {
  color_.SetOnChanged([this] {
    InitPopupStyle();
    update();
  });

  setAttribute(Qt::WA_TranslucentBackground);
  setProperty("ps_groupable", true);
  setStyleSheet(R"(
    QComboBox {
      border: none;
      background: transparent;
      padding: 0px;
    }
    QComboBox::drop-down {
      border: none;
      width: 0;
    }
    QComboBox::down-arrow {
      image: none;
      width: 0;
      border: none;
    }
  )");
  InitPopupStyle();
}

void PsComboBox::InitPopupStyle() {
  auto* view = new QListView(this);
  const auto& cfg = color_.Get();
#if PS_LEGACY_UI
  QString style = QString(R"(
    QListView {
      background: #F3F4F6;
      border: 1px solid %1;
      outline: none;
    }
    QListView::item {
      color: %2;
      min-height: 60px;
    }
    QListView::item:selected {
      background: %3;
      color: %4;
    }
  )").arg(PsColor::WidgetColor::kBorderDefault.name(),
          PsColor::WidgetColor::kTextDefault.name(), cfg.base.name(),
          ps_style::LegacyTextColor(cfg.base).name());
#else
  QString style = QString(R"(
    QListView {
      background: white;
      outline: none;
    }
    QListView::item {
      color: %1;
      min-height: 80px;
    }
    QListView::item:selected {
      background: %2;
      color: white;
    }
  )").arg(PsColor::WidgetColor::kTextDefault.name(),
          cfg.base.name());
#endif
  view->setStyleSheet(style);
  view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setView(view);
}

PsColor& PsComboBox::Color() { return color_; }

void PsComboBox::SetBorderRadius(qreal radius) {
  base_border_radius_ = radius;
  if (corner_top_left_.has_value()) corner_top_left_ = radius;
  if (corner_top_right_.has_value()) corner_top_right_ = radius;
  if (corner_bottom_right_.has_value()) corner_bottom_right_ = radius;
  if (corner_bottom_left_.has_value()) corner_bottom_left_ = radius;
  update();
}

void PsComboBox::DetectNeighbors() {
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
    if (!sibling || sibling == this || !sibling->isVisible()) continue;
    if (!sibling->property("ps_groupable").toBool()) continue;

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

  bool changed =
      (old_tl != corner_top_left_ || old_tr != corner_top_right_ ||
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

QPainterPath PsComboBox::BuildRoundedRectPath(const QRectF& rect) const {
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

void PsComboBox::showEvent(QShowEvent* event) {
  QComboBox::showEvent(event);
  DetectNeighbors();
}

void PsComboBox::resizeEvent(QResizeEvent* event) {
  QComboBox::resizeEvent(event);
  neighbor_cache_valid_ = false;
  DetectNeighbors();
}

void PsComboBox::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);

  if (!neighbor_cache_valid_ || property("ps_cache_dirty").toBool()) {
    setProperty("ps_cache_dirty", false);
    DetectNeighbors();
  }

  QPainter painter(this);

  const QRectF r = rect().adjusted(3.0, 1.0, -3.0, -4.0);
  const auto& cfg = color_.Get();
  const bool is_active = (is_focused_ || is_popup_visible_) && isEnabled();

#if PS_LEGACY_UI
  QRectF legacy_r = rect().adjusted(1.0, 1.0, -1.0, -1.0);
  ps_style::DrawLegacyInputPanel(&painter, legacy_r, QColor(0xF3, 0xF4, 0xF6),
                                 is_active, isEnabled());

  qreal arrow_cx = legacy_r.right() - 16;
  qreal arrow_cy = legacy_r.center().y();
  QPolygonF arrow;
  if (is_popup_visible_) {
    arrow << QPointF(arrow_cx - 5, arrow_cy + 3)
          << QPointF(arrow_cx, arrow_cy - 4)
          << QPointF(arrow_cx + 5, arrow_cy + 3);
  } else {
    arrow << QPointF(arrow_cx - 5, arrow_cy - 3)
          << QPointF(arrow_cx, arrow_cy + 4)
          << QPointF(arrow_cx + 5, arrow_cy - 3);
  }
  painter.setPen(Qt::NoPen);
  painter.setBrush(is_active ? cfg.base : PsColor::WidgetColor::kTextDefault);
  painter.drawPolygon(arrow);

  QColor text_color = !isEnabled() ? PsColor::WidgetColor::kTextDisabled
                                   : PsColor::WidgetColor::kTextDefault;
  painter.setPen(text_color);
  painter.setFont(font());
  QFontMetrics fm(font());
  QRectF text_r = legacy_r.adjusted(10, 0, -32, 0);
  QString elided = fm.elidedText(currentText(), Qt::ElideRight,
                                 static_cast<int>(text_r.width()));
  painter.drawText(text_r, Qt::AlignVCenter | Qt::AlignLeft, elided);
  return;
#endif

  painter.setRenderHint(QPainter::Antialiasing);

  QPainterPath path = BuildRoundedRectPath(r);

  // 1. 光晕（先画，后续白色背景会覆盖内部）
  if (is_active) {
    for (int i = 3; i >= 1; --i) {
      qreal expand = i * 1.5;
      int alpha = static_cast<int>(30.0 / i);
      painter.setPen(Qt::NoPen);
      painter.setBrush(
          QColor(cfg.base.red(), cfg.base.green(), cfg.base.blue(), alpha));

      QRectF glow_r = r.adjusted(-expand, -expand, expand, expand);

      auto old_tl = corner_top_left_;
      auto old_tr = corner_top_right_;
      auto old_br = corner_bottom_right_;
      auto old_bl = corner_bottom_left_;

      corner_top_left_ = old_tl.has_value()
                             ? std::optional<qreal>(*old_tl + expand)
                             : std::nullopt;
      corner_top_right_ = old_tr.has_value()
                              ? std::optional<qreal>(*old_tr + expand)
                              : std::nullopt;
      corner_bottom_right_ = old_br.has_value()
                                 ? std::optional<qreal>(*old_br + expand)
                                 : std::nullopt;
      corner_bottom_left_ = old_bl.has_value()
                                ? std::optional<qreal>(*old_bl + expand)
                                : std::nullopt;

      QPainterPath glow_path = BuildRoundedRectPath(glow_r);
      painter.drawPath(glow_path);

      corner_top_left_ = old_tl;
      corner_top_right_ = old_tr;
      corner_bottom_right_ = old_br;
      corner_bottom_left_ = old_bl;
    }
  }

  // 2. 背景（白色覆盖光晕内部）
  painter.setPen(Qt::NoPen);
  painter.setBrush(isEnabled() ? Qt::white
                               : PsColor::WidgetColor::kBgDisabled);
  painter.drawPath(path);

  // 3. 边框
  if (is_active) {
    painter.setPen(QPen(cfg.base, 1.8));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
  } else if (!isEnabled()) {
    painter.setPen(QPen(PsColor::WidgetColor::kBorderDisabled, 1.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
  } else if (is_hovered_) {
    painter.setPen(QPen(QColor(PsColor::WidgetColor::kBorderDefault).darker(130), 1.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
  } else {
    painter.setPen(QPen(PsColor::WidgetColor::kBorderDefault, 1.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
  }

  {
    qreal arrow_cx = r.right() - 16;
    qreal arrow_cy = r.center().y();
    qreal arrow_w = 5.0;
    qreal arrow_h = 3.5;

    QColor arrow_color = !isEnabled() ? PsColor::WidgetColor::kBorderDefault
                         : is_active  ? cfg.base
                                      : QColor(PsColor::WidgetColor::kBorderDefault).darker(130);
    painter.setPen(
        QPen(arrow_color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);

    QPainterPath arrow_path;
    if (is_popup_visible_) {
      arrow_path.moveTo(arrow_cx - arrow_w, arrow_cy + arrow_h);
      arrow_path.lineTo(arrow_cx, arrow_cy - arrow_h);
      arrow_path.lineTo(arrow_cx + arrow_w, arrow_cy + arrow_h);
    } else {
      arrow_path.moveTo(arrow_cx - arrow_w, arrow_cy - arrow_h);
      arrow_path.lineTo(arrow_cx, arrow_cy + arrow_h);
      arrow_path.lineTo(arrow_cx + arrow_w, arrow_cy - arrow_h);
    }
    painter.drawPath(arrow_path);
  }

  {
    QColor text_color = !isEnabled() ? PsColor::WidgetColor::kTextDisabled
                                     : PsColor::WidgetColor::kTextDefault;
    painter.setPen(text_color);
    QFont f = font();
    painter.setFont(f);

    QFontMetrics fm(f);
    QRectF text_r = r.adjusted(10, 0, -32, 0);
    QString elided = fm.elidedText(currentText(), Qt::ElideRight,
                                   static_cast<int>(text_r.width()));
    painter.drawText(text_r, Qt::AlignVCenter | Qt::AlignLeft, elided);
  }
}

void PsComboBox::focusInEvent(QFocusEvent* event) {
  is_focused_ = true;
  update();
  QComboBox::focusInEvent(event);
}

void PsComboBox::focusOutEvent(QFocusEvent* event) {
  is_focused_ = false;
  update();
  QComboBox::focusOutEvent(event);
}

void PsComboBox::showPopup() {
  is_popup_visible_ = true;
  update();
  QComboBox::showPopup();
}

void PsComboBox::hidePopup() {
  is_popup_visible_ = false;
  update();
  QComboBox::hidePopup();
}

void PsComboBox::enterEvent(QEvent* event) {
  is_hovered_ = true;
  update();
  QComboBox::enterEvent(event);
}

void PsComboBox::leaveEvent(QEvent* event) {
  is_hovered_ = false;
  update();
  QComboBox::leaveEvent(event);
}
