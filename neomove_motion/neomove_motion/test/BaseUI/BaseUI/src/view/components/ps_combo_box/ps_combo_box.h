// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#ifndef BASE_UI_SRC_VIEW_COMPONENTS_PS_COMBO_BOX_PS_COMBO_BOX_H_
#define BASE_UI_SRC_VIEW_COMPONENTS_PS_COMBO_BOX_PS_COMBO_BOX_H_

#include <QComboBox>
#include <optional>

#include "view/components/ps_color/ps_color.h"

class PsComboBox : public QComboBox {
  Q_OBJECT

 public:
  explicit PsComboBox(QWidget* parent = nullptr);

  PsColor& Color();

  void SetBorderRadius(qreal radius);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void focusInEvent(QFocusEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;
  void showPopup() override;
  void hidePopup() override;
  void enterEvent(QEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void showEvent(QShowEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

 private:
  void InitPopupStyle();

  void DetectNeighbors();
  QPainterPath BuildRoundedRectPath(const QRectF& rect) const;

  PsColor color_ = PsColor(PsColor::Color::kBlue);
  qreal base_border_radius_ = 8.0;
  std::optional<qreal> corner_top_left_;
  std::optional<qreal> corner_top_right_;
  std::optional<qreal> corner_bottom_right_;
  std::optional<qreal> corner_bottom_left_;
  bool neighbor_cache_valid_ = false;

  bool is_focused_ = false;
  bool is_hovered_ = false;
  bool is_popup_visible_ = false;
};



#endif  // BASE_UI_SRC_VIEW_COMPONENTS_PS_COMBO_BOX_PS_COMBO_BOX_H_
