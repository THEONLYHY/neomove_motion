// copyright 2025 YottaImage. All rights reserved.
#include "neomove_motion_mgr_context_impl.h"

#include <common/encode_helper.h>
#include <common/json_config/json_config_helper.h>
#include <error_info/error_codes.h>
#include <glog/glog_helper.h>
#include <nlohmann/json.hpp>

#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

namespace {

constexpr char kNeoMoveConfigRoot[] = "neomove";
constexpr char kNeoMoveHomeConfigRoot[] = "home";
constexpr char kHomeTypeKey[] = "home_type";
constexpr char kVelocityFastKey[] = "velocity_fast";
constexpr char kVelocitySlowKey[] = "velocity_slow";
constexpr char kAccKey[] = "acc";
constexpr char kDecKey[] = "dec";

// JsonConfig 将运动插件配置放在 "neomove" 根节点下。
// 统一在这里拼接 key，避免各配置项使用不一致的字面量前缀。
std::string MakeNeoMoveConfigKey(const char* key) {
  return std::string(kNeoMoveConfigRoot) + "/" + key;
}

// 宿主可能传入绝对配置文件路径，也可能传入相对配置目录的文件名。
// Windows 下先把 UTF-8 字符串转换为宽字符路径，再交给 std::filesystem，
// 这样中文路径也能正常处理。
std::filesystem::path MakeControllerConfigFilePath(const char* path,
                                                   size_t path_len,
                                                   const char* file,
                                                   size_t file_len) {
  std::string path_str(path, path_len);
  std::string file_str(file, file_len);
  std::wstring path_wide = encode_helper::Utf82Unicode(path_str.c_str());
  std::wstring file_wide = encode_helper::Utf82Unicode(file_str.c_str());

  std::filesystem::path file_path(file_wide);
  if (file_path.is_absolute()) {
    return file_path;
  }

  return std::filesystem::path(path_wide) / file_path;
}

bool ReadOptionalInt(const nlohmann::json& node, const char* key,
                     int default_value, int* value) {
  if (!node.contains(key)) {
    *value = default_value;
    return true;
  }
  if (!node.at(key).is_number_integer()) {
    return false;
  }
  *value = node.at(key).get<int>();
  return true;
}

bool ReadOptionalDouble(const nlohmann::json& node, const char* key,
                        double default_value, double* value) {
  if (!node.contains(key)) {
    *value = default_value;
    return true;
  }
  if (!node.at(key).is_number()) {
    return false;
  }
  *value = node.at(key).get<double>();
  return true;
}

bool IsPositiveFinite(double value) {
  return std::isfinite(value) && value > 0.0;
}

}  // namespace

NeoMoveHomeParamConfig DefaultNeoMoveHomeParamConfig() {
  NeoMoveHomeParamConfig config{};
  config.home_type = 35;
  config.velocity_fast = 10.0;
  config.velocity_slow = 1.0;
  config.acc = 100.0;
  config.dec = 100.0;
  return config;
}

NeoMoveMotionMgrContextImpl& NeoMoveMotionMgrContextImpl::GetCurrent() {
  static NeoMoveMotionMgrContextImpl instance;
  return instance;
}

NeoMoveMotionMgrContextImpl::~NeoMoveMotionMgrContextImpl() {
  if (initialized_) {
    // 正常情况下由 manager 负责关闭；这里作为单例析构时的兜底，
    // 防止进程卸载或异常宿主销毁顺序导致控制器未关闭。
    Finalize();
  }
}

bool NeoMoveMotionMgrContextImpl::IsValidControllerType(int controller_type) {
  // 该白名单需要与 NeoMove CPlusPlus.h 暴露的控制器常量保持一致。
  // 未知值按配置错误处理。
  switch (controller_type) {
    case NM_CONTROLLERTYPE_R2_M100:
    case NM_CONTROLLERTYPE_R2_M300:
    case NM_CONTROLLERTYPE_G2_M100:
    case NM_CONTROLLERTYPE_E2_M300:
    case NM_CONTROLLERTYPE_E2_M200A:
    case NM_CONTROLLERTYPE_E2_M100:
      return true;
    default:
      return false;
  }
}

// 业务白名单，不是SDK全量枚举
bool NeoMoveMotionMgrContextImpl::IsValidHomeType(int home_type) {
  switch (home_type) {
    case 30:
    case 33:
    case 34:
    case 35:
      return true;
    default:
      return false;
  }
}

