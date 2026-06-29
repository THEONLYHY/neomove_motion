// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#ifndef BASE_UI_SRC_VIEW_COMPONENTS_PS_LOADING_SPINNER_PS_LOADING_SPINNER_H_
#define BASE_UI_SRC_VIEW_COMPONENTS_PS_LOADING_SPINNER_PS_LOADING_SPINNER_H_

#include <QTimer>
#include <QWidget>

#include "view/components/ps_color/ps_color.h"

class PsLoadingSpinner : public QWidget {
  Q_OBJECT

 public:
  explicit PsLoadingSpinner(QWidget* parent = nullptr);

  void SetDotCount(int count);
  void SetRingDiameter(qreal diameter);
  void SetDotDiameter(qreal diameter);
  PsColor& Color();

  void Start();
  void Stop();
  bool IsRunning() const;

  QSize sizeHint() const override;

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  void OnTimeout();

  int dot_count_ = 16;
  qreal ring_diameter_ = 60.0;
  qreal dot_diameter_ = 8.0;
  PsColor color_;

  QTimer timer_;
  double phase_ = 0.0;
};



#endif  // BASE_UI_SRC_VIEW_COMPONENTS_PS_LOADING_SPINNER_PS_LOADING_SPINNER_H_
