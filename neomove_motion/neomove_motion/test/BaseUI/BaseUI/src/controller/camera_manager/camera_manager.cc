// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/28 17:13

#include "camera_manager.h"

#include <algorithm_process/algorithm_process.h>
#include <base.pb.h>
#include <common/message_loop.h>
#include <glog/glog_helper.h>
#include <google/protobuf/protobuf_helper.h>
#include <grab/camera_config.pb.h>
#include <main_process/module_mgr.h>

#include <algorithm>
#include <functional>

#include "controller/real_time_data/real_time_data.h"
#include "model/model_mgr.h"

void CameraConfigCallbackImpl::OnCameraCurrentConfig(int camera_id,
                                                     int err_code,
                                                     int32_t msg_type,
                                                     const char* buf,
                                                     int32_t len) {
  Q_UNUSED(msg_type);
  if (err_code != 0) {
    LOG(ERROR) << "OnCameraConfig err_code:" << err_code
               << " camera_id:" << camera_id;
    return;
  }

  camera_config::CameraConfig camera_config;
  camera_config.ParseFromArray(buf, len);

  CameraParaConfig camera_para_config;
  camera_para_config.id = camera_id;
  camera_para_config.exposure_time = camera_config.exposure_time();
  camera_para_config.gain = camera_config.gain();
  camera_para_config.frame_rate = camera_config.frame_rate();
  camera_para_config.frame_count = camera_config.frame_count();
  camera_para_config.image_type = camera_config.image_type();
  camera_para_config.roi.left = camera_config.roi().left();
  camera_para_config.roi.top = camera_config.roi().top();
  camera_para_config.roi.bottom = camera_config.roi().bottom();
  camera_para_config.roi.right = camera_config.roi().right();
  camera_para_config.reverse_image = camera_config.reverse_image();
  camera_para_config.trigger_type = camera_config.trigger_type();

  if (camera_para_changed_fun_) {
    camera_para_changed_fun_(camera_para_config);
  }
}

CameraManager::CameraManager(QObject* parent) : QObject(parent) {
  camera_config_callback_->SetCallback(std::bind(
      &CameraManager::CameraParaChanged, this, std::placeholders::_1));

  request_camera_para_timer_ = new QTimer(this);
  request_camera_para_timer_->setInterval(1000);
  connect(request_camera_para_timer_, &QTimer::timeout, this, [this]() {
    if (!initialized_) {
      return;
    }

    int camera_id = RealTimeDataSinglton::GetInstance()->camera_index();
    if (camera_id <= 0) {
      return;
    }

    RequestCameraPara(camera_id);
  });
  request_camera_para_timer_->start();
}

CameraManager::~CameraManager() {
  Stop();
}

void CameraManager::Stop() {
  if (!initialized_) {
    return;
  }
  initialized_ = false;
  if (request_camera_para_timer_) {
    request_camera_para_timer_->stop();
  }
  if (image_share_client_) {
    image_share_client_->StopListening();
  }
}