NeoMoveHomeParamConfig NeoMoveMotionMgrContextImpl::GetHomeParamConfig(
    int axis_index) const {
  auto iter = home_param_configs_.find(axis_index);
  if (iter == home_param_configs_.end()) {
    return DefaultNeoMoveHomeParamConfig();
  }
  return iter->second;
}

int NeoMoveMotionMgrContextImpl::LoadHomeConfig(
    const std::filesystem::path& config_file) {
  // JsonConfig 只暴露按路径读标量和数组长度，不能枚举对象 key。
  // home 使用 {"0": {...}} 这种按轴对象结构，这里直接解析同一个 json 文件
  // 来发现已配置轴；controller_* 标量配置仍由 JsonConfig 读取。
  std::ifstream config_stream(config_file);
  if (!config_stream.is_open()) {
    LOG(ERROR) << "NeoMove home config load failed, file="
               << encode_helper::Unicode2Utf8(config_file.wstring().c_str());
    last_failed_api_ = "LoadConfig";
    return MotionErrors::ConfigLoadFailed;
  }

  nlohmann::json root;
  try {
    config_stream >> root;
  } catch (const nlohmann::json::exception& e) {
    LOG(ERROR) << "NeoMove home config parse failed: " << e.what();
    last_failed_api_ = "LoadConfig";
    return MotionErrors::ConfigLoadFailed;
  }

  if (!root.contains(kNeoMoveConfigRoot) ||
      !root.at(kNeoMoveConfigRoot).contains(kNeoMoveHomeConfigRoot)) {
    return 0;
  }

  const nlohmann::json& home_root =
      root.at(kNeoMoveConfigRoot).at(kNeoMoveHomeConfigRoot);
  if (!home_root.is_object()) {
    LOG(ERROR) << "Invalid NeoMove home config: neomove/home must be object";
    last_failed_api_ = "LoadConfig";
    return MotionErrors::ParamInvalid;
  }

  for (auto iter = home_root.begin(); iter != home_root.end(); ++iter) {
    int axis_index = 0;
    try {
      size_t parsed_size = 0;
      axis_index = std::stoi(iter.key(), &parsed_size);
      if (parsed_size != iter.key().size() || axis_index < 0) {
        LOG(ERROR) << "Invalid NeoMove home axis key=" << iter.key();
        last_failed_api_ = "LoadConfig";
        return MotionErrors::ParamInvalid;
      }
    } catch (const std::exception&) {
      LOG(ERROR) << "Invalid NeoMove home axis key=" << iter.key();
      last_failed_api_ = "LoadConfig";
      return MotionErrors::ParamInvalid;
    }

    if (!iter.value().is_object()) {
      LOG(ERROR) << "Invalid NeoMove home config for axis " << axis_index
                 << ": axis node must be object";
      last_failed_api_ = "LoadConfig";
      return MotionErrors::ParamInvalid;
    }

    NeoMoveHomeParamConfig home_param = DefaultNeoMoveHomeParamConfig();
    const nlohmann::json& axis_home = iter.value();
    if (!ReadOptionalInt(axis_home, kHomeTypeKey, home_param.home_type,
                         &home_param.home_type) ||
        !ReadOptionalDouble(axis_home, kVelocityFastKey,
                            home_param.velocity_fast,
                            &home_param.velocity_fast) ||
        !ReadOptionalDouble(axis_home, kVelocitySlowKey,
                            home_param.velocity_slow,
                            &home_param.velocity_slow) ||
        !ReadOptionalDouble(axis_home, kAccKey, home_param.acc,
                            &home_param.acc) ||
        !ReadOptionalDouble(axis_home, kDecKey, home_param.dec,
                            &home_param.dec)) {
      LOG(ERROR) << "Invalid NeoMove home config field type for axis "
                 << axis_index;
      last_failed_api_ = "LoadConfig";
      return MotionErrors::ParamInvalid;
    }

    if (!IsValidHomeType(home_param.home_type)) {
      LOG(ERROR) << "Invalid NeoMove home_type=" << home_param.home_type
                 << ", axis=" << axis_index;
      last_failed_api_ = "LoadConfig";
      return MotionErrors::ParamInvalid;
    }

    if (!IsPositiveFinite(home_param.velocity_fast) ||
        !IsPositiveFinite(home_param.velocity_slow) ||
        !IsPositiveFinite(home_param.acc) ||
        !IsPositiveFinite(home_param.dec)) {
      LOG(ERROR) << "Invalid NeoMove home numeric value, axis=" << axis_index
                 << ", velocity_fast=" << home_param.velocity_fast
                 << ", velocity_slow=" << home_param.velocity_slow
                 << ", acc=" << home_param.acc
                 << ", dec=" << home_param.dec;
      last_failed_api_ = "LoadConfig";
      return MotionErrors::ParamInvalid;
    }

    home_param_configs_[axis_index] = home_param;
  }

  return 0;
}

