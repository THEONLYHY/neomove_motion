#ifndef COUPLING_MACHINE_SRC_VIEW_TOOLS_IMAGE_SHOW_VIEW_IMAGE_SHOW_VIEW_H_
#define COUPLING_MACHINE_SRC_VIEW_TOOLS_IMAGE_SHOW_VIEW_IMAGE_SHOW_VIEW_H_

#include <yotta_qt_plugin/image_view/image_view.h>
#include <yotta_qt_plugin/image_view/items/image_view_header.h>

#include <atomic>
#include <QPointer>
#include <QWidget>
#include <string>

#include "model/module_info/module_info.h"
#include "ui_image_show_view.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ImageShowViewClass;
};
QT_END_NAMESPACE

class ImageShowView : public QWidget {
  Q_OBJECT

 public:
  ImageShowView(QWidget *parent = nullptr);
  ~ImageShowView();
  const std::string &module_ids() { return module_ids_; }
  int camera_id() const { return camera_id_; }
  void set_camera_id(const int camera_id);
  int mode_idx() const { return mode_idx_; }
  std::string camera_ids() const {
    return module_info_mgr_->GetModuleInfo(module_idx_)
        .camera_name_list[camera_idx_];
  }
  void ShowImage(QImage);  //显示图像
  bool IsShow(void) {
    return show_;
  }
  QPointer<ImageView> image_view() { return image_view_; }
  void showEvent(QShowEvent* event) override {
    show_ = true;
  }
  void hideEvent(QHideEvent *event) override { show_ = false; }
  bool SaveImage(const QString &image_path);

public slots:
  void OnModuleChanged();     //模块更改
  void OnParaChanged();       //相机序号,显示模式更改
  void OnImageSaveClicked();  //存图
 signals:
  void SignCameraChanged(const std::string &module_ids, const int camera_id,
                         const int image_source);
  void SignClicked(const QPointF &pos, int camera_id);

 private:
  void ImageSave();

 private:
  std::string module_ids_;
  int module_idx_ = 0;
  int camera_idx_ = 0;
  int camera_id_ = 0;
  int mode_idx_ = 0; 

  bool is_first_image_ = true;

  Ui::ImageShowViewClass *ui;
  QImage image_;
  QPointer<ImageView> image_view_;
  ModuleInfoMgrPtr module_info_mgr_;
  std::atomic_bool show_ = false;
};

#endif  // COUPLING_MACHINE_SRC_VIEW_TOOLS_IMAGE_SHOW_VIEW_IMAGE_SHOW_VIEW_H_
