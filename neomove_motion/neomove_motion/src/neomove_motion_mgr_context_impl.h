// copyright 2025 YottaImage. All rights reserved.
#ifndef NEOMOVE_MOTION_MGR_CONTEXT_IMPL_H_
#define NEOMOVE_MOTION_MGR_CONTEXT_IMPL_H_

#include <motion/motion_mgr_context.h>

#include <memory>
#include <string>

#include "NeoMove CPlusPlus.h"

class NeoMoveMotionMgrContextImpl : public yotta::MotionMgrContext {
 public:
  static NeoMoveMotionMgrContextImpl& GetCurrent();

  int Init(const char* path, size_t path_len,
           const char* file, size_t file_len);
  int Finalize();

  double YOTTA_API_CALL GetAxisMultiplier(void) override;

  int controller_index() const { return controller_index_; }
  bool is_initialized() const { return initialized_; }

  static void SetGlobalConfig(const std::string& path,
                              const std::string& file);
  static void SetControllerIP(const std::string& ip);

 private:
  NeoMoveMotionMgrContextImpl() = default;
  ~NeoMoveMotionMgrContextImpl();

  int controller_index_ = 0;
  bool initialized_ = false;

  static std::string config_path_;
  static std::string config_file_;
  static std::string controller_ip_;
  static int controller_type_;
};

#endif  // NEOMOVE_MOTION_MGR_CONTEXT_IMPL_H_
