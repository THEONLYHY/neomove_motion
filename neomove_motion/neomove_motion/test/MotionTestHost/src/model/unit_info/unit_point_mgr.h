#ifndef COUPLING_MACHINE_SRC_MODEL_UNIT_INFO_UNIT_POINT_MGR_H_
#define COUPLING_MACHINE_SRC_MODEL_UNIT_INFO_UNIT_POINT_MGR_H_
#include <common/json_config/json_config_helper.h>

#include <atomic>
#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "model/device_info/motion_struct.h"

class UnitPointMgr : boost::noncopyable,
                  public std::enable_shared_from_this<UnitPointMgr> {
 public:
  UnitPointMgr();
  ~UnitPointMgr(){};
  // 从Json加载数据,异步，加载完成后调用callback
  bool Init(const std::wstring& file_path);
  // 是否初始化完成
  bool IsReady(void) { return ready_; }

  int GetUnitPointCount();
  UnitPoint GetUnitPoint(int index);
  UnitPoint GetUnitPointByIds(std::string point_ids); 
  void AddUnitPoint(const UnitPoint& value);  //增加点位,有则修改
  void DelUnitPoint(const std::string& point_name);

 private:
  int SaveToJson();

 private:
  std::wstring file_path_;
  std::atomic_bool ready_ = false;

  std::mutex point_list_mutex_;
  std::vector<UnitPoint> vec_point_info_;
  std::unique_ptr<JsonConfig> json_config_;
};

// 对外都使用shared_ptr
using UnitPointMgrPtr = std::shared_ptr<UnitPointMgr>;

#endif  // COUPLING_MACHINE_SRC_MODEL_UNIT_INFO_UNIT_POINT_MGR_H_
