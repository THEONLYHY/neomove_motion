// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/29 10:05

#include "ps_arrow_button.h"

#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTransform>
#include <QtMath>

#include "view/components/ps_style/ps_legacy_painter.h"

namespace {

QColor LegacyArrowFill(bool pressed, bool enabled) {
  if (!enabled) return QColor(0x3E, 0x66, 0x3E);
  return pressed ? QColor(0x05, 0x6C, 0x18) : QColor(0x00, 0x8C, 0x20);
}

QColor LegacyArrowOutline(bool enabled) {
  return enabled ? QColor(0xFF, 0xFF, 0xFF) : QColor(0x9A, 0xA3, 0xAE);
}

bool IsDiagonalDirection(PsArrowButton::Direction direction) {
  return direction == PsArrowButton::Direction::UpRight ||
         direction == PsArrowButton::Direction::UpLeft ||
         direction == PsArrowButton::Direction::DownRight ||
         direction == PsArrowButton::Direction::DownLeft;
}

bool IsVerticalDirection(PsArrowButton::Direction direction) {
  return direction == PsArrowButton::Direction::Up ||
         direction == PsArrowButton::Direction::Down;
}

qreal LegacyDirectionAngle(PsArrowButton::Direction direction) {
  switch (direction) {
    case PsArrowButton::Direction::Left:
      return 180.0;
    case PsArrowButton::Direction::Up:
      return -90.0;
    case PsArrowButton::Direction::Down:
      return 90.0;
    case PsArrowButton::Direction::UpRight:
      return -45.0;
    case PsArrowButton::Direction::UpLeft:
      return -135.0;
    case PsArrowButton::Direction::DownRight:
      return 45.0;
    case PsArrowButton::Direction::DownLeft:
      return 135.0;
    case PsArrowButton::Direction::Right:
      return 0.0;
  }
  return 0.0;
}

QRectF CenteredRect(const QPointF& center, qreal width, qreal height) {
  return QRectF(center.x() - width / 2.0, center.y() - height / 2.0, width,
                height);
}

QTransform LegacyDirectionTransform(const QRectF& local_rect,
                                    PsArrowButton::Direction direction) {
  QTransform transform;
  const QPointF center = local_rect.center();
  transform.translate(center.x(), center.y());
  transform.rotate(LegacyDirectionAngle(direction));
  transform.translate(-center.x(), -center.y());
  return transform;
}

QRectF LegacyStraightLocalRect(const QRectF& r,
                               PsArrowButton::Direction direction) {
  const QRectF g = r.normalized();
  const QPointF center = g.center();
  if (IsDiagonalDirection(direction)) {
    const qreal side = qMin(g.width(), g.height()) * 0.78;
    return CenteredRect(center, side, side);
  }
  if (IsVerticalDirection(direction)) {
    return CenteredRect(center, g.height(), g.width());
  }
  return g;
}

QPainterPath BuildLegacyRightStraightArrowPath(const QRectF& local_rect) {
  QPainterPath path;
  const QRectF g = local_rect.normalized();
  const qreal tail_space = qMin(g.width() * 0.22, g.height() * 0.56);
  QRectF a = g;
  a.setLeft(g.left() + tail_space);

  const qreal head_w = qMin(a.width() * 0.34, a.height() * 0.74);
  const qreal body_h = a.height() * 0.42;
  const qreal x0 = a.left();
  const qreal x1 = a.right() - head_w;
  const qreal x2 = a.right();
  const qreal cy = a.center().y();
  const qreal y0 = cy - body_h / 2.0;
  const qreal y1 = cy + body_h / 2.0;

  path.moveTo(x0, y0);
  path.lineTo(x1, y0);
  path.lineTo(x1, a.top());
  path.lineTo(x2, cy);
  path.lineTo(x1, a.bottom());
  path.lineTo(x1, y1);
  path.lineTo(x0, y1);
  path.closeSubpath();
  return path;
}

QPainterPath BuildLegacyStraightTailPath(const QRectF& r,
                                         PsArrowButton::Direction direction) {
  const QRectF local_rect = LegacyStraightLocalRect(r, direction);
  const QRectF g = local_rect.normalized();
  const qreal tail_space = qMin(g.width() * 0.22, g.height() * 0.56);
  const qreal body_h = g.height() * 0.42;
  const qreal block_len =
      qMax<qreal>(4.0, qMin(tail_space * 0.28, body_h * 0.45));
  const qreal gap = qMax<qreal>(3.0, qMin(tail_space * 0.16, body_h * 0.22));
  const qreal total_w = block_len * 2.0 + gap;
  const qreal x = g.left() + qMax<qreal>(0.0, (tail_space - total_w) / 2.0);
  const qreal y = g.center().y() - body_h / 2.0;

  QPainterPath path;
  path.addRect(QRectF(x, y, block_len, body_h));
  path.addRect(QRectF(x + block_len + gap, y, block_len, body_h));
  return LegacyDirectionTransform(local_rect, direction).map(path);
}

void AddOrientedRect(QPainterPath* path, const QPointF& center,
                     const QPointF& axis, const QPointF& normal,
                     qreal length, qreal thickness) {
  const QPointF half_axis = axis * (length / 2.0);
  const QPointF half_normal = normal * (thickness / 2.0);
  path->moveTo(center - half_axis - half_normal);
  path->lineTo(center + half_axis - half_normal);
  path->lineTo(center + half_axis + half_normal);
  path->lineTo(center - half_axis + half_normal);
  path->closeSubpath();
}

QPainterPath BuildLegacyCurvedArrowPath(const QRectF& r,
                                        PsArrowButton::Direction direction) {
  QPainterPath path;
  const QRectF g = r.normalized();
  const qreal cx = g.center().x();
  const qreal cy = g.center().y();
  const qreal max_r = qMin(g.width(), g.height()) / 2.0;
  const qreal outer_r = max_r * 0.95;
  const qreal inner_r = max_r * 0.48;
  const qreal mid_r = (outer_r + inner_r) / 2.0;

  qreal start_angle = -25.0;
  qreal sweep_angle = 190.0;
  bool is_ccw = true;
  switch (direction) {
    case PsArrowButton::Direction::Left:
    case PsArrowButton::Direction::Down:
      start_angle = 205.0;
      sweep_angle = -190.0;
      is_ccw = false;
      break;
    case PsArrowButton::Direction::Right:
    case PsArrowButton::Direction::Up:
    default:
      break;
  }

  const qreal end_angle = start_angle + sweep_angle;
  const qreal end_rad = qDegreesToRadians(end_angle);

  const QRectF outer_rect(cx - outer_r, cy - outer_r, outer_r * 2.0,
                          outer_r * 2.0);
  const QRectF inner_rect(cx - inner_r, cy - inner_r, inner_r * 2.0,
                          inner_r * 2.0);

  const QPointF outer_end(cx + outer_r * qCos(end_rad),
                          cy - outer_r * qSin(end_rad));
  const QPointF inner_end(cx + inner_r * qCos(end_rad),
                          cy - inner_r * qSin(end_rad));
  const QPointF mid_end(cx + mid_r * qCos(end_rad),
                        cy - mid_r * qSin(end_rad));

  const QPointF tangent(is_ccw ? -qSin(end_rad) : qSin(end_rad),
                        is_ccw ? -qCos(end_rad) : qCos(end_rad));
  const QPointF normal(qCos(end_rad), -qSin(end_rad));

  const qreal base_ext = max_r * 0.12;
  const qreal tip_len = max_r * 0.26;
  const QPointF head_outer = outer_end + normal * base_ext;
  const QPointF head_inner = inner_end - normal * base_ext;
  const QPointF tip = mid_end + tangent * tip_len;

  path.arcMoveTo(outer_rect, start_angle);
  path.arcTo(outer_rect, start_angle, sweep_angle);
  path.lineTo(head_outer);
  path.lineTo(tip);
  path.lineTo(head_inner);
  path.lineTo(inner_end);
  path.arcTo(inner_rect, end_angle, -sweep_angle);
  path.closeSubpath();
  return path;
}

QPainterPath BuildLegacyCurvedTailPath(const QRectF& r,
                                       PsArrowButton::Direction direction) {
  const QRectF g = r.normalized();
  const qreal cx = g.center().x();
  const qreal cy = g.center().y();
  const qreal max_r = qMin(g.width(), g.height()) / 2.0;
  const qreal outer_r = max_r * 0.95;
  const qreal inner_r = max_r * 0.48;
  const qreal mid_r = (outer_r + inner_r) / 2.0;
  const qreal shaft_w = outer_r - inner_r;

  qreal start_angle = -25.0;
  bool is_ccw = true;
  switch (direction) {
    case PsArrowButton::Direction::Left:
    case PsArrowButton::Direction::Down:
      start_angle = 205.0;
      is_ccw = false;
      break;
    case PsArrowButton::Direction::Right:
    case PsArrowButton::Direction::Up:
    default:
      break;
  }

  const qreal start_rad = qDegreesToRadians(start_angle);
  const QPointF mid_start(cx + mid_r * qCos(start_rad),
                          cy - mid_r * qSin(start_rad));
  const QPointF tangent(is_ccw ? -qSin(start_rad) : qSin(start_rad),
                        is_ccw ? -qCos(start_rad) : qCos(start_rad));
  const QPointF normal(qCos(start_rad), -qSin(start_rad));

  const qreal block_len = qMax<qreal>(4.0, shaft_w * 0.46);
  const qreal gap = qMax<qreal>(3.0, shaft_w * 0.22);

  QPainterPath path;
  AddOrientedRect(&path, mid_start - tangent * (gap + block_len / 2.0),
                  tangent, normal, block_len, shaft_w);
  AddOrientedRect(&path,
                  mid_start -
                      tangent * (gap * 2.0 + block_len * 1.5),
                  tangent, normal, block_len, shaft_w);
  return path;
}

bool IsLegacySquareIcon(const QRectF& rect, PsArrowButton::ArrowStyle style,
                        PsArrowButton::Direction direction,
                        const QString& text) {
  return style == PsArrowButton::ArrowStyle::Straight && text.isEmpty() &&
         !IsDiagonalDirection(direction) &&
         rect.width() <= 90.0 && rect.height() <= 90.0;
}

void DrawLegacyArrowText(QPainter* painter, const QRectF& r,
                         const QString& text, bool enabled) {
  if (text.isEmpty()) return;

  QFont f = painter->font();
  f.setPixelSize(qMax(10.0, qMin(r.width(), r.height()) / 4.2));
  f.setBold(true);
  painter->setFont(f);

  painter->setPen(QPen(QColor(0, 0, 0, enabled ? 130 : 70), 1.0));
  painter->drawText(r.translated(1.0, 1.0), Qt::AlignCenter, text);
  painter->setPen(enabled ? QColor(0xFF, 0xFF, 0xFF) : QColor(0xC8, 0xCF, 0xD8));
  painter->drawText(r, Qt::AlignCenter, text);
}

}  // namespace