// 配置文件格式。所有字段均可选，缺失时回退到 LoadConfig() 和
// DefaultNeoMoveHomeParamConfig() 中定义的默认值。
// {
//   "neomove": {
//     "controller_index": 0,
//     "controller_type": 5,
//     "controller_ip": "172.30.30.10",
//     "home": {
//       "0": {
//         "home_type": 35,
//         "velocity_fast": 10.0,
//         "velocity_slow": 1.0,
//         "acc": 100.0,
//         "dec": 100.0
//       }
//     }
//   }
// }
//

int NeoMoveMotionMgrContextImpl::LoadConfig(const char* path, size_t path_len,
                                             const char* file,
                                             size_t file_len) {
  if (!path || !file) {
    LOG(ERROR) << "NeoMove config load failed: invalid path or file pointer.";
    last_failed_api_ = "LoadConfig";
    return MotionErrors::ParamInvalid;
  }

  controller_index_ = 0;
  controller_type_ = NM_CONTROLLERTYPE_E2_M300;
  controller_ip_.clear();
  home_param_configs_.clear();

  // 读取可选配置前先恢复安全默认值。缺失 json 字段时会回退到
  // controller 0、E2_M300 和默认 IP 连接方式。
  std::filesystem::path config_file =
      MakeControllerConfigFilePath(path, path_len, file, file_len);
  std::wstring config_file_wide = config_file.wstring();
  if (!config_file.is_absolute()) {
    LOG(ERROR) << "NeoMove config load failed: controller config path must be "
                  "absolute, file="
               << encode_helper::Unicode2Utf8(config_file_wide.c_str());
    last_failed_api_ = "LoadConfig";
    return MotionErrors::ParamInvalid;
  }

  std::unique_ptr<JsonConfig> config = std::make_unique<NlohmanJsonConfig>();
  if (!config->Init(config_file_wide)) {
    LOG(ERROR) << "NeoMove config load failed, file="
               << encode_helper::Unicode2Utf8(config_file_wide.c_str());
    last_failed_api_ = "LoadConfig";
    return MotionErrors::ConfigLoadFailed;
  }

  bool read_success = false;
  // controller_index 是可选项；如果配置中存在，必须能映射到有效 SDK 控制器编号。
  // 负数编号会在调用 NM_Open() 前被拒绝。
  int controller_index = config->GetInt(
      MakeNeoMoveConfigKey("controller_index"), controller_index_,
      &read_success);
  if (read_success) {
    if (controller_index < 0) {
      LOG(ERROR) << "Invalid NeoMove controller_index=" << controller_index;
      last_failed_api_ = "LoadConfig";
      return MotionErrors::ParamInvalid;
    }
    controller_index_ = controller_index;
  }

  // controller_type 用于在 NM_Open() 前选择硬件系列。
  // 如果把不支持的值传入 SDK，后续连接错误会变得难以判断。
  int controller_type = config->GetInt(
      MakeNeoMoveConfigKey("controller_type"), controller_type_,
      &read_success);
  if (read_success) {
    if (!IsValidControllerType(controller_type)) {
      LOG(ERROR) << "Invalid NeoMove controller_type=" << controller_type;
      last_failed_api_ = "LoadConfig";
      return MotionErrors::ParamInvalid;
    }
    controller_type_ = controller_type;
  }

  // controller_ip 是可选项。SDK API 接收固定长度 char 缓冲区，
  // 所以在 Init() 复制之前先校验配置长度。
  std::string controller_ip = config->GetString(
      MakeNeoMoveConfigKey("controller_ip"), "", &read_success);
  if (read_success) {
    if (controller_ip.size() >= 256) {
      LOG(ERROR) << "Invalid NeoMove controller_ip length="
                 << controller_ip.size();
      last_failed_api_ = "LoadConfig";
      return MotionErrors::ParamInvalid;
    }
    controller_ip_ = controller_ip;
  }

  int ret = LoadHomeConfig(config_file);
  if (ret != 0) {
    return ret;
  }

  LOG(INFO) << "NeoMove config loaded, file="
            << encode_helper::Unicode2Utf8(config_file_wide.c_str())
            << ", controller_index=" << controller_index_
            << ", controller_type=" << controller_type_
            << ", controller_ip=" << controller_ip_
            << ", configured_home_axes=" << home_param_configs_.size();
  return 0;
}

