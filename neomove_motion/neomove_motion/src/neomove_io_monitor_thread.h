// copyright 2025 YottaImage. All rights reserved.
#ifndef NEOMOVE_IO_MONITOR_THREAD_H_
#define NEOMOVE_IO_MONITOR_THREAD_H_

#include <base.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "neomove_axis.h"
#include "neomove_motion_mgr_context_impl.h"
#include "neomove_output_io.h"

class NeoMoveIoMonitorThread {
 public:
  using IoKey = std::pair<int, int>;
  using IoWatcherEntry = std::pair<yotta::IO::IOWatcher*, yotta::IO*>;
  using AxisPtr = NeoMoveAxis*;
  using AxisWatcherEntry = std::pair<yotta::Axis::Watcher*, AxisPtr>;

  NeoMoveIoMonitorThread();
  ~NeoMoveIoMonitorThread();

  void RegisterInputIo(int addr, int bit, yotta::IO::IOWatcher* watcher,
                       yotta::IO* io_obj);
  void UnregisterInputIo(int addr, int bit);
  void RegisterOutputIo(int addr, int bit, yotta::IO::IOWatcher* watcher,
                        yotta::IO* io_obj);
  void UnregisterOutputIo(int addr, int bit);

  void RegisterAxis(int axis_id, yotta::Axis::Watcher* watcher,
                    AxisPtr axis_obj);
  void UnregisterAxis(int axis_id);

  void Start();
  void Stop();

 private:
  void MonitorLoop();
  void MonitorInputIo();
  void MonitorOutputIo();
  void MonitorAxis();

  int GetControllerIndex();

  std::thread thread_;
  std::atomic<bool> running_{false};

  std::mutex input_io_mutex_;
  std::map<IoKey, IoWatcherEntry> input_io_watchers_;
  std::map<IoKey, unsigned char> last_input_io_state_;

  std::mutex output_io_mutex_;
  std::map<IoKey, IoWatcherEntry> output_io_watchers_;
  std::map<IoKey, unsigned char> last_output_io_state_;

  std::mutex axis_mutex_;
  std::map<int, AxisWatcherEntry> axis_watchers_;
  std::map<int, bool> last_axis_alarm_state_;
  std::map<int, int> last_axis_op_state_;
};

#endif  // NEOMOVE_IO_MONITOR_THREAD_H_
