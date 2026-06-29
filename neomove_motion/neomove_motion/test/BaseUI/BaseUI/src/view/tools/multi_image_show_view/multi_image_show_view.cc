#include "multi_image_show_view.h"

#include <QDockWidget>
#include <QEvent>
#include <QLayout>
#include <QMetaType>
#include <QPoint>
#include <QRect>
#include <QSizePolicy>
#include <QThread>
#include <QWidget>

#include "controller/camera_manager/camera_manager.h"
#include "model/module_info/module_info.h"
#include "model/model_mgr.h"

MultiImageShowView::MultiImageShowView(QWidget* parent)
    : QWidget(parent), ui(new Ui::MultiImageShowViewClass()) {
  ui->setupUi(this);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  ui->camera_gridlayout->setSizeConstraint(QLayout::SetNoConstraint);

  camera_num_ = GetConfiguredCameraCount();

  for (int i = 0; i < camera_num_; i++) {
    QPointer<ImageShowView> image_show_view = new ImageShowView;
    image_view_list_.push_back(image_show_view);
    ui->camera_gridlayout->addWidget(image_show_view, i / camera_col_,
                                     i % camera_col_, 1, 1);
    ui->camera_gridlayout->setRowStretch(i / camera_col_, 1);
    ui->camera_gridlayout->setColumnStretch(i % camera_col_, 1);

    image_show_view->setSizePolicy(QSizePolicy::Expanding,
                                   QSizePolicy::Expanding);
    connect(image_show_view, &ImageShowView::SignCameraChanged, this,
            [this, i](const std::string& module_ids, const int camera_id,
                      const int image_source) {
              CameraChanged(i, module_ids, camera_id, image_source);
            });

    connect(image_show_view, &ImageShowView::SignClicked, this,
            [this, i](const QPointF& pos, const int camera_id) {
              emit SigClicked(i, pos, camera_id);
            });
  }

  ConnectCamera();
  ShowSingleCamera();
  for (const auto& image_show_view : image_view_list_) {
    if (image_show_view) {
      image_show_view->OnParaChanged();
    }
  }
}

MultiImageShowView::~MultiImageShowView() {
  ClearAnchorWidget();
  delete ui;
}

void MultiImageShowView::ShowEmbedded(QLayout* layout) {
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(
        this, [this, layout]() { ShowEmbedded(layout); }, Qt::QueuedConnection);
    return;
  }
  if (!layout) {
    HidePage();
    return;
  }

  ClearAnchorWidget();
  hide();
  QDockWidget* dock_widget = FloatDockWidget();
  dock_widget->hide();
  if (embedded_layout_) {
    embedded_layout_->removeWidget(this);
    embedded_layout_->removeWidget(dock_widget);
  } else if (dock_widget->parentWidget() &&
             dock_widget->parentWidget()->layout()) {
    dock_widget->parentWidget()->layout()->removeWidget(dock_widget);
  }
  if (dock_widget->widget() == this) {
    dock_widget->setWidget(nullptr);
  }
  if (parentWidget() && parentWidget() != dock_widget &&
      parentWidget()->layout()) {
    parentWidget()->layout()->removeWidget(this);
  }

  setMinimumSize(QSize(0, 0));
  setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  dock_widget->setMinimumSize(QSize(0, 0));
  dock_widget->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
  dock_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setWindowFlags(Qt::Widget);
  if (layout->indexOf(this) < 0) {
    layout->addWidget(this);
  }
  embedded_layout_ = layout;
  layout->activate();
  show();
}

void MultiImageShowView::ShowPage() {
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(this, [this]() { ShowPage(); },
                              Qt::QueuedConnection);
    return;
  }

  QRect page_geometry = geometry();
  if (page_geometry.width() <= 0 || page_geometry.height() <= 0) {
    page_geometry.setSize(PreferredWindowSize());
  }
  ShowFloating(QRect(page_geometry.topLeft(), page_geometry.size()));
}

void MultiImageShowView::ShowPage(QWidget* anchor_widget) {
  if (!anchor_widget) {
    ShowPage();
    return;
  }
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(
        this, [this, anchor_widget]() { ShowPage(anchor_widget); },
        Qt::QueuedConnection);
    return;
  }

  if (anchor_widget_ && anchor_widget_ != anchor_widget) {
    anchor_widget_->removeEventFilter(this);
  }
  anchor_widget_ = anchor_widget;
  anchor_widget_->installEventFilter(this);
  ShowFloating(AnchorGlobalGeometry(anchor_widget));
}

