#ifndef COUPLING_MACHINE_SRC_VIEW_TOOLS_MULTI_IMAGE_SHOW_VIEW_MULTI_IMAGE_SHOW_VIEW_H_
#define COUPLING_MACHINE_SRC_VIEW_TOOLS_MULTI_IMAGE_SHOW_VIEW_MULTI_IMAGE_SHOW_VIEW_H_

#include <singleton.h>

#include <atomic>
#include <QDockWidget>
#include <QEvent>
#include <QLayout>
#include <QPointer>
#include <QWidget>
#include <string>
#include <vector>

#include "ui_multi_image_show_view.h"
#include "view/tools/multi_image_show_view/image_show_view/image_show_view.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MultiImageShowViewClass;
};
QT_END_NAMESPACE

class MultiImageShowView : public QWidget {
  Q_OBJECT
  SINGLETON(MultiImageShowView);

 public:
  ~MultiImageShowView();
  void ShowEmbedded(QLayout* layout);
  void ShowPage();
  void ShowPage(QWidget* anchor_widget);
  void ShowPage(int x, int y);
  void ShowPage(int x, int y, int width, int height);
  void HidePage();
  void ShowSingleCamera();
  void ShowCameraAll();
  void ShowCameraMulti(int camera_num, int camera_col);
  const std::string &module_ids(int camera_idx);
  int camera_id(int camera_idx);

  void showEvent(QShowEvent *event) override;
  void hideEvent(QHideEvent *event) override;
  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

  bool SaveBeamImage(const QString &image_path);

 protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

signals:
  void SignCameraChanged(int camera_idx, const std::string &module_ids,
                         const int camera_id, const int image_source);
  void SigClicked(int camera_idx, const QPointF &pos, const int camera_id);
 public slots:
  void OnShowQImage(int camera_id, int image_source, QImage);
  void OnDrawRectItem(std::string module_ids, int camera_id, QRect box);
  void OnPointClicked(int camera_idx, const QPointF &pos, const int camera_id);

 private:
  MultiImageShowView(QWidget *parent = nullptr);

  void ConnectCamera();
  int GetConfiguredCameraCount() const;
  QSize PreferredWindowSize() const;
  void ClearAnchorWidget();
  QRect AnchorGlobalGeometry(QWidget* anchor_widget) const;
  void ShowFloating(const QRect& global_geometry);
  void ApplyFloatingGeometry(const QRect& global_geometry);
  void UpdateFloatingGeometryFromAnchor();
  QDockWidget* FloatDockWidget();
  void ClearLayout(QLayout *layout);
  void CameraChanged(int camera_idx, const std::string &module_ids,
                     const int camera_id, const int image_source);
 private:
  int camera_num_ = 0;                                    //相机个数
  int camera_col_ = 3;                                    //图像窗口列数
  std::vector<QPointer<ImageShowView>> image_view_list_;  //图像显示控件
  QPointer<QDockWidget> float_dock_widget_;
  QPointer<QLayout> embedded_layout_;
  QPointer<QWidget> anchor_widget_;
 private:
  Ui::MultiImageShowViewClass *ui;
  std::atomic_bool show_ = false;
};

using MultiImageShowViewSingleton = yotta::Singleton<MultiImageShowView>;

#endif  // COUPLING_MACHINE_SRC_VIEW_TOOLS_MULTI_IMAGE_SHOW_VIEW_MULTI_IMAGE_SHOW_VIEW_H_
