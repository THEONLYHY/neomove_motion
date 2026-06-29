// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 15:27

#ifndef PROBE_STATION8_SRC_VIEW_TOOLS_POINT_SETTING_VIEW_POINT_SETTING_VIEW_H_
#define PROBE_STATION8_SRC_VIEW_TOOLS_POINT_SETTING_VIEW_POINT_SETTING_VIEW_H_

#include <config/point/point_config.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QPointer>
#include <QWidget>
#include <vector>

#include "config/config_factory.h"
#include "model/model_mgr.h"
#include "model/unit_info/unit_info_mgr.h"
#include "model/unit_info/unit_point_mgr.h"
#include "ui_point_setting_view.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class point_setting_viewClass;
};
QT_END_NAMESPACE

class PointSettingView : public QWidget {
  Q_OBJECT

 public:
  PointSettingView(QWidget* parent = nullptr);
  ~PointSettingView();

 private slots:
  void OnUnitClicked(int unit_index);
  void InitTitleTable();
  void InitPointListTable();
  void OnSetButtonClicked();     // 设置
  void OnGotoButtonClicked();    // 定点
  void OnPointSelect(int, int);  // 点击当前行
  void ToggleLockState();

  void SwitchPage();
  void AddPoint();
  void DeletePoint();

 protected:
  void showEvent(QShowEvent* event) override;  // 自动隐藏Point页面

 private:
  void GotoPoint();
  void ReadPoint(UnitPoint& point_para, int row);
  bool SavePointConfig(const std::string& point_ids, int row);
  // void AppendItemByRow(int row, const UnitPoint &point_para);  // 添加点位
  void AppendItemByRow(int row, std::string point_ids);  // 添加点位
  int GetRowCount(const std::string& unit_name);         // 获取模块已保存行数
  // int GetRowOffset(const std::string &unit_name);  //
  // 获取前几个模块的点位数量
  bool SavePoint(int row);  // 保存当前行点位
  void SetLockedState(bool locked);
  int RowFromButton(const std::vector<QPointer<QPushButton>>& buttons,
                    QObject* sender) const;
  std::string PointIdsAtRow(int row) const;
  bool HasCheckedAxis() const;
  void ClearAxisSelection();
  void SelectPointRow(int row);

 private:
  /* 清除axis_display
  QPointer<QDockWidget> dock_widget_axis_display_;
  */

 private:
  yotta::AxisConfigPtr axis_config_;    // axis.json
  yotta::PointConfigPtr point_config_;  // point.json
  UnitInfoMgrPtr unit_info_mgr_;        // unit.json
  UnitPointMgrPtr unit_point_mgr_;      // unit_point.json
  UnitInfo current_unit_info_;          // 当前单元
  std::string current_unit_ids_;        // 当前Unit名
  int current_unit_index_ = 0;          // 当前Unit序号
  UnitPoint current_point_;             // 当前行的点位数据
  QButtonGroup btn_group_unit_;
  int current_row_ = 0;                              // 当前行索引
  int axis_num_ = 0;                                 // 轴数列数
  std::vector<QString> axis_names_;                  // 第一行轴名称
  bool is_save_ = true;                              // 最新行是否保存
  bool point_page_ = true;                           // 图像和点位列表切换
  bool is_locked_ = true;                            // 当前点位操作是否锁定
  std::vector<QPointer<QCheckBox>> checkboxs_axis_;  // 轴选择框
  std::vector<QPointer<QPushButton>> buttons_set_;   // 设置按钮
  std::vector<QPointer<QPushButton>> buttons_goto_point_;  // 定点按钮
 private:
  Ui::point_setting_viewClass* ui;
  QSize button_size_;         // 上一行,下一行,定点,设置按钮大小
  QString qss_table_button_;  // 上一行,下一行,定点,设置按钮样式
  QString qss_unit_button_;   // Unit按钮样式
};

#endif  // PROBE_STATION8_SRC_VIEW_TOOLS_POINT_SETTING_VIEW_POINT_SETTING_VIEW_H_