void MultiImageShowView::ShowPage(int x, int y) {
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(this, [this, x, y]() { ShowPage(x, y); },
                              Qt::QueuedConnection);
    return;
  }

  ClearAnchorWidget();
  ShowFloating(QRect(QPoint(x, y), PreferredWindowSize()));
}

void MultiImageShowView::ShowPage(int x, int y, int width, int height) {
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(
        this, [this, x, y, width, height]() { ShowPage(x, y, width, height); },
        Qt::QueuedConnection);
    return;
  }

  QSize target_size(width, height);
  if (!target_size.isValid() || target_size.width() <= 0 ||
      target_size.height() <= 0) {
    target_size = PreferredWindowSize();
  }
  ClearAnchorWidget();
  ShowFloating(QRect(QPoint(x, y), target_size));
}

void MultiImageShowView::ShowFloating(const QRect& global_geometry) {
  QRect target_geometry = global_geometry;
  if (!target_geometry.isValid() || target_geometry.width() <= 0 ||
      target_geometry.height() <= 0) {
    target_geometry.setSize(PreferredWindowSize());
  }

  QDockWidget* dock_widget = FloatDockWidget();
  hide();
  dock_widget->hide();
  if (embedded_layout_) {
    embedded_layout_->removeWidget(this);
    embedded_layout_->removeWidget(dock_widget);
    embedded_layout_.clear();
  } else if (dock_widget->parentWidget() &&
             dock_widget->parentWidget()->layout()) {
    dock_widget->parentWidget()->layout()->removeWidget(dock_widget);
  }
  if (parentWidget() && parentWidget() != dock_widget &&
      parentWidget()->layout()) {
    parentWidget()->layout()->removeWidget(this);
  }
  if (dock_widget->widget() != this) {
    dock_widget->setWidget(this);
  }
  dock_widget->setFloating(true);
  dock_widget->setWindowFlags(Qt::Window | Qt::FramelessWindowHint |
                              Qt::WindowStaysOnTopHint);
  ApplyFloatingGeometry(target_geometry);
  dock_widget->show();
  show();
  dock_widget->raise();
  dock_widget->activateWindow();
}

void MultiImageShowView::HidePage() {
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(this, [this]() { HidePage(); },
                              Qt::QueuedConnection);
    return;
  }

  if (float_dock_widget_) {
    float_dock_widget_->hide();
  }
  hide();
  ClearAnchorWidget();
}

QSize MultiImageShowView::sizeHint() const { return QSize(0, 0); }

QSize MultiImageShowView::minimumSizeHint() const { return QSize(0, 0); }

QSize MultiImageShowView::PreferredWindowSize() const {
  QSize preferred_size = size();
  if (!preferred_size.isValid() || preferred_size.width() <= 0 ||
      preferred_size.height() <= 0) {
    preferred_size = sizeHint();
  }
  if (!preferred_size.isValid() || preferred_size.width() <= 0 ||
      preferred_size.height() <= 0) {
    preferred_size = QSize(1089, 692);
  }
  return preferred_size.expandedTo(minimumSizeHint());
}

void MultiImageShowView::ClearAnchorWidget() {
  if (anchor_widget_) {
    anchor_widget_->removeEventFilter(this);
    anchor_widget_.clear();
  }
}

QRect MultiImageShowView::AnchorGlobalGeometry(QWidget* anchor_widget) const {
  if (!anchor_widget) {
    return QRect(QPoint(0, 0), PreferredWindowSize());
  }

  const QPoint anchor_pos = anchor_widget->mapToGlobal(QPoint(0, 0));
  QSize anchor_size = anchor_widget->size();
  if (!anchor_size.isValid() || anchor_size.width() <= 0 ||
      anchor_size.height() <= 0) {
    anchor_size = PreferredWindowSize();
  }
  return QRect(anchor_pos, anchor_size);
}

void MultiImageShowView::ApplyFloatingGeometry(const QRect& global_geometry) {
  if (!global_geometry.isValid() || global_geometry.width() <= 0 ||
      global_geometry.height() <= 0) {
    return;
  }

  const QSize size = global_geometry.size();
  setMinimumSize(QSize(0, 0));
  setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  resize(size);

  if (!float_dock_widget_) {
    return;
  }

  float_dock_widget_->setMinimumSize(QSize(0, 0));
  float_dock_widget_->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
  float_dock_widget_->setSizePolicy(QSizePolicy::Expanding,
                                    QSizePolicy::Expanding);
  float_dock_widget_->resize(size);
  float_dock_widget_->setGeometry(global_geometry);
}

