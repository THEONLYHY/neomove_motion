#include "MotionTestHost.h"

#include "common/message_loop.h"
#include "config/config_factory.h"
#include "main_process/module_mgr.h"
#include "model/model_mgr.h"
#include "service/service_framework.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    common::MessageLoopPtr ui_message_loop =
        common::MessageLoop::Create(common::kUi, false);

    ServiceFrameworkHelper service_framework;
    if (!service_framework.InitDll() || !service_framework->Init()) {
        qWarning() << "ServiceFramework initialization failed.";
    }

    yotta::ConfigFactory::GetInstance()->Init(0);
    ModelMgrSinglton::GetInstance()->Init(nullptr);

    auto module_mgr = main_process::GetModuleMgr();
    if (!module_mgr) {
        qWarning() << "ModuleMgr is null. Check module_runtime_d.dll.";
    } else {
        const QByteArray config_dir =
            QDir::toNativeSeparators(
                QCoreApplication::applicationDirPath() + "/config")
                .toUtf8();
        const QByteArray config_file("neomove.json");
        const int ret =
            module_mgr->Init(config_dir.constData(), config_dir.size(),
                             config_file.constData(), config_file.size(),
                             AlgorithmProcessCallbackWeakPtr{});
        if (ret != 0) {
            qWarning() << "ModuleMgr initialization failed, ret =" << ret;
        }
    }

    MotionTestHost window;
    window.show();
    return app.exec();
}
