// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#ifndef BASE_UI_SRC_VIEW_COMPONENTS_PS_TABLE_WIDGET_PS_TABLE_WIDGET_H_
#define BASE_UI_SRC_VIEW_COMPONENTS_PS_TABLE_WIDGET_PS_TABLE_WIDGET_H_

#include <QTableWidget>

#include "view/components/ps_color/ps_color.h"

class PsTableWidget : public QTableWidget {
  Q_OBJECT

 public:
  explicit PsTableWidget(QWidget* parent = nullptr);

  void SetBorderRadius(qreal radius);
  void SetShowBlankRows(bool show);
  int GetDrawHeight() const;
  PsColor& Color();

 protected:
  void resizeEvent(QResizeEvent* event) override;

 private:
  void InitCornerOverlay();

  PsColor color_;
  qreal border_radius_ = 8.0;
  bool show_blank_rows_ = true;
  QWidget* corner_overlay_ = nullptr;
};



#endif  // BASE_UI_SRC_VIEW_COMPONENTS_PS_TABLE_WIDGET_PS_TABLE_WIDGET_H_
