// copyright 2025 YottaImage. All rights reserved.
// author xiaozhijin
// date 2026/04/22 17:35

#include "ps_table_widget.h"

#include <QHeaderView>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QStyledItemDelegate>
#include <QTimer>

#include "view/components/ps_style/ps_legacy_painter.h"

class PsItemDelegate : public QStyledItemDelegate {
 public:
  explicit PsItemDelegate(QObject* parent = nullptr)
      : QStyledItemDelegate(parent) {}

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);

    auto* table = qobject_cast<PsTableWidget*>(parent());
    const auto& cfg =
        table ? table->Color().Get() : PsColor(PsColor::Color::kWhite).Get();

    QRect rect = option.rect;
    int rowCount = index.model()->rowCount();
    bool isLastRow = (index.row() == rowCount - 1);

#if PS_LEGACY_UI
    QColor fill = (option.state & QStyle::State_Selected)
                      ? PsColor::WidgetColor::kSelectedRow
                      : cfg.base;
    painter->fillRect(rect, fill);
    painter->setPen(QColor(0x8E, 0x97, 0xA4));
    painter->drawLine(rect.bottomLeft() - QPoint(0, 1),
                      rect.bottomRight() - QPoint(0, 1));
    painter->drawLine(rect.topRight(), rect.bottomRight());
#else
    if (option.state & QStyle::State_Selected) {
      painter->fillRect(rect, PsColor::WidgetColor::kSelectedRow);
    } else {
      painter->fillRect(rect, cfg.base);
    }

    if (!isLastRow) {
      QColor divider = cfg.border;
      divider.setAlpha(80);
      painter->setPen(divider);
      painter->drawLine(rect.bottomLeft() - QPoint(0, 1),
                        rect.bottomRight() - QPoint(0, 1));
    }
#endif

    QString text = index.data(Qt::DisplayRole).toString();

    // 读取 item 通过 setFont() 设置的字体
    QVariant fontData = index.data(Qt::FontRole);
    if (fontData.isValid() && fontData.canConvert<QFont>()) {
      painter->setFont(fontData.value<QFont>());
    }

    // 读取 item 通过 setTextAlignment() 设置的对齐方式
    int alignment = Qt::AlignVCenter | Qt::AlignLeft;
    QVariant alignData = index.data(Qt::TextAlignmentRole);
    if (alignData.isValid() && alignData.canConvert<int>()) {
      alignment = alignData.value<int>();
    }

    // 优先使用 item 通过 setForeground() 设置的前景色，未设置则用 PsColor 默认文字色
    QVariant fg = index.data(Qt::ForegroundRole);
    if (fg.isValid() && fg.canConvert<QBrush>()) {
      painter->setPen(fg.value<QBrush>().color());
    } else {
#if PS_LEGACY_UI
      painter->setPen(ps_style::LegacyTextColor(cfg.base));
#else
      painter->setPen(cfg.text);
#endif
    }

    QRect textRect = rect.adjusted(12, 0, -12, 0);
    painter->drawText(textRect, alignment, text);

    painter->restore();
  }

  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override {
    QSize base = QStyledItemDelegate::sizeHint(option, index);
    return base + QSize(24, 0);
  }
};

class PsHeaderView : public QHeaderView {
 public:
  explicit PsHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr)
      : QHeaderView(orientation, parent) {
    setSectionsClickable(true);
    setHighlightSections(false);
  }

 protected:
  void paintSection(QPainter* painter, const QRect& rect,
                    int logicalIndex) const override {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);

    auto* table = qobject_cast<PsTableWidget*>(parentWidget());
    const auto& cfg =
        table ? table->Color().Get() : PsColor(PsColor::Color::kWhite).Get();

#if PS_LEGACY_UI
    {
      painter->fillRect(rect, cfg.top);
      painter->setPen(QColor(0x8E, 0x97, 0xA4));
      painter->drawRect(rect.adjusted(0, 0, -1, -1));
      painter->setPen(ps_style::LegacyTextColor(cfg.top));
      QFont f = painter->font();
      f.setPixelSize(13);
      f.setWeight(QFont::Medium);
      painter->setFont(f);
      QRect textRect = rect.adjusted(12, 0, -12, 0);
      painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
                        model()
                            ->headerData(logicalIndex, orientation(),
                                         Qt::DisplayRole)
                            .toString());
      painter->restore();
      return;
    }
#endif

    painter->fillRect(rect, cfg.top);

    painter->setPen(cfg.border);
    painter->drawLine(rect.bottomLeft() - QPoint(0, 1),
                      rect.bottomRight() - QPoint(0, 1));

    QString text =
        model()
            ->headerData(logicalIndex, orientation(), Qt::DisplayRole)
            .toString();
    QColor header_text = cfg.text;
    header_text.setAlpha(160);
    painter->setPen(header_text);

    QFont f = painter->font();
    f.setPixelSize(13);
    f.setWeight(QFont::Medium);
    painter->setFont(f);

    QRect textRect = rect.adjusted(12, 0, -12, 0);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

    painter->restore();
  }
};

