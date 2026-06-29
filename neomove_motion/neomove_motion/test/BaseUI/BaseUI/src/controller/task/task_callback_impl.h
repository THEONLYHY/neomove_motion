// copyright 2025 YottaImage. All rights reserved.
// author panming
// date 2025/09/04 15:30
#ifndef BASE_UI_SRC_CONTROLLER_TASK_TASK_CALLBACK_IMPL_H_
#define BASE_UI_SRC_CONTROLLER_TASK_TASK_CALLBACK_IMPL_H_

#include <main_process/step.h>
#include <main_process/task.h>
#include <main_process/task_callback.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

class TaskCallbackImpl : public main_process::TaskCallback,
                         public std::enable_shared_from_this<TaskCallbackImpl> {
 public:
  bool OnTaskStart(std::shared_ptr<main_process::Task> task) override;
  bool OnStepStart(std::shared_ptr<main_process::Task> task,
                   main_process::StepPtr step) override;
  bool OnActionStart(std::shared_ptr<main_process::Task> task,
                     main_process::StepPtr step,
                     action::ActionPtr action_to_run, size_t idx) override;
  bool OnActionEnd(std::shared_ptr<main_process::Task> task,
                   main_process::StepPtr step, action::ActionPtr action_run_end,
                   size_t idx) override;
  bool OnStepEnd(std::shared_ptr<main_process::Task> task,
                 main_process::StepPtr step) override;
  void OnTaskEnd(std::shared_ptr<main_process::Task> task) override;

  void OnActionFailed(std::shared_ptr<main_process::Task> task,
                      main_process::StepPtr step,
                      action::ActionPtr action, size_t idx,
                      ExecutionErrorPtr error) override;
  void OnStepFailed(std::shared_ptr<main_process::Task> task,
                    main_process::StepPtr step,
                    ExecutionErrorPtr error) override;
  void OnTaskFailed(std::shared_ptr<main_process::Task> task,
                    ExecutionErrorPtr error) override;

  void Quit(void);
  void SetModuleIds(const std::string& module_ids) { module_ids_ = module_ids; }

 private:
  void CompleteTaskWait(const std::string& task_ids, int64_t result);

  std::atomic_bool quit_ = false;
  std::string module_ids_;
};

using TaskCallbackImplPtr = std::shared_ptr<TaskCallbackImpl>;
using TaskCallbackImplWeakPtr = std::weak_ptr<TaskCallbackImpl>;

#endif  // BASE_UI_SRC_CONTROLLER_TASK_TASK_CALLBACK_IMPL_H_
