#pragma once

#include <QtWidgets/QMainWindow>

#include "ui_MotionTestHost.h"

class MotionTestHost : public QMainWindow {
    Q_OBJECT

public:
    explicit MotionTestHost(QWidget *parent = nullptr);
    ~MotionTestHost();

private:
    Ui::MotionTestHostClass ui;
};
