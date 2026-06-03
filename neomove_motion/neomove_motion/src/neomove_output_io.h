// copyright 2025 YottaImage. All rights reserved.
#ifndef NEOMOVE_OUTPUT_IO_H_
#define NEOMOVE_OUTPUT_IO_H_

#include <motion/io.h>

#include "neomove_input_io.h"

class NeoMoveOutputIo : public NeoMoveInputIo, public virtual yotta::OutputIO {
 public:
  explicit NeoMoveOutputIo(int addr, int bit);
  virtual ~NeoMoveOutputIo() = default;

  ExecutionErrorPtr GetError() const override {
    return NeoMoveInputIo::GetError();
  }

  int YOTTA_API_CALL SetAddress(int addr, int bit) override {
    return NeoMoveInputIo::SetAddress(addr, bit);
  }

  int YOTTA_API_CALL GetAddress(int* addr, int* bit) const override {
    return NeoMoveInputIo::GetAddress(addr, bit);
  }

  int YOTTA_API_CALL ReadValue(unsigned char* byte_val) override;
  void YOTTA_API_CALL StartWatching(IOWatcher* callback) override;
  int YOTTA_API_CALL WriteValue(unsigned char byte_val) override;
  void* YOTTA_API_CALL QueryInterface(const char* interface_name,
                                      size_t length) override {
    return NeoMoveInputIo::QueryInterface(interface_name, length);
  }

 private:
  int TurnBitOn(unsigned char bit_pos);
  int TurnBitOff(unsigned char bit_pos);
};

#endif  // NEOMOVE_OUTPUT_IO_H_
