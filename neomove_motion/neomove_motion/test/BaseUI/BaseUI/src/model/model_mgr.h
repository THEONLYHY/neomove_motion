// copyright 2025 YottaImage. All rights reserved.
// author panming
// date 2025/05/09 10:50
#ifndef BASE_UI_SRC_MODEL_MODEL_MGR_H_
#define BASE_UI_SRC_MODEL_MODEL_MGR_H_

#include <singleton.h>
#include <main_process/module_mgr.h>

#include <functional>

#include "controller/real_time_data/real_time_data.h"
#include "model/calibration/calibration_para.h"
#include "model/camera_config/camera_config.h"
#include "model/db.h"
#include "model/module_info/module_info.h"
#include "model/unit_info/unit_info_mgr.h"
#include "model/unit_info/unit_point_mgr.h"
#include "model/user_manage/user_manage.h"

// 模型数据的类型
enum class ModelType {
  kModelUndefine,
  kModelModuleMgr,
  kModelCameraConfig,
  kModelModuleInfo,
  kModelUnitInfo,
  kModelUnitPoint,
  kModelUserManage,
  kModelRealTimeData,
  kModelCalibration,
};
using ModelMgrReadyCallback =
    std::function<void(ModelType model_type, bool suc)>;

// 所有的模型管理器，包括整个项目的数据
// BaseUI 只保留管理器骨架，具体项目按需添加自己的 model 成员。
class ModelMgr {
  SINGLETON(ModelMgr);

 private:
  ModelMgr() = default;

 public:
  bool Init(ModelMgrReadyCallback callback = nullptr);
  DBPtr db(void) const { return db_; }
  main_process::ModuleMgrPtr module_mgr(void) const {
    return main_process::GetModuleMgr();
  }
  CameraConfigMgrPtr camera_config(void) const { return camera_config_; }
  ModuleInfoMgrPtr module_info_mgr(void) const { return module_info_mgr_; }
  UnitInfoMgrPtr unit_info_mgr(void) const { return unit_info_mgr_; }
  UnitPointMgrPtr unit_point_mgr(void) const { return unit_point_mgr_; }
  CalibrationParaMgrPtr calibration_mgr(void) const { return calibration_mgr_; }
  UserManagerPtr user_mgr(void) const { return user_mgr_; }
  RealTimeData* real_time_data(void) const {
    return RealTimeDataSinglton::GetInstance();
  }

 private:
  bool InitOnIOThread(ModelMgrReadyCallback callback);

 private:
  DBPtr db_;
  CameraConfigMgrPtr camera_config_{new CameraConfigMgr};
  ModuleInfoMgrPtr module_info_mgr_{new ModuleInfoMgr};
  UnitInfoMgrPtr unit_info_mgr_{new UnitInfoMgr};
  UnitPointMgrPtr unit_point_mgr_{new UnitPointMgr};
  CalibrationParaMgrPtr calibration_mgr_{new CalibrationParaMgr};
  UserManagerPtr user_mgr_{new UserManager};
};

// 外部直接使用此接口
using ModelMgrSinglton = yotta::Singleton<ModelMgr>;

#endif  // BASE_UI_SRC_MODEL_MODEL_MGR_H_