void MultiImageShowView::UpdateFloatingGeometryFromAnchor() {
  if (!anchor_widget_ || !float_dock_widget_ ||
      !float_dock_widget_->isVisible()) {
    return;
  }
  ApplyFloatingGeometry(AnchorGlobalGeometry(anchor_widget_));
}

QDockWidget* MultiImageShowView::FloatDockWidget() {
  if (float_dock_widget_) {
    return float_dock_widget_;
  }

  float_dock_widget_ = new QDockWidget;
  QWidget* title_bar = new QWidget(float_dock_widget_);
  title_bar->setFixedHeight(0);
  float_dock_widget_->setTitleBarWidget(title_bar);
  float_dock_widget_->setFeatures(QDockWidget::NoDockWidgetFeatures);
  float_dock_widget_->hide();
  return float_dock_widget_;
}

void MultiImageShowView::CameraChanged(int camera_idx,
                                       const std::string& module_ids,
                                       const int camera_id,
                                       const int image_source) {
  CameraManagerSinglton::GetInstance()->SetCameraIndex(camera_id);
  CameraManagerSinglton::GetInstance()->SetHighLow(image_source);
  emit SignCameraChanged(camera_idx, module_ids, camera_id, image_source);
}

int MultiImageShowView::GetConfiguredCameraCount() const {
  ModuleInfoMgrPtr module_info_mgr =
      ModelMgrSinglton::GetInstance()->module_info_mgr();
  if (!module_info_mgr) {
    return 0;
  }

  int camera_count = 0;
  for (int i = 0; i < module_info_mgr->GetModuleCount(); ++i) {
    const ModuleInfo module_info = module_info_mgr->GetModuleInfo(i);
    camera_count += static_cast<int>(module_info.camera_id_list.size());
  }
  return camera_count;
}
// 清空布局但不删除控件
void MultiImageShowView::ClearLayout(QLayout* layout) {
  if (!layout) return;

  QLayoutItem* item;
  while ((item = layout->takeAt(0)) != nullptr) {
    if (item->widget()) {
      // 如果有控件，只是从布局中移除，不删除
      // item->widget()->hide();
      item->widget()->setParent(nullptr);
    }
    // 删除布局项，但不删除控件
    delete item;
  }
}

void MultiImageShowView::ConnectCamera() {
  connect(CameraManagerSinglton::GetInstance(), &CameraManager::SignImageChanged,
          this, &MultiImageShowView::OnShowQImage, Qt::UniqueConnection);
}

void MultiImageShowView::ShowSingleCamera() {
  if (image_view_list_.size()) {
    // 清空布局
    ClearLayout(ui->camera_gridlayout);

    // 只显示第一个相机窗口，占满整个布局
    ui->camera_gridlayout->addWidget(image_view_list_[0], 0, 0, 1, 1);

    // 设置第一个窗口独占所有空间
    ui->camera_gridlayout->setRowStretch(0, 1);
    ui->camera_gridlayout->setColumnStretch(0, 1);

    // 设置第一个窗口的大小策略为扩展，使其占满整个空间
    image_view_list_[0]->setSizePolicy(QSizePolicy::Expanding,
                                       QSizePolicy::Expanding);
    image_view_list_[0]->show();

    // 隐藏其他所有窗口
    for (int i = 1; i < image_view_list_.size(); i++) {
      image_view_list_[i]->hide();
    }
  }
  ui->camera_gridlayout->activate();
}
void MultiImageShowView::ShowCameraAll() {  // 清空布局
  ClearLayout(ui->camera_gridlayout);
  // 计算行数
  int camera_row =
      (static_cast<int>(image_view_list_.size()) + camera_col_ - 1) /
      camera_col_;

  for (int i = 0; i < image_view_list_.size(); i++) {
    // 计算当前相机在网格中的位置
    int row = i / camera_col_;
    int col = camera_col_ - 1 - (i % camera_col_);
    image_view_list_[i]->set_camera_id(col);
    // 将相机窗口添加到网格布局中
    ui->camera_gridlayout->addWidget(image_view_list_[i], row, col, 1, 1);

    // 设置每个窗口的大小策略为扩展，使其平均分配空间
    image_view_list_[i]->setSizePolicy(QSizePolicy::Expanding,
                                       QSizePolicy::Expanding);
    image_view_list_[i]->show();
  }

  // 设置所有行和列的拉伸因子为1，使窗口平均分配空间
  for (int row = 0; row < camera_row; row++) {
    ui->camera_gridlayout->setRowStretch(row, 1);
  }
  for (int col = 0; col < camera_col_; col++) {
    ui->camera_gridlayout->setColumnStretch(col, 1);
  }
}

