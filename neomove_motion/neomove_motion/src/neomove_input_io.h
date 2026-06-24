// copyright 2025 YottaImage. All rights reserved.
#ifndef NEOMOVE_INPUT_IO_H_
#define NEOMOVE_INPUT_IO_H_

#include <error_info/error_codes.h>
#include <error_info/impl/execution_error_impl.h>
#include <motion/io.h>

#include <memory>
#include <mutex>
#include <sstream>

#include "NeoMove CPlusPlus.h"

class NeoMoveIoMonitorThread;

class NeoMoveInputIo : public yotta::InputIO {
 public:
  explicit NeoMoveInputIo(int addr, int bit);
  virtual ~NeoMoveInputIo() = default;

  ExecutionErrorPtr GetError() const override {
    std::lock_guard<std::mutex> lock(error_mutex_);
    return last_error_;
  }

  int YOTTA_API_CALL SetAddress(int addr, int bit) override;
  int YOTTA_API_CALL GetAddress(int* addr, int* bit) const override;
  int YOTTA_API_CALL ReadValue(unsigned char* byte_val) override;
  void YOTTA_API_CALL StartWatching(IOWatcher* callback) override;
  void* YOTTA_API_CALL QueryInterface(const char* interface_name,
                                      size_t length) override;

  void SetIoMonitorThread(std::weak_ptr<NeoMoveIoMonitorThread> monitor);

  int addr() const { return addr_; }
  int bit() const { return bit_; }

 protected:
  int GetControllerIndex();

  int addr_;
  int bit_;
  std::weak_ptr<NeoMoveIoMonitorThread> io_monitor_thread_;
  yotta::IOWatcherPtr io_watcher_ptr_;

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
        .WithEntity("neomove_io_" + std::to_string(addr_) + "_" +
                    std::to_string(bit_));
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

#endif  // NEOMOVE_INPUT_IO_H_
