// copyright 2025 YottaImage. All rights reserved.
#include "neomove_input_io.h"

#include <glog/glog_helper.h>

#include "neomove_io_monitor_thread.h"
#include "neomove_motion_mgr_context_impl.h"

using namespace yotta;

NeoMoveInputIo::NeoMoveInputIo(int addr, int bit) : addr_(addr), bit_(bit) {}

int NeoMoveInputIo::GetControllerIndex() {
  return NeoMoveMotionMgrContextImpl::GetCurrent().controller_index();
}

int YOTTA_API_CALL NeoMoveInputIo::SetAddress(int addr, int bit) {
  addr_ = addr;
  bit_ = bit;
  return 0;
}

int YOTTA_API_CALL NeoMoveInputIo::GetAddress(int* addr, int* bit) const {
  if (addr) *addr = addr_;
  if (bit) *bit = bit_;
  return 0;
}

int YOTTA_API_CALL NeoMoveInputIo::ReadValue(unsigned char* byte_val) {
  ClearError();
  if (!byte_val) {
    SetError(MotionErrors::ParamInvalid, "byte_val 参数为空");
    return 1;
  }

  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  unsigned short pdo_val = 0;

  int ret = NM_EtherCATReadPDO(GetControllerIndex(), 0, uIndex, gIndex,
                                &pdo_val, 1);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "ReadValue 失败",
             "neomove_api=NM_EtherCATReadPDO, ret=" + ToHex(ret));
    return ret;
  }

  if (bit_ >= 0) {
    *byte_val = (pdo_val & (1 << bit_)) ? 1 : 0;
  } else {
    *byte_val = static_cast<unsigned char>(pdo_val & 0xFF);
  }
  return 0;
}

void YOTTA_API_CALL NeoMoveInputIo::StartWatching(IOWatcher* callback) {
  if (auto monitor = io_monitor_thread_.lock()) {
    monitor->RegisterInputIo(addr_, bit_, callback, this);
  }
}

void* NeoMoveInputIo::QueryInterface(const char* interface_name,
                                      size_t length) {
  return nullptr;
}

void NeoMoveInputIo::SetIoMonitorThread(
    std::weak_ptr<NeoMoveIoMonitorThread> monitor) {
  io_monitor_thread_ = monitor;
}
