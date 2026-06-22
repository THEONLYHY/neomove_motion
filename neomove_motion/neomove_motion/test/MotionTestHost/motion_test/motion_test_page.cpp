#include "motion_test_page.h"

#include <common/message_loop.h>
#include <main_process/module_mgr.h>
#include <motion/acc_dec_profile.h>

#include <QButtonGroup>
#include <QDateTime>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMetaObject>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>

#include <chrono>
#include <thread>

#include "config/config_factory.h"
#include "controller/device_status_monitor/device_status_monitor.h"

namespace {

constexpr int kTextBufferSize = 256;

QString ToQString(const std::string& value) {
  return QString::fromStdString(value);
}

std::string ReadAxisIds(const yotta::AxisConfigItemPtr& axis) {
  if (!axis) {
    return {};
  }
  char ids[kTextBufferSize] = {};
  axis->GetIds(ids, sizeof(ids));
  return ids;
}

std::string ReadSpeedIds(const yotta::AxisSpeedConfigPtr& speed) {
  if (!speed) {
    return {};
  }
  char ids[kTextBufferSize] = {};
  speed->GetIds(ids, sizeof(ids));
  return ids;
}

std::string ReadIoIds(const yotta::IoPortConfigPtr& io) {
  if (!io) {
    return {};
  }
  char ids[kTextBufferSize] = {};
  io->GetIds(ids, sizeof(ids));
  return ids;
}

QDoubleSpinBox* CreatePositionSpin(double value = 0.0) {
  auto* spin = new QDoubleSpinBox;
  spin->setDecimals(4);
  spin->setRange(-1000000.0, 1000000.0);
  spin->setSingleStep(1.0);
  spin->setValue(value);
  return spin;
}

}  // namespace

MotionTestPage::MotionTestPage(QWidget* parent) : QWidget(parent) {
  InitUi();
  LoadConfig();
  RefreshUnits();
  RefreshIo();
  RefreshStatus();

  status_timer_ = new QTimer(this);
  connect(status_timer_, &QTimer::timeout, this,
          [this]() { RefreshStatus(); });
  status_timer_->start(300);
}

MotionTestPage::~MotionTestPage() {
  if (status_timer_) {
    status_timer_->stop();
  }
}

