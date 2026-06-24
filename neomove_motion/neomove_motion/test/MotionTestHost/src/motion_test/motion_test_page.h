#pragma once

#include <config/axis/axis_config.h>
#include <config/io/io_config.h>
#include <limit_motion/impl/acc_dec_profile_impl.h>
#include <limit_motion/limit_motion_mgr.h>
#include <motion/axis.h>

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QTimer>
#include <QWidget>

#include <functional>
#include <string>
#include <vector>

#include "model/model_mgr.h"

class MotionTestPage : public QWidget {
 public:
  explicit MotionTestPage(QWidget* parent = nullptr);
  ~MotionTestPage() override;

 private:
  void InitUi();
  void LoadConfig();
  void RefreshUnits();
  void RefreshAxes();
  void RefreshSpeeds();
  void RefreshIo();
  void RefreshStatus();

  void StartJog(bool positive);
  void StopAxis();
  void MovePoint();
  void HomeAxis();
  void HomeAllAxes();
  void RunTwoStepMove();
  void SetOutputIo();
  void RunDiMoveDo();

  std::string CurrentAxisId() const;
  std::string CurrentSpeedId() const;
  std::vector<std::string> CurrentUnitAxisIds() const;
  yotta::AxisConfigItemPtr CurrentAxisConfig() const;
  bool BuildProfile(const std::string& axis_id, const std::string& speed_id,
                    yotta::ProfileType profile_type,
                    yotta::AccDecProfileImpl* profile);
  yotta::LimitMotionMgrPtr GetLimitMotionMgr();
  yotta::Axis* GetAxis(const std::string& axis_id);

  void RunMotionTask(const QString& name, std::function<void()> task);
  void AppendLog(const QString& message);
  void AppendLogFromAnyThread(const QString& message);

 private:
  yotta::AxisConfigPtr axis_config_;
  yotta::IoConfigPtr io_config_;
  UnitInfoMgrPtr unit_info_mgr_;

  QComboBox* unit_combo_ = nullptr;
  QComboBox* axis_combo_ = nullptr;
  QComboBox* speed_combo_ = nullptr;
  QComboBox* profile_combo_ = nullptr;

  QLabel* position_label_ = nullptr;
  QLabel* axis_state_label_ = nullptr;
  QLabel* home_state_label_ = nullptr;

  QRadioButton* absolute_radio_ = nullptr;
  QRadioButton* relative_radio_ = nullptr;
  QDoubleSpinBox* target_position_spin_ = nullptr;
  QDoubleSpinBox* jog_velocity_spin_ = nullptr;

  QComboBox* sequence_speed_1_ = nullptr;
  QComboBox* sequence_speed_2_ = nullptr;
  QDoubleSpinBox* sequence_pos_1_ = nullptr;
  QDoubleSpinBox* sequence_pos_2_ = nullptr;

  QComboBox* di_combo_ = nullptr;
  QComboBox* di_state_combo_ = nullptr;
  QComboBox* do_combo_ = nullptr;
  QComboBox* do_state_combo_ = nullptr;
  QDoubleSpinBox* link_target_spin_ = nullptr;

  QPlainTextEdit* log_edit_ = nullptr;
  QTimer* status_timer_ = nullptr;
};
