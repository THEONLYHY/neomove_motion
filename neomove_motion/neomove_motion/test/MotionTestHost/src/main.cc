#include "MotionTestHost.h"

#include "common/encode_helper.h"
#include "common/message_loop.h"
#include "common/path/path_utils.h"
#include "config/config_factory.h"
#include "controller/device_status_monitor/device_status_monitor.h"
#include "glog/glog_helper.h"
#include "main_process/module_mgr.h"
#include "model/model_mgr.h"
#include "service/service_framework.h"

#include <ShlObj.h>
#include <Windows.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QtWidgets/QApplication>

namespace {

void InitLog() {
    std::wstring log_dir = path_utils::GetFullPathFromCurrentModule(L"log/");
    SHCreateDirectory(nullptr, log_dir.c_str());
    std::string utf8_log_dir = encode_helper::Unicode2Utf8(log_dir.c_str());
    glog_helper::InitGLog("MotionTestHost.exe", utf8_log_dir,
                          utf8_log_dir + "MotionTestHost");
}

bool SetMotionDllSearchDirectory(QApplication* app) {
    const QString motion_dir =
        QDir::toNativeSeparators(QCoreApplication::applicationDirPath() +
                                 "/motion");
    // public::LibraryLoader currently calls LoadLibraryW() directly. The
    // NeoMove plugin depends on DLLs next to neomove_motion.mot, so the host
    // must put the motion directory into the process DLL search path first.
    const BOOL ok = SetDllDirectoryW(motion_dir.toStdWString().c_str());
    if (app) {
        app->setProperty("motion_dll_dir", motion_dir);
        app->setProperty("motion_dll_dir_set", ok != FALSE);
        app->setProperty("motion_dll_dir_error",
                         ok ? 0 : static_cast<int>(GetLastError()));
    }
    qInfo() << "SetDllDirectoryW motion dir =" << motion_dir
            << "ok =" << (ok != FALSE)
            << "error =" << (ok ? 0 : GetLastError());
    return ok != FALSE;
}

}  // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    InitLog();

    common::MessageLoopPtr ui_message_loop =
        common::MessageLoop::Create(common::kUi, false);
    // 运动测试页会把耗时运动任务投递到 MOTION loop，避免阻塞 Qt UI 线程。
    common::MessageLoopPtr motion_message_loop =
        common::MessageLoop::Create(common::kMotion, true);
    if (!motion_message_loop || !motion_message_loop->Valid()) {
        qWarning() << "MOTION MessageLoop initialization failed.";
    }

    ServiceFrameworkHelper service_framework;
    const bool service_framework_dll_ready = service_framework.InitDll();
    const bool service_framework_ready =
        service_framework_dll_ready && service_framework->Init();
    app.setProperty("service_framework_dll_init", service_framework_dll_ready);
    app.setProperty("service_framework_runtime_init", service_framework_ready);
    app.setProperty("service_framework_init", service_framework_ready);
    if (!service_framework_ready) {
        qWarning() << "ServiceFramework initialization failed.";
    }

    yotta::ConfigFactory::GetInstance()->Init(0);
    ModelMgrSinglton::GetInstance()->Init(nullptr);
    SetMotionDllSearchDirectory(&app);

    auto module_mgr = main_process::GetModuleMgr();
    bool module_mgr_init_called = false;
    app.setProperty("module_mgr_available", module_mgr != nullptr);
    app.setProperty("module_init_ret", -1);
    app.setProperty("device_status_monitor_started", false);
    if (!module_mgr) {
        qWarning() << "ModuleMgr is null. Check module_runtime_d.dll.";
    } else {
        const QByteArray config_dir =
            QDir::toNativeSeparators(
                QCoreApplication::applicationDirPath() + "/config")
                .toUtf8();
        const QByteArray config_file("neomove.json");
        app.setProperty("module_init_path", QString::fromUtf8(config_dir));
        app.setProperty("module_init_file", QString::fromUtf8(config_file));
        const int ret =
            module_mgr->Init(config_dir.constData(), config_dir.size(),
                             config_file.constData(), config_file.size(),
                             AlgorithmProcessCallbackWeakPtr{});
        module_mgr_init_called = true;
        app.setProperty("module_init_ret", ret);
        qInfo() << "ModuleMgr::Init path =" << config_dir
                << "file =" << config_file << "ret =" << ret;
        if (ret != 0) {
            qWarning() << "ModuleMgr initialization failed, ret =" << ret;
        }

        if (module_mgr->GetLimitMotionMgr()) {
            // ModuleMgr::Init may return a later config/action-chain error even
            // after LimitMotionMgr is ready. The UI status monitor only needs
            // the motion manager, so start it once that dependency exists.
            DeviceStatusMonitorSinglton::GetInstance()->Start();
            app.setProperty("device_status_monitor_started", true);
            qInfo() << "DeviceStatusMonitor started after LimitMotionMgr init.";
        } else {
            qWarning() << "DeviceStatusMonitor not started: LimitMotionMgr is null.";
        }
    }

    MotionTestHost window;
    window.show();
    const int ret = app.exec();

    // 退出时先停止测试页面的后台访问，再释放运动模块，避免监控线程或
    // MOTION loop 继续访问 ModuleMgr::Uninit() 后释放的运动对象。
    DeviceStatusMonitorSinglton::GetInstance()->Stop();
    if (motion_message_loop) {
        motion_message_loop->ClearTask();
        motion_message_loop->Uninit();
    }
    if (module_mgr && module_mgr_init_called) {
        module_mgr->Uninit();
    }
    return ret;
}
