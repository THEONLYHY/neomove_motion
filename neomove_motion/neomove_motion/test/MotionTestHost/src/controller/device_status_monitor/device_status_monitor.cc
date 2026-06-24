#include "device_status_monitor.h"

#include <glog/glog_helper.h>

#include <thread>
using namespace std::chrono;

#include <config/config_factory.h>
#include <limit_motion/limit_motion_mgr.h>
#include <main_process/module_mgr.h>

DeviceStatusMonitor::DeviceStatusMonitor() {}

DeviceStatusMonitor::~DeviceStatusMonitor() { Stop(); }

void DeviceStatusMonitor::Start() {
  if (is_running_) return;

  if (!initialized_) {
    Init();
  }

  is_running_ = true;
  monitoring_thread_.Run(&DeviceStatusMonitor::MonitoringLoop, this);
  LOG(INFO) << "DeviceStatusMonitor started";
}

void DeviceStatusMonitor::Stop() {
  is_running_ = false;
  monitoring_thread_.Stop();
  LOG(INFO) << "DeviceStatusMonitor stopped";
}

void DeviceStatusMonitor::RefreshDeviceList() {
  // Temporarily stop monitoring to avoid race conditions
  bool was_running = is_running_;
  if (was_running) {
    Stop();
  }

  // Clear existing device lists and cache
  {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    analog_ios_.clear();
    input_ios_.clear();
    output_ios_.clear();
    axes_.clear();
    analog_io_cache_.clear();
    input_io_cache_.clear();
    output_io_cache_.clear();
    axis_state_cache_.clear();
    axis_home_state_cache_.clear();
    axis_pos_cache_.clear();
  }

  // Re-initialize device lists
  initialized_ = false;
  Init();

  // Restart monitoring if it was running
  if (was_running) {
    Start();
  }

  LOG(INFO) << "DeviceStatusMonitor device list refreshed";
}

void DeviceStatusMonitor::Init() {
  yotta::LimitMotionMgrPtr limit_motion =
      main_process::GetModuleMgr()->GetLimitMotionMgr();
  if (!limit_motion) {
    LOG(ERROR) << "Failed to get LimitMotionMgr";
    return;
  }

  auto io_config = yotta::ConfigFactory::GetInstance()->GetIoConfig();
  auto axis_config = yotta::ConfigFactory::GetInstance()->GetAxisConfig();

  // Load Analog IOs
  if (io_config) {
    auto analog_io_config = io_config->GetAnalogIo();
    if (analog_io_config) {
      for (int i = 0; i < analog_io_config->GetAnalogIoPortConfigCount(); ++i) {
        auto port = analog_io_config->GetAnalogIoPortConfig(i);
        char id_buf[256] = {0};
        port->GetIds(id_buf, 256);
        std::string id = id_buf;
        yotta::AnalogIO* analog_io = limit_motion->GetAnalogIo(id.c_str());
        if (analog_io) {
          analog_ios_.push_back({id, analog_io});
        }
      }
    }
  }
  
  // Load Input IOs
  if (io_config) {
    auto input_config = io_config->GetInputIo();
    if (input_config) {
      for (int i = 0; i < input_config->GetIoPortConfigCount(); ++i) {
        auto port = input_config->GetIoPortConfig(i);
        char id_buf[256] = {0};
        port->GetIds(id_buf, 256);
        std::string id = id_buf;

        yotta::InputIO* io = limit_motion->GetInputIoByIds(id.c_str());
        if (io) {
          input_ios_.push_back({id, io});
        }
      }
    }

    auto axis_io_count = io_config->GetAxisIoCount();
    for (int i = 0; i < axis_io_count; ++i) {
      auto axis_io_cfg = io_config->GetAxisIo(i);
      for (int j = 0; j < axis_io_cfg->GetIoPortConfigCount(); ++j) {
        auto port = axis_io_cfg->GetIoPortConfig(j);
        char id_buf[256] = {0};
        port->GetIds(id_buf, 256);
        std::string id = id_buf;

        yotta::InputIO* io = limit_motion->GetInputIoByIds(id.c_str());
        if (io) {
          input_ios_.push_back({id, io});
        }
      }
    }
  }

  // Load Output IOs
  if (io_config) {
    auto output_config = io_config->GetOutputIo();
    if (output_config) {
      for (int i = 0; i < output_config->GetIoPortConfigCount(); ++i) {
        auto port = output_config->GetIoPortConfig(i);
        char id_buf[256] = {0};
        port->GetIds(id_buf, 256);
        std::string id = id_buf;

        yotta::OutputIO* io = limit_motion->GetOutputIoByIds(id.c_str());
        if (io) {
          output_ios_.push_back({id, io});
        }
      }
    }
  }

  // Load Axes
  if (axis_config) {
    for (int i = 0; i < axis_config->GetAxisCount(); ++i) {
      auto axis_item = axis_config->GetAxis(i);
      char id_buf[256] = {0};
      axis_item->GetIds(id_buf, 256);
      std::string id = id_buf;

      yotta::Axis* axis = limit_motion->GetAxisByIds(id.c_str());
      if (axis) {
        axes_.push_back({id, axis});
      }
    }
  }

  initialized_ = true;
  LOG(INFO) << "DeviceStatusMonitor initialized. Monitoring "
            << input_ios_.size() << " inputs, " << output_ios_.size()
            << " outputs, " << axes_.size() << " axes.";
}

