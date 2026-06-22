#ifndef SRC_VIEW_PAGES_SYSTEM_SETTING_AXIS_SETTING_AXIS_SETTING_H_
#define SRC_VIEW_PAGES_SYSTEM_SETTING_AXIS_SETTING_AXIS_SETTING_H_

#include <config/axis/axis_config.h>

#include <QButtonGroup>
#include <QPointer>
#include <QWidget>

#include "config/config_factory.h"
#include "model/model_mgr.h"
#include "ui_axis_setting.h"
#include "view/tools/limit_setting_view/limit_setting_view.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class axis_settingClass;
};
QT_END_NAMESPACE

class AxisSetting : public QWidget {
  Q_OBJECT

 public:
  AxisSetting(QWidget *parent = nullptr);
  ~AxisSetting();

  void ReTranslate();

 private slots:
  void OnModuleClicked(int);
  void OnAxisClicked(int id);
  void OnParaVelRegionLimitClicked(int id);

  //参数
  void OnParaSaveClicked();
  //速度
  void OnAddSpeedButtonClicked();
  void OnDelSpeedButtonClicked();
  void OnSaveSpeedButtonClicked();
  void OnSpeedListSelect(int, int);
  //区域
  void OnAddRegionButtonClicked();
  void OnDelRegionButtonClicked();
  void OnSaveRegionButtonClicked();
  void OnRegionListSelect(int, int);


 private:
  void InitUi();
  void ClearLayout(QLayout *layout);
  yotta::AxisConfigItemPtr GetCurrentAxisConfig() const;
  void ShowModule();
  void ShowModuleAxis();

 private:
  yotta::AxisConfigPtr axis_config_;
  std::string module_id_;
  std::string axis_ids_;
  std::vector<std::string> axis_id_list_;
  QButtonGroup btn_group_module_;
  QButtonGroup btn_group_axis_;
  QButtonGroup btn_group_region_limit_;
  QPointer<LimitSettingView> limit_setting_;
  int speed_index_ = 0;
  int region_index_ = 0;
  int limit_index_ = 0;

 private:
  Ui::axis_settingClass *ui_;
};

#endif  // SRC_VIEW_PAGES_SYSTEM_SETTING_AXIS_SETTING_AXIS_SETTING_H_