class PsCornerOverlay : public QWidget {
 public:
  explicit PsCornerOverlay(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
  }

  qreal radius = 8.0;

 protected:
  void paintEvent(QPaintEvent*) override {
    auto* table = qobject_cast<PsTableWidget*>(parentWidget());
    if (!table) return;

    int h = table->GetDrawHeight();
    if (h <= 0) return;

    const auto& cfg = table->Color().Get();

#if PS_LEGACY_UI
    {
      QRectF content(0.5, 0.5, width() - 1.0, h - 1.0);
      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, false);
      p.setPen(QPen(cfg.border, 1.0));
      p.setBrush(Qt::NoBrush);
      p.drawRect(content);
      return;
    }
#endif

    qreal r = qMin(radius, h / 2.0);
    QRectF content(0.5, 0.5, width() - 1.0, h - 1.0);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPainterPath full;
    full.addRect(rect());

    QPainterPath rounded;
    rounded.addRoundedRect(content, r, r);

    QColor maskColor = PsColor::WidgetColor::kPageBg;
    if (parentWidget() && parentWidget()->parentWidget()) {
      maskColor =
          parentWidget()->parentWidget()->palette().color(QPalette::Window);
    }

    p.setPen(Qt::NoPen);
    p.setBrush(maskColor);
    p.drawPath(full - rounded);

    p.setPen(QPen(cfg.border, 1.0));
    p.setBrush(Qt::NoBrush);
    p.drawPath(rounded);
  }
};

PsTableWidget::PsTableWidget(QWidget* parent) : QTableWidget(parent) {
  color_.SetOnChanged([this] {
    if (corner_overlay_) corner_overlay_->update();
    viewport()->update();
    update();
  });

  setAttribute(Qt::WA_TranslucentBackground);
  setFrameShape(QFrame::NoFrame);
  setShowGrid(false);
  verticalHeader()->setVisible(false);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setSelectionMode(QAbstractItemView::SingleSelection);
  setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  setFocusPolicy(Qt::NoFocus);

  color_.SetBaseColor(QColor(0xFF, 0xFF, 0xFF));

  viewport()->setAttribute(Qt::WA_TranslucentBackground);
  QPalette vp_pal = viewport()->palette();
  vp_pal.setColor(QPalette::Base, Qt::transparent);
  viewport()->setPalette(vp_pal);

  auto* customHeader = new PsHeaderView(Qt::Horizontal, this);
  customHeader->setFixedHeight(44);
  setHorizontalHeader(customHeader);

  setItemDelegate(new PsItemDelegate(this));

  InitCornerOverlay();

  QTimer* update_timer = new QTimer(this);
  update_timer->setSingleShot(true);
  update_timer->setInterval(0);

  connect(update_timer, &QTimer::timeout, this, [this]() {
    if (corner_overlay_) corner_overlay_->update();
    viewport()->update();
  });

  connect(model(), &QAbstractItemModel::rowsInserted, this,
          [update_timer]() { update_timer->start(); });
  connect(model(), &QAbstractItemModel::rowsRemoved, this,
          [update_timer]() { update_timer->start(); });
}

void PsTableWidget::InitCornerOverlay() {
  corner_overlay_ = new PsCornerOverlay(this);
  static_cast<PsCornerOverlay*>(corner_overlay_)->radius = border_radius_;
  corner_overlay_->setGeometry(0, 0, width(), height());
  corner_overlay_->raise();
}

PsColor& PsTableWidget::Color() { return color_; }

void PsTableWidget::SetBorderRadius(qreal radius) {
  border_radius_ = radius;
  static_cast<PsCornerOverlay*>(corner_overlay_)->radius = radius;
  corner_overlay_->update();
  update();
}

void PsTableWidget::SetShowBlankRows(bool show) {
  if (show_blank_rows_ != show) {
    show_blank_rows_ = show;
    corner_overlay_->update();
    viewport()->update();
    update();
  }
}

int PsTableWidget::GetDrawHeight() const {
  int content_h = 0;
  if (horizontalHeader()->isVisible()) {
    content_h += horizontalHeader()->height();
  }

  for (int i = 0; i < rowCount(); ++i) {
    content_h += rowHeight(i);
  }

  return show_blank_rows_ ? height() : qMin(content_h, height());
}

void PsTableWidget::resizeEvent(QResizeEvent* event) {
  QTableWidget::resizeEvent(event);
  corner_overlay_->setGeometry(0, 0, width(), height());
  corner_overlay_->raise();
}
