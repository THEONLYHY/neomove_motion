

#include <common/message_loop.h>
#include <common/path/path_utils.h>
#include <glog/glog_helper.h>
#include <service/service_framework.h>

#ifdef USING_MODULE_MGR_LIB
#include <main_process/main_process_helper.h>
#endif

#include <service/service_framework.h>

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFont>
#include <QGuiApplication>
#include <QMetaType>
#include <QObject>
#include <QTranslator>
#include <exception>
#include <memory>
#include <string>

#include "view/main_window/base_main_window.h"
#include "view/tools/virtual_keyboard/virtual_keyboard.h"

void HandlePendingConfigLoad();

//  使用命名空间来引入时间字面量
using namespace std::chrono_literals;

int main(int argc, char* argv[]) {
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
      Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
  QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
  // 创建 Qt 应用程序对象
  QApplication app(argc, argv);
  // 设置全局默认字体
  QFont appFont("微软雅黑", 10);
  app.setFont(appFont);
  qRegisterMetaType<std::string>("std::string");
  // 设置控制台输出为 UTF-8 编码
  // QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
  SetConsoleOutputCP(CP_UTF8);

  // 创建 exe 目录下的日志目录，保持和 GLog 输出目录一致
  std::wstring exe_path = path_utils::GetFullPathFromCurrentModule(L"log/");
  QDir().mkpath(QString::fromStdWString(exe_path));
  // 初始化 GLog 日志系统
  std::string utf8_path = encode_helper::Unicode2Utf8(exe_path.c_str());
  glog_helper::InitGLog("ProbeStation8.exe", utf8_path,
                        utf8_path + "ProbeStation8");
#ifndef _DEBUG
  // 自动清理 30 天前的日志
  FLAGS_timestamp_in_logfile_name = true;
  google::EnableLogCleaner(24h * 30);
#endif
  FLAGS_logbuflevel = -1;  // 设置日志缓冲级别

  // 记录程序开始运行的日志信息
  LOG(INFO) << "开始运行";

  // 加载conifg文件夹
  HandlePendingConfigLoad();

  // 初始化UI线程的loop
  std::shared_ptr<common::MessageLoop> ui_loop =
      common::MessageLoop::Create(common::kUi, false);
  // 开启io的新线程
  std::shared_ptr<common::MessageLoop> io_loop =
      common::MessageLoop::Create(common::kIo, true);
  // 开启kMotion的新线程
  std::shared_ptr<common::MessageLoop> motion_loop =
      common::MessageLoop::Create(common::kMotion, true);

  // 加载默认翻译文件
  QTranslator* translator = new QTranslator;
  if (translator->load(
          QString ::fromStdWString(path_utils::GetFullPathFromCurrentModule(
              L"ProbeStation_zh_CN.qm")))) {
    qApp->installTranslator(translator);
  }

  VirtualKeyboardSingleton::GetInstance()->InstallGlobalFilter();

  ServiceFrameworkHelper helper;
  helper.InitDll();
  // assert(helper && L"确认service_framework.dll跟public一致");
  if (!helper) {
    LOG(ERROR) << "确认service_framework.dll跟public一致";
    return -1;
  }
  helper->Init();

  BaseMainWindow main_window;
  main_window.showFullScreen();

  return app.exec();
}
