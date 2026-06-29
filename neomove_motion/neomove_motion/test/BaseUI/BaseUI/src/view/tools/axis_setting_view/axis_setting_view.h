// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#ifndef PROBE_STATION8_SRC_VIEW_TOOLS_AXIS_SETTING_VIEW_AXIS_SETTING_VIEW_H_
#define PROBE_STATION8_SRC_VIEW_TOOLS_AXIS_SETTING_VIEW_AXIS_SETTING_VIEW_H_

#include <config/axis/axis_config.h>

#include <QButtonGroup>
#include <QPointer>
#include <QWidget>

#include "config/config_factory.h"
#include "model/model_mgr.h"
#include "ui_axis_setting_view.h"
#include "view/tools/limit_setting_view/limit_setting_view.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class AxisSettingViewClass;
};
QT_END_NAMESPACE

class AxisSettingView : public QWidget {
  Q_OBJECT

 public:
  AxisSettingView(QWidget *parent = nullptr);
  ~AxisSettingView();

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
  void ClearLayout(QLayout *layout);
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
  QPointer<LimitSettingView> limit_setting_view_;
  int speed_index_ = 0;
  int region_index_ = 0;
  int limit_index_ = 0;

 private:
  Ui::AxisSettingViewClass *ui;
};

#endif  // PROBE_STATION8_SRC_VIEW_TOOLS_AXIS_SETTING_VIEW_AXIS_SETTING_VIEW_H_