void MotionTestPage::InitUi() {
  auto* root_layout = new QVBoxLayout(this);

  auto* top_group = new QGroupBox(QStringLiteral("设备选择"));
  auto* top_layout = new QGridLayout(top_group);
  unit_combo_ = new QComboBox;
  axis_combo_ = new QComboBox;
  speed_combo_ = new QComboBox;
  profile_combo_ = new QComboBox;
  profile_combo_->addItem(QStringLiteral("梯形"), static_cast<int>(
                                                yotta::ProfileType::kTrapezoidal));
  profile_combo_->addItem(QStringLiteral("S曲线"),
                          static_cast<int>(yotta::ProfileType::kSCurve));

  position_label_ = new QLabel(QStringLiteral("--"));
  axis_state_label_ = new QLabel(QStringLiteral("--"));
  home_state_label_ = new QLabel(QStringLiteral("--"));

  top_layout->addWidget(new QLabel(QStringLiteral("工位")), 0, 0);
  top_layout->addWidget(unit_combo_, 0, 1);
  top_layout->addWidget(new QLabel(QStringLiteral("轴")), 0, 2);
  top_layout->addWidget(axis_combo_, 0, 3);
  top_layout->addWidget(new QLabel(QStringLiteral("速度")), 0, 4);
  top_layout->addWidget(speed_combo_, 0, 5);
  top_layout->addWidget(new QLabel(QStringLiteral("曲线")), 0, 6);
  top_layout->addWidget(profile_combo_, 0, 7);
  top_layout->addWidget(new QLabel(QStringLiteral("位置")), 1, 0);
  top_layout->addWidget(position_label_, 1, 1);
  top_layout->addWidget(new QLabel(QStringLiteral("伺服状态")), 1, 2);
  top_layout->addWidget(axis_state_label_, 1, 3);
  top_layout->addWidget(new QLabel(QStringLiteral("回零状态")), 1, 4);
  top_layout->addWidget(home_state_label_, 1, 5);
  root_layout->addWidget(top_group);

  connect(unit_combo_, qOverload<int>(&QComboBox::currentIndexChanged), this,
          [this]() { RefreshAxes(); });
  connect(axis_combo_, qOverload<int>(&QComboBox::currentIndexChanged), this,
          [this]() {
            RefreshSpeeds();
            RefreshStatus();
          });

  auto* action_layout = new QGridLayout;

  auto* jog_group = new QGroupBox(QStringLiteral("JOG 点动"));
  auto* jog_layout = new QFormLayout(jog_group);
  jog_velocity_spin_ = CreatePositionSpin(1.0);
  jog_velocity_spin_->setRange(0.0001, 1000000.0);
  auto* jog_button_layout = new QHBoxLayout;
  auto* jog_negative = new QPushButton(QStringLiteral("负向按住"));
  auto* jog_positive = new QPushButton(QStringLiteral("正向按住"));
  auto* stop_button = new QPushButton(QStringLiteral("停止"));
  jog_button_layout->addWidget(jog_negative);
  jog_button_layout->addWidget(jog_positive);
  jog_button_layout->addWidget(stop_button);
  jog_layout->addRow(QStringLiteral("JOG速度覆盖"), jog_velocity_spin_);
  jog_layout->addRow(jog_button_layout);
  action_layout->addWidget(jog_group, 0, 0);

  connect(jog_negative, &QPushButton::pressed, this,
          [this]() { StartJog(false); });
  connect(jog_positive, &QPushButton::pressed, this,
          [this]() { StartJog(true); });
  connect(jog_negative, &QPushButton::released, this,
          [this]() { StopAxis(); });
  connect(jog_positive, &QPushButton::released, this,
          [this]() { StopAxis(); });
  connect(stop_button, &QPushButton::clicked, this, [this]() { StopAxis(); });

  auto* point_group = new QGroupBox(QStringLiteral("定点运动"));
  auto* point_layout = new QFormLayout(point_group);
  absolute_radio_ = new QRadioButton(QStringLiteral("绝对"));
  relative_radio_ = new QRadioButton(QStringLiteral("相对"));
  absolute_radio_->setChecked(true);
  auto* mode_layout = new QHBoxLayout;
  mode_layout->addWidget(absolute_radio_);
  mode_layout->addWidget(relative_radio_);
  target_position_spin_ = CreatePositionSpin();
  auto* move_button = new QPushButton(QStringLiteral("执行定点运动"));
  point_layout->addRow(QStringLiteral("模式"), mode_layout);
  point_layout->addRow(QStringLiteral("目标/距离"), target_position_spin_);
  point_layout->addRow(move_button);
  action_layout->addWidget(point_group, 0, 1);
  connect(move_button, &QPushButton::clicked, this, [this]() { MovePoint(); });

  auto* home_group = new QGroupBox(QStringLiteral("回零"));
  auto* home_layout = new QVBoxLayout(home_group);
  auto* home_axis_button = new QPushButton(QStringLiteral("当前轴回零"));
  auto* home_all_button = new QPushButton(QStringLiteral("当前工位全部轴回零"));
  home_layout->addWidget(home_axis_button);
  home_layout->addWidget(home_all_button);
  action_layout->addWidget(home_group, 0, 2);
  connect(home_axis_button, &QPushButton::clicked, this,
          [this]() { HomeAxis(); });
  connect(home_all_button, &QPushButton::clicked, this,
          [this]() { HomeAllAxes(); });

  auto* sequence_group = new QGroupBox(QStringLiteral("不同速度运动"));
  auto* sequence_layout = new QGridLayout(sequence_group);
  sequence_speed_1_ = new QComboBox;
  sequence_speed_2_ = new QComboBox;
  sequence_pos_1_ = CreatePositionSpin();
  sequence_pos_2_ = CreatePositionSpin();
  auto* sequence_button = new QPushButton(QStringLiteral("运行两段连续运动"));
  sequence_layout->addWidget(new QLabel(QStringLiteral("第1段速度")), 0, 0);
  sequence_layout->addWidget(sequence_speed_1_, 0, 1);
  sequence_layout->addWidget(new QLabel(QStringLiteral("第1段目标")), 0, 2);
  sequence_layout->addWidget(sequence_pos_1_, 0, 3);
  sequence_layout->addWidget(new QLabel(QStringLiteral("第2段速度")), 1, 0);
  sequence_layout->addWidget(sequence_speed_2_, 1, 1);
  sequence_layout->addWidget(new QLabel(QStringLiteral("第2段目标")), 1, 2);
  sequence_layout->addWidget(sequence_pos_2_, 1, 3);
  sequence_layout->addWidget(sequence_button, 2, 0, 1, 4);
  action_layout->addWidget(sequence_group, 1, 0, 1, 2);
  connect(sequence_button, &QPushButton::clicked, this,
          [this]() { RunTwoStepMove(); });

  auto* io_group = new QGroupBox(QStringLiteral("IO 操作 / 联动"));
  auto* io_layout = new QGridLayout(io_group);
  di_combo_ = new QComboBox;
  di_state_combo_ = new QComboBox;
  di_state_combo_->addItem(QStringLiteral("0"), 0);
  di_state_combo_->addItem(QStringLiteral("1"), 1);
  do_combo_ = new QComboBox;
  do_state_combo_ = new QComboBox;
  do_state_combo_->addItem(QStringLiteral("0"), 0);
  do_state_combo_->addItem(QStringLiteral("1"), 1);
  link_target_spin_ = CreatePositionSpin();
  auto* set_do_button = new QPushButton(QStringLiteral("写 DO"));
  auto* link_button = new QPushButton(QStringLiteral("等待DI->运动->写DO"));
  io_layout->addWidget(new QLabel(QStringLiteral("DI")), 0, 0);
  io_layout->addWidget(di_combo_, 0, 1);
  io_layout->addWidget(new QLabel(QStringLiteral("DI目标")), 0, 2);
  io_layout->addWidget(di_state_combo_, 0, 3);
  io_layout->addWidget(new QLabel(QStringLiteral("DO")), 1, 0);
  io_layout->addWidget(do_combo_, 1, 1);
  io_layout->addWidget(new QLabel(QStringLiteral("DO目标")), 1, 2);
  io_layout->addWidget(do_state_combo_, 1, 3);
  io_layout->addWidget(new QLabel(QStringLiteral("联动目标位置")), 2, 0);
  io_layout->addWidget(link_target_spin_, 2, 1);
  io_layout->addWidget(set_do_button, 3, 0, 1, 2);
  io_layout->addWidget(link_button, 3, 2, 1, 2);
  action_layout->addWidget(io_group, 1, 2);
  connect(set_do_button, &QPushButton::clicked, this,
          [this]() { SetOutputIo(); });
  connect(link_button, &QPushButton::clicked, this,
          [this]() { RunDiMoveDo(); });

  root_layout->addLayout(action_layout);

  log_edit_ = new QPlainTextEdit;
  log_edit_->setReadOnly(true);
  log_edit_->setMaximumBlockCount(300);
  root_layout->addWidget(log_edit_, 1);
}

