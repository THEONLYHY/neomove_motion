// copyright 2025 YottaImage. All rights reserved.
#ifndef NEOMOVE_MOTION_MGR_H_
#define NEOMOVE_MOTION_MGR_H_

#include <base.h>
#include <common/thread_id_checker.h>
#include <error_info/error_codes.h>
#include <error_info/impl/execution_error_impl.h>
#include <motion/motion_mgr.h>
#include <singleton.h>

#include <iomanip>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <vector>

#include "neomove_analog_io.h"
#include "neomove_axis.h"
#include "neomove_event_control.h"
#include "neomove_input_io.h"
#include "neomove_io_monitor_thread.h"
#include "neomove_motion_control.h"
#include "neomove_motion_mgr_context_impl.h"
#include "neomove_output_io.h"

class NeoMoveMotionMgr : public yotta::MotionMgr {
 public:
  SINGLETON(NeoMoveMotionMgr);
  virtual ~NeoMoveMotionMgr();

  ExecutionErrorPtr GetError() const override {
    std::lock_guard<std::mutex> lock(error_mutex_);
    return last_error_;
  }

  int YOTTA_API_CALL Initialize(const char* path, size_t path_len,
                                const char* file, size_t file_len) override;
  int YOTTA_API_CALL Finalize() override;
  int YOTTA_API_CALL SetAxisMultiplier(double multiplier) override;
  double YOTTA_API_CALL GetAxisMultiplier(void) override {
    return axis_multiplier_;
  }

  yotta::Axis* YOTTA_API_CALL GetAxis(int id) override;
  yotta::InputIO* YOTTA_API_CALL GetInputIo(int addr, int bit) override;
  yotta::OutputIO* YOTTA_API_CALL GetOutputIo(int addr, int bit) override;
  yotta::MotionControl* YOTTA_API_CALL GetMotionControl(int major_ver,
                                                        int minor_ver) override;
  yotta::MotionMgrContext* YOTTA_API_CALL GetMotionMgrContext() override;
  yotta::EventControl* YOTTA_API_CALL GetEventControl() override;
  yotta::AnalogIO* YOTTA_API_CALL GetAnalogIo(int addr) override;

  int YOTTA_API_CALL RegisterService(const char* service_name,
                                     void* service) override;
  int YOTTA_API_CALL QueryService(const char* service_name,
                                  void** service) override;

 private:
  NeoMoveMotionMgr();
  void ClearResources();

  bool initialized_ = false;
  double axis_multiplier_ = 1.0;

  std::unique_ptr<NeoMoveMotionControl> motion_control_;
  std::unique_ptr<NeoMoveEventControl> event_control_;
  std::map<int, std::unique_ptr<NeoMoveAxis>> axes_;
  std::map<int, std::unique_ptr<NeoMoveAnalogIo>> analog_ios_;
  std::map<std::pair<int, int>, std::unique_ptr<NeoMoveInputIo>> input_ios_;
  std::map<std::pair<int, int>, std::unique_ptr<NeoMoveOutputIo>> ouput_ios_;

  std::map<std::string, void*> services_;

  std::shared_ptr<NeoMoveIoMonitorThread> io_monitor_thread_;
  std::shared_mutex mutex_;

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
    last_error_->WithMessage(message).WithEntity("neomove_motion_mgr");
    if (!custom_data.empty()) {
      last_error_->WithCustomData(custom_data);
    }
  }

  static std::string ToHex(int value) {
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase << value;
    return oss.str();
  }

  friend double GetNeoMoveAxisMultiplier();
};

using NeoMoveMotionMgrSingleton = yotta::Singleton<NeoMoveMotionMgr>;

double GetNeoMoveAxisMultiplier();

#endif  // NEOMOVE_MOTION_MGR_H_
