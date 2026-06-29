// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#include "camera_calibration_view.h"

#include <action/model/action_model.h>
#include <action/model/alg_process_model.h>
#include <action/total_actions.h>
#include <main_process/module_mgr.h>

#include "config/config_factory.h"
#include "controller/motion_control/motion_control.h"
#include "controller/real_time_data/real_time_data.h"
#include "model/model_mgr.h"
#include "model/unit_info/unit_info_mgr.h"
#include "view/tools/popup_dialog/popup_dialog.h"

CameraCalibrationView::CameraCalibrationView(QWidget* parent)
    : QWidget(parent) {
  ui.setupUi(this);

  {
    // 设置表格列数
    ui.camera_calibration_list->setColumnCount(1);
    ui.camera_calibration_list->setHorizontalHeaderLabels(
        {"Calibration List"});
    // 设置表格属性
    ui.camera_calibration_list->setSelectionBehavior(
        QAbstractItemView::SelectRows);
    ui.camera_calibration_list->setSelectionMode(
        QAbstractItemView::SingleSelection);
    // ui.camera_calibration_list->setAlternatingRowColors(true);

    // 设置列宽 - 充满整个表格宽度
    ui.camera_calibration_list->horizontalHeader()->setStretchLastSection(true);

    // 设置每行固定高度
    ui.camera_calibration_list->verticalHeader()->setDefaultSectionSize(69);
    ui.camera_calibration_list->verticalHeader()->setSectionResizeMode(
        QHeaderView::Fixed);

    connect(ui.camera_calibration_list, &QTableWidget::cellClicked, this,
            &CameraCalibrationView::OnCameraCalibrationlListItemClicked);
  }

  camera_calibration_para_mgr_ =
      ModelMgrSinglton::GetInstance()->calibration_mgr();
  if (!camera_calibration_para_mgr_) {
    return;
  }
  for (int i = 0; i < camera_calibration_para_mgr_->GetCalibrationParaCount();
       i++) {
    camera_calibration_para_ =
        camera_calibration_para_mgr_->GetCalibrationPara(i);

    int RowCont = ui.camera_calibration_list->rowCount();
    ui.camera_calibration_list->insertRow(RowCont);
    ui.camera_calibration_list->setItem(
        RowCont, 0,
        new QTableWidgetItem(QString::fromStdString(
            camera_calibration_para_->ids() + "\n" +
            camera_calibration_para_->axis_x_ids() + " " +
            camera_calibration_para_->axis_y_ids())));
  }
  if (ui.camera_calibration_list->rowCount()) {
    camera_calibration_para_ =
        camera_calibration_para_mgr_->GetCalibrationPara(0);
    ui.camera_calibration_list->selectRow(0);
  }

  connect(ui.pushButton_creact_temp, &QPushButton::clicked, this,
          &CameraCalibrationView::onClickedCreacTemplate);
  connect(ui.pushButton_calibration, &QPushButton::clicked, this,
          &CameraCalibrationView::onClickedCalibreation);

  connect(ui.pushButton_w_add, &QPushButton::clicked, this, &CameraCalibrationView::onClickedWAdd);
  connect(ui.pushButton_w_sub, &QPushButton::clicked, this, &CameraCalibrationView::onClickedWSub);
  connect(ui.pushButton_h_add, &QPushButton::clicked, this, &CameraCalibrationView::onClickedHAdd);
  connect(ui.pushButton_h_sub, &QPushButton::clicked, this, &CameraCalibrationView::onClickedHSub);
  connect(ui.pushButton_clear, &QPushButton::clicked, this, &CameraCalibrationView::onClickedClear);
}
CameraCalibrationView::~CameraCalibrationView() {}
void CameraCalibrationView::OnCameraCalibrationlListItemClicked(int row,
                                                                int column) {
  if (row >= 0 && row < ui.camera_calibration_list->rowCount() &&
      row < camera_calibration_para_mgr_->GetCalibrationParaCount()) {
    camera_calibration_para_ =
        camera_calibration_para_mgr_->GetCalibrationPara(row);
  }
}

void CameraCalibrationView::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  connect(this, &CameraCalibrationView::BoxChanged,
          this, &CameraCalibrationView::OnBoxChanged, Qt::UniqueConnection);
}

void CameraCalibrationView::OnBoxChanged(QRect box) {
  MultiImageShowView* image_view = MultiImageShowViewSingleton::GetInstance();
  image_view->OnDrawRectItem(image_view->module_ids(0), image_view->camera_id(0),
                             box);
}