void MotionTestPage::LoadConfig() {
  axis_config_ = yotta::ConfigFactory::GetInstance()->GetAxisConfig();
  io_config_ = yotta::ConfigFactory::GetInstance()->GetIoConfig();
  unit_info_mgr_ = ModelMgrSinglton::GetInstance()->unit_info_mgr();

  if (!axis_config_) {
    AppendLog(QStringLiteral("axis_config 为空，请检查 config/axis.json 是否加载"));
  }
  if (!io_config_) {
    AppendLog(QStringLiteral("io_config 为空，请检查 config/io.json 是否加载"));
  }
  if (!unit_info_mgr_) {
    AppendLog(QStringLiteral("unit_info_mgr 为空，请检查 ModelMgr 初始化和 config/unit.json"));
  }
}

void MotionTestPage::RefreshUnits() {
  unit_combo_->clear();
  if (unit_info_mgr_) {
    for (int i = 0; i < unit_info_mgr_->GetUnitCount(); ++i) {
      auto unit = unit_info_mgr_->GetUnitInfo(i);
      if (unit) {
        unit_combo_->addItem(ToQString(unit->unit_ids));
      }
    }
  }
  if (unit_combo_->count() == 0) {
    unit_combo_->addItem(QStringLiteral("全部轴"));
  }
  RefreshAxes();
}

