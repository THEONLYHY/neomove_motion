// copyright 2025 YottaImage. All rights reserved.
#include "neomove_event_control.h"

#include <glog/glog_helper.h>

#include <thread>

#include "neomove_motion_mgr_context_impl.h"
#include "neomove_sdk_guard.h"

using namespace yotta;

NeoMoveEventControl::NeoMoveEventControl() {
  LOG(INFO) << "NeoMoveEventControl constructed.";
}

NeoMoveEventControl::~NeoMoveEventControl() {
  LOG(INFO) << "NeoMoveEventControl destructed.";
}

int NeoMoveEventControl::GetControllerIndex() {
  return NeoMoveMotionMgrContextImpl::GetCurrent().controller_index();
}

double NeoMoveEventControl::GetMultiplier() {
  return NeoMoveMotionMgrContextImpl::GetCurrent().GetAxisMultiplier();
}

void* NeoMoveEventControl::QueryInterface(const char* interface_name,
                                           size_t length) {
  return nullptr;
}

int YOTTA_API_CALL NeoMoveEventControl::SyncTriggerCameraByEqualStep(
    Axis* axis, double offset, double step, int grab_count,
    OutputIO* output_io[], size_t io_count, AccDecProfile* profile) {
  ClearError();
  std::lock_guard<std::mutex> lock(touch_probe_mutex_);

  if (!axis || !profile || grab_count <= 0) {
    SetError(MotionErrors::ParamInvalid,
             "SyncTriggerCameraByEqualStep 参数无效");
    return 1;
  }

  LOG(INFO) << "SyncTriggerCameraByEqualStep: axis=" << axis->id()
            << ", offset=" << offset << ", step=" << step
            << ", grab_count=" << grab_count;

  double start_pos = 0;
  int ret = axis->GetActualPosition(&start_pos);
  if (ret != 0) {
    SetError(MotionErrors::MoveFailed, "获取起始位置失败");
    return ret;
  }

  double scan_start = start_pos - grab_count * step / 2.0 + offset;
  double scan_end = start_pos + grab_count * step / 2.0 + offset;

  // Use NeoMove line compare for equal-step trigger
  // The line compare generates trigger pulses at equal distance intervals
  for (size_t i = 0; i < io_count; ++i) {
    neomove_sdk_guard::Call([&]() {
      return NM_Trigger_SetLineCompareParam(
          GetControllerIndex(), 0, 0, 0, 0, scan_start, scan_end, fabs(step));
    });
    neomove_sdk_guard::Call([&]() {
      return NM_Trigger_EnableLineCompare(GetControllerIndex(), 0, 0, 0, 1);
    });
  }

  // Move to scan start position
  ret = axis->AsyncMoveTo(scan_start, profile);
  if (ret != 0) return ret;
  ret = axis->Wait();
  if (ret != 0) return ret;

  // Move through the scan range
  ret = axis->AsyncMoveTo(scan_end, profile);
  if (ret != 0) return ret;
  ret = axis->Wait();
  if (ret != 0) return ret;

  // Disable line compare
  for (size_t i = 0; i < io_count; ++i) {
    neomove_sdk_guard::Call([&]() {
      return NM_Trigger_EnableLineCompare(GetControllerIndex(), 0, 0, 0, 0);
    });
  }

  // Return to start position
  axis->AsyncMoveTo(start_pos, profile);
  axis->Wait();

  return 0;
}

int YOTTA_API_CALL NeoMoveEventControl::SyncSoftwareTouchProbe(
    Axis* axis, double offset, double step, int touch_count,
    InputIO* input_io[], size_t io_count, AccDecProfile* profile) {
  ClearError();
  std::lock_guard<std::mutex> lock(touch_probe_mutex_);

  if (!axis || !profile || touch_count <= 0) {
    SetError(MotionErrors::ParamInvalid, "SyncSoftwareTouchProbe 参数无效");
    return 1;
  }

  LOG(INFO) << "SyncSoftwareTouchProbe: axis=" << axis->id()
            << ", touch_count=" << touch_count;

  double start_pos = 0;
  int ret = axis->GetActualPosition(&start_pos);
  if (ret != 0) return ret;

  touch_probe_data_.clear();
  touch_probe_data_.resize(io_count);
  probe_motion_finished_ = false;

  double scan_start = start_pos - touch_count * step / 2.0 + offset;
  double scan_end = start_pos + touch_count * step / 2.0 + offset;

  // Move to scan start
  ret = axis->AsyncMoveTo(scan_start, profile);
  if (ret != 0) return ret;
  ret = axis->Wait();
  if (ret != 0) return ret;

  // Move through the scan range and capture latch positions
  ret = axis->AsyncMoveTo(scan_end, profile);
  if (ret != 0) return ret;

  // Poll for latch positions during motion
  for (size_t i = 0; i < io_count; ++i) {
    double latch_pos = 0;
    for (int t = 0; t < touch_count; ++t) {
      // Use both rising and falling edge latch modes
      for (int mode = 0; mode < 6; ++mode) {
        int latch_ret = neomove_sdk_guard::Call([&]() {
          return NM_SingleAxis_Get_LatchPosition(
              GetControllerIndex(), axis->id(), mode, &latch_pos);
        });
        if (latch_ret == NM_RETURN_OK) {
          touch_probe_data_[i].push_back(latch_pos);
          neomove_sdk_guard::Call([&]() {
            return NM_SingleAxis_Clear_LatchPosition(
                GetControllerIndex(), axis->id(), mode);
          });
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }

  ret = axis->Wait();
  probe_motion_finished_ = true;

  // Return to start
  axis->AsyncMoveTo(start_pos, profile);
  axis->Wait();

  return ret;
}

int YOTTA_API_CALL NeoMoveEventControl::GetTouchProbleCounterValues(
    double** counter_values, size_t* count) {
  if (!counter_values || !count) return 1;

  size_t total = 0;
  for (auto& data : touch_probe_data_) {
    total += data.size();
  }

  if (total == 0) {
    *counter_values = nullptr;
    *count = 0;
    return 0;
  }

  *counter_values = new double[total];
  size_t idx = 0;
  for (auto& data : touch_probe_data_) {
    for (double val : data) {
      (*counter_values)[idx++] = val;
    }
  }
  *count = total;
  return 0;
}

int YOTTA_API_CALL NeoMoveEventControl::FreeTouchProbleCounterValues(
    double** counter_values) {
  if (counter_values && *counter_values) {
    delete[] *counter_values;
    *counter_values = nullptr;
  }
  return 0;
}
