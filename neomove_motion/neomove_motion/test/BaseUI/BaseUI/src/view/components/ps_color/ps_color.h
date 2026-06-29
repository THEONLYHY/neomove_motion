// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#ifndef BASE_UI_SRC_VIEW_COMPONENTS_PS_COLOR_PS_COLOR_H_
#define BASE_UI_SRC_VIEW_COMPONENTS_PS_COLOR_PS_COLOR_H_

#include <QColor>
#include <QMap>

#include <functional>

#include "model/error_define.h"

class PsColor {
 public:
  enum class Color : uint32_t {
    kTransparent = 0x000000,
#if PS_LEGACY_UI
    kBlue = 0x250DFF,
    kWhite = 0xF3F4F6,
    kGreen = 0x1FA463,
    kRed = 0xD64545,
    kOrange = 0xFFD966,
    kYellow = 0xFFFF00,
    kTeal = 0x00A9D6,
    kGray = 0x7A869A,
    kPurple = 0xF700FF,
#else
    kBlue = 0x3B82F6,
    kWhite = 0xF3F4F6,
    kGreen = 0x10B981,
    kRed = 0xEF4444,
    kOrange = 0xF97316,
    kYellow = 0xEAB308,
    kTeal = 0x0D9488,
    kGray = 0x64748B,
    kPurple = 0x8B5CF6,
#endif
  };

  // 控件专属辅助色，不走 SetBaseColor，直接当 QColor 用
  struct WidgetColor {
    static QColor kFocusBorder;       // 聚焦边框 #3B82F6
    static QColor kBorderDefault;     // 默认边框 #D1D5DB
    static QColor kBorderDisabled;    // 禁用边框 #E5E7EB
    static QColor kTextDefault;       // 默认文字 #374151
    static QColor kTextDisabled;      // 禁用文字 #B0B0B0
    static QColor kBgDisabled;        // 禁用背景 #F3F4F6
    static QColor kSelectedRow;       // 选中行高亮 #EAF1FB
    static QColor kTrackBg;           // 进度条轨道 #D1D9E6
    static QColor kPageBg;            // 页面底色 #F0F2F5
  };

  struct Derived {
    QColor base;
    QColor top;
    QColor border;
    QColor shadow;
    QColor text;
    int shadow_alpha = 0;
    int highlight_alpha = 0;
  };

  PsColor();
  explicit PsColor(const QColor& color);
  explicit PsColor(Color color);

  void SetBaseColor(const QColor& color);
  void SetBaseColor(Color color);
  void Clear();
  void SetOnChanged(std::function<void()> on_changed);

  bool IsValid() const;
  const QColor& Base() const;
  const Derived& Get() const;
  static constexpr bool IsLegacyUi() { return PS_LEGACY_UI != 0; }

 private:
  static QColor ToQColor(Color color);
  static Derived BuildDerived(const QColor& color);

  QColor base_color_;
  std::function<void()> on_changed_;

  static QMap<QRgb, Derived> cache_;
};



#endif  // BASE_UI_SRC_VIEW_COMPONENTS_PS_COLOR_PS_COLOR_H_
