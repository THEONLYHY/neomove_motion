// copyright 2025 YottaImage. All rights reserved.
// author panming
// date 2025/01/15
#ifndef BASE_UI_SRC_CONTROLLER_ACTION_OPEN_CAMERA_OPEN_CAMERA_ACTION_H_
#define BASE_UI_SRC_CONTROLLER_ACTION_OPEN_CAMERA_OPEN_CAMERA_ACTION_H_

#include <action/impl/action_base.h>
#include <action/impl/action_base_with_model.h>
#include <action/model/action_model_impl.h>
#include <action/model/open_camera_model.h>
#include <action/total_actions.h>
#include <algorithm_process/algorithm_process.h>
#include <algorithm_process/camera.h>
#include <algorithm_process/service_manager.h>
#include <glog/glog_helper.h>
#include <main_process/module.h>

#include "model/camera_config/camera_config.h"
#include "model/model_mgr.h"
#include "model/module_info/module_info.h"
#include "controller/camera_manager/camera_manager.h"

// 打开相机Action,用来更新相机参数
//{
//  "type": "open_camera",
//  "param": {
//    "camera_id": 3,
//    "camera_config_ids": "上料上相机标定拍照"
//  }
//}
class OpenCameraAction : public action::ActionBaseWithModel<
                             ActionModelImpl<action::OpenCameraModel>> {
 public:
  OpenCameraAction(const char *type_name, action::ActionFramework *framework)
      : ActionBaseWithModel(type_name, framework) {
    LOG(INFO) << "OpenCameraAction constructed, type: "
              << (type_name ? type_name : "null");
  }
  int SetActionModel(action::ActionModelPtr action_model) override {
    int ret = ActionBaseWithModel::SetActionModel(action_model);
    if (ret != 0) {
      LOG(ERROR) << "SetActionModel failed, ret=" << ret;
      return ret;
    }
    DCHECK(axis_config_);
    if (!axis_config_ || !action_model_) {
      LOG(ERROR) << "axis_config_ is null";
      return 1;
    }
    try {
      const nlohmann::json &json_data = action_model_->GetJsonData();
      const nlohmann::json &param_json = json_data.at("param");
      camera_id_ = param_json["camera_id"].get<int>();
      camera_config_ids_ = param_json["camera_config_ids"].get<std::string>();
    } catch (const std::exception &e) {
      LOG(ERROR) << "parse json error, msg=" << e.what();
      return 2;
    }
    return 0;
  }
  int SyncFromModel() override { return 0; }
  int RunImpl(action::RunDataSource data_src) override {
    //更新相机参数
    ModuleInfoMgrPtr module_info_mgr_ =
        ModelMgrSinglton::GetInstance()->module_info_mgr();
    if (!module_info_mgr_) {
      LOG(ERROR) << "module_info_mgr_ = NULL";
    }
    CameraParaConfig camera_config =
        ModelMgrSinglton::GetInstance()->camera_config()->GetCameraInfoByIds(
            camera_config_ids_);
    if (camera_id_ != camera_config.id) {
      LOG(ERROR) << "打开相机配置错误! camera_id_ != camera_config.id";
      return 2;
    }
    CameraManagerSinglton::GetInstance()->SetCameraIndex(camera_id_);
    bool all_camera_open = true;
    for (int i = 0; i < module_info_mgr_->GetModuleCount(); i++) {
      ModuleInfo module_info = module_info_mgr_->GetModuleInfo(i);
      for (int j = 0; j < module_info.camera_id_list.size(); j++) {
        if (camera_id_ == module_info.camera_id_list[j]) {
          main_process::ModulePtr module_ptr =
              main_process::GetModuleMgr()->GetModuleByIds(module_info.ids);
          if (!module_ptr) {
            return -1;
          }
          AlgorithmProcessPtr alg_proc = module_ptr->GetAlgorithmProcess();
          if (!alg_proc) {
            return -1;
          }
          CameraPtr camera_ptr = alg_proc->camera();
          if (camera_ptr) {
            std::string camera_config_str = CameraConfigToJson(camera_config);
            int32_t ret = camera_ptr->StartGrab(
                camera_id_,
                static_cast<int32_t>(yotta::BufferType::kBufferTypeJson),
                camera_config_str.c_str(),
                static_cast<int32_t>(camera_config_str.size()));
            if (ret != 0) {
              all_camera_open = false;
            }
          } else {
            LOG(ERROR) << "获取相机失败";
          }
        }
      }
    }
    return 0;
  }

 private:
  int camera_id_ = 0;
  std::string camera_config_ids_;
};

#endif  // BASE_UI_SRC_CONTROLLER_ACTION_OPEN_CAMERA_OPEN_CAMERA_ACTION_H_