PsArrowButton::PsArrowButton(QWidget* parent) : QPushButton(parent) {
  color_.SetOnChanged([this] { update(); });
  setAttribute(Qt::WA_TranslucentBackground);
}

PsArrowButton::PsArrowButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent) {
  color_.SetOnChanged([this] { update(); });
  setAttribute(Qt::WA_TranslucentBackground);
}

PsColor& PsArrowButton::Color() { return color_; }

void PsArrowButton::SetDirection(Direction dir) {
  direction_ = dir;
  update();
}

void PsArrowButton::SetArrowStyle(ArrowStyle style) {
  style_ = style;
  update();
}

void PsArrowButton::SetBorderRadius(qreal radius) {
  border_radius_ = radius;
  update();
}

QPainterPath PsArrowButton::BuildArrowPath(const QRectF& r) {
  QPainterPath path;
  qreal w = r.width();
  qreal h = r.height();
  qreal tip_ratio = 0.35;
  qreal body_ratio = 0.6;
  qreal rad = border_radius_;

  if (style_ == ArrowStyle::Curved) {
    qreal cx = r.center().x();
    qreal cy = r.center().y();
    qreal max_r = qMin(w, h) / 2.0;
    qreal outer_r = max_r * 0.95;
    qreal inner_r = max_r * 0.24;
    qreal mid_r = (outer_r + inner_r) / 2.0;

    qreal start_angle;
    qreal sweep_angle;
    bool is_ccw;
    switch (direction_) {
      case Direction::Right:
      case Direction::Up:
        start_angle = 20.0;
        sweep_angle = 140.0;
        is_ccw = true;
        break;
      case Direction::Left:
      case Direction::Down:
        start_angle = 160.0;
        sweep_angle = -140.0;
        is_ccw = false;
        break;
      default:
        start_angle = 20.0;
        sweep_angle = 140.0;
        is_ccw = true;
        break;
    }

    qreal end_angle = start_angle + sweep_angle;
    qreal end_rad = qDegreesToRadians(end_angle);

    QRectF outer_rect(cx - outer_r, cy - outer_r, outer_r * 2, outer_r * 2);
    QRectF inner_rect(cx - inner_r, cy - inner_r, inner_r * 2, inner_r * 2);

    QPointF outer_end(cx + outer_r * qCos(end_rad),
                      cy - outer_r * qSin(end_rad));
    QPointF inner_end(cx + inner_r * qCos(end_rad),
                      cy - inner_r * qSin(end_rad));
    QPointF mid_end(cx + mid_r * qCos(end_rad), cy - mid_r * qSin(end_rad));

    QPointF tangent(is_ccw ? -qSin(end_rad) : qSin(end_rad),
                    is_ccw ? -qCos(end_rad) : qCos(end_rad));
    QPointF normal(qCos(end_rad), -qSin(end_rad));

    qreal base_ext = max_r * 0.18;
    qreal tip_len = max_r * 0.36;
    QPointF head_outer = outer_end + normal * base_ext;
    QPointF head_inner = inner_end - normal * base_ext;
    QPointF tip = mid_end + tangent * tip_len;

    path.arcMoveTo(outer_rect, start_angle);
    path.arcTo(outer_rect, start_angle, sweep_angle);
    path.lineTo(head_outer);
    path.lineTo(tip);
    path.lineTo(head_inner);
    path.lineTo(inner_end);
    path.arcTo(inner_rect, end_angle, -sweep_angle);
    path.closeSubpath();
  } else {
    switch (direction_) {
      case Direction::Right: {
        qreal neck_x = r.right() - w * tip_ratio;
        qreal inset = h * (1.0 - body_ratio) / 2.0;
        qreal bt = r.top() + inset;
        qreal bb = r.bottom() - inset;

        path.moveTo(r.right(), r.center().y());
        path.lineTo(neck_x, r.top());
        path.lineTo(neck_x, bt);
        path.lineTo(r.left() + rad, bt);
        path.quadTo(r.left(), bt, r.left(), bt + rad);
        path.lineTo(r.left(), bb - rad);
        path.quadTo(r.left(), bb, r.left() + rad, bb);
        path.lineTo(neck_x, bb);
        path.lineTo(neck_x, r.bottom());
        path.closeSubpath();
        break;
      }
      case Direction::Left: {
        qreal neck_x = r.left() + w * tip_ratio;
        qreal inset = h * (1.0 - body_ratio) / 2.0;
        qreal bt = r.top() + inset;
        qreal bb = r.bottom() - inset;

        path.moveTo(r.left(), r.center().y());
        path.lineTo(neck_x, r.top());
        path.lineTo(neck_x, bt);
        path.lineTo(r.right() - rad, bt);
        path.quadTo(r.right(), bt, r.right(), bt + rad);
        path.lineTo(r.right(), bb - rad);
        path.quadTo(r.right(), bb, r.right() - rad, bb);
        path.lineTo(neck_x, bb);
        path.lineTo(neck_x, r.bottom());
        path.closeSubpath();
        break;
      }
      case Direction::Up: {
        qreal neck_y = r.top() + h * tip_ratio;
        qreal inset = w * (1.0 - body_ratio) / 2.0;
        qreal bl = r.left() + inset;
        qreal br = r.right() - inset;

        path.moveTo(r.center().x(), r.top());
        path.lineTo(r.left(), neck_y);
        path.lineTo(bl, neck_y);
        path.lineTo(bl, r.bottom() - rad);
        path.quadTo(bl, r.bottom(), bl + rad, r.bottom());
        path.lineTo(br - rad, r.bottom());
        path.quadTo(br, r.bottom(), br, r.bottom() - rad);
        path.lineTo(br, neck_y);
        path.lineTo(r.right(), neck_y);
        path.closeSubpath();
        break;
      }
      case Direction::Down: {
        qreal neck_y = r.bottom() - h * tip_ratio;
        qreal inset = w * (1.0 - body_ratio) / 2.0;
        qreal bl = r.left() + inset;
        qreal br = r.right() - inset;

        path.moveTo(r.center().x(), r.bottom());
        path.lineTo(r.left(), neck_y);
        path.lineTo(bl, neck_y);
        path.lineTo(bl, r.top() + rad);
        path.quadTo(bl, r.top(), bl + rad, r.top());
        path.lineTo(br - rad, r.top());
        path.quadTo(br, r.top(), br, r.top() + rad);
        path.lineTo(br, neck_y);
        path.lineTo(r.right(), neck_y);
        path.closeSubpath();
        break;
      }
      case Direction::UpRight:
      case Direction::UpLeft:
      case Direction::DownRight:
      case Direction::DownLeft: {
        // 构建标准 Right 路径后旋转
        qreal neck_x = r.right() - w * tip_ratio;
        qreal inset = h * (1.0 - body_ratio) / 2.0;
        qreal bt = r.top() + inset;
        qreal bb = r.bottom() - inset;

        path.moveTo(r.right(), r.center().y());
        path.lineTo(neck_x, r.top());
        path.lineTo(neck_x, bt);
        path.lineTo(r.left() + rad, bt);
        path.quadTo(r.left(), bt, r.left(), bt + rad);
        path.lineTo(r.left(), bb - rad);
        path.quadTo(r.left(), bb, r.left() + rad, bb);
        path.lineTo(neck_x, bb);
        path.lineTo(neck_x, r.bottom());
        path.closeSubpath();

        qreal angle = 0;
        switch (direction_) {
          case Direction::UpRight:   angle = -45.0; break;
          case Direction::UpLeft:    angle = -135.0; break;
          case Direction::DownRight: angle = 45.0; break;
          case Direction::DownLeft:  angle = 135.0; break;
          default: break;
        }
        QTransform t;
        QPointF center = r.center();
        t.translate(center.x(), center.y());
        t.rotate(angle);
        t.translate(-center.x(), -center.y());
        path = t.map(path);
        break;
      }
    }
  }
  return path;
}

