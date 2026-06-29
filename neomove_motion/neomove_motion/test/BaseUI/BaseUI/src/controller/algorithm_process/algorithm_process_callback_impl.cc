// copyright 2025 YottaImage. All rights reserved.
// author panming
// date 2025/05/28 23:31
#include "algorithm_process_callback_impl.h"

#include <algorithm_process/algorithm_process.h>
#include <algorithm_process/algorithm_process_mgr.h>
#include <algorithm_process/algorithm_process_rpc_interface.h>
#include <algorithm_process/camera.h>
#include <algorithm_process/service_manager.h>
#include <base.h>
#include <common/json_config/json_config_helper.h>
#include <common/message_loop.h>
#include <common/path/path_utils.h>
#include <glog/glog_helper.h>
#include <main_process/algorithm_step_mgr.h>

#include <fstream>
#include <future>
#include <memory>
#include <nlohmann/json.hpp>

#include "controller/algorithm_result_wait_manager/algorithm_result_wait_manager.h"
#include "controller/camera_manager/camera_manager.h"
#include "model/model_mgr.h"
#include "model/module_info/module_info.h"

nlohmann::json AlgorithmProcessCallbackImpl::ReadCameraRealTimeConfigJson(
    const std::wstring& camera_config_path) {
  nlohmann::json root_json;
  try {
    std::ifstream json_file(camera_config_path);
    if (!json_file.is_open()) {
      std::ostringstream oss;
      LOG(ERROR) << "can not open camera_config_path json file: "
                 << camera_config_path;
      return root_json;
    }
    json_file >> root_json;
  } catch (const nlohmann::json::exception& e) {
    LOG(ERROR) << "JSON parse error: " << e.what();
    return root_json;
  } catch (const std::exception& e) {
    LOG(ERROR) << "read json error: " << e.what();
    return root_json;
  }
  return root_json;
}

using namespace common;
using namespace main_process;
// 算法进程跟主进程连接好了
void AlgorithmProcessCallbackImpl::OnConnected(const std::string& mod_id) {
  LOG(INFO) << "[" << mod_id << "]OnConnected";
}

AlgorithmProcessPtr AlgorithmProcessCallbackImpl::GetAlgProcess(
    const std::string& mod_id) {
  ModulePtr module_ptr = main_process::GetModuleMgr()->GetModuleByIds(mod_id);
  // assert(module_ptr);
  if (!module_ptr) {
    LOG(ERROR) << "module_ptr is null";
    return nullptr;
  }
  return module_ptr->GetAlgorithmProcess();
}

