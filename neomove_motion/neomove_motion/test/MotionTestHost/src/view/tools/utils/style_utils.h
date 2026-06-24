#ifndef SRC_VIEW_TOOLS_UTILS_STYLE_UTILS_H_
#define SRC_VIEW_TOOLS_UTILS_STYLE_UTILS_H_

#include <QString>
#include <QStyle>
#include <QVariant>
#include <QWidget>

namespace StyleUtils {

// 为控件设置动态样式类
// @param widget 目标控件，为空时直接返回
// @param class_name 样式类名，对应 QSS 中的 [class="xxx"] 选择器
inline void ApplyStyle(QWidget* widget, const QString& class_name) {
  if (!widget) return;

  widget->setProperty("class", class_name);
  widget->style()->unpolish(widget);
  widget->style()->polish(widget);
  widget->update();  // 强制重绘，确保 QSS 属性选择器变更后立即生效
}

}  // namespace StyleUtils

#endif  // SRC_VIEW_TOOLS_UTILS_STYLE_UTILS_H_
