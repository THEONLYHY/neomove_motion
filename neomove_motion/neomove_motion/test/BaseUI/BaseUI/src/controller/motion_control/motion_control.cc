// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/05/22 09:56

#include "controller/motion_control/motion_control.h"

#include <algorithm_process/algorithm_process.h>
#include <common/message_loop.h>
#include <glog/glog_helper.h>
#include <main_process/module_mgr.h>

#include <chrono>
#include <future>

#include "controller/async_wait/async_wait.h"
#include "controller/device_status_monitor/device_status_monitor.h"
#include "controller/log_manager/log_view_sink.h"
#include "model/error_define.h"

MotionControl::MotionControl() {
  main_process::ModuleMgrPtr module_mgr = main_process::GetModuleMgr();
  if (!module_mgr) {
    LOG(ERROR) << "GetModuleMgr failed";
    return;
  }

  AlgorithmStepMgrPtr algorithm_step_mgr = module_mgr->GetAlgorithmStepMgr();
  if (!algorithm_step_mgr) {
    LOG(ERROR) << "GetAlgorithmStepMgr failed";
    return;
  }
  task_callback_impl_ptr_ = std::make_shared<TaskCallbackImpl>();
  axis_config_ = yotta::ConfigFactory::GetInstance()->GetAxisConfig();
  point_config_ = yotta::ConfigFactory::GetInstance()->GetPointConfig();
  limit_motion_ = module_mgr->GetLimitMotionMgr();
}

MotionControl::~MotionControl() {}

int MotionControl::DoTask(std::string task_name) {
  LOG_OPERATE("CLEAR");
  LOG_OPERATE(task_name);

  main_process::ModuleMgrPtr module_mgr = main_process::GetModuleMgr();
  if (!module_mgr) {
    LOG(ERROR) << "GetModuleMgr failed";
    return kMotionObjectError;
  }
  for (int i = 0; i < module_mgr->GetModuleCount(); i++) {
    main_process::ModulePtr module_ptr = module_mgr->GetModule(i);
    if (!module_ptr) {
      return kMotionObjectError;
    }
    main_process::TaskPtr task = module_ptr->GetTaskByIds(task_name);
    if (task) {
      return task->Run(task_callback_impl_ptr_);
    }
  }
  LOG(ERROR) << "未配置Task :" << task_name;
  return kMotionConfigError;
}

int MotionControl::DoTaskSync(std::string task_name, int delay_time) {
  LOG_OPERATE("CLEAR");
  LOG_OPERATE(task_name);

  using ResultWaitMgrSinglton = AsyncWaitManagerSinglton<int64_t, int64_t>;
  int64_t wait_key = std::hash<std::string>{}(task_name);
  main_process::ModuleMgrPtr module_mgr = main_process::GetModuleMgr();
  if (!module_mgr) {
    LOG(ERROR) << "GetModuleMgr failed";
    return kMotionObjectError;
  }
  for (int i = 0; i < module_mgr->GetModuleCount(); i++) {
    main_process::ModulePtr module_ptr = module_mgr->GetModule(i);
    if (!module_ptr) {
      return kMotionObjectError;
    }
    main_process::TaskPtr task = module_ptr->GetTaskByIds(task_name);
    if (task) {
      task->Run(task_callback_impl_ptr_);

      auto future = ResultWaitMgrSinglton::GetInstance()->Register(wait_key);
      if (future.wait_for(std::chrono::seconds(delay_time)) ==
          std::future_status::ready) {
        return static_cast<int>(future.get());
      } else {
        ResultWaitMgrSinglton::GetInstance()->Remove(wait_key);
        LOG(ERROR) << "Task :" << task_name << "运行超时!";
        task->Stop();
        Stop();
        return kMotionConfigError;
      }
    }
  }
  LOG(ERROR) << "未配置Task :" << task_name;
  return kMotionConfigError;
}

void MotionControl::StopTask(const std::string& task_name) {
  LOG_OPERATE(task_name);
  main_process::ModuleMgrPtr module_mgr = main_process::GetModuleMgr();
  if (!module_mgr) {
    return;
  }
  for (int i = 0; i < module_mgr->GetModuleCount(); i++) {
    main_process::ModulePtr module_ptr = module_mgr->GetModule(i);
    if (!module_ptr) {
      return;
    }
    main_process::TaskPtr task = module_ptr->GetTaskByIds(task_name);
    if (task) {
      task->Stop();
    }
  }
}

