#include "MotionTestHost.h"

#include <QTabWidget>

#include "axis_setting/axis_setting.h"
#include "io_setting/io_setting.h"
#include "motion_test/motion_test_page.h"
#include "point_setting/point_setting.h"

MotionTestHost::MotionTestHost(QWidget *parent)
    : QMainWindow(parent) {
    ui.setupUi(this);

    auto *tab_widget = new QTabWidget(this);
    tab_widget->addTab(new IoSetting(tab_widget), QStringLiteral("IO"));
    tab_widget->addTab(new AxisSetting(tab_widget), QStringLiteral("AXIS"));
    tab_widget->addTab(new PointSetting(tab_widget), QStringLiteral("点位设置"));
    tab_widget->addTab(new MotionTestPage(tab_widget), QStringLiteral("运动测试"));

    setCentralWidget(tab_widget);
    resize(1460, 975);
}

MotionTestHost::~MotionTestHost() = default;
