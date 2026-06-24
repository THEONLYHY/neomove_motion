#ifndef POINT_SETTING_POINT_SETTING_H_
#define POINT_SETTING_POINT_SETTING_H_

#include <config/point/point_config.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QWidget>

#include "config/config_factory.h"
#include "model/model_mgr.h"
#include "model/unit_info/unit_info_mgr.h"
#include "model/unit_info/unit_point_mgr.h"
#include "ui_point_setting.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class point_settingClass;
}
QT_END_NAMESPACE

class PointSetting : public QWidget {
  Q_OBJECT

  static constexpr int kUnitButtonWidth = 140;
  static constexpr int kUnitButtonHeight = 69;
  static constexpr int kTableButtonWidth = 80;
  static constexpr int kTableButtonHeight = 54;
  static constexpr int kPositionPrecision = 4;
  static constexpr int kAxisIdBufferSize = 256;

 public:
  PointSetting(QWidget *parent = nullptr);
  ~PointSetting();

 private slots:
  void OnUnitClicked(int unit_index);
  void OnSetButtonClicked();     // 设置
  void OnGotoButtonClicked();    // 定点
  void OnPointSelect(int, int);  // 点击当前行
  void SwitchPage();
  void AddPoint();
  void DeletePoint();

 protected:
  void showEvent(QShowEvent *event) override;

 private:
  void AddUnitButtons();
  void InitTitleTable();
  void InitPointListTable();
  void GotoPoint();
  void ReadPoint(UnitPoint &point_para, int row);
  void AppendItemByRow(int row, std::string point_ids);
  int GetRowCount(const std::string &unit_name);
  void SavePoint(int row);
  void InitUI();

 private:
  Ui::point_settingClass *ui;

 private:
  yotta::AxisConfigPtr axis_config_;
  yotta::PointConfigPtr point_config_;
  UnitInfoMgrPtr unit_info_mgr_;
  UnitPointMgrPtr unit_point_mgr_;
  UnitInfo current_unit_info_;
  std::string current_unit_ids_;
  int current_unit_index_ = 0;
  UnitPoint current_point_;
  QButtonGroup btn_group_unit_;
  int current_row_ = 0;
  int axis_num_ = 0;
  std::vector<QString> axis_names_;
  bool is_save_ = true;
  bool point_page_ = false;
  std::vector<QPointer<QCheckBox>> checkboxs_axis_;
  std::vector<QPointer<QPushButton>> buttons_set_;
  std::vector<QPointer<QPushButton>> buttons_goto_point_;
};

#endif  // POINT_SETTING_POINT_SETTING_H_