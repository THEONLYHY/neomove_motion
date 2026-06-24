// copyright 2025 YottaImage. All rights reserved.
#include "neomove_output_io.h"

#include <glog/glog_helper.h>

#include "neomove_io_monitor_thread.h"
#include "neomove_motion_mgr_context_impl.h"

using namespace yotta;

NeoMoveOutputIo::NeoMoveOutputIo(int addr, int bit)
    : NeoMoveInputIo(addr, bit) {}

int YOTTA_API_CALL NeoMoveOutputIo::ReadValue(unsigned char* byte_val) {
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
    SetError(MotionErrors::MoveFailed, "ReadValue(output) 失败",
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

void YOTTA_API_CALL NeoMoveOutputIo::StartWatching(IOWatcher* callback) {
  if (!callback) {
    return;
  }
  io_watcher_ptr_ = std::shared_ptr<IOWatcher>(callback, [](IOWatcher*) {});
  if (auto monitor = io_monitor_thread_.lock()) {
    monitor->RegisterOutputIo(addr_, bit_, callback,
                              static_cast<yotta::IO*>(
                                  static_cast<NeoMoveInputIo*>(this)));
  }
}

int YOTTA_API_CALL NeoMoveOutputIo::WriteValue(unsigned char byte_val) {
  ClearError();
  if (bit_ >= 0) {
    return byte_val ? TurnBitOn(bit_) : TurnBitOff(bit_);
  }

  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  unsigned short pdo_val = byte_val;

  int ret = NM_EtherCATWritePDO(GetControllerIndex(), 0, uIndex, gIndex,
                                 &pdo_val, 1);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "WriteValue 失败",
             "neomove_api=NM_EtherCATWritePDO, ret=" + ToHex(ret));
  }
  return ret;
}

int NeoMoveOutputIo::TurnBitOn(unsigned char bit_pos) {
  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  unsigned short pdo_val = 0;

  int ret = NM_EtherCATReadPDO(GetControllerIndex(), 0, uIndex, gIndex,
                                &pdo_val, 1);
  if (ret != NM_RETURN_OK) return ret;

  pdo_val |= (1 << bit_pos);

  ret = NM_EtherCATWritePDO(GetControllerIndex(), 0, uIndex, gIndex,
                              &pdo_val, 1);
  return ret;
}

int NeoMoveOutputIo::TurnBitOff(unsigned char bit_pos) {
  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  unsigned short pdo_val = 0;

  int ret = NM_EtherCATReadPDO(GetControllerIndex(), 0, uIndex, gIndex,
                                &pdo_val, 1);
  if (ret != NM_RETURN_OK) return ret;

  pdo_val &= ~(1 << bit_pos);

  ret = NM_EtherCATWritePDO(GetControllerIndex(), 0, uIndex, gIndex,
                              &pdo_val, 1);
  return ret;
}
