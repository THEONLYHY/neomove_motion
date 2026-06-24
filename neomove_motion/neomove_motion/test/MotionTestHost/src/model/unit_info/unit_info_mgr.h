#ifndef COUPLING_MACHINE_SRC_MODEL_UNIT_INFO_UNIT_INFO_H_
#define COUPLING_MACHINE_SRC_MODEL_UNIT_INFO_UNIT_INFO_H_

#include <common/json_config/json_config_helper.h>

#include <atomic>
#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "controller/robot_manual/robot_define.h"

//模块管理
class UnitInfoMgr : boost::noncopyable,
                    public std::enable_shared_from_this<UnitInfoMgr> {
 public:
  UnitInfoMgr(){};
  ~UnitInfoMgr(){};
  // 从Json加载数据,异步，加载完成后调用callback
  bool Init(const std::wstring& file_path);
  // 是否初始化完成
  bool IsReady(void) { return ready_; }

  int GetUnitCount();
  UnitInfoPtr GetUnitInfo(int index);
  UnitInfoPtr GetUnitInfo(std::string unit_ids);
  std::string GetUnitIdByAxisIds(std::string);
 private:
  std::atomic_bool ready_ = false;
  std::unique_ptr<JsonConfig> json_config_;

  std::mutex axis_list_mutex_;
  std::vector<UnitInfoPtr> module_info_;  // 轴属性 (使用shared_ptr存储)
};

// 对外都使用shared_ptr
using UnitInfoMgrPtr = std::shared_ptr<UnitInfoMgr>;

#endif  // COUPLING_MACHINE_SRC_MODEL_UNIT_INFO_UNIT_INFO_H_