void MultiImageShowView::ShowCameraMulti(int camera_num, int camera_col) {
  // 清空布局
  ClearLayout(ui->camera_gridlayout);

  // 计算行数
  int camera_row = (camera_num + camera_col - 1) / camera_col;

  for (int i = 0; i < image_view_list_.size(); i++) {
    if (i < camera_num) {
      // 计算当前相机在网格中的位置
      int row = i / camera_col;
      int col = camera_col - 1 - (i % camera_col);

      // 将相机窗口添加到网格布局中
      ui->camera_gridlayout->addWidget(image_view_list_[i], row, col, 1, 1);

      // 设置每个窗口的大小策略为扩展，使其平均分配空间
      image_view_list_[i]->setSizePolicy(QSizePolicy::Expanding,
                                         QSizePolicy::Expanding);
      image_view_list_[i]->show();
    } else {
      // 隐藏多余的相机窗口
      image_view_list_[i]->hide();
    }
  }

  // 设置所有行和列的拉伸因子为1，使窗口平均分配空间
  for (int row = 0; row < camera_row; row++) {
    ui->camera_gridlayout->setRowStretch(row, 1);
  }
  for (int col = 0; col < camera_col; col++) {
    ui->camera_gridlayout->setColumnStretch(col, 1);
  }

  ui->camera_gridlayout->activate();
}

bool MultiImageShowView::SaveBeamImage(const QString& image_path) {
  if (image_view_list_[2]->camera_ids() == "光斑相机") {
    return image_view_list_[2]->SaveImage(image_path);
  } else if (image_view_list_[0]->camera_ids() == "光斑相机") {
    return image_view_list_[0]->SaveImage(image_path);
  } else if (image_view_list_[1]->camera_ids() == "光斑相机") {
    return image_view_list_[1]->SaveImage(image_path);
  } else {
    return false;
  }
}

void MultiImageShowView::OnShowQImage(int camera_id, int image_source,
                                      QImage qimg) {
  for (int i = 0; i < image_view_list_.size(); i++) {
    if (image_view_list_[i]->camera_id() == camera_id &&
        image_view_list_[i]->mode_idx() == image_source) {
      if (image_view_list_[i]->IsShow()) {
        image_view_list_[i]->ShowImage(qimg);
        break;
      }
    }
  }
}

void MultiImageShowView::OnDrawRectItem(std::string module_ids, int camera_id,
                                        QRect box) {
  for (int i = 0; i < image_view_list_.size(); i++) {
    if (image_view_list_[i]->module_ids() == module_ids &&
        image_view_list_[i]->camera_id() == camera_id) {
      image_view_list_[i]->image_view()->ClearAllItem();
      // image_view_list_[i]->image_view()->ClearItem(yotta_qt_plugin::ShapeType::kTypeRectangle);

      yotta_qt_plugin::RoiRect roi(box.center(), box.width(), box.height());
      image_view_list_[i]->image_view()->AddItem(roi, false);
    }
  }
}

void MultiImageShowView::OnPointClicked(int camera_idx, const QPointF& pos,
                                        const int camera_id) {
  Q_UNUSED(camera_idx);
  Q_UNUSED(pos);
  Q_UNUSED(camera_id);
}
const std::string& MultiImageShowView::module_ids(int camera_idx) {
  static const std::string empty_string = "";
  if (camera_idx >= 0 && camera_idx < image_view_list_.size()) {
    return image_view_list_[camera_idx]->module_ids();
  }
  return empty_string;
}
int MultiImageShowView::camera_id(int camera_idx) {
  if (camera_idx >= 0 && camera_idx < image_view_list_.size()) {
    return image_view_list_[camera_idx]->camera_id();
  }
  return -1;
}

void MultiImageShowView::showEvent(QShowEvent* event) { show_ = true; }

void MultiImageShowView::hideEvent(QHideEvent* event) { show_ = false; }

bool MultiImageShowView::eventFilter(QObject* watched, QEvent* event) {
  if (watched == anchor_widget_) {
    switch (event->type()) {
      case QEvent::Resize:
      case QEvent::Move:
      case QEvent::Show:
      case QEvent::LayoutRequest:
        UpdateFloatingGeometryFromAnchor();
        break;
      default:
        break;
    }
  }
  return QWidget::eventFilter(watched, event);
}
