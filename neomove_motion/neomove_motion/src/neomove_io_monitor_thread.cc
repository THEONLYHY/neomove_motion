// copyright 2025 YottaImage. All rights reserved.
#include "neomove_io_monitor_thread.h"

#include <glog/glog_helper.h>

#include <chrono>
#include <cstring>

#include "neomove_pdo_utils.h"
#include "neomove_sdk_guard.h"

namespace {

constexpr int kMonitorPollIntervalMs = 20;

}  // namespace

NeoMoveIoMonitorThread::NeoMoveIoMonitorThread() {}

NeoMoveIoMonitorThread::~NeoMoveIoMonitorThread() { Stop(); }

int NeoMoveIoMonitorThread::GetControllerIndex() {
  return NeoMoveMotionMgrContextImpl::GetCurrent().controller_index();
}

void NeoMoveIoMonitorThread::Start() {
  LOG(INFO) << "NeoMoveIoMonitorThread::Start called";
  if (running_) return;
  running_ = true;
  thread_ = std::thread(&NeoMoveIoMonitorThread::MonitorLoop, this);
}

void NeoMoveIoMonitorThread::Stop() {
  running_ = false;
  if (thread_.joinable()) {
    thread_.join();
  }
  LOG(INFO) << "NeoMoveIoMonitorThread stopped.";
}

void NeoMoveIoMonitorThread::RegisterAxis(int axis_id,
                                           yotta::Axis::Watcher* watcher,
                                           AxisPtr axis_obj) {
  LOG(INFO) << "Register Axis watcher: axis_id=" << axis_id;
  std::lock_guard<std::mutex> lock(axis_mutex_);
  axis_watchers_[axis_id] = {watcher, axis_obj};
  last_axis_alarm_state_.erase(axis_id);
  last_axis_op_state_.erase(axis_id);
}

void NeoMoveIoMonitorThread::UnregisterAxis(int axis_id) {
  LOG(INFO) << "Unregister Axis watcher: axis_id=" << axis_id;
  std::lock_guard<std::mutex> lock(axis_mutex_);
  axis_watchers_.erase(axis_id);
  last_axis_alarm_state_.erase(axis_id);
  last_axis_op_state_.erase(axis_id);
}

void NeoMoveIoMonitorThread::RegisterInputIo(int addr, int bit,
                                              yotta::IO::IOWatcher* watcher,
                                              yotta::IO* io_obj) {
  LOG(INFO) << "Register InputIO watcher: addr=" << addr << ", bit=" << bit;
  std::lock_guard<std::mutex> lock(input_io_mutex_);
  input_io_watchers_[{addr, bit}] = {watcher, io_obj};
  last_input_io_state_.erase({addr, bit});
}

void NeoMoveIoMonitorThread::UnregisterInputIo(int addr, int bit) {
  LOG(INFO) << "Unregister InputIO watcher: addr=" << addr << ", bit=" << bit;
  std::lock_guard<std::mutex> lock(input_io_mutex_);
  input_io_watchers_.erase({addr, bit});
  last_input_io_state_.erase({addr, bit});
}

void NeoMoveIoMonitorThread::RegisterOutputIo(int addr, int bit,
                                               yotta::IO::IOWatcher* watcher,
                                               yotta::IO* io_obj) {
  LOG(INFO) << "Register OutputIO watcher: addr=" << addr << ", bit=" << bit;
  std::lock_guard<std::mutex> lock(output_io_mutex_);
  output_io_watchers_[{addr, bit}] = {watcher, io_obj};
  last_output_io_state_.erase({addr, bit});
}

void NeoMoveIoMonitorThread::UnregisterOutputIo(int addr, int bit) {
  LOG(INFO) << "Unregister OutputIO watcher: addr=" << addr << ", bit=" << bit;
  std::lock_guard<std::mutex> lock(output_io_mutex_);
  output_io_watchers_.erase({addr, bit});
  last_output_io_state_.erase({addr, bit});
}

void NeoMoveIoMonitorThread::MonitorLoop() {
  SetThreadDescription(GetCurrentThread(), L"NeoMoveMonitor");
  LOG(INFO) << "NeoMoveIoMonitorThread::MonitorLoop started.";
  while (running_) {
    MonitorInputIo();
    MonitorOutputIo();
    MonitorAxis();
    // Polling observes IO/axis state only. It exits when Stop() clears
    // running_, and uses a low frequency to avoid starving controller commands.
    std::this_thread::sleep_for(
        std::chrono::milliseconds(kMonitorPollIntervalMs));
  }
  LOG(INFO) << "NeoMoveIoMonitorThread::MonitorLoop exited.";
}