void MotionControl::Stop() {
  LOG_OPERATE("STOP");
  main_process::ModuleMgrPtr module_mgr = main_process::GetModuleMgr();
  if (!module_mgr) {
    return;
  }
  for (int i = 0; i < module_mgr->GetModuleCount(); i++) {
    main_process::ModulePtr module_ptr = module_mgr->GetModule(i);
    if (!module_ptr) {
      return;
    }
    for (int j = 0; j < module_ptr->GetTaskCount(); j++) {
      main_process::TaskPtr task = module_ptr->GetTask(j);
      if (task) {
        task->Stop();
      }
    }
  }

  yotta::AxisConfigPtr axis_config =
      yotta::ConfigFactory::GetInstance()->GetAxisConfig();
  yotta::LimitMotionMgrPtr limit_motion = module_mgr->GetLimitMotionMgr();
  if (axis_config && limit_motion) {
    for (int i = 0; i < axis_config->GetAxisCount(); i++) {
      yotta::AxisConfigItemPtr axis_item = axis_config->GetAxisById(i);
      if (!axis_item) {
        continue;
      }
      char axis_ids_char[256] = {0};
      axis_item->GetIds(axis_ids_char, sizeof(axis_ids_char));
      yotta::Axis* axis_motion = limit_motion->GetAxisByIds(axis_ids_char);
      if (axis_motion) {
        axis_motion->Stop();
      }
    }
  }

  using ResultWaitMgrSinglton = AsyncWaitManagerSinglton<int64_t, int64_t>;
  ResultWaitMgrSinglton::GetInstance()->CompleteAll(0);
}

AlgorithmStepParamPtr MotionControl::GetAlgStepPara(int step_id) {
  main_process::ModuleMgrPtr module_mgr = main_process::GetModuleMgr();
  if (!module_mgr) {
    return AlgorithmStepParamPtr();
  }
  AlgorithmStepMgrPtr algorithm_step_mgr = module_mgr->GetAlgorithmStepMgr();
  if (!algorithm_step_mgr) {
    return AlgorithmStepParamPtr();
  }
  return algorithm_step_mgr->GetStep(step_id);
}

void MotionControl::AlgStepParaSetRoi(int step_id, const std::string& key,
                                      const yotta::RectInt& val) {
  AlgorithmStepParamPtr algorithm_step_param = GetAlgStepPara(step_id);
  if (algorithm_step_param) {
    algorithm_step_param->SetRoi(key, val);
  }
}

void MotionControl::AlgStepParaSetString(int step_id, const std::string& key,
                                         const std::string& val) {
  AlgorithmStepParamPtr algorithm_step_param = GetAlgStepPara(step_id);
  if (algorithm_step_param) {
    algorithm_step_param->SetString(key, val);
  }
}

void MotionControl::AlgStepParaSetInt(int step_id, const std::string& key,
                                      int val) {
  AlgorithmStepParamPtr algorithm_step_param = GetAlgStepPara(step_id);
  if (algorithm_step_param) {
    algorithm_step_param->SetInt(key, val);
  }
}

void MotionControl::AlgStepParaSetDouble(int step_id, const std::string& key,
                                         double val) {
  AlgorithmStepParamPtr algorithm_step_param = GetAlgStepPara(step_id);
  if (algorithm_step_param) {
    algorithm_step_param->SetDouble(key, val);
  }
}

bool MotionControl::InitAccDecProfile(
    const std::string& axis_ids, const std::string& speed_ids,
    yotta::AccDecProfileImpl* acc_dec_profile) {
  if (!axis_config_) {
    LOG(ERROR) << "axis_config_ is null";
    return false;
  }

  if (!acc_dec_profile) {
    LOG(ERROR) << "acc_dec_profile is null";
    return false;
  }

  yotta::AxisConfigItemPtr axis_item =
      axis_config_->GetAxisByIds(axis_ids.c_str());
  if (!axis_item) {
    LOG(ERROR) << "axis_ids: " << axis_ids << " not found";
    return false;
  }

  yotta::AxisSpeedConfigPtr speed = nullptr;
  if (speed_ids.empty()) {
    speed = axis_item->GetDefaultSpeed();
  } else {
    speed = axis_item->GetSpeedByIds(speed_ids.c_str());
  }
  if (!speed) {
    LOG(ERROR) << "speed_ids: " << speed_ids << " not found";
    return false;
  }

  double acc_val = 0.0;
  speed->GetAcc(&acc_val);
  acc_dec_profile->set_acceleration(acc_val);

  double dec_val = 0.0;
  speed->GetDec(&dec_val);
  acc_dec_profile->set_deceleration(dec_val);

  double vel_val = 0.0;
  speed->GetVelocity(&vel_val);
  acc_dec_profile->set_velocity(vel_val);

  return true;
}

