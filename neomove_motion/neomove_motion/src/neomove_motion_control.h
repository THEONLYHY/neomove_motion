// copyright 2025 YottaImage. All rights reserved.
#ifndef NEOMOVE_MOTION_CONTROL_H_
#define NEOMOVE_MOTION_CONTROL_H_

#include <common/thread_safe_map.h>
#include <error_info/error_codes.h>
#include <error_info/impl/execution_error_impl.h>
#include <motion/acc_dec_profile.h>
#include <motion/axis.h>
#include <motion/motion_control.h>
#include <motion/motion_mode.h>

#include <mutex>
#include <sstream>
#include <string>

#include "NeoMove CPlusPlus.h"

class NeoMoveMotionControl : public yotta::MotionControl {
 public:
  NeoMoveMotionControl();
  virtual ~NeoMoveMotionControl();

  ExecutionErrorPtr GetError() const override {
    std::lock_guard<std::mutex> lock(error_mutex_);
    return last_error_;
  }

  int YOTTA_API_CALL RegisterMotionMode(yotta::MotionMode* mode) override;
  int YOTTA_API_CALL UnregisterMotionMode(const char* name) override;
  int YOTTA_API_CALL UnregisterMotionMode(yotta::MotionMode* mode) override;
  yotta::MotionMode* YOTTA_API_CALL GetMotionMode(const char* name) override;

  int YOTTA_API_CALL AsyncLinearIntplPos(
      yotta::Axis* axes[], double dest_pos[],
      yotta::AccDecProfile* profiles[], size_t array_count) override;

 private:
  int GetControllerIndex();

  ThreadSafeMap<std::string, yotta::MotionMode*> motion_modes_;

  mutable ExecutionErrorPtr last_error_;
  mutable std::mutex error_mutex_;

  void ClearError() const {
    std::lock_guard<std::mutex> lock(error_mutex_);
    last_error_ = nullptr;
  }

  void SetError(int32_t code, const std::string& message,
                const std::string& custom_data = "") const {
    std::lock_guard<std::mutex> lock(error_mutex_);
    last_error_ = ExecutionError::Create(code, "Motion");
    last_error_->WithMessage(message).WithEntity("neomove_motion_control");
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

#endif  // NEOMOVE_MOTION_CONTROL_H_
