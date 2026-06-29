// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#include "alarm_lamp_view.h"

#include <algorithm>

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

#include "view/components/ps_color/ps_color.h"

namespace {
// iOS/macOS 风格配色，与 Ps 控件色系对齐
const QColor kHousingBg(QStringLiteral("#F8FAFC"));
const QColor kHousingBorder(QStringLiteral("#E2E8F0"));
const QColor kLampOffBg(QStringLiteral("#E5E7EB"));
const QColor kLampOffBorder(QStringLiteral("#D1D5DB"));
const QColor kRedOnColor(QStringLiteral("#EF4444"));
const QColor kRedOnBorder(QStringLiteral("#DC2626"));
const QColor kYellowOnColor(QStringLiteral("#F59E0B"));
const QColor kYellowOnBorder(QStringLiteral("#D97706"));
const QColor kGreenOnColor(QStringLiteral("#10B981"));
const QColor kGreenOnBorder(QStringLiteral("#059669"));
const double kHousingCorner = 12.0;
const double kLampGapRatio = 0.12;
const double kLampWidthRatio = 0.68;
const double kOuterPadding = 2.0;

QColor LampColor(bool is_on, const QColor& on_color, const QColor& off_color) {
  return is_on ? on_color : off_color;
}

void DrawLegacyAlarmLamp(QPainter* painter, const QRectF& view_rect,
                         const bool lamps_on[3]) {
  const double housing_width =
      std::min(view_rect.width() - kOuterPadding * 2.0, view_rect.height() * 0.56);
  const double housing_height =
      std::max(1.0, view_rect.height() - kOuterPadding * 2.0);
  const QRectF housing_rect((view_rect.width() - housing_width) / 2.0,
                            (view_rect.height() - housing_height) / 2.0,
                            housing_width, housing_height);

  painter->setRenderHint(QPainter::Antialiasing, false);
  painter->setPen(Qt::NoPen);
  painter->setBrush(QColor(0, 0, 0, 55));
  painter->drawRect(housing_rect.translated(2.0, 2.0));

  QRectF case_rect = housing_rect.adjusted(0.5, 0.5, -0.5, -0.5);
  painter->setBrush(QColor(0x1B, 0x22, 0x2B));
  painter->drawRect(case_rect);

  painter->setPen(QPen(QColor(0x58, 0x66, 0x76), 1.0));
  painter->drawLine(case_rect.topLeft(), case_rect.topRight());
  painter->drawLine(case_rect.topLeft(), case_rect.bottomLeft());
  painter->setPen(QPen(QColor(0x05, 0x07, 0x0A), 1.0));
  painter->drawLine(case_rect.topRight(), case_rect.bottomRight());
  painter->drawLine(case_rect.bottomLeft(), case_rect.bottomRight());
  painter->setPen(QPen(QColor(0x05, 0x07, 0x0A), 1.0));
  painter->drawRect(case_rect);

  const double inner_margin = housing_width * 0.15;
  const QRectF inner_rect = housing_rect.adjusted(inner_margin, inner_margin,
                                                   -inner_margin, -inner_margin);
  const double lamp_gap = inner_rect.height() * kLampGapRatio;
  const double lamp_diameter =
      std::min(inner_rect.width() * 0.72,
               (inner_rect.height() - lamp_gap * 2.0) / 3.0);
  const double lamp_left = inner_rect.center().x() - lamp_diameter / 2.0;
  const double lamp_top =
      inner_rect.top() + (inner_rect.height() - (lamp_diameter * 3.0 + lamp_gap * 2.0)) / 2.0;

  const QColor on_colors[3] = {QColor(0xFF, 0x20, 0x20),
                               QColor(0xFF, 0xD5, 0x00),
                               QColor(0x00, 0xF0, 0x30)};
  const QColor off_colors[3] = {QColor(0x50, 0x12, 0x12),
                                QColor(0x54, 0x46, 0x10),
                                QColor(0x0B, 0x3C, 0x1D)};

  painter->setRenderHint(QPainter::Antialiasing, true);
  for (int i = 0; i < 3; ++i) {
    const double top = lamp_top + i * (lamp_diameter + lamp_gap);
    const QRectF lamp_rect(lamp_left, top, lamp_diameter, lamp_diameter);
    const QColor fill = LampColor(lamps_on[i], on_colors[i], off_colors[i]);

    painter->setPen(QPen(QColor(0x05, 0x07, 0x0A), 2.0));
    painter->setBrush(QColor(0x0A, 0x0E, 0x12));
    painter->drawEllipse(lamp_rect.adjusted(-2.0, -2.0, 2.0, 2.0));

    painter->setPen(QPen(lamps_on[i] ? fill.darker(145) : QColor(0x18, 0x1F, 0x26),
                         1.0));
    painter->setBrush(fill);
    painter->drawEllipse(lamp_rect);

    painter->setPen(QPen(lamps_on[i] ? QColor(255, 255, 255, 80)
                                      : QColor(255, 255, 255, 28),
                         1.0));
    painter->drawArc(lamp_rect.adjusted(3.0, 3.0, -3.0, -3.0), 60 * 16,
                     110 * 16);
  }
}
}  // namespace

