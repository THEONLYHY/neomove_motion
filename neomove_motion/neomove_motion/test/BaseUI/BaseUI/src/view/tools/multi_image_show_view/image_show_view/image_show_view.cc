#include "image_show_view.h"

#include <common/message_loop.h>
#include <common/path/path_utils.h>
#include <glog/glog_helper.h>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QLayout>
#include <QSizePolicy>

#include "model/model_mgr.h"

ImageShowView::ImageShowView(QWidget* parent)
    : QWidget(parent), ui(new Ui::ImageShowViewClass()) {
  ui->setupUi(this);

  image_view_ = new ImageView;
  image_view_->setFrameShape(QFrame::NoFrame);
  image_view_->SetRendererPreference(
      yotta_qt_plugin::RendererPreference::kForceSoftware);
  image_view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  image_view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  image_view_->setSizePolicy(QSizePolicy::Expanding,
                             QSizePolicy::Expanding);
  ui->show_window_lay->setSizeConstraint(QLayout::SetNoConstraint);
  ui->show_window_lay->addWidget(image_view_);
  connect(image_view_, &ImageView::Clicked, this,
          [this](const QPointF& pos) { emit SignClicked(pos, camera_id_); });

  module_info_mgr_ = ModelMgrSinglton::GetInstance()->module_info_mgr();
  if (!module_info_mgr_) {
    LOG(ERROR) << "module_info_mgr_ = NULL";
    return;
  }
  for (int i = 0; i < module_info_mgr_->GetModuleCount(); i++) {
    ui->module_list->addItem(
        QString::fromStdString(module_info_mgr_->GetModuleInfo(i).name));
  }
  connect(ui->module_list, SIGNAL(currentIndexChanged(int)), this,
          SLOT(OnModuleChanged()));
  connect(ui->camera_list, SIGNAL(currentIndexChanged(int)), this,
          SLOT(OnParaChanged()));
  connect(ui->mode_list, SIGNAL(currentIndexChanged(int)), this,
          SLOT(OnParaChanged()));
  QObject::connect(ui->camera_save, SIGNAL(clicked()), this,
                   SLOT(OnImageSaveClicked()));
  if (module_info_mgr_->GetModuleCount()) {
    OnModuleChanged();
  }
}
ImageShowView::~ImageShowView() { delete ui; }

void ImageShowView::ShowImage(QImage imgShow) {
  if (imgShow.isNull()) {
    return;
  }
  image_ = imgShow;
  image_view_->SetImageItem(imgShow);
  image_view_->show();
  image_view_->update();
  if (is_first_image_) {
    image_view_->ZoomFit();
    is_first_image_ = false;
  }
  image_view_->DrawAssistLine(yotta_qt_plugin::AssistLine::kAssistCross,
                              Qt::blue);
}

void ImageShowView::set_camera_id(const int camera_id) {
    ui->camera_list->setCurrentIndex(camera_id);
}


void ImageShowView::OnModuleChanged() {
  module_idx_ = ui->module_list->currentIndex();
  if (!module_info_mgr_) {
    LOG(ERROR) << "module_info = NULL";
    image_view_->Reset();
    return;
  }
  module_ids_ = module_info_mgr_->GetModuleInfo(module_idx_).ids;
  ModuleInfo module_info = module_info_mgr_->GetModuleInfo(module_idx_);
  ui->camera_list->clear();
  for (int i = 0; i < module_info.camera_name_list.size(); i++) {
    ui->camera_list->addItem(
        QString::fromStdString(module_info.camera_name_list[i]));
  }
  if (module_info.camera_name_list.size()) {
    ui->camera_list->setCurrentIndex(0);
  }

  image_view_->Reset();
}
void ImageShowView::OnParaChanged() {
  module_idx_ = ui->module_list->currentIndex();
  module_ids_ = module_info_mgr_->GetModuleInfo(module_idx_).ids;
  camera_idx_ = ui->camera_list->currentIndex();
  mode_idx_ = ui->mode_list->currentIndex();
  if (module_idx_ < 0 || camera_idx_ < 0 || mode_idx_ < 0) {
    // LOG(ERROR) << "ImageShowView Para Error module_idx_ < 0 || camera_idx_ <0
    // "
    //              "|| mode_idx_ < 0";
    return;
  }
  camera_id_ =
      module_info_mgr_->GetModuleInfo(module_idx_).camera_id_list[camera_idx_];

  image_view_->Reset();
  is_first_image_ = true;

  emit SignCameraChanged(module_ids_, camera_id_, mode_idx_);
}
void ImageShowView::OnImageSaveClicked() {
  common::MessageLoop::GetMessageLoop(common::kIo)
      ->PostTask(std::bind(&ImageShowView::ImageSave, this));
}
void ImageShowView::ImageSave() {
  QDateTime current_time = QDateTime::currentDateTime();
  QString date = current_time.toString("yyyy-MM-dd");
  QString time = current_time.toString("yyyy-MM-dd_HH-mm-ss.zzz");

  QString path =
      QString::fromStdWString(path_utils::GetFullPathFromCurrentModule(
                                  L"ImageSave\\%1\\%2-%3-%4.bmp"))
          .arg(date)
          .arg(QString::fromStdString(module_ids_))
          .arg(QString::fromStdString(camera_ids()))
          .arg(time);
  SaveImage(path);
}

bool ImageShowView::SaveImage(const QString& image_path) {
  QFileInfo file(image_path);
  QDir().mkpath(file.absolutePath());
  if (image_.save(image_path)) {
    LOG(INFO) << "图像保存成功: " << image_path.toStdString();
    return true;
  } else {
    LOG(ERROR) << "截图保存失败，路径可能无效: " << image_path.toStdString();
    return false;
  }
}
