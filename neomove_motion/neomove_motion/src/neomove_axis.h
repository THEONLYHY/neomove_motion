// copyright 2025 YottaImage. All rights reserved.
#ifndef NEOMOVE_AXIS_H_
#define NEOMOVE_AXIS_H_

#include <error_info/error_codes.h>
#include <error_info/impl/execution_error_impl.h>
#include <motion/acc_dec_profile.h>
#include <motion/axis.h>
#include <motion/trigger.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

#include "NeoMove CPlusPlus.h"

class NeoMoveIoMonitorThread;

class NeoMoveAxis : public yotta::Axis {
public:
  explicit NeoMoveAxis(int axis_index);
  virtual ~NeoMoveAxis() = default;

  ExecutionErrorPtr GetError() const override {
    std::lock_guard<std::mutex> lock(error_mutex_);
    return last_error_;
  }
  // 状态
  int YOTTA_API_CALL id() override;
  int YOTTA_API_CALL name(char* name_buf, size_t name_buf_size) override;
  int YOTTA_API_CALL SetAxisWatcher(Watcher* watcher) override;
  int YOTTA_API_CALL state(AxisState* state) override;
  int YOTTA_API_CALL home_state(AxisHomeState* state) override;
  int YOTTA_API_CALL operation_state(AxisOperationState* operation_state) override;
  // 伺服和报警
  int YOTTA_API_CALL SetServoOn() override;
  int YOTTA_API_CALL SetServoOff() override;
  int YOTTA_API_CALL ClearAmpAlarm() override;
  int YOTTA_API_CALL ClearAxisAlarm() override;
  // 总线轴模式
  int YOTTA_API_CALL SetAxisCommandMode(AxisCommandMode mode) override;
  int YOTTA_API_CALL GetAxisCommandMode(AxisCommandMode* mode) override;
  // 运动
  int YOTTA_API_CALL Home() override;
  int YOTTA_API_CALL GetActualPosition(double* position) override;
  int YOTTA_API_CALL GetActualVelocity(double* velocity) override;
  int YOTTA_API_CALL GetTargetPosition(double* target_position) override;
  int YOTTA_API_CALL AsyncMoveTo(double dest_pos,
                                yotta::AccDecProfile* profile) override;
  int YOTTA_API_CALL Wait() override;
  int YOTTA_API_CALL StartJog(yotta::AccDecProfile* profile,
                              bool positive) override;
  int YOTTA_API_CALL StartTriggerPos(double dest_pos, yotta::Trigger* trigger,
                                    yotta::AccDecProfile* profile) override;
  void YOTTA_API_CALL Pause() override;
  void YOTTA_API_CALL Resume() override;
  void YOTTA_API_CALL Stop() override;
  void YOTTA_API_CALL QuickStop() override;
  void YOTTA_API_CALL TimedStop(double time_milliseconds) override;

  void YOTTA_API_CALL DecelerationStop(double deceleration) override;
  void* YOTTA_API_CALL QueryInterface(const char* interface_name,
                                      size_t length) override;

  void SetIoMonitorThread(std::weak_ptr<NeoMoveIoMonitorThread> monitor);

private:
    // 获取控制器编号
  int GetControllerIndex();
  // 轴单位倍率
  double GetMultiplier();

  void ApplyPitchCompensation();
  void DisablePitchCompensation();

  int axis_index_; // 轴号
  AxisCommandMode current_mode_ = AxisCommandMode::kPosition; // 当前缓存模式
  std::atomic<int> last_move_direction_{0}; // +1=正向, -1=负向, 0=未知
  Watcher* axis_watcher_ = nullptr; // 轴状态回调
  std::weak_ptr<NeoMoveIoMonitorThread> io_monitor_thread_; // 监控线程弱引用

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
    last_error_->WithMessage(message)
        .WithEntity("neomove_axis_" + std::to_string(axis_index_));
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

#endif  // NEOMOVE_AXIS_H_