QPainterPath PsArrowButton::BuildLegacyArrowGlyphPath(const QRectF& r) {
  QRectF g = r.normalized();
  if (style_ == ArrowStyle::Curved) {
    return BuildArrowPath(g);
  }

  if (direction_ != Direction::Left && direction_ != Direction::Right) {
    const qreal side = qMin(g.width(), g.height());
    g = QRectF(g.center().x() - side / 2.0, g.center().y() - side / 2.0,
               side, side);
  }

  const qreal head_w = g.width() * 0.38;
  const qreal body_h = g.height() * 0.34;
  const qreal x0 = g.left();
  const qreal x1 = g.right() - head_w;
  const qreal x2 = g.right();
  const qreal cy = g.center().y();
  const qreal y0 = cy - body_h / 2.0;
  const qreal y1 = cy + body_h / 2.0;

  QPainterPath path;
  path.moveTo(x0, y0);
  path.lineTo(x1, y0);
  path.lineTo(x1, g.top());
  path.lineTo(x2, cy);
  path.lineTo(x1, g.bottom());
  path.lineTo(x1, y1);
  path.lineTo(x0, y1);
  path.closeSubpath();

  qreal angle = 0.0;
  switch (direction_) {
    case Direction::Left:
      angle = 180.0;
      break;
    case Direction::Up:
      angle = -90.0;
      break;
    case Direction::Down:
      angle = 90.0;
      break;
    case Direction::UpRight:
      angle = -45.0;
      break;
    case Direction::UpLeft:
      angle = -135.0;
      break;
    case Direction::DownRight:
      angle = 45.0;
      break;
    case Direction::DownLeft:
      angle = 135.0;
      break;
    case Direction::Right:
      break;
  }

  if (qFuzzyIsNull(angle)) return path;

  QTransform transform;
  const QPointF center = g.center();
  transform.translate(center.x(), center.y());
  transform.rotate(angle);
  transform.translate(-center.x(), -center.y());
  return transform.map(path);
}