AlarmLampView::AlarmLampView(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);
  ui.btn_red_->hide();
  ui.btn_yellow_->hide();
  ui.btn_green_->hide();

  flash_timer_ = new QTimer(this);
  flash_timer_->setInterval(500);
  connect(flash_timer_, &QTimer::timeout, this, &AlarmLampView::OnFlashTimeout);
}

AlarmLampView::~AlarmLampView() {
  if (flash_timer_) {
    flash_timer_->stop();
  }
}

void AlarmLampView::SetAlarmType(int type) {
  if (current_alarm_type_ == type && (type == 1 || type == 2)) {
    return;
  }

  current_alarm_type_ = type;
  flash_timer_->stop();
  flash_state_ = false;

  if (type == 1 || type == 2) {
    flash_timer_->start();
  }

  update();
}

void AlarmLampView::ReTranslate() { ui.retranslateUi(this); }

void AlarmLampView::OnFlashTimeout() {
  flash_state_ = !flash_state_;
  update();
}

void AlarmLampView::paintEvent(QPaintEvent *event) {
  QWidget::paintEvent(event);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  const QRectF view_rect = rect();
  if (view_rect.width() <= 0.0 || view_rect.height() <= 0.0) {
    return;
  }

  if (PsColor::IsLegacyUi()) {
    const bool lamps_on[3] = {LampIsOn(1), LampIsOn(2), LampIsOn(3)};
    DrawLegacyAlarmLamp(&painter, view_rect, lamps_on);
    return;
  }

  const double housing_width =
      std::min(view_rect.width() - kOuterPadding * 2.0, view_rect.height() * 0.58);
  const double housing_height = std::max(1.0, view_rect.height() - kOuterPadding * 2.0);
  const QRectF housing_rect((view_rect.width() - housing_width) / 2.0,
                            (view_rect.height() - housing_height) / 2.0,
                            housing_width, housing_height);

  // 1. 外壳阴影
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(0, 0, 0, 15));
  painter.drawRoundedRect(housing_rect.translated(0, 2), kHousingCorner, kHousingCorner);

  // 2. 外壳背景 (浅色圆角)
  painter.setBrush(kHousingBg);
  painter.drawRoundedRect(housing_rect, kHousingCorner, kHousingCorner);

  // 3. 外壳边框
  painter.setPen(QPen(kHousingBorder, 1.0));
  painter.setBrush(Qt::NoBrush);
  painter.drawRoundedRect(housing_rect, kHousingCorner, kHousingCorner);

  // 4. 计算灯泡位置
  const double inner_margin = housing_width * 0.15;
  const QRectF inner_rect = housing_rect.adjusted(inner_margin, inner_margin,
                                                   -inner_margin, -inner_margin);
  const double lamp_gap = inner_rect.height() * kLampGapRatio;
  const double lamp_diameter =
      std::min(inner_rect.width() * kLampWidthRatio,
               (inner_rect.height() - lamp_gap * 2.0) / 3.0);
  const double lamp_left = inner_rect.center().x() - lamp_diameter / 2.0;
  const double lamp_top =
      inner_rect.top() + (inner_rect.height() - (lamp_diameter * 3.0 + lamp_gap * 2.0)) / 2.0;

  struct LampStyle {
    bool is_on;
    QColor bg_color;
    QColor border_color;
  };

  const LampStyle lamps[3] = {
      {LampIsOn(1), kRedOnColor, kRedOnBorder},
      {LampIsOn(2), kYellowOnColor, kYellowOnBorder},
      {LampIsOn(3), kGreenOnColor, kGreenOnBorder},
  };

  for (int i = 0; i < 3; ++i) {
    const double top = lamp_top + i * (lamp_diameter + lamp_gap);
    const QRectF lamp_rect(lamp_left, top, lamp_diameter, lamp_diameter);

    if (lamps[i].is_on) {
      // 亮灯外发光
      for (int g = 3; g >= 1; --g) {
        qreal expand = g * 2.5;
        int alpha = 25 / g;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(lamps[i].bg_color.red(), lamps[i].bg_color.green(),
                                lamps[i].bg_color.blue(), alpha));
        painter.drawEllipse(lamp_rect.adjusted(-expand, -expand, expand, expand));
      }
      // 亮灯本体
      painter.setPen(QPen(lamps[i].border_color, 1.0));
      painter.setBrush(lamps[i].bg_color);
      painter.drawEllipse(lamp_rect);
      // 高光 (iOS 玻璃感)
      QRectF highlight = lamp_rect.adjusted(lamp_diameter * 0.18, lamp_diameter * 0.12,
                                            -lamp_diameter * 0.28, -lamp_diameter * 0.38);
      painter.setPen(Qt::NoPen);
      painter.setBrush(QColor(255, 255, 255, 60));
      painter.drawEllipse(highlight);
    } else {
      // 灭灯
      painter.setPen(QPen(kLampOffBorder, 1.0));
      painter.setBrush(kLampOffBg);
      painter.drawEllipse(lamp_rect);
    }
  }
}

bool AlarmLampView::LampIsOn(int lamp_type) const {
  if (lamp_type == 3) {
    return current_alarm_type_ == 3;
  }

  return current_alarm_type_ == lamp_type && flash_state_;
}
