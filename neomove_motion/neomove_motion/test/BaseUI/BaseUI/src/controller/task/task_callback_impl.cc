// copyright 2025 YottaImage. All rights reserved.
// author panming
// date 2025/09/04 15:30
#include "controller/task/task_callback_impl.h"

#include <QtGlobal>
#include <glog/glog_helper.h>

#include "controller/async_wait/async_wait.h"

using namespace main_process;

bool TaskCallbackImpl::OnTaskStart(std::shared_ptr<main_process::Task> task) {
  LOG(INFO) << "Task-" << (task ? task->ids() : "") << "_Start";
  return !quit_;
}

bool TaskCallbackImpl::OnStepStart(std::shared_ptr<main_process::Task> task,
                                   main_process::StepPtr step) {
  Q_UNUSED(task);
  LOG(INFO) << "Step-" << (step ? step->ids() : "") << "_Start";
  return !quit_;
}

bool TaskCallbackImpl::OnActionStart(std::shared_ptr<main_process::Task> task,
                                     main_process::StepPtr step,
                                     action::ActionPtr action_to_run,
                                     size_t idx) {
  Q_UNUSED(task);
  Q_UNUSED(step);
  Q_UNUSED(action_to_run);
  Q_UNUSED(idx);
  return !quit_;
}

bool TaskCallbackImpl::OnActionEnd(std::shared_ptr<main_process::Task> task,
                                   main_process::StepPtr step,
                                   action::ActionPtr action_run_end,
                                   size_t idx) {
  Q_UNUSED(task);
  Q_UNUSED(step);
  Q_UNUSED(action_run_end);
  Q_UNUSED(idx);
  return !quit_;
}

bool TaskCallbackImpl::OnStepEnd(std::shared_ptr<main_process::Task> task,
                                 main_process::StepPtr step) {
  Q_UNUSED(task);
  LOG(INFO) << "Step-" << (step ? step->ids() : "") << "_End";
  return !quit_;
}

void TaskCallbackImpl::OnTaskEnd(std::shared_ptr<main_process::Task> task) {
  std::string task_ids = task ? task->ids() : "";
  LOG(INFO) << "Task-" << task_ids << "_End";
  CompleteTaskWait(task_ids, 0);
}

void TaskCallbackImpl::OnActionFailed(std::shared_ptr<main_process::Task> task,
                                      main_process::StepPtr step,
                                      action::ActionPtr action, size_t idx,
                                      ExecutionErrorPtr error) {
  Q_UNUSED(task);
  Q_UNUSED(step);
  Q_UNUSED(action);
  Q_UNUSED(idx);
  LOG(ERROR) << "Action failed, code=" << (error ? error->code() : 0)
             << ", message=" << (error ? error->message() : "");
}

void TaskCallbackImpl::OnStepFailed(std::shared_ptr<main_process::Task> task,
                                    main_process::StepPtr step,
                                    ExecutionErrorPtr error) {
  Q_UNUSED(task);
  LOG(ERROR) << "Step-" << (step ? step->ids() : "")
             << "_Failed, code=" << (error ? error->code() : 0)
             << ", message=" << (error ? error->message() : "");
}

void TaskCallbackImpl::OnTaskFailed(std::shared_ptr<main_process::Task> task,
                                    ExecutionErrorPtr error) {
  std::string task_ids = task ? task->ids() : "";
  int64_t error_code = error ? error->code() : -1;
  LOG(ERROR) << "Task-" << task_ids << "_Failed, code=" << error_code
             << ", message=" << (error ? error->message() : "");
  CompleteTaskWait(task_ids, error_code);
}

void TaskCallbackImpl::Quit(void) { quit_ = true; }

void TaskCallbackImpl::CompleteTaskWait(const std::string& task_ids,
                                        int64_t result) {
  using ResultWaitMgrSinglton = AsyncWaitManagerSinglton<int64_t, int64_t>;
  int64_t wait_key = static_cast<int64_t>(std::hash<std::string>{}(task_ids));
  ResultWaitMgrSinglton::GetInstance()->Complete(wait_key, result);
}
