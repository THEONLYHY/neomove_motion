// copyright 2025 YottaImage. All rights reserved.
#ifndef NEOMOVE_ANALOG_IO_H_
#define NEOMOVE_ANALOG_IO_H_

#include <error_info/error_codes.h>
#include <error_info/impl/execution_error_impl.h>
#include <motion/analog_io.h>

#include <mutex>
#include <sstream>

#include "NeoMove CPlusPlus.h"

class NeoMoveAnalogIo : public yotta::AnalogIO {
 public:
  explicit NeoMoveAnalogIo(int addr);
  virtual ~NeoMoveAnalogIo() = default;

  ExecutionErrorPtr GetError() const override {
    std::lock_guard<std::mutex> lock(error_mutex_);
    return last_error_;
  }

  int YOTTA_API_CALL SetAddress(int addr) override;
  int YOTTA_API_CALL GetAddress(int* addr) const override;

  int YOTTA_API_CALL SetOutChar(char analogData) override;
  int YOTTA_API_CALL SetOutUChar(unsigned char analogData) override;
  int YOTTA_API_CALL SetOutShort(short analogData) override;
  int YOTTA_API_CALL SetOutUShort(unsigned short analogData) override;
  int YOTTA_API_CALL SetOutInt(int analogData) override;
  int YOTTA_API_CALL SetOutUInt(unsigned int analogData) override;

  int YOTTA_API_CALL GetInChar(char* pAnalogData) override;
  int YOTTA_API_CALL GetInUChar(unsigned char* pAnalogData) override;
  int YOTTA_API_CALL GetInShort(short* pAnalogData) override;
  int YOTTA_API_CALL GetInUShort(unsigned short* pAnalogData) override;
  int YOTTA_API_CALL GetInInt(int* pAnalogData) override;
  int YOTTA_API_CALL GetInUInt(unsigned int* pAnalogData) override;

  int YOTTA_API_CALL GetOutChar(char* pAnalogData) override;
  int YOTTA_API_CALL GetOutUChar(unsigned char* pAnalogData) override;
  int YOTTA_API_CALL GetOutShort(short* pAnalogData) override;
  int YOTTA_API_CALL GetOutUShort(unsigned short* pAnalogData) override;
  int YOTTA_API_CALL GetOutInt(int* pAnalogData) override;
  int YOTTA_API_CALL GetOutUInt(unsigned int* pAnalogData) override;

  void* YOTTA_API_CALL QueryInterface(const char* interface_name,
                                      size_t length) override;

 private:
  int GetControllerIndex();
  int addr_;

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
        .WithEntity("neomove_analog_io_" + std::to_string(addr_));
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

#endif  // NEOMOVE_ANALOG_IO_H_
