#ifndef SRC_VIEW_TOOLS_LIMIT_SETTING_VIEW_LIMIT_SETTING_VIEW_H_
#define SRC_VIEW_TOOLS_LIMIT_SETTING_VIEW_LIMIT_SETTING_VIEW_H_

#include <config/axis/axis_config.h>
#include <config/io/axis_io_config.h>
#include <config/io/io_config.h>
#include <config/io/multi_io_port_config.h>
#include <config/limit_rule/limit_rule_mgr_config.h>

#include <QComboBox>
#include <QDialog>

#include "config/config_factory.h"
#include "model/model_mgr.h"
#include "ui_limit_setting_view.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class limit_setting_viewClass;
}
QT_END_NAMESPACE

class LimitSettingView : public QDialog {
  Q_OBJECT

 public:
  LimitSettingView(QWidget* parent = nullptr);
  ~LimitSettingView();
  void SetCondition(int limit_type, std::string axis_ids, std::string io_ids);

 private slots:
  void ShowLimitCondition();
  void ReadLimitCondition();
  void OnLimitListSelect(int, int);

  void OnAddLimitButtonClicked();
  void OnDelLimitButtonClicked();
  void OnSaveLimitButtonClicked();

  void OnAddLimitAxisStatesButtonClicked();
  void OnDelLimitAxisStatesButtonClicked();

  void OnAddLimitAxisRegionStatesButtonClicked();
  void OnDelLimitAxisRegionStatesButtonClicked();

  void OnAddLimitIOStatesButtonClicked();
  void OnDelLimitIOStatesButtonClicked();

 private:
  void InitUI();
  QComboBox* GetAxisLimitTypeComboBox();
  QComboBox* GetIoLimitTypeComboBox();
  QComboBox* GetAxisNameComboBox();
  QComboBox* GetAxisRegionNameComboBox(std::string axis_name);
  QComboBox* GetIONameComboBox();
  QComboBox* GetAxisStatesComboBox();
  QComboBox* GetAxisRegionStatesComboBox();
  QComboBox* GetIOStatesComboBox();

 private:
  int limit_type_ = 0;  // 0:轴限位,1:IO限位
  std::string axis_ids_;
  std::string io_ids_;
  int limit_index_ = 0;  // 限位列表选中行数
  yotta::IoConfigPtr io_config_;           // io.json
  yotta::AxisConfigPtr axis_config_;       // axis.json
  yotta::LimitRuleMgrConfigPtr limit_rule_mgr_config_;  // limit.json

 private:
  Ui::limit_setting_viewClass* ui;
};

#endif  // SRC_VIEW_TOOLS_LIMIT_SETTING_VIEW_LIMIT_SETTING_VIEW_H_