int AlgorithmProcessCallbackImpl::OnAlgorithmInitEnd(
    const std::string& mod_id) {
  LOG(INFO) << "[" << mod_id << "]AlgInitEnd";

  camera_json_ = ReadCameraRealTimeConfigJson(
      path_utils::GetFullPathFromCurrentModule(L"config\\camera.json"));

  ModuleInfoMgrPtr module_info_mgr_ =
      ModelMgrSinglton::GetInstance()->module_info_mgr();
  if (!module_info_mgr_) {
    LOG(ERROR) << "module_info_mgr_ = NULL";
    return -1;
  }
  ModuleInfo module_info = module_info_mgr_->GetModuleInfo(mod_id);

  AlgorithmProcessPtr alg_proc = GetAlgProcess(mod_id);
  if (!alg_proc) {
    return -1;
  }

  CameraPtr camera_ptr = alg_proc->camera();
  nlohmann::json camera_real_time_configs = camera_json_["camera_configs"];
  bool all_camera_open = true;
  for (nlohmann::json& camera_config : camera_real_time_configs) {
    int id = camera_config["id"];
    std::string camera_config_str = camera_config.dump();
    for (int j = 0; j < module_info.camera_id_list.size(); j++) {
      if (id == module_info.camera_id_list[j]) {
        if (camera_ptr) {
          int32_t ret = camera_ptr->StartGrab(
              id, static_cast<int32_t>(yotta::BufferType::kBufferTypeJson),
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
  // ModelMgrSinglton::GetInstance()->run_state()->set_camera_state(
  //    all_camera_open);

  // ModelMgrSinglton::GetInstance()->run_state()->set_alg_init(true);

  bool camera_manager_ready = CameraManagerSinglton::GetInstance()->Init(0);
  if (camera_manager_ready) {
  } else {
    LOG(ERROR)
        << "CameraManager init failed, image view will use fallback state only";
  }

  return 0;
}

TaskCallbackPtr AlgorithmProcessCallbackImpl::GetTaskCallbackPtr(
    const std::string& mod_id) {
  // ui线程
  TaskCallbackPtr task_callback_ptr;
  auto it = task_callback_map_.find(mod_id);
  if (it == task_callback_map_.end()) {
    auto task_callback_impl_ptr = std::make_shared<TaskCallbackImpl>();
    task_callback_map_[mod_id] = task_callback_impl_ptr;
    task_callback_impl_ptr->SetModuleIds(mod_id);
    task_callback_ptr = task_callback_impl_ptr;
  } else {
    task_callback_ptr = it->second;
  }
  return task_callback_ptr;
}

// 运动就绪的回调，表示算法进程就绪，可以开始采集图像了
int AlgorithmProcessCallbackImpl::OnAlgorithmIsReady(const std::string& mod_id,
                                                     int64_t sub_step_id,
                                                     int32_t msg_type,
                                                     const char* pb_buf,
                                                     int32_t pb_len) {
  LOG(INFO) << GetLogHeader(mod_id, sub_step_id, msg_type, pb_len)
            << "OnAlgorithmIsReady:" << (pb_buf ? pb_buf : "null");

  return 0;
}

// 算法进程收到了图像了，主进程调用算法进程的WaitingForAlgorithmResult开始等待收算法结果
int AlgorithmProcessCallbackImpl::OnImageIsReady(const std::string& mod_id,
                                                 int64_t sub_step_id,
                                                 int32_t msg_type,
                                                 const char* pb_buf,
                                                 int32_t pb_len) {
  LOG(INFO) << GetLogHeader(mod_id, sub_step_id, msg_type, pb_len)
            << "OnImageReady:" << (pb_buf ? pb_buf : "null");

  return 0;
}

// 算法结果的回调，一个sub_step_id对应一个算法结果,一个sub_step_id代表从
// alg_process->MotionIsReady开始到AlgorithmProcessCallbackImpl::OnAlgorithmResult
// 的一次交互流程完成了。
int AlgorithmProcessCallbackImpl::OnAlgorithmResult(const std::string& mod_id,
                                                    int64_t sub_step_id,
                                                    int32_t msg_type,
                                                    const char* pb_result_buf,
                                                    int32_t pb_result_len) {
  LOG(INFO) << GetLogHeader(mod_id, sub_step_id, msg_type, pb_result_len)
            << "OnAlgorithmResult:" << (pb_result_buf ? pb_result_buf : "null");

  // 构建结果
  AlgorithmResult result;
  try {
    std::string json_str;
    if (pb_result_buf && pb_result_len > 0) {
      json_str.assign(pb_result_buf, pb_result_len);
    }
    nlohmann::json result_json = nlohmann::json::parse(json_str);

    //获取算法结果
    std::string errms = result_json["errms"];
    int result_f = result_json["outval"][0];
    if (result_f != 0) {
      LOG(ERROR) << "算法返回异常 " << errms;
    }
    double result_y = result_json["outval"][1];
    double result_x = result_json["outval"][2];

    result.mod_id = mod_id;
    result.sub_step_id = sub_step_id;
    result.msg = errms;
    result.msg_type = msg_type;
    result.result.push_back(result_f);
    result.result.push_back(result_y);
    result.result.push_back(result_x);
  } catch (const std::exception& e) {
    // LOG(ERROR) << "OnAlgorithmResult::Run error:" << e.what();
  }

  AlgorithmResultWaitMgrSinglton::GetInstance()->Complete(sub_step_id, result);
  return 0;
}

void AlgorithmProcessCallbackImpl::OnDisconnected(
    const std::string& module_ids) {
  LOG(INFO) << "[" << module_ids << "]OnDisconnected";
}

void AlgorithmProcessCallbackImpl::Quit(void) {
  for (auto& it : task_callback_map_) {
    it.second->Quit();
  }

  // ModuleInfoMgrPtr module_info_mgr =
  //    ModelMgrSinglton::GetInstance()->module_info_mgr();
  // if (!module_info_mgr) {
  //  return;
  //}

  // for (int i = 0; i < module_info_mgr->GetModuleCount(); i++) {
  //  ModuleInfo module_info = module_info_mgr->GetModuleInfo(i);
  //  AlgorithmProcessPtr alg_proc = GetAlgProcess(module_info.ids);
  //  if (!alg_proc || !alg_proc->camera()) {
  //    continue;
  //  }
  //  for (int camera_id : module_info.camera_id_list) {
  //    alg_proc->camera()->StopGrab(camera_id);
  //    LOG(INFO) << "[" << module_info.ids << "] StopGrab camera_id="
  //              << camera_id;
  //  }
  //}
}

std::string AlgorithmProcessCallbackImpl::GetLogHeader(
    const std::string& mod_id, int64_t sub_step_id, int32_t msg_type,
    int32_t pb_result_len) {
  std::string header = "module[" + mod_id + "]sub_step_id[";
  header += std::to_string(sub_step_id) + "]msg_type[";
  header += std::to_string(msg_type) + "]result_len[";
  header += std::to_string(pb_result_len) + "]";
  return header;
}
