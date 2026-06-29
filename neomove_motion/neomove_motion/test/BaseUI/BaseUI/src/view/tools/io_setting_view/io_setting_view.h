// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#ifndef PROBE_STATION8_SRC_VIEW_TOOLS_IO_SETTING_VIEW_IO_SETTING_VIEW_H_
#define PROBE_STATION8_SRC_VIEW_TOOLS_IO_SETTING_VIEW_IO_SETTING_VIEW_H_

#include <config/io/axis_io_config.h>
#include <config/io/io_config.h>
#include <config/io/multi_io_port_config.h>

#include <QButtonGroup>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QWidget>
#include <unordered_map>

#include "config/config_factory.h"
#include "model/model_mgr.h"
#include "ui_io_setting_view.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class IoSettingviewClass;
};
QT_END_NAMESPACE

class IoSettingView : public QWidget {
  Q_OBJECT

 public:
  IoSettingView(QWidget *parent = nullptr);
  ~IoSettingView();

  void showEvent(QShowEvent *event) override;
  void hideEvent(QHideEvent *event) override;
 private slots:
  void OnModuleClicked(int);
  void SlotIoClick(std::string io_ids, bool);  // io开关点击槽函数
  void OnSaveClicked();
  void UpdateTimer();  // 定时器执行函数
 private:
  void ShowModule();  // 事件过滤器
  bool eventFilter(QObject *watched, QEvent *event) override;

 private:
  yotta::IoConfigPtr io_config_;  // io.json
  std::string module_id_;         //当前模块
  QButtonGroup btn_group_module_;
  QButtonGroup tab_btn_group_;

  std::vector<QLabel *> io_input_;
  std::vector<QLabel *> io_output_;
  std::vector<QLabel *> io_axis_;
  std::map<QLabel *, QLabel *> io_in_relax;
  std::map<QLabel *, QPushButton *> io_out_relax;
  std::map<QLabel *, QLabel *> io_axis_relax;

  // Cache for IO values to avoid unnecessary UI updates
  // Key: io_ids string, Value: cached state (0=offline, 1=off, 2=on)
  std::unordered_map<std::string, uint8_t> cached_io_values_;

 private:
  Ui::IoSettingviewClass *ui = nullptr;
  QTimer *time_monitor_ = nullptr;  //计时器,用来监控IO状态
};

#endif  // PROBE_STATION8_SRC_VIEW_TOOLS_IO_SETTING_VIEW_IO_SETTING_VIEW_H_