int MotionControl::MoveOneAxis(const std::string& axis_ids,
                               const std::string& speed_ids,
                               yotta::AccDecProfileImpl& acc_dec_profile,
                               double target_pos) {
  int axis_state = 0;
  if (DeviceStatusMonitorSinglton::GetInstance()->GetAxisState(axis_ids,
                                                               axis_state)) {
    if (axis_state != int(yotta::Axis::AxisState::kServoOn)) {
      LOG_OPERATE("轴未使能,不能执行运动: Axis :  " + axis_ids);
      return kMotionBlocked;
    }
  }

  int axis_home_state = 0;
  if (DeviceStatusMonitorSinglton::GetInstance()->GetAxisHomeState(
          axis_ids, axis_home_state)) {
    if (axis_home_state != int(yotta::Axis::AxisHomeState::kHomeOn)) {
      LOG_OPERATE("轴未回零,不能执行运动: Axis :  " + axis_ids);
      return kMotionBlocked;
    }
  }

  if (!InitAccDecProfile(axis_ids, speed_ids, &acc_dec_profile)) {
    return kMotionConfigError;
  }
  yotta::LimitMotionMgrPtr limit_motion =
      main_process::GetModuleMgr()->GetLimitMotionMgr();
  if (!limit_motion) {
    LOG(ERROR) << "get limit_motion error";
    return kMotionObjectError;
  }
  yotta::Axis* target_axis = limit_motion->GetAxisByIds(axis_ids.c_str());
  if (!target_axis) {
    LOG(ERROR) << "target_axis: " << axis_ids << " not found";
    return kMotionObjectError;
  }

  int ret = target_axis->AsyncMoveTo(target_pos, &acc_dec_profile);
  if (ret != 0) {
    LOG(ERROR) << "axis async move failed, axis_ids:" << axis_ids
               << " ret=" << ret;
    return ret;
  }
  ret = target_axis->Wait();
  if (ret != 0) {
    LOG(ERROR) << "axis wait failed, axis_ids:" << axis_ids << " ret=" << ret;
  }
  return ret;
}

int MotionControl::GoTo(const std::string& axis_ids,
                        const std::string& speed_ids, double position) {
  yotta::AccDecProfileImpl acc_dec_profile(0);
  return MoveOneAxis(axis_ids, speed_ids, acc_dec_profile, position);
}

int MotionControl::GoTo(const std::string& point_ids) {
  if (point_ids.empty()) {
    return kMotionConfigError;
  }
  if (!point_config_) {
    LOG(ERROR) << "point_config_ is null";
    return kMotionObjectError;
  }

  yotta::PointConfigItemPtr target_point =
      point_config_->GetPointByIds(point_ids.c_str());
  if (!target_point) {
    LOG(ERROR) << "get point by ids failed, point_ids: " << point_ids;
    return kMotionConfigError;
  }

  for (size_t i = 0; i < target_point->GetPointAxisItemCount(); ++i) {
    yotta::PointAxisConfigItemPtr point_axis =
        target_point->GetPointAxisItem(i);
    if (!point_axis) {
      continue;
    }
    char axis_ids[256] = {0};
    double pos = 0.0;
    int ret = point_axis->GetAxisIds(axis_ids, sizeof(axis_ids));
    ret += point_axis->GetPosition(&pos);
    if (ret != 0) {
      LOG(ERROR) << "get point position failed, point ids=" << point_ids;
      return ret;
    }

    yotta::AccDecProfileImpl acc_dec_profile(1);
    ret = MoveOneAxis(axis_ids, "运行速度", acc_dec_profile, pos);
    if (ret != kMotionOk) {
      LOG(ERROR) << "轴运动失败! axis_ids = " << axis_ids;
      return ret;
    }
  }
  return kMotionOk;
}

int MotionControl::TurnOnIO(const std::string& io_ids, bool on) {
  if (!limit_motion_) {
    LOG(ERROR) << "limit_motion_ is null";
    return kMotionObjectError;
  }
  yotta::OutputIO* output_io = limit_motion_->GetOutputIoByIds(io_ids.c_str());
  if (!output_io) {
    LOG(ERROR) << "get output_io error: " << io_ids;
    return kMotionObjectError;
  }
  return output_io->WriteValue(on);
}
