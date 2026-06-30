// copyright 2025 YottaImage. All rights reserved.
#include "neomove_sdk_guard.h"

#include "neomove_motion_mgr_context_impl.h"

namespace neomove_sdk_guard {

std::mutex& Mutex() {
  return NeoMoveMotionMgrContextImpl::GetCurrent().sdk_mutex();
}

}  // namespace neomove_sdk_guard
