// copyright 2025 YottaImage. All rights reserved.
#include "neomove_analog_io.h"

#include <glog/glog_helper.h>
#include <cstring>

#include "neomove_motion_mgr_context_impl.h"
#include "neomove_pdo_utils.h"

using namespace yotta;

NeoMoveAnalogIo::NeoMoveAnalogIo(int addr) : addr_(addr) {}

int NeoMoveAnalogIo::GetControllerIndex() {
  return NeoMoveMotionMgrContextImpl::GetCurrent().controller_index();
}

int YOTTA_API_CALL NeoMoveAnalogIo::SetAddress(int addr) {
  addr_ = addr;
  return 0;
}

int YOTTA_API_CALL NeoMoveAnalogIo::GetAddress(int* addr) const {
  if (addr) *addr = addr_;
  return 0;
}

void* NeoMoveAnalogIo::QueryInterface(const char* interface_name,
                                       size_t length) {
  return nullptr;
}

// Analog IO uses EtherCAT PDO with the address mapped to uIndex/gIndex.
// Since NeoMove does not have a native analog IO API, these are implemented
// via NM_EtherCATReadPDO / NM_EtherCATWritePDO.

int YOTTA_API_CALL NeoMoveAnalogIo::SetOutChar(char analogData) {
  ClearError();
  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  unsigned short val = static_cast<unsigned short>(analogData);
  int ret = neomove_pdo::WriteWord(GetControllerIndex(), uIndex, gIndex, val);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "SetOutChar 失败",
             "neomove_api=NM_EtherCATWritePDO, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAnalogIo::SetOutUChar(unsigned char analogData) {
  ClearError();
  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  unsigned short val = analogData;
  int ret = neomove_pdo::WriteWord(GetControllerIndex(), uIndex, gIndex, val);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "SetOutUChar 失败",
             "neomove_api=NM_EtherCATWritePDO, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAnalogIo::SetOutShort(short analogData) {
  ClearError();
  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  unsigned short val = static_cast<unsigned short>(analogData);
  int ret = neomove_pdo::WriteWord(GetControllerIndex(), uIndex, gIndex, val);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "SetOutShort 失败",
             "neomove_api=NM_EtherCATWritePDO, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAnalogIo::SetOutUShort(unsigned short analogData) {
  ClearError();
  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  unsigned short val = analogData;
  int ret = neomove_pdo::WriteWord(GetControllerIndex(), uIndex, gIndex, val);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "SetOutUShort 失败",
             "neomove_api=NM_EtherCATWritePDO, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAnalogIo::SetOutInt(int analogData) {
  ClearError();
  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  // Write 2 shorts for a 32-bit int
  unsigned short vals[2] = {};
  vals[0] = static_cast<unsigned short>(analogData & 0xFFFF);
  vals[1] = static_cast<unsigned short>((analogData >> 16) & 0xFFFF);
  int ret = neomove_pdo::WriteWords(GetControllerIndex(), uIndex, gIndex, vals,
                                    2);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "SetOutInt 失败",
             "neomove_api=NM_EtherCATWritePDO, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAnalogIo::SetOutUInt(unsigned int analogData) {
  ClearError();
  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  unsigned short vals[2] = {};
  vals[0] = static_cast<unsigned short>(analogData & 0xFFFF);
  vals[1] = static_cast<unsigned short>((analogData >> 16) & 0xFFFF);
  int ret = neomove_pdo::WriteWords(GetControllerIndex(), uIndex, gIndex, vals,
                                    2);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "SetOutUInt 失败",
             "neomove_api=NM_EtherCATWritePDO, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAnalogIo::GetInChar(char* pAnalogData) {
  ClearError();
  if (!pAnalogData) return 1;
  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  unsigned short val = 0;
  int ret = neomove_pdo::ReadWord(GetControllerIndex(), uIndex, gIndex, &val);
  *pAnalogData = static_cast<char>(val);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "GetInChar 失败",
             "neomove_api=NM_EtherCATReadPDO, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAnalogIo::GetInUChar(unsigned char* pAnalogData) {
  ClearError();
  if (!pAnalogData) return 1;
  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  unsigned short val = 0;
  int ret = neomove_pdo::ReadWord(GetControllerIndex(), uIndex, gIndex, &val);
  *pAnalogData = static_cast<unsigned char>(val);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "GetInUChar 失败",
             "neomove_api=NM_EtherCATReadPDO, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAnalogIo::GetInShort(short* pAnalogData) {
  ClearError();
  if (!pAnalogData) return 1;
  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  unsigned short val = 0;
  int ret = neomove_pdo::ReadWord(GetControllerIndex(), uIndex, gIndex, &val);
  *pAnalogData = static_cast<short>(val);
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "GetInShort 失败",
             "neomove_api=NM_EtherCATReadPDO, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAnalogIo::GetInUShort(unsigned short* pAnalogData) {
  ClearError();
  if (!pAnalogData) return 1;
  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  unsigned short val = 0;
  int ret = neomove_pdo::ReadWord(GetControllerIndex(), uIndex, gIndex, &val);
  *pAnalogData = val;
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "GetInUShort 失败",
             "neomove_api=NM_EtherCATReadPDO, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAnalogIo::GetInInt(int* pAnalogData) {
  ClearError();
  if (!pAnalogData) return 1;
  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  unsigned short vals[2] = {};
  int ret = neomove_pdo::ReadWords(GetControllerIndex(), uIndex, gIndex, vals,
                                   2);
  *pAnalogData = static_cast<int>(vals[0] | (vals[1] << 16));
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "GetInInt 失败",
             "neomove_api=NM_EtherCATReadPDO, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAnalogIo::GetInUInt(unsigned int* pAnalogData) {
  ClearError();
  if (!pAnalogData) return 1;
  unsigned int uIndex = (addr_ >> 16) & 0xFFFF;
  unsigned int gIndex = addr_ & 0xFFFF;
  unsigned short vals[2] = {};
  int ret = neomove_pdo::ReadWords(GetControllerIndex(), uIndex, gIndex, vals,
                                   2);
  *pAnalogData = static_cast<unsigned int>(vals[0] | (vals[1] << 16));
  if (ret != NM_RETURN_OK) {
    SetError(MotionErrors::MoveFailed, "GetInUInt 失败",
             "neomove_api=NM_EtherCATReadPDO, ret=" + ToHex(ret));
  }
  return ret;
}

int YOTTA_API_CALL NeoMoveAnalogIo::GetOutChar(char* pAnalogData) {
  return GetInChar(pAnalogData);
}
int YOTTA_API_CALL NeoMoveAnalogIo::GetOutUChar(unsigned char* pAnalogData) {
  return GetInUChar(pAnalogData);
}
int YOTTA_API_CALL NeoMoveAnalogIo::GetOutShort(short* pAnalogData) {
  return GetInShort(pAnalogData);
}
int YOTTA_API_CALL NeoMoveAnalogIo::GetOutUShort(unsigned short* pAnalogData) {
  return GetInUShort(pAnalogData);
}
int YOTTA_API_CALL NeoMoveAnalogIo::GetOutInt(int* pAnalogData) {
  return GetInInt(pAnalogData);
}
int YOTTA_API_CALL NeoMoveAnalogIo::GetOutUInt(unsigned int* pAnalogData) {
  return GetInUInt(pAnalogData);
}
