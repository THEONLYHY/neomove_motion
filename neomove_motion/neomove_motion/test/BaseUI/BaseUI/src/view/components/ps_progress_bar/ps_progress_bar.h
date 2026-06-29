// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#ifndef BASE_UI_SRC_VIEW_COMPONENTS_PS_PROGRESS_BAR_PS_PROGRESS_BAR_H_
#define BASE_UI_SRC_VIEW_COMPONENTS_PS_PROGRESS_BAR_PS_PROGRESS_BAR_H_

#include <QProgressBar>

#include "view/components/ps_color/ps_color.h"

class PsProgressBar : public QProgressBar {
  Q_OBJECT

 public:
  explicit PsProgressBar(QWidget* parent = nullptr);

  PsColor& Color();
  void SetBorderRadius(qreal radius);
  void SetTrackVisible(bool visible);

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  PsColor color_ = PsColor(PsColor::Color::kBlue);
  qreal border_radius_ = 8.0;
  bool track_visible_ = true;
};



#endif  // BASE_UI_SRC_VIEW_COMPONENTS_PS_PROGRESS_BAR_PS_PROGRESS_BAR_H_
