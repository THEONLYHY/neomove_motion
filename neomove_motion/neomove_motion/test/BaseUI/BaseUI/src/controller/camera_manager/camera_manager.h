// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/28 17:13

#ifndef BASE_UI_SRC_CONTROLLER_CAMERA_MANAGER_CAMERA_MANAGER_H_
#define BASE_UI_SRC_CONTROLLER_CAMERA_MANAGER_CAMERA_MANAGER_H_

#include <algorithm_process/algorithm_process.h>
#include <algorithm_process/camera.h>
#include <singleton.h>

#include <QImage>
#include <QObject>
#include <QTimer>
#include <functional>
#include <map>
#include <memory>
#include <mutex>

#include "model/camera_config/camera_config.h"
#include "qt_common/ipc/image_share_channel.h"

typedef std::function<void(const CameraParaConfig&)> CameraParaChangedFun;

class CameraConfigCallbackImpl : public CameraCallback {
 public:
  void OnCameraCurrentConfig(int camera_id, int err_code, int32_t msg_type,
                             const char* buf, int32_t len) override;
  void SetCallback(CameraParaChangedFun camera_para_changed_fun) {
    camera_para_changed_fun_ = camera_para_changed_fun;
  }

 private:
  CameraParaChangedFun camera_para_changed_fun_;
};

class CameraManager : public QObject {
  Q_OBJECT
  SINGLETON(CameraManager);

 public:
  explicit CameraManager(QObject* parent = nullptr);
  ~CameraManager() ;

  // 绑定算法模块并开始监听所有相机图像。
  bool Init(int module_index = 0);
  // 停止监听，断开与算法进程的相机连接
  void Stop();
  // 切换当前显示相机
  bool SetCameraIndex(int camera_id);
  // 切换当前高低倍
  bool SetHighLow(int high_low);
  // 按比例调整“当前相机 + 当前高低倍”对应配置的曝光。
  bool ScaleExposure(double scale);
  bool SetCameraPara(const CameraParaConfig& camera_config);

signals:
  void SignCurrentImageChanged(int camera_id, const QImage& image);
  void SignImageChanged(int camera_id, int image_source, const QImage& image);

 private:
  bool RequestCameraPara(int camera_id);
  void ImageRecived(bool suc, const QImage& image,
                    qt_common::PackageHeaderPtr header, std::string msg);
  void CameraParaChanged(const CameraParaConfig& camera_config);

 private:
  mutable std::mutex mutex_;
  qt_common::ImageShareClientQtPtr image_share_client_;
  std::shared_ptr<CameraConfigCallbackImpl> camera_config_callback_{
      new CameraConfigCallbackImpl()};
  QTimer* request_camera_para_timer_ = nullptr;
  int module_index_ = 0;
  bool initialized_ = false;
  CameraParaConfig current_camera_para_;
  std::map<int, QImage> camera_image_cache_;
};

using CameraManagerSinglton = yotta::Singleton<CameraManager>;

#endif  // BASE_UI_SRC_CONTROLLER_CAMERA_MANAGER_CAMERA_MANAGER_H_
