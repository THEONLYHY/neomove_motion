// copyright 2025 YottaImage. All rights reserved.
#ifndef NEOMOVE_SDK_GUARD_H_
#define NEOMOVE_SDK_GUARD_H_

#include <mutex>

namespace neomove_sdk_guard {

std::mutex& Mutex();

template <typename Callable>
auto Call(Callable&& callable) -> decltype(callable()) {
  std::lock_guard<std::mutex> lock(Mutex());
  return callable();
}

}  // namespace neomove_sdk_guard

#endif  // NEOMOVE_SDK_GUARD_H_
