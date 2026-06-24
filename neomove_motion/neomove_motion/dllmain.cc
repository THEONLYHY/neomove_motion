// dllmain.cpp : Defines the entry point for the DLL application.
#define WIN32_LEAN_AND_MEAN
#include <common/common_helper.h>
#include <common/file/file_version.h>
#include <common/path/path_utils.h>
#include <glog/glog_helper.h>
#include <windows.h>

#include <chrono>

#include "src/neomove_motion_mgr.h"
using namespace std::chrono;

HMODULE gModule = nullptr;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      gModule = hModule;
      break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}

yotta::MotionMgr* YOTTA_API_CALL GetMotionMgr(int major_version,
                                              int minor_verison) {
  static std::once_flag flag;
  std::call_once(flag, []() {
    FLAGS_logbuflevel = -1;
    std::wstring exe_path = path_utils::GetFullPathFromCurrentExe(L"log/");
    std::string utf8_path = encode_helper::Unicode2Utf8(exe_path.c_str());
    glog_helper::InitGLog("neomove_motion.mot", utf8_path,
                          utf8_path + "neomove_motion");
    google::EnableLogCleaner(24h * 30);
  });

  std::wstring dll_ver;
  wchar_t dll_file[MAX_PATH] = {};
  GetModuleFileName(gModule, dll_file, MAX_PATH);
  file_util::GetFileVersion(dll_file, dll_ver);
  LOG(INFO) << "version: " << dll_ver;

  LOG(INFO) << "GetMotionMgr, major:" << major_version
            << ",minor:" << minor_verison;
  if (major_version == yotta::motion::kMajorVersion &&
      minor_verison >= yotta::motion::kMinorVersion) {
    yotta::MotionMgr* mgr = NeoMoveMotionMgrSingleton::GetInstance();
    LOG(INFO) << "GetMotionMgr: this=0x" << std::hex << mgr;
    return mgr;
  }
  return nullptr;
}

void YOTTA_API_CALL SetLogLevel(int log_level, const char* log_level_command) {
  FLAGS_minloglevel = static_cast<google::LogSeverity>(log_level);
  LOG(INFO) << "SetLogLevel: log_level=" << log_level
            << ",log_level_command=" << log_level_command;
}
