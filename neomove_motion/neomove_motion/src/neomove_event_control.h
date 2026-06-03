// copyright 2025 YottaImage. All rights reserved.
#ifndef NEOMOVE_EVENT_CONTROL_H_
#define NEOMOVE_EVENT_CONTROL_H_

#include <common/thread.h>
#include <error_info/error_codes.h>
#include <error_info/impl/execution_error_impl.h>
#include <motion/event_control.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <sstream>
#include <vector>

#include "NeoMove CPlusPlus.h"

class NeoMoveEventControl : public yotta::EventControl {
 public:
  NeoMoveEventControl();
  virtual ~NeoMoveEventControl();

  ExecutionErrorPtr GetError() const override {
    std::lock_guard<std::mutex> lock(error_mutex_);
    return last_error_;
  }

  int YOTTA_API_CALL SyncTriggerCameraByEqualStep(
      yotta::Axis* axis, double offset, double step, int grab_count,
      yotta::OutputIO* output_io[], size_t io_count,
      yotta::AccDecProfile* profile) override;

  int YOTTA_API_CALL SyncSoftwareTouchProbe(
      yotta::Axis* axis, double offset, double step, int touch_count,
      yotta::InputIO* input_io[], size_t io_count,
      yotta::AccDecProfile* profile) override;

  int YOTTA_API_CALL GetTouchProbleCounterValues(
      double** counter_values, size_t* count) override;

  int YOTTA_API_CALL FreeTouchProbleCounterValues(
      double** counter_values) override;

  void* YOTTA_API_CALL QueryInterface(const char* interface_name,
                                      size_t length) override;

 private:
  int GetControllerIndex();
  double GetMultiplier();

  mutable ExecutionErrorPtr last_error_;
  mutable std::mutex error_mutex_;

  std::mutex touch_probe_mutex_;
  std::vector<std::vector<double>> touch_probe_data_;
  std::atomic<bool> probe_motion_finished_{false};

  void ClearError() const {
    std::lock_guard<std::mutex> lock(error_mutex_);
    last_error_ = nullptr;
  }

  void SetError(int32_t code, const std::string& message,
                const std::string& custom_data = "") const {
    std::lock_guard<std::mutex> lock(error_mutex_);
    last_error_ = ExecutionError::Create(code, "Motion");
    last_error_->WithMessage(message).WithEntity("neomove_event_control");
    if (!custom_data.empty()) {
      last_error_->WithCustomData(custom_data);
    }
  }

  static std::string ToHex(int value) {
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase << value;
    return oss.str();
  }
};

#endif  // NEOMOVE_EVENT_CONTROL_H_
