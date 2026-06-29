// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/05/22 09:56

#ifndef BASE_UI_SRC_CONTROLLER_MOTION_CONTROL_MOTION_CONTROL_H_
#define BASE_UI_SRC_CONTROLLER_MOTION_CONTROL_MOTION_CONTROL_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // !WIN32_LEAN_AND_MEAN

#include <limit_motion/impl/acc_dec_profile_impl.h>
#include <limit_motion/limit_motion_helper.h>
#include <main_process/algorithm_step_mgr.h>
#include <main_process/algorithm_step_param.h>
#include <singleton.h>

#include <memory>
#include <string>

#include "config/config_factory.h"
#include "controller/algorithm_process/algorithm_process_callback_impl.h"
#include "controller/task/task_callback_impl.h"

class MotionControl {
  SINGLETON(MotionControl);

 public:
  MotionControl();
  ~MotionControl();

  int DoTask(std::string task_name);
  int DoTaskSync(std::string task_name, int delay_time = 300);
  void StopTask(const std::string& task_name);
  void Stop();

  AlgorithmStepParamPtr GetAlgStepPara(int step_id);
  void AlgStepParaSetRoi(int alg_step_id, const std::string& key,
                         const yotta::RectInt& val);
  void AlgStepParaSetString(int alg_step_id, const std::string& key,
                            const std::string& val);
  void AlgStepParaSetInt(int alg_step_id, const std::string& key, int val);
  void AlgStepParaSetDouble(int alg_step_id, const std::string& key,
                            double val);

  int GoTo(const std::string& point_ids);
  int GoTo(const std::string& axis_ids, const std::string& speed_ids,
           double position);
  int TurnOnIO(const std::string& io_ids, bool on);

 private:
  bool InitAccDecProfile(const std::string& axis_ids,
                         const std::string& speed_ids,
                         yotta::AccDecProfileImpl* acc_dec_profile);
  int MoveOneAxis(const std::string& axis_ids, const std::string& speed_ids,
                  yotta::AccDecProfileImpl& acc_dec_profile, double target_pos);

 private:
  TaskCallbackImplPtr task_callback_impl_ptr_;
  yotta::AxisConfigPtr axis_config_;
  yotta::PointConfigPtr point_config_;
  yotta::LimitMotionMgrPtr limit_motion_;
  AlgorithmProcessCallbackImplPtr alg_callback_{
      std::make_shared<AlgorithmProcessCallbackImpl>()};
};

using MotionControlSinglton = yotta::Singleton<MotionControl>;

#endif  // BASE_UI_SRC_CONTROLLER_MOTION_CONTROL_MOTION_CONTROL_H_
