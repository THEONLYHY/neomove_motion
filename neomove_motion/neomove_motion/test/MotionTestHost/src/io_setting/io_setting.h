#ifndef SRC_VIEW_PAGES_SYSTEM_SETTING_IO_SETTING_IO_SETTING_H_
#define SRC_VIEW_PAGES_SYSTEM_SETTING_IO_SETTING_IO_SETTING_H_

#include <config/io/axis_io_config.h>
#include <config/io/io_config.h>
#include <config/io/multi_io_port_config.h>

#include <QButtonGroup>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QWidget>
#include <unordered_map>

#include "config/config_factory.h"
#include "model/model_mgr.h"
#include "ui_io_setting.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class io_settingClass;
}
QT_END_NAMESPACE

class IoSetting : public QWidget {
  Q_OBJECT

 public:
  explicit IoSetting(QWidget* parent = nullptr);
  ~IoSetting() override;

  void showEvent(QShowEvent* event) override;
  void hideEvent(QHideEvent* event) override;

 private slots:
  void OnModuleClicked(int);
  void SlotIoClick(std::string io_ids, bool);  // io开关点击槽函数
  void OnSaveClicked();
  void UpdateTimer();  // 定时器执行函数

 private:
  void ShowModule();  // 显示模块
  bool eventFilter(QObject* watched, QEvent* event) override;
  void ReTranslate();

 private:
  yotta::IoConfigPtr io_config_;  // io.json
  std::string module_id_;         // 当前模块
  QButtonGroup btn_group_module_;

  std::vector<QLineEdit*> io_input_;
  std::vector<QLineEdit*> io_output_;
  std::vector<QLineEdit*> io_axis_;
  std::map<QLineEdit*, QPushButton*> io_in_relax;
  std::map<QLineEdit*, QPushButton*> io_out_relax;
  std::map<QLineEdit*, QPushButton*> io_axis_relax;

  // Cache for IO values to avoid unnecessary UI updates
  // Key: io_ids string, Value: cached state (0=offline, 1=off, 2=on)
  std::unordered_map<std::string, uint8_t> cached_io_values_;

 private:
  Ui::io_settingClass* ui_ = nullptr;
  QPointer<QTimer> time_monitor_;  // 计时器,用来监控IO状态
};

#endif  // SRC_VIEW_PAGES_SYSTEM_SETTING_IO_SETTING_IO_SETTING_H_