int NeoMoveMotionMgrContextImpl::Init(const char* path, size_t path_len,
                                       const char* file, size_t file_len) {
  if (initialized_) {
    LOG(WARNING) << "NeoMoveMotionMgrContextImpl already initialized";
    return 0;
  }
  last_failed_api_.clear();

  // Init() 会记录具体失败步骤，让 NeoMoveMotionMgr 能对外暴露可定位的错误信息，
  // 而不是只返回原始 SDK 错误码。
  LOG(INFO) << "NeoMoveMotionMgrContextImpl::Init, path="
            << std::string(path, path_len)
            << ", file=" << std::string(file, file_len);

  // 修改 SDK 控制器状态前，先加载并校验宿主配置。
  int ret = LoadConfig(path, path_len, file, file_len);
  if (ret != 0) {
    return ret;
  }

  // 连接设备前必须先选择控制器类型。
  ret = NM_SetControllerType(controller_type_);
  if (ret != NM_RETURN_OK) {
    LOG(ERROR) << "NM_SetControllerType failed, ret=" << ret
               << ", controller_type=" << controller_type_;
    last_failed_api_ = "NM_SetControllerType";
    return ret;
  }

  if (!controller_ip_.empty()) {
    // NM_SetControllerIP 需要可写 C 缓冲区。LoadConfig() 的长度校验确保
    // 这里的截断路径不会丢失合法配置。
    char ip[256]{};
    strncpy_s(ip, sizeof(ip), controller_ip_.c_str(), _TRUNCATE);
    ret = NM_SetControllerIP(controller_index_, ip);
    if (ret != NM_RETURN_OK) {
      LOG(ERROR) << "NM_SetControllerIP failed, ret=" << ret
                 << ", controller_index=" << controller_index_;
      last_failed_api_ = "NM_SetControllerIP";
      return ret;
    }
  }

  // NM_Open 是真正连接配置控制器的步骤。成功后 initialized_ 置为 true，
  // Finalize() 负责用同一个 controller_index 调用 NM_Close。
  ret = NM_Open(controller_index_);
  if (ret != NM_RETURN_OK  && ret != NM_RETURN_ERROR_ALREADYOPEN ) {
    LOG(ERROR) << "NM_Open failed, ret=" << ret
               << ", controller_index=" << controller_index_;
    last_failed_api_ = "NM_Open";
    return ret;
  }

  if (ret == NM_RETURN_ERROR_ALREADYOPEN) {
   LOG(INFO) << "NM_Open already open, controller_index=" << controller_index_;
  }

  initialized_ = true;
  last_failed_api_.clear();
  LOG(INFO) << "NeoMoveMotionMgrContextImpl initialized successfully";
  return 0;
}

int NeoMoveMotionMgrContextImpl::Finalize() {
  if (!initialized_) {
    return 0;
  }

  // 关闭该上下文拥有的 SDK 连接。更上层的 manager 资源由
  // NeoMoveMotionMgr::ClearResources() 释放。
  LOG(INFO) << "NeoMoveMotionMgrContextImpl::Finalize";
  int ret = NM_Close(controller_index_);
  if (ret != NM_RETURN_OK) {
    LOG(ERROR) << "NM_Close failed, ret=" << ret;
  }
  initialized_ = false;
  return ret;
}

double YOTTA_API_CALL NeoMoveMotionMgrContextImpl::GetAxisMultiplier(void) {
  // 这里使用前向声明：具体实现在 NeoMoveMotionMgr 定义之后提供。
  extern double GetNeoMoveAxisMultiplier();
  return GetNeoMoveAxisMultiplier();
}