void MotionTestPage::RefreshAxes() {
  axis_combo_->clear();

  if (unit_info_mgr_) {
    auto unit = unit_info_mgr_->GetUnitInfo(unit_combo_->currentText().toStdString());
    if (unit) {
      for (const auto& axis_id : unit->axis_ids) {
        axis_combo_->addItem(ToQString(axis_id));
      }
    }
  }

  if (axis_combo_->count() == 0 && axis_config_) {
    for (int i = 0; i < axis_config_->GetAxisCount(); ++i) {
      auto axis = axis_config_->GetAxis(i);
      std::string axis_id = ReadAxisIds(axis);
      if (!axis_id.empty()) {
        axis_combo_->addItem(ToQString(axis_id));
      }
    }
  }

  RefreshSpeeds();
}

void MotionTestPage::RefreshSpeeds() {
  const QString current_speed = speed_combo_->currentText();
  speed_combo_->clear();
  sequence_speed_1_->clear();
  sequence_speed_2_->clear();

  auto axis = CurrentAxisConfig();
  if (axis) {
    for (int i = 0; i < axis->GetSpeedCount(); ++i) {
      std::string speed_id = ReadSpeedIds(axis->GetSpeed(i));
      if (!speed_id.empty()) {
        speed_combo_->addItem(ToQString(speed_id));
        sequence_speed_1_->addItem(ToQString(speed_id));
        sequence_speed_2_->addItem(ToQString(speed_id));
      }
    }
  }

  int index = speed_combo_->findText(current_speed);
  if (index >= 0) {
    speed_combo_->setCurrentIndex(index);
  }
}

void MotionTestPage::RefreshIo() {
  di_combo_->clear();
  do_combo_->clear();
  if (!io_config_) {
    return;
  }

  auto input = io_config_->GetInputIo();
  if (input) {
    for (int i = 0; i < input->GetIoPortConfigCount(); ++i) {
      std::string io_id = ReadIoIds(input->GetIoPortConfig(i));
      if (!io_id.empty()) {
        di_combo_->addItem(ToQString(io_id));
      }
    }
  }

  auto output = io_config_->GetOutputIo();
  if (output) {
    for (int i = 0; i < output->GetIoPortConfigCount(); ++i) {
      std::string io_id = ReadIoIds(output->GetIoPortConfig(i));
      if (!io_id.empty()) {
        do_combo_->addItem(ToQString(io_id));
      }
    }
  }
}

void MotionTestPage::RefreshStatus() {
  std::string axis_id = CurrentAxisId();
  if (axis_id.empty()) {
    position_label_->setText(QStringLiteral("--"));
    axis_state_label_->setText(QStringLiteral("--"));
    home_state_label_->setText(QStringLiteral("--"));
    return;
  }

  double pos = 0.0;
  if (DeviceStatusMonitorSinglton::GetInstance()->GetAxisPos(axis_id, pos)) {
    position_label_->setText(QString::number(pos, 'f', 4));
  } else {
    position_label_->setText(QStringLiteral("N/A"));
  }

  int axis_state = 0;
  if (DeviceStatusMonitorSinglton::GetInstance()->GetAxisState(axis_id,
                                                               axis_state)) {
    axis_state_label_->setText(QString::number(axis_state));
  } else {
    axis_state_label_->setText(QStringLiteral("N/A"));
  }

  int home_state = 0;
  if (DeviceStatusMonitorSinglton::GetInstance()->GetAxisHomeState(axis_id,
                                                                   home_state)) {
    home_state_label_->setText(QString::number(home_state));
  } else {
    home_state_label_->setText(QStringLiteral("N/A"));
  }
}