void DeviceStatusMonitor::MonitoringLoop() {
  // Pre-allocate vectors to avoid reallocation in loop
  std::vector<std::pair<std::string, double>> temp_analog_values;
  std::vector<std::pair<std::string, uint8_t>> temp_input_values;
  std::vector<std::pair<std::string, uint8_t>> temp_output_values;
  std::vector<std::pair<std::string, int>> temp_axis_states;
  std::vector<std::pair<std::string, int>> temp_axis_home_states;
  std::vector<std::pair<std::string, double>> temp_axis_positions;

  temp_analog_values.reserve(analog_ios_.size());
  temp_input_values.reserve(input_ios_.size());
  temp_output_values.reserve(output_ios_.size());
  temp_axis_states.reserve(axes_.size());
  temp_axis_home_states.reserve(axes_.size());
  temp_axis_positions.reserve(axes_.size());

  while (is_running_) {
    // Clear temp vectors (keeps capacity)
    temp_analog_values.clear();
    temp_input_values.clear();
    temp_output_values.clear();
    temp_axis_states.clear();
    temp_axis_home_states.clear();
    temp_axis_positions.clear();

    // Poll Analog
    for (const auto& item : analog_ios_) {
      short val = 0;
      if (item.ptr->GetInShort(&val) == 0) {
        temp_analog_values.emplace_back(item.id, val);
      }
    }

    // Poll Inputs
    for (const auto& item : input_ios_) {
      unsigned char val = 0;
      if (item.ptr->ReadValue(&val) == 0) {
        temp_input_values.emplace_back(item.id, val);
      }
    }

    // Poll Outputs
    for (const auto& item : output_ios_) {
      unsigned char val = 0;
      if (item.ptr->ReadValue(&val) == 0) {
        temp_output_values.emplace_back(item.id, val);
      }
    }

    // Poll Axes
    for (const auto& item : axes_) {
      yotta::Axis::AxisState state;
      if (item.ptr->state(&state) == 0) {
        temp_axis_states.emplace_back(item.id, static_cast<int>(state));
      }

      yotta::Axis::AxisHomeState home_state;
      if (item.ptr->home_state(&home_state) == 0) {
        temp_axis_home_states.emplace_back(item.id,
                                           static_cast<int>(home_state));
      }

      double pos = 0;
      if (item.ptr->GetActualPosition(&pos) == 0) {
        temp_axis_positions.emplace_back(item.id, pos);
      }
    }

    // Update Cache
    {
      std::unique_lock<std::shared_mutex> lock(mutex_);
      for (const auto& val : temp_analog_values)
        analog_io_cache_[val.first] = val.second;
      for (const auto& val : temp_input_values)
        input_io_cache_[val.first] = val.second;
      for (const auto& val : temp_output_values)
        output_io_cache_[val.first] = val.second;
      for (const auto& val : temp_axis_states)
        axis_state_cache_[val.first] = val.second;
      for (const auto& val : temp_axis_home_states)
        axis_home_state_cache_[val.first] = val.second;
      for (const auto& val : temp_axis_positions)
        axis_pos_cache_[val.first] = val.second;
    }

    std::this_thread::sleep_for(milliseconds(monitoring_gap_));
  }
}

bool DeviceStatusMonitor::GetInputIoValue(const std::string& id,
                                          uint8_t& value) {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  auto it = input_io_cache_.find(id);
  if (it != input_io_cache_.end()) {
    value = it->second;
    return true;
  }
  return false;
}

bool DeviceStatusMonitor::GetAnalogIoValue(const std::string& id,
                                           double& value) {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  auto it = analog_io_cache_.find(id);
  if (it != analog_io_cache_.end()) {
    value = it->second;
    return true;
  }
  return false;
}

bool DeviceStatusMonitor::GetOutputIoValue(const std::string& id,
                                           uint8_t& value) {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  auto it = output_io_cache_.find(id);
  if (it != output_io_cache_.end()) {
    value = it->second;
    return true;
  }
  return false;
}

bool DeviceStatusMonitor::GetAxisState(const std::string& id, int& state) {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  auto it = axis_state_cache_.find(id);
  if (it != axis_state_cache_.end()) {
    state = it->second;
    return true;
  }
  return false;
}

bool DeviceStatusMonitor::GetAxisHomeState(const std::string& id, int& state) {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  auto it = axis_home_state_cache_.find(id);
  if (it != axis_home_state_cache_.end()) {
    state = it->second;
    return true;
  }
  return false;
}

bool DeviceStatusMonitor::GetAxisPos(const std::string& id, double& pos) {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  auto it = axis_pos_cache_.find(id);
  if (it != axis_pos_cache_.end()) {
    pos = it->second;
    return true;
  }
  return false;
}
