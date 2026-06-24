#ifndef COUPLING_MACHINE_SRC_CONTROLLER_DEVICE_STATUS_MONITOR_DEVICE_STATUS_MONITOR_H_
#define COUPLING_MACHINE_SRC_CONTROLLER_DEVICE_STATUS_MONITOR_DEVICE_STATUS_MONITOR_H_

#include <common/thread.h>
#include <singleton.h>
#include <limit_motion/limit_axis.h>
#include <limit_motion/limit_io.h>
#include <limit_motion/limit_analog_io.h>

#include <atomic>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

// Forward declarations
namespace yotta {
class AnalogIO;
class InputIO;
class OutputIO;
class Axis;
}  // namespace yotta

class DeviceStatusMonitor {
  SINGLETON(DeviceStatusMonitor);

 public:
  DeviceStatusMonitor();
  ~DeviceStatusMonitor();

  void Start();
  void Stop();

  // Thread-safe getters
  // Returns true if ID found, false otherwise
  bool GetAnalogIoValue(const std::string& id, double& value);
  bool GetInputIoValue(const std::string& id, uint8_t& value);
  bool GetOutputIoValue(const std::string& id, uint8_t& value);
  bool GetAxisState(const std::string& id, int& state);  // state casted to int
  bool GetAxisPos(const std::string& id, double& pos);
  bool GetAxisHomeState(const std::string& id,
                        int& state);  // state casted to int

  // Refresh device list when IO/Axis configuration changes
  void RefreshDeviceList();

 private:
  void Init();
  void MonitoringLoop();

 private:
  std::atomic<bool> is_running_{false};
  common::Thread monitoring_thread_;
  int monitoring_gap_ = 200;  //纯粹刷新UI，应该够了

  mutable std::shared_mutex mutex_;  // Read-write lock for better concurrency

  // Cache
  std::map<std::string, double> analog_io_cache_;
  std::map<std::string, uint8_t> input_io_cache_;
  std::map<std::string, uint8_t> output_io_cache_;
  std::map<std::string, int> axis_state_cache_;
  std::map<std::string, int> axis_home_state_cache_;
  std::map<std::string, double> axis_pos_cache_;

  // Hardware pointers (cached for performance)
  struct InputIoInfo {
    std::string id;
    yotta::InputIO* ptr = nullptr;
  };
  struct AnalogIoInfo {
    std::string id;
    yotta::AnalogIO* ptr = nullptr;
  };
  struct OutputIoInfo {
    std::string id;
    yotta::OutputIO* ptr = nullptr;
  };
  struct AxisInfo {
    std::string id;
    yotta::Axis* ptr = nullptr;
  };

  std::vector<AnalogIoInfo> analog_ios_;
  std::vector<InputIoInfo> input_ios_;
  std::vector<OutputIoInfo> output_ios_;
  std::vector<AxisInfo> axes_;

  bool initialized_ = false;
};

using DeviceStatusMonitorSinglton = yotta::Singleton<DeviceStatusMonitor>;

#endif  // COUPLING_MACHINE_SRC_CONTROLLER_DEVICE_STATUS_MONITOR_DEVICE_STATUS_MONITOR_H_
