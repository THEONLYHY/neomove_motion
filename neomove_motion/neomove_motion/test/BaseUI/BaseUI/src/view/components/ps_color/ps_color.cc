// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#include "ps_color.h"

#include <QtMath>

QMap<QRgb, PsColor::Derived> PsColor::cache_;

namespace {

qreal Luma(const QColor& color) {
  return 0.2126 * color.redF() + 0.7152 * color.greenF() +
         0.0722 * color.blueF();
}

}  // namespace

PsColor::PsColor() = default;

PsColor::PsColor(const QColor& color) { SetBaseColor(color); }

PsColor::PsColor(Color color) { SetBaseColor(color); }

void PsColor::SetBaseColor(const QColor& color) {
  if (base_color_ == color) return;
  base_color_ = color;
  if (on_changed_) on_changed_();
}

void PsColor::SetBaseColor(Color color) { SetBaseColor(ToQColor(color)); }

void PsColor::Clear() {
  if (!base_color_.isValid()) return;
  base_color_ = QColor();
  if (on_changed_) on_changed_();
}

void PsColor::SetOnChanged(std::function<void()> on_changed) {
  on_changed_ = std::move(on_changed);
}

bool PsColor::IsValid() const { return base_color_.isValid(); }

const QColor& PsColor::Base() const { return base_color_; }

const PsColor::Derived& PsColor::Get() const {
  static const Derived kFallback = BuildDerived(ToQColor(Color::kBlue));
  if (!base_color_.isValid()) {
    return kFallback;
  }

  const QRgb key = base_color_.rgb();
  auto it = cache_.find(key);
  if (it == cache_.end()) {
    it = cache_.insert(key, BuildDerived(base_color_));
  }
  return it.value();
}

QColor PsColor::ToQColor(Color color) {
  if (color == Color::kTransparent) {
    return QColor(0, 0, 0, 0);
  }
  const uint32_t rgb = static_cast<uint32_t>(color);
  return QColor((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

#if PS_LEGACY_UI
QColor PsColor::WidgetColor::kFocusBorder = QColor(0x25, 0x0D, 0xFF);
QColor PsColor::WidgetColor::kBorderDefault = QColor(0x8E, 0x97, 0xA4);
QColor PsColor::WidgetColor::kBorderDisabled = QColor(0xA8, 0xB0, 0xBC);
QColor PsColor::WidgetColor::kTextDefault = QColor(0x11, 0x18, 0x27);
QColor PsColor::WidgetColor::kTextDisabled = QColor(0x6B, 0x72, 0x80);
QColor PsColor::WidgetColor::kBgDisabled = QColor(0xD7, 0xDC, 0xE2);
QColor PsColor::WidgetColor::kSelectedRow = QColor(0xFF, 0xF0, 0xA8);
QColor PsColor::WidgetColor::kTrackBg = QColor(0xB8, 0xC0, 0xCA);
QColor PsColor::WidgetColor::kPageBg = QColor(0xC9, 0xCF, 0xD8);
#else
QColor PsColor::WidgetColor::kFocusBorder = QColor(0x3B, 0x82, 0xF6);
QColor PsColor::WidgetColor::kBorderDefault = QColor(0xD1, 0xD5, 0xDB);
QColor PsColor::WidgetColor::kBorderDisabled = QColor(0xE5, 0xE7, 0xEB);
QColor PsColor::WidgetColor::kTextDefault = QColor(0x37, 0x41, 0x51);
QColor PsColor::WidgetColor::kTextDisabled = QColor(0xB0, 0xB0, 0xB0);
QColor PsColor::WidgetColor::kBgDisabled = QColor(0xF3, 0xF4, 0xF6);
QColor PsColor::WidgetColor::kSelectedRow = QColor(0xEA, 0xF1, 0xFB);
QColor PsColor::WidgetColor::kTrackBg = QColor(0xD1, 0xD9, 0xE6);
QColor PsColor::WidgetColor::kPageBg = QColor(0xF0, 0xF2, 0xF5);
#endif

PsColor::Derived PsColor::BuildDerived(const QColor& color) {
  Derived derived;

  // 透明模式：只保留文字色，背景/边框/阴影全透明
  if (color.alpha() == 0) {
    derived.base = QColor(0, 0, 0, 0);
    derived.top = QColor(0, 0, 0, 0);
    derived.border = QColor(0, 0, 0, 0);
    derived.shadow = QColor(0, 0, 0, 0);
    derived.text = WidgetColor::kTextDefault;
    derived.shadow_alpha = 0;
    derived.highlight_alpha = 0;
    return derived;
  }

#if PS_LEGACY_UI
  derived.base = color;
  derived.top = color.lighter(145);
  derived.border = color.darker(155);
  derived.shadow = color.darker(205);
  derived.text = Luma(color) >= 0.62 ? QColor("#111827") : QColor(Qt::white);
  derived.shadow_alpha = 120;
  derived.highlight_alpha = 210;
  return derived;
#endif

  derived.base = color;

  const qreal luma = Luma(color);
  const bool is_light = luma >= 0.72;
  const bool is_mid_light = luma >= 0.55;

  derived.top = is_light
                    ? color.lighter(103)
                    : (is_mid_light ? color.lighter(112) : color.lighter(128));
  derived.border = is_light
                       ? color.darker(110)
                       : (is_mid_light ? color.darker(118) : color.darker(132));
  derived.shadow = is_light ? QColor("#000000") : color.darker(185);
  derived.text = luma >= 0.68 ? QColor("#374151") : QColor(Qt::white);
  derived.shadow_alpha = is_light ? 30 : 50;
  derived.highlight_alpha = is_light ? 180 : 105;

  return derived;
}
