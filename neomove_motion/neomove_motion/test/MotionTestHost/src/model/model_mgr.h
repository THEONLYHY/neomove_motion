#ifndef NEOMOVE_MOTION_TEST_HOST_SRC_MODEL_MODEL_MGR_H_
#define NEOMOVE_MOTION_TEST_HOST_SRC_MODEL_MODEL_MGR_H_

#include <functional>
#include <memory>

#include <singleton.h>

#include "model/unit_info/unit_info_mgr.h"
#include "model/unit_info/unit_point_mgr.h"

enum class ModelType {
  kModelUndefine,
  kModelModuleMgr,
};

using ModelMgrReadyCallback =
    std::function<void(ModelType model_type, bool success)>;

class ModelMgr {
  SINGLETON(ModelMgr);

 public:
  bool Init(ModelMgrReadyCallback callback);

  UnitInfoMgrPtr unit_info_mgr() const { return unit_info_mgr_; }
  UnitPointMgrPtr unit_point_mgr() const { return unit_point_mgr_; }

 private:
  ModelMgr() = default;

  UnitInfoMgrPtr unit_info_mgr_{new UnitInfoMgr()};
  UnitPointMgrPtr unit_point_mgr_{new UnitPointMgr()};
};

using ModelMgrSinglton = yotta::Singleton<ModelMgr>;

#endif  // NEOMOVE_MOTION_TEST_HOST_SRC_MODEL_MODEL_MGR_H_