void NeoMoveIoMonitorThread::MonitorInputIo() {
  std::lock_guard<std::mutex> lock(input_io_mutex_);
  for (auto& pair : input_io_watchers_) {
    const auto& key = pair.first;
    auto& entry = pair.second;
    int addr = key.first;
    int bit = key.second;

    unsigned int uIndex = (addr >> 16) & 0xFFFF;
    unsigned int gIndex = addr & 0xFFFF;
    unsigned short pdo_val = 0;

    int ret = neomove_pdo::ReadWord(GetControllerIndex(), uIndex, gIndex,
                                    &pdo_val);
    if (ret != NM_RETURN_OK) continue;

    unsigned char new_state = 0;
    if (bit >= 0) {
      new_state = (pdo_val & (1 << bit)) ? 1 : 0;
    } else {
      new_state = static_cast<unsigned char>(pdo_val & 0xFF);
    }

    auto it = last_input_io_state_.find(key);
    if (it != last_input_io_state_.end()) {
      if (new_state != it->second) {
        last_input_io_state_[key] = new_state;
        if (entry.first) {
          entry.first->OnValueChanged(addr, bit, new_state);
        }
      }
    } else {
      last_input_io_state_[key] = new_state;
      if (entry.first) {
        entry.first->OnValueChanged(addr, bit, new_state);
      }
    }
  }
}

void NeoMoveIoMonitorThread::MonitorOutputIo() {
  std::lock_guard<std::mutex> lock(output_io_mutex_);
  for (auto& pair : output_io_watchers_) {
    const auto& key = pair.first;
    auto& entry = pair.second;
    int addr = key.first;
    int bit = key.second;

    unsigned int uIndex = (addr >> 16) & 0xFFFF;
    unsigned int gIndex = addr & 0xFFFF;
    unsigned short pdo_val = 0;

    int ret = neomove_pdo::ReadWord(GetControllerIndex(), uIndex, gIndex,
                                    &pdo_val);
    if (ret != NM_RETURN_OK) continue;

    unsigned char new_state = 0;
    if (bit >= 0) {
      new_state = (pdo_val & (1 << bit)) ? 1 : 0;
    } else {
      new_state = static_cast<unsigned char>(pdo_val & 0xFF);
    }

    auto it = last_output_io_state_.find(key);
    if (it != last_output_io_state_.end()) {
      if (new_state != it->second) {
        last_output_io_state_[key] = new_state;
        if (entry.first) {
          entry.first->OnValueChanged(addr, bit, new_state);
        }
      }
    } else {
      last_output_io_state_[key] = new_state;
      if (entry.first) {
        entry.first->OnValueChanged(addr, bit, new_state);
      }
    }
  }
}

void NeoMoveIoMonitorThread::MonitorAxis() {
  std::lock_guard<std::mutex> lock(axis_mutex_);

  for (auto& pair : axis_watchers_) {
    int axis_id = pair.first;
    auto& entry = pair.second;

    NM_AXISSTATUS axis_status{};
    int ret = neomove_sdk_guard::Call([&]() {
      return NM_GetAxisStatus(GetControllerIndex(), axis_id, &axis_status);
    });
    if (ret != NM_RETURN_OK) continue;

    // 更新轴对象内部缓存
    if (entry.second) {
      entry.second->UpdateCachedStatus(axis_status);
    }

    // 通知 watcher（如果有）
    bool new_alarm = axis_status.ampAlarm != 0;
    auto alarm_it = last_axis_alarm_state_.find(axis_id);
    if (alarm_it != last_axis_alarm_state_.end()) {
      if (new_alarm != alarm_it->second && entry.first) {
        entry.first->OnAlarm(axis_id, new_alarm,
                             static_cast<int>(axis_status.ampAlarmCode));
      }
    } else {
      if (entry.first) {
        entry.first->OnAlarm(axis_id, new_alarm,
                             static_cast<int>(axis_status.ampAlarmCode));
      }
    }
    last_axis_alarm_state_[axis_id] = new_alarm;

    int new_op_state = static_cast<int>(yotta::Axis::AxisOperationState::kIdle);
    if (axis_status.homing) {
      new_op_state = static_cast<int>(yotta::Axis::AxisOperationState::kHome);
    } else if (!axis_status.motionComplete) {
      new_op_state = static_cast<int>(yotta::Axis::AxisOperationState::kPos);
    }

    auto op_it = last_axis_op_state_.find(axis_id);
    if (op_it != last_axis_op_state_.end()) {
      if (new_op_state != op_it->second && entry.first) {
        entry.first->OnAxisOperationState(axis_id, new_op_state);
      }
    } else {
      if (entry.first) {
        entry.first->OnAxisOperationState(axis_id, new_op_state);
      }
    }
    last_axis_op_state_[axis_id] = new_op_state;
  }
}