void MotionTestPage::StartJog(bool positive) {
  const std::string axis_id = CurrentAxisId();
  const std::string speed_id = CurrentSpeedId();
  const auto profile_type = static_cast<yotta::ProfileType>(
      profile_combo_->currentData().toInt());
  const double jog_velocity = jog_velocity_spin_->value();

  RunMotionTask(positive ? QStringLiteral("JOG 正向")
                         : QStringLiteral("JOG 负向"),
                [this, axis_id, speed_id, profile_type, jog_velocity,
                 positive]() {
                  yotta::Axis* axis = GetAxis(axis_id);
                  if (!axis) {
                    return;
                  }
                  yotta::AccDecProfileImpl profile(0);
                  if (!BuildProfile(axis_id, speed_id, profile_type, &profile)) {
                    return;
                  }
                  profile.set_velocity(jog_velocity);
                  int ret = axis->StartJog(&profile, positive);
                  AppendLogFromAnyThread(QStringLiteral("StartJog ret=%1").arg(ret));
                });
}

void MotionTestPage::StopAxis() {
  const std::string axis_id = CurrentAxisId();
  RunMotionTask(QStringLiteral("停止当前轴"), [this, axis_id]() {
    yotta::Axis* axis = GetAxis(axis_id);
    if (!axis) {
      return;
    }
    axis->Stop();
    AppendLogFromAnyThread(QStringLiteral("Stop 已下发"));
  });
}

void MotionTestPage::MovePoint() {
  const std::string axis_id = CurrentAxisId();
  const std::string speed_id = CurrentSpeedId();
  const auto profile_type = static_cast<yotta::ProfileType>(
      profile_combo_->currentData().toInt());
  const bool absolute = absolute_radio_->isChecked();
  const double input_pos = target_position_spin_->value();

  RunMotionTask(QStringLiteral("定点运动"), [this, axis_id, speed_id,
                                          profile_type, absolute, input_pos]() {
    yotta::Axis* axis = GetAxis(axis_id);
    if (!axis) {
      return;
    }
    double target = input_pos;
    if (!absolute) {
      double current = 0.0;
      if (axis->GetActualPosition(&current) != 0) {
        AppendLogFromAnyThread(QStringLiteral("读取当前位置失败"));
        return;
      }
      target = current + input_pos;
    }
    yotta::AccDecProfileImpl profile(0);
    if (!BuildProfile(axis_id, speed_id, profile_type, &profile)) {
      return;
    }
    int ret = axis->AsyncMoveTo(target, &profile);
    if (ret == 0) {
      ret = axis->Wait();
    }
    AppendLogFromAnyThread(QStringLiteral("定点运动结束 ret=%1 target=%2")
                               .arg(ret)
                               .arg(target));
  });
}

void MotionTestPage::HomeAxis() {
  const std::string axis_id = CurrentAxisId();
  RunMotionTask(QStringLiteral("当前轴回零"), [this, axis_id]() {
    yotta::Axis* axis = GetAxis(axis_id);
    if (!axis) {
      return;
    }
    int ret = axis->Home();
    AppendLogFromAnyThread(QStringLiteral("当前轴回零 ret=%1").arg(ret));
  });
}

void MotionTestPage::HomeAllAxes() {
  const std::vector<std::string> axis_ids = CurrentUnitAxisIds();
  RunMotionTask(QStringLiteral("当前工位全部轴回零"), [this, axis_ids]() {
    for (const auto& axis_id : axis_ids) {
      yotta::Axis* axis = GetAxis(axis_id);
      if (!axis) {
        continue;
      }
      int ret = axis->Home();
      AppendLogFromAnyThread(
          QStringLiteral("%1 回零 ret=%2").arg(ToQString(axis_id)).arg(ret));
      if (ret != 0) {
        break;
      }
    }
  });
}