void CameraCalibrationView::onClickedCreacTemplate() {
  PopupDialogSingleton::GetInstance()->ShowPage();
  if (!PopupDialogSingleton::GetInstance()->PopupInfo("是否创建标定模版?")) {
    return;
  }
  if (RealTimeDataSinglton::GetInstance()->is_first_page() != 1) {
    PopupDialogSingleton::GetInstance()->PopupInfo(
        "只在第一段界面可修改config参数");
    return;
  }
  if (!camera_calibration_para_) {
    LOG(ERROR) << "标定配置参数为空!";
    return;
  }
  if (camera_calibration_para_->camera_id() !=
      RealTimeDataSinglton::GetInstance()->camera_index()) {
    LOG(ERROR) << "标定配置相机与当前相机不符!";
    return;
  }

  QRect rect = QRect(image_w_ / 2 - box_width_ / 2,
                     image_h_ / 2 - box_height_ / 2, box_width_, box_height_);
  yotta::RectInt template_roi;
  template_roi.left = rect.top();
  template_roi.top = rect.left();
  template_roi.right = rect.bottom();
  template_roi.bottom = rect.right();

  int high_low = RealTimeDataSinglton::GetInstance()->high_low();
  if (RealTimeDataSinglton::GetInstance()->camera_index() == 1) {
    MotionControlSinglton::GetInstance()->AlgStepParaSetRoi(8, "roi_rects",
                                                            template_roi);
    if (high_low) {
      MotionControlSinglton::GetInstance()->DoTask("桥相机高倍标定创建模板");
    } else {
      MotionControlSinglton::GetInstance()->DoTask("桥相机低倍标定创建模板");
    }
  } else if (RealTimeDataSinglton::GetInstance()->camera_index() == 2) {
    MotionControlSinglton::GetInstance()->AlgStepParaSetRoi(81, "roi_rects",
                                                            template_roi);

    if (high_low) {
      MotionControlSinglton::GetInstance()->DoTask("针相机高倍标定创建模板");
    } else {
      MotionControlSinglton::GetInstance()->DoTask("针相机低倍标定创建模板");
    }
  }
}
void CameraCalibrationView::onClickedCalibreation() {
  PopupDialogSingleton::GetInstance()->ShowPage();
  if (!PopupDialogSingleton::GetInstance()->PopupInfo("是否开始自动标定?")) {
    return;
  }
  if (RealTimeDataSinglton::GetInstance()->is_first_page() != 1) {
    PopupDialogSingleton::GetInstance()->PopupInfo(
        "只在第一段界面可修改config参数");
    return;
  }
  if (!camera_calibration_para_) {
    LOG(ERROR) << "标定配置参数为空!";
    return;
  }
  if (camera_calibration_para_->camera_id() !=
      RealTimeDataSinglton::GetInstance()->camera_index()) {
    LOG(ERROR) << "标定配置相机与当前相机不符!";
    return;
  }
  RealTimeDataSinglton::GetInstance()->set_calibration_para_ids(
      camera_calibration_para_->ids());
  int high_low = RealTimeDataSinglton::GetInstance()->high_low();
  if (RealTimeDataSinglton::GetInstance()->camera_index() == 1) {
    if (high_low) {
      MotionControlSinglton::GetInstance()->DoTask("桥相机高倍标定");
    } else {
      MotionControlSinglton::GetInstance()->DoTask("桥相机低倍标定");
    }
  } else if (RealTimeDataSinglton::GetInstance()->camera_index() == 2) {
    if (high_low) {
      MotionControlSinglton::GetInstance()->DoTask("针相机高倍标定");
    } else {
      MotionControlSinglton::GetInstance()->DoTask("针相机低倍标定");
    }
  }
}