QPainterPath PsArrowButton::BuildLegacyStraightArrowPath(const QRectF& r) {
  const QRectF local_rect = LegacyStraightLocalRect(r, direction_);
  const QPainterPath path = BuildLegacyRightStraightArrowPath(local_rect);
  return LegacyDirectionTransform(local_rect, direction_).map(path);
}

void PsArrowButton::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  const auto& cfg = color_.Get();
  const bool is_visually_pressed = is_pressed_;

  qreal margin_x = 4.0;
  qreal margin_top = 2.0;
  qreal margin_bottom = 4.0;
  QRectF r = rect().adjusted(margin_x, margin_top, -margin_x, -margin_bottom);

  if (is_visually_pressed && isEnabled()) r.translate(0, 1.5);

  QPainterPath arrow = BuildArrowPath(r);

#if PS_LEGACY_UI
  {
    QRectF legacy_rect = rect().adjusted(2.0, 2.0, -3.0, -3.0);
    if (is_visually_pressed && isEnabled()) legacy_rect.translate(1.0, 1.0);

    if (IsLegacySquareIcon(legacy_rect, style_, direction_, text())) {
      PsColor panel_color(PsColor::Color::kBlue);
      ps_style::DrawLegacyBeveledPanel(&painter, legacy_rect,
                                       panel_color.Get(),
                                       is_visually_pressed, isEnabled());

      const qreal pad_x = qMax<qreal>(6.0, legacy_rect.width() * 0.26);
      const qreal pad_y = qMax<qreal>(6.0, legacy_rect.height() * 0.22);
      QRectF glyph_rect = legacy_rect.adjusted(pad_x, pad_y, -pad_x, -pad_y);
      QPainterPath glyph = BuildLegacyArrowGlyphPath(glyph_rect);
      painter.setPen(Qt::NoPen);
      painter.setBrush(isEnabled()
                           ? (is_visually_pressed ? QColor(0x69, 0x7D, 0xF4)
                                                  : QColor(0xFF, 0xFF, 0xFF))
                           : QColor(0x9A, 0xA3, 0xAE));
      painter.drawPath(glyph);
      return;
    }

    QRectF arrow_rect = legacy_rect.adjusted(4.0, 4.0, -4.0, -4.0);
    const QColor fill = LegacyArrowFill(is_visually_pressed, isEnabled());
    const QColor outline = LegacyArrowOutline(isEnabled());

    painter.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath legacy_arrow =
        style_ == ArrowStyle::Curved ? BuildLegacyCurvedArrowPath(arrow_rect, direction_)
                                     : BuildLegacyStraightArrowPath(arrow_rect);
    QPainterPath legacy_tail =
        style_ == ArrowStyle::Curved ? BuildLegacyCurvedTailPath(arrow_rect, direction_)
                                     : BuildLegacyStraightTailPath(arrow_rect, direction_);

    if (isEnabled() && !is_visually_pressed) {
      painter.setPen(Qt::NoPen);
      painter.setBrush(QColor(0, 0, 0, 55));
      painter.drawPath(legacy_tail.translated(2.0, 2.0));
      painter.drawPath(legacy_arrow.translated(2.0, 2.0));
    }

    painter.setPen(QPen(outline, 2.0, Qt::SolidLine, Qt::SquareCap,
                        Qt::MiterJoin));
    painter.setBrush(fill);
    painter.drawPath(legacy_tail);

    painter.setPen(QPen(outline, 2.0, Qt::SolidLine, Qt::SquareCap,
                        Qt::MiterJoin));
    painter.setBrush(fill);
    painter.drawPath(legacy_arrow);

    painter.setPen(QPen(fill.darker(170), 1.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(legacy_arrow);

    DrawLegacyArrowText(&painter, arrow_rect, text(), isEnabled());
    return;
  }
#endif

  // Layer 1 - Shadow
  if (isEnabled()) {
    if (is_visually_pressed) {
      painter.setOpacity(0.06);
      painter.setBrush(Qt::black);
      painter.setPen(Qt::NoPen);
      painter.drawPath(arrow.translated(0, 0.5));
      painter.setOpacity(1.0);
    } else {
      painter.save();
      for (int i = 1; i <= 3; ++i) {
        painter.setOpacity((cfg.shadow_alpha / 255.0) / i);
        qreal spread = i * 1.8;
        painter.setBrush(cfg.shadow);
        painter.setPen(Qt::NoPen);
        painter.drawPath(arrow.translated(spread * 0.3, spread));
      }
      painter.restore();
    }
  }

  // Layer 2 - Body gradient
  QLinearGradient gradient(r.topLeft(), r.bottomLeft());
  if (!isEnabled()) {
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
  painter.drawPath(arrow);

  // Layer 3 - Highlight stroke
  if (isEnabled()) {
    painter.setPen(QPen(QColor(255, 255, 255, cfg.highlight_alpha), 1.1));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(arrow);
  }

  // Layer 4 - Axis label
  if (isEnabled() && !text().isEmpty()) {
    QFont f = font();
    f.setPointSize(qMax(8.0, qMin(r.width(), r.height()) / 5.0));
    painter.setFont(f);
    painter.setPen(cfg.text);
    if (style_ == ArrowStyle::Curved) {
      // 旋转按钮：文字绘制在箭头上半部分
      QRectF text_rect = r.adjusted(0, 2, 0, -r.height() * 0.4);
      painter.drawText(text_rect, Qt::AlignCenter, text());
    } else {
      painter.drawText(r, Qt::AlignCenter, text());
    }
  }
}

void PsArrowButton::enterEvent(QEvent* event) {
  is_hovered_ = true;
  update();
  QPushButton::enterEvent(event);
}

void PsArrowButton::leaveEvent(QEvent* event) {
  is_hovered_ = false;
  update();
  QPushButton::leaveEvent(event);
}

void PsArrowButton::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    is_pressed_ = true;
    update();
  }
  QPushButton::mousePressEvent(event);
}

void PsArrowButton::mouseReleaseEvent(QMouseEvent* event) {
  is_pressed_ = false;
  update();
  QPushButton::mouseReleaseEvent(event);
}