void MotionTestPage::RunTwoStepMove() {
  const std::string axis_id = CurrentAxisId();
  const auto profile_type = static_cast<yotta::ProfileType>(
      profile_combo_->currentData().toInt());
  const std::string speed_1 = sequence_speed_1_->currentText().toStdString();
  const std::string speed_2 = sequence_speed_2_->currentText().toStdString();
  const double pos_1 = sequence_pos_1_->value();
  const double pos_2 = sequence_pos_2_->value();

  RunMotionTask(QStringLiteral("两段连续运动"),
                [this, axis_id, profile_type, speed_1, speed_2, pos_1, pos_2]() {
                  yotta::Axis* axis = GetAxis(axis_id);
                  if (!axis) {
                    return;
                  }
                  yotta::AccDecProfileImpl profile_1(0);
                  if (!BuildProfile(axis_id, speed_1, profile_type, &profile_1)) {
                    return;
                  }
                  int ret = axis->AsyncMoveTo(pos_1, &profile_1);
                  if (ret == 0) {
                    ret = axis->Wait();
                  }
                  AppendLogFromAnyThread(QStringLiteral("第1段 ret=%1").arg(ret));
                  if (ret != 0) {
                    return;
                  }

                  yotta::AccDecProfileImpl profile_2(0);
                  if (!BuildProfile(axis_id, speed_2, profile_type, &profile_2)) {
                    return;
                  }
                  ret = axis->AsyncMoveTo(pos_2, &profile_2);
                  if (ret == 0) {
                    ret = axis->Wait();
                  }
                  AppendLogFromAnyThread(QStringLiteral("第2段 ret=%1").arg(ret));
                });
}

void MotionTestPage::SetOutputIo() {
  const std::string io_id = do_combo_->currentText().toStdString();
  const bool state = do_state_combo_->currentData().toInt() != 0;
  yotta::LimitMotionMgrPtr limit_motion = GetLimitMotionMgr();
  if (!limit_motion) {
    return;
  }
  yotta::OutputIO* output_io = limit_motion->GetOutputIoByIds(io_id.c_str());
  if (!output_io) {
    AppendLog(QStringLiteral("未获取到 DO: %1").arg(ToQString(io_id)));
    return;
  }
  int ret = output_io->WriteValue(state);
  AppendLog(QStringLiteral("写 DO %1=%2 ret=%3")
                .arg(ToQString(io_id))
                .arg(state ? 1 : 0)
                .arg(ret));
}