void CameraCalibrationView::onClickedWAdd() {
  if (!camera_calibration_para_) {
    LOG(ERROR) << "标定配置参数为空!";
    return;
  }
  if (camera_calibration_para_->camera_id() !=
      RealTimeDataSinglton::GetInstance()->camera_index()) {
    LOG(ERROR) << "标定配置相机与当前相机不符!";
    return;
  }

  box_width_ += box_step_;
  int camera_id = RealTimeDataSinglton::GetInstance()->camera_index();
  image_w_ = 1280;
  image_h_ = 960;
  CameraConfigMgrPtr camera_config_mgr =
      ModelMgrSinglton::GetInstance()->camera_config();
  for (int i = 0; i < camera_config_mgr->GetCameraInfoCount(); i++) {
    CameraParaConfig camera_config = camera_config_mgr->GetCameraInfo(i);
    if (camera_config.id == camera_id) {
      image_w_ = camera_config.roi.right - camera_config.roi.left;
      image_h_ = camera_config.roi.bottom - camera_config.roi.top;
    }
  }

  box_width_ = box_width_ > image_w_ ? image_w_ : box_width_;
  emit BoxChanged(QRect(image_w_ / 2 - box_width_ / 2,
                        image_h_ / 2 - box_height_ / 2, box_width_,
                        box_height_));
}
void CameraCalibrationView::onClickedWSub() {
  if (!camera_calibration_para_) {
    LOG(ERROR) << "标定配置参数为空!";
    return;
  }
  if (camera_calibration_para_->camera_id() !=
      RealTimeDataSinglton::GetInstance()->camera_index()) {
    LOG(ERROR) << "标定配置相机与当前相机不符!";
    return;
  }

  box_width_ -= box_step_;
  box_width_ = box_width_ < box_step_ ? box_step_ : box_width_;
  int camera_id = RealTimeDataSinglton::GetInstance()->camera_index();
  image_w_ = 1280;
  image_h_ = 960;
  CameraConfigMgrPtr camera_config_mgr =
      ModelMgrSinglton::GetInstance()->camera_config();
  for (int i = 0; i < camera_config_mgr->GetCameraInfoCount(); i++) {
    CameraParaConfig camera_config = camera_config_mgr->GetCameraInfo(i);
    if (camera_config.id == camera_id) {
      image_w_ = camera_config.roi.right - camera_config.roi.left;
      image_h_ = camera_config.roi.bottom - camera_config.roi.top;
    }
  }

  emit BoxChanged(QRect(image_w_ / 2 - box_width_ / 2,
                        image_h_ / 2 - box_height_ / 2, box_width_,
                        box_height_));
}
void CameraCalibrationView::onClickedHAdd() {
  if (!camera_calibration_para_) {
    LOG(ERROR) << "标定配置参数为空!";
    return;
  }
  if (camera_calibration_para_->camera_id() !=
      RealTimeDataSinglton::GetInstance()->camera_index()) {
    LOG(ERROR) << "标定配置相机与当前相机不符!";
    return;
  }

  box_height_ += box_step_;
  int camera_id = RealTimeDataSinglton::GetInstance()->camera_index();
  image_w_ = 1280;
  image_h_ = 960;
  CameraConfigMgrPtr camera_config_mgr =
      ModelMgrSinglton::GetInstance()->camera_config();
  for (int i = 0; i < camera_config_mgr->GetCameraInfoCount(); i++) {
    CameraParaConfig camera_config = camera_config_mgr->GetCameraInfo(i);
    if (camera_config.id == camera_id) {
      image_w_ = camera_config.roi.right - camera_config.roi.left;
      image_h_ = camera_config.roi.bottom - camera_config.roi.top;
    }
  }

  box_height_ = box_height_ > image_h_ ? image_h_ : box_height_;

  emit BoxChanged(QRect(image_w_ / 2 - box_width_ / 2,
                        image_h_ / 2 - box_height_ / 2, box_width_,
                        box_height_));
}
void CameraCalibrationView::onClickedHSub() {
  if (!camera_calibration_para_) {
    LOG(ERROR) << "标定配置参数为空!";
    return;
  }
  if (camera_calibration_para_->camera_id() !=
      RealTimeDataSinglton::GetInstance()->camera_index()) {
    LOG(ERROR) << "标定配置相机与当前相机不符!";
    return;
  }

  box_height_ -= box_step_;
  box_height_ = box_height_ < box_step_ ? box_step_ : box_height_;
  int camera_id = RealTimeDataSinglton::GetInstance()->camera_index();

  image_w_ = 1280;
  image_h_ = 960;
  CameraConfigMgrPtr camera_config_mgr =
      ModelMgrSinglton::GetInstance()->camera_config();
  for (int i = 0; i < camera_config_mgr->GetCameraInfoCount(); i++) {
    CameraParaConfig camera_config = camera_config_mgr->GetCameraInfo(i);
    if (camera_config.id == camera_id) {
      image_w_ = camera_config.roi.right - camera_config.roi.left;
      image_h_ = camera_config.roi.bottom - camera_config.roi.top;
    }
  }

  emit BoxChanged(QRect(image_w_ / 2 - box_width_ / 2,
                        image_h_ / 2 - box_height_ / 2, box_width_,
                        box_height_));
}
void CameraCalibrationView::onClickedClear() {}