bool CameraManager::Init(int module_index) {
  //if (initialized_) {
  //  if (module_index_ != module_index) {
  //    LOG(WARNING) << "CameraManager already initialized with module_index:"
  //                 << module_index_
  //                 << ", ignore new module_index:" << module_index;
  //  }
  //  return true;
  //}

  module_index_ = module_index;

  main_process::ModuleMgrPtr module_mgr =
      ModelMgrSinglton::GetInstance()->module_mgr();
  if (!module_mgr) {
    LOG(ERROR) << "module_mgr is null";
    return false;
  }

  main_process::ModulePtr module_ptr = module_mgr->GetModule(module_index_);
  if (!module_ptr) {
    LOG(ERROR) << "module_ptr is null, module_index:" << module_index_;
    return false;
  }

  AlgorithmProcessPtr alg_proc = module_ptr->GetAlgorithmProcess();
  if (!alg_proc) {
    LOG(ERROR) << "algorithm process is null, module_index:" << module_index_;
    return false;
  }

  image_share_client_ = alg_proc->GetImageShareClient();
  if (!image_share_client_) {
    LOG(ERROR) << "GetImageShareClient failed";
    return false;
  }

  image_share_client_->StartListening(std::bind(
      &CameraManager::ImageRecived, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

  initialized_ = true;
  return true;
}

bool CameraManager::RequestCameraPara(int camera_id) {
  if (camera_id <= 0) {
    return false;
  }

  main_process::ModuleMgrPtr module_mgr =
      ModelMgrSinglton::GetInstance()->module_mgr();
  if (!module_mgr) {
    return false;
  }

  main_process::ModulePtr module_ptr = module_mgr->GetModule(module_index_);
  if (!module_ptr) {
    return false;
  }

  AlgorithmProcessPtr alg_proc = module_ptr->GetAlgorithmProcess();
  if (!alg_proc || !alg_proc->camera()) {
    return false;
  }

  alg_proc->camera()->SetCameraCallback(
      camera_config_callback_,
      common::MessageLoop::GetMessageLoop(common::kUi));
  alg_proc->camera()->GetCurrentConfig(camera_id);
  return true;
}

bool CameraManager::SetCameraIndex(int camera_id) {
  RealTimeDataSinglton::GetInstance()->set_camera_index(camera_id);
  if (camera_id <= 0) {
    return true;
  }
  bool refresh_camera_para_ok = RequestCameraPara(camera_id);
  if (!refresh_camera_para_ok) {
    LOG(ERROR) << "refresh camera para failed, camera_id:" << camera_id;
  }

  return refresh_camera_para_ok;
}
bool CameraManager::SetHighLow(int high_low) {
  RealTimeDataSinglton::GetInstance()->set_high_low(high_low);
  int camera_id = RealTimeDataSinglton::GetInstance()->camera_index();
  if (camera_id <= 0) {
    return true;
  }
  bool refresh_camera_para_ok = RequestCameraPara(camera_id);
  if (!refresh_camera_para_ok) {
    LOG(ERROR) << "refresh camera para failed, camera_id:" << camera_id
               << " high_low:" << high_low;
  }

  return refresh_camera_para_ok;
}
bool CameraManager::ScaleExposure(double scale) {
  if (scale <= 0) {
    LOG(ERROR) << "invalid exposure scale: " << scale;
    return false;
  }

  int camera_id = RealTimeDataSinglton::GetInstance()->camera_index();
  if (camera_id <= 0) {
    LOG(ERROR) << "invalid current camera id: " << camera_id;
    return false;
  }

  std::string current_high_low =
      RealTimeDataSinglton::GetInstance()->high_low() == 1 ? "high" : "low";
  CameraParaConfig camera_para;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    camera_para = current_camera_para_;
  }

  if (camera_para.id != camera_id) {
    LOG(ERROR) << "current camera para is not ready, camera_id:" << camera_id;
    if (!RequestCameraPara(camera_id)) {
      return false;
    }
    return false;
  }
  camera_para.high_low = current_high_low;
  camera_para.exposure_time *= scale;
  camera_para.exposure_time =
      camera_para.exposure_time < 1 ? 1 : camera_para.exposure_time;

  main_process::ModuleMgrPtr module_mgr =
      ModelMgrSinglton::GetInstance()->module_mgr();
  if (!module_mgr) {
    LOG(ERROR) << "module_mgr is null";
    return false;
  }

  main_process::ModulePtr module_ptr = module_mgr->GetModule(module_index_);
  if (!module_ptr) {
    LOG(ERROR) << "module_ptr is null, module_index:" << module_index_;
    return false;
  }

  AlgorithmProcessPtr alg_proc = module_ptr->GetAlgorithmProcess();
  if (!alg_proc || !alg_proc->camera()) {
    LOG(ERROR) << "camera is null, module_index:" << module_index_;
    return false;
  }

  std::string camera_config_str = CameraConfigToJson(camera_para);
  int32_t ret = alg_proc->camera()->StartGrab(
      camera_para.id, static_cast<int32_t>(yotta::BufferType::kBufferTypeJson),
      camera_config_str.c_str(),
      static_cast<int32_t>(camera_config_str.size()));
  {
    std::lock_guard<std::mutex> lock(mutex_);
    current_camera_para_ = camera_para;
  }
  return true;
}
bool CameraManager::SetCameraPara(const CameraParaConfig& camera_config) {
  main_process::ModuleMgrPtr module_mgr =
      ModelMgrSinglton::GetInstance()->module_mgr();
  if (!module_mgr) {
    LOG(ERROR) << "module_mgr is null";
    return false;
  }

  main_process::ModulePtr module_ptr = module_mgr->GetModule(module_index_);
  if (!module_ptr) {
    LOG(ERROR) << "module_ptr is null, module_index:" << module_index_;
    return false;
  }

  AlgorithmProcessPtr alg_proc = module_ptr->GetAlgorithmProcess();
  if (!alg_proc || !alg_proc->camera()) {
    LOG(ERROR) << "camera is null, module_index:" << module_index_;
    return false;
  }

  std::string camera_config_str = CameraConfigToJson(camera_config);
  int32_t ret = alg_proc->camera()->StartGrab(
      camera_config.id,
      static_cast<int32_t>(yotta::BufferType::kBufferTypeJson),
      camera_config_str.c_str(),
      static_cast<int32_t>(camera_config_str.size()));
  {
    std::lock_guard<std::mutex> lock(mutex_);
    current_camera_para_ = camera_config;
  }
  return true;
}

void CameraManager::ImageRecived(bool suc, const QImage& image,
                                 qt_common::PackageHeaderPtr header,
                                 std::string msg) {
  Q_UNUSED(msg);
  if (!suc || !header) {
    return;
  }

  {
    std::lock_guard<std::mutex> lock(mutex_);
    camera_image_cache_[header->camera_id] = image;
  }

  emit SignImageChanged(header->camera_id,
                        static_cast<int>(header->image_source) - 1, image);
  emit SignCurrentImageChanged(header->camera_id, image);
}

void CameraManager::CameraParaChanged(const CameraParaConfig& camera_config) {
  int current_camera_id = RealTimeDataSinglton::GetInstance()->camera_index();
  if (camera_config.id <= 0 || camera_config.id != current_camera_id) {
    return;
  }

  {
    std::lock_guard<std::mutex> lock(mutex_);
    current_camera_para_ = camera_config;
  }
}