void MotionTestPage::RunDiMoveDo() {
  const std::string axis_id = CurrentAxisId();
  const std::string speed_id = CurrentSpeedId();
  const auto profile_type = static_cast<yotta::ProfileType>(
      profile_combo_->currentData().toInt());
  const std::string di_id = di_combo_->currentText().toStdString();
  const uint8_t di_target = static_cast<uint8_t>(di_state_combo_->currentData().toInt());
  const std::string do_id = do_combo_->currentText().toStdString();
  const bool do_target = do_state_combo_->currentData().toInt() != 0;
  const double target = link_target_spin_->value();

  RunMotionTask(QStringLiteral("DI触发运动+DO完成"),
                [this, axis_id, speed_id, profile_type, di_id, di_target, do_id,
                 do_target, target]() {
                  yotta::LimitMotionMgrPtr limit_motion = GetLimitMotionMgr();
                  if (!limit_motion) {
                    return;
                  }
                  yotta::InputIO* input_io =
                      limit_motion->GetInputIoByIds(di_id.c_str());
                  yotta::OutputIO* output_io =
                      limit_motion->GetOutputIoByIds(do_id.c_str());
                  yotta::Axis* axis = limit_motion->GetAxisByIds(axis_id.c_str());
                  if (!input_io || !output_io || !axis) {
                    AppendLogFromAnyThread(QStringLiteral("DI/DO/轴对象获取失败"));
                    return;
                  }

                  bool triggered = false;
                  for (int i = 0; i < 100; ++i) {
                    uint8_t value = 0;
                    if (input_io->ReadValue(&value) == 0 && value == di_target) {
                      triggered = true;
                      break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                  }
                  if (!triggered) {
                    AppendLogFromAnyThread(QStringLiteral("等待 DI 超时"));
                    return;
                  }

                  yotta::AccDecProfileImpl profile(0);
                  if (!BuildProfile(axis_id, speed_id, profile_type, &profile)) {
                    return;
                  }
                  int ret = axis->AsyncMoveTo(target, &profile);
                  if (ret == 0) {
                    ret = axis->Wait();
                  }
                  if (ret == 0) {
                    ret = output_io->WriteValue(do_target);
                  }
                  AppendLogFromAnyThread(
                      QStringLiteral("联动流程结束 ret=%1").arg(ret));
                });
}

std::string MotionTestPage::CurrentAxisId() const {
  return axis_combo_->currentText().toStdString();
}

std::string MotionTestPage::CurrentSpeedId() const {
  return speed_combo_->currentText().toStdString();
}

std::vector<std::string> MotionTestPage::CurrentUnitAxisIds() const {
  std::vector<std::string> axis_ids;
  if (unit_info_mgr_) {
    auto unit = unit_info_mgr_->GetUnitInfo(unit_combo_->currentText().toStdString());
    if (unit) {
      axis_ids = unit->axis_ids;
    }
  }
  if (axis_ids.empty()) {
    std::string axis_id = CurrentAxisId();
    if (!axis_id.empty()) {
      axis_ids.push_back(axis_id);
    }
  }
  return axis_ids;
}

yotta::AxisConfigItemPtr MotionTestPage::CurrentAxisConfig() const {
  if (!axis_config_) {
    return nullptr;
  }
  std::string axis_id = CurrentAxisId();
  if (axis_id.empty()) {
    return nullptr;
  }
  return axis_config_->GetAxisByIds(axis_id.c_str());
}

bool MotionTestPage::BuildProfile(const std::string& axis_id,
                                  const std::string& speed_id,
                                  yotta::ProfileType profile_type,
                                  yotta::AccDecProfileImpl* profile) {
  if (!axis_config_ || !profile) {
    AppendLogFromAnyThread(QStringLiteral("速度配置不可用"));
    return false;
  }
  auto axis = axis_config_->GetAxisByIds(axis_id.c_str());
  if (!axis) {
    AppendLogFromAnyThread(QStringLiteral("未找到轴配置: %1").arg(ToQString(axis_id)));
    return false;
  }
  auto speed = speed_id.empty() ? axis->GetDefaultSpeed()
                                : axis->GetSpeedByIds(speed_id.c_str());
  if (!speed) {
    AppendLogFromAnyThread(
        QStringLiteral("未找到速度配置: %1").arg(ToQString(speed_id)));
    return false;
  }

  double value = 0.0;
  speed->GetVelocity(&value);
  profile->set_velocity(value);
  speed->GetAcc(&value);
  profile->set_acceleration(value);
  speed->GetDec(&value);
  profile->set_deceleration(value);
  profile->set_type(profile_type);
  return true;
}

yotta::LimitMotionMgrPtr MotionTestPage::GetLimitMotionMgr() {
  yotta::LimitMotionMgrPtr limit_motion =
      main_process::GetModuleMgr()->GetLimitMotionMgr();
  if (!limit_motion) {
    AppendLogFromAnyThread(
        QStringLiteral("LimitMotionMgr 为空：测试项目当前未初始化运动硬件"));
  }
  return limit_motion;
}

yotta::Axis* MotionTestPage::GetAxis(const std::string& axis_id) {
  yotta::LimitMotionMgrPtr limit_motion = GetLimitMotionMgr();
  if (!limit_motion) {
    return nullptr;
  }
  yotta::Axis* axis = limit_motion->GetAxisByIds(axis_id.c_str());
  if (!axis) {
    AppendLogFromAnyThread(QStringLiteral("未获取到轴对象: %1").arg(ToQString(axis_id)));
  }
  return axis;
}

void MotionTestPage::RunMotionTask(const QString& name,
                                   std::function<void()> task) {
  auto loop = common::MessageLoop::GetMessageLoop(common::kMotion);
  if (!loop) {
    AppendLog(QStringLiteral("%1 未执行：MOTION MessageLoop 未初始化").arg(name));
    return;
  }

  QPointer<MotionTestPage> self(this);
  loop->PostTask([self, name, task]() {
    if (!self) {
      return;
    }
    self->AppendLogFromAnyThread(QStringLiteral("开始: %1").arg(name));
    task();
    self->AppendLogFromAnyThread(QStringLiteral("结束: %1").arg(name));
  });
}

void MotionTestPage::AppendLog(const QString& message) {
  const QString line =
      QStringLiteral("[%1] %2")
          .arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz")))
          .arg(message);
  log_edit_->appendPlainText(line);
}

void MotionTestPage::AppendLogFromAnyThread(const QString& message) {
  QPointer<MotionTestPage> self(this);
  QMetaObject::invokeMethod(this, [self, message]() {
    if (self) {
      self->AppendLog(message);
    }
  });
}
