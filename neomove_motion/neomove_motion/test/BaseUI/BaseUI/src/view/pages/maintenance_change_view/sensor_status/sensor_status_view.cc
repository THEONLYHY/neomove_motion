// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/07/15 19:50

#include "sensor_status_view.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLayoutItem>
#include <QLineEdit>

#include "config/io/axis_io_config.h"
#include "config/io/multi_analog_io_port_config.h"
#include "controller/device_status_monitor/device_status_monitor.h"
#include "view/components/ps_label/ps_label.h"
#include "view/components/ps_line_edit/ps_line_edit.h"
#include "view/components/ps_color/ps_color.h"

namespace {

void ClearLayout(QLayout* layout) {
  if (!layout) {
    return;
  }

  QLayoutItem* item = nullptr;
  while ((item = layout->takeAt(0)) != nullptr) {
    if (item->widget()) {
      item->widget()->deleteLater();
    } else if (item->layout()) {
      ClearLayout(item->layout());
      delete item->layout();
    }
    delete item;
  }
}

QWidget* CreateSensorRow(const QString& sensor_id, QLineEdit** value_edit,
                         QWidget* parent) {
  QWidget* row_widget = new QWidget(parent);
  QHBoxLayout* row_layout = new QHBoxLayout(row_widget);
  row_layout->setContentsMargins(0, 0, 0, 0);
  row_layout->setSpacing(0);

  QLabel* id_label = new PsLabel(sensor_id, row_widget);
  id_label->setFixedSize(170, 72);
  id_label->setAlignment(Qt::AlignCenter);
  id_label->setEnabled(false);

  QLineEdit* current_value_edit = new PsLineEdit(row_widget);
  current_value_edit->setFixedSize(85, 72);
  current_value_edit->setAlignment(Qt::AlignCenter);
  current_value_edit->setEnabled(false);
  current_value_edit->setReadOnly(true);
  current_value_edit->setText("--");

  row_layout->addWidget(id_label);
  row_layout->addWidget(current_value_edit);
  row_layout->addStretch();

  if (value_edit) {
    *value_edit = current_value_edit;
  }
  return row_widget;
}

void ApplySensorValueStyle(QLineEdit* value_edit, bool got_value, bool is_on) {
  auto* ps_edit = qobject_cast<PsLineEdit*>(value_edit);
  if (!ps_edit) return;

  if (!got_value) {
    ps_edit->Color().SetBaseColor(PsLineEdit::kDefaultColor);
  } else {
    ps_edit->Color().SetBaseColor(is_on ? PsColor::Color::kGreen
                                        : PsColor::Color::kRed);
  }
  ps_edit->update();
}

}  // namespace

SensorStatusView::SensorStatusView(QWidget* parent)
    : QWidget(parent), ui(new Ui::sensor_status_viewClass()) {
  ui->setupUi(this);
  InitSensor();

  connect(&timer_sensor_, &QTimer::timeout, this,
          &SensorStatusView::TimerSensor);
  timer_sensor_.setInterval(1000);
  timer_sensor_.start();
}

SensorStatusView::~SensorStatusView() { delete ui; }

void SensorStatusView::ReTranslate() { ui->retranslateUi(this); }

void SensorStatusView::TimerSensor() { UpdateSensorValues(); }

void SensorStatusView::InitSensor() {
  io_config_ = yotta::ConfigFactory::GetInstance()->GetIoConfig();

  UnitInfoMgrPtr unit_mgr = ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (unit_mgr && unit_mgr->GetUnitCount() > 0) {
    module_id_ = unit_mgr->GetUnitInfo(0)->unit_ids;
  }
  ShowModule();
}

void SensorStatusView::ShowModule() {
  UnitInfoMgrPtr unit_mgr = ModelMgrSinglton::GetInstance()->unit_info_mgr();
  if (!unit_mgr) {
    return;
  }

  UnitInfoPtr module_info = unit_mgr->GetUnitInfo(module_id_);
  if (!module_info) {
    return;
  }

  ClearLayout(ui->verticalLayout_left);
  ClearLayout(ui->verticalLayout_mid);
  ClearLayout(ui->verticalLayout_right);
  ClearLayout(ui->verticalLayout_analog);
  input_line_edits_.clear();
  input_sensor_ids_.clear();
  axis_line_edits_.clear();
  axis_sensor_ids_.clear();
  analog_line_edits_.clear();
  analog_sensor_ids_.clear();

  const bool split_input_columns = module_info->io_input_names.size() > 9;
  int current_col = 0;
  for (const std::string& io_name : module_info->io_input_names) {
    QString sensor_id = QString::fromStdString(io_name);
    QLineEdit* value_edit = nullptr;
    QWidget* row_widget = CreateSensorRow(sensor_id, &value_edit, this);
    if (split_input_columns) {
      if (current_col == 0) {
        ui->verticalLayout_left->addWidget(row_widget);
      } else {
        ui->verticalLayout_mid->addWidget(row_widget);
      }
      current_col = (current_col + 1) % 2;
    } else {
      ui->verticalLayout_left->addWidget(row_widget);
    }
    input_line_edits_.insert(sensor_id, value_edit);
    input_sensor_ids_.append(sensor_id);
  }
  ui->verticalLayout_left->addStretch();
  if (split_input_columns) {
    ui->verticalLayout_mid->addStretch();
  }

  // 轴传感器标题行
  QWidget* axis_header = new QWidget(this);
  QHBoxLayout* header_layout = new QHBoxLayout(axis_header);
  header_layout->setContentsMargins(0, 0, 0, 0);
  header_layout->setSpacing(0);

  QLabel* header_name = new PsLabel(tr("Axis"), axis_header);
  header_name->setFixedSize(170, 72);
  header_name->setAlignment(Qt::AlignCenter);
  header_layout->addWidget(header_name);

  for (const QString& h : {tr("limit+"), tr("limit-"), tr("home")}) {
    QLabel* header_label = new PsLabel(h, axis_header);
    header_label->setFixedSize(85, 72);
    header_label->setAlignment(Qt::AlignCenter);
    header_layout->addWidget(header_label);
  }
  ui->verticalLayout_right->addWidget(axis_header);

  // 按 unit.json 的 axis_ids 顺序遍历 (X, Y, Z, R)
  for (const auto& target_axis_id : module_info->axis_ids) {
    yotta::AxisIoConfigPtr matched_cfg = nullptr;
    for (int i = 0; i < io_config_->GetAxisIoCount(); ++i) {
      yotta::AxisIoConfigPtr cfg = io_config_->GetAxisIo(i);
      char buf[256] = {0};
      cfg->GetIds(buf, 256);
      if (target_axis_id == buf) {
        matched_cfg = cfg;
        break;
      }
    }
    if (!matched_cfg) {
      continue;
    }

    QWidget* axis_row = new QWidget(this);
    QHBoxLayout* axis_layout = new QHBoxLayout(axis_row);
    axis_layout->setContentsMargins(0, 0, 0, 0);
    axis_layout->setSpacing(0);

    QLabel* axis_label = new PsLabel(
        QString::fromStdString(target_axis_id), axis_row);
    axis_label->setFixedSize(170, 72);
    axis_label->setAlignment(Qt::AlignCenter);
    axis_label->setEnabled(false);
    axis_layout->addWidget(axis_label);

    for (int j = 0; j < matched_cfg->GetIoPortConfigCount(); ++j) {
      yotta::IoPortConfigPtr port = matched_cfg->GetIoPortConfig(j);
      char sensor_buf[256] = {0};
      port->GetIds(sensor_buf, 256);
      QString sensor_id = QString::fromUtf8(sensor_buf);

      QLineEdit* value_edit = new PsLineEdit(axis_row);
      value_edit->setFixedSize(85, 72);
      value_edit->setAlignment(Qt::AlignCenter);
      value_edit->setEnabled(false);
      value_edit->setReadOnly(true);
      value_edit->setText("--");

      axis_layout->addWidget(value_edit);

      axis_line_edits_.insert(sensor_id, value_edit);
      axis_sensor_ids_.append(sensor_id);
    }

    ui->verticalLayout_right->addWidget(axis_row);
  }
  ui->verticalLayout_right->addStretch();

  // 模拟量IO
  yotta::MultiAnalogIoPortConfigPtr analog_io_config = io_config_->GetAnalogIo();
  if (analog_io_config) {
    for (int i = 0; i < analog_io_config->GetAnalogIoPortConfigCount(); ++i) {
      yotta::AnalogIoPortConfigPtr port =
          analog_io_config->GetAnalogIoPortConfig(i);
      char id_buf[256] = {0};
      port->GetIds(id_buf, 256);
      QString sensor_id = QString::fromUtf8(id_buf);

      QLineEdit* value_edit = nullptr;
      QWidget* row_widget = CreateSensorRow(sensor_id, &value_edit, this);
      ui->verticalLayout_analog->addWidget(row_widget);

      analog_line_edits_.insert(sensor_id, value_edit);
      analog_sensor_ids_.append(sensor_id);
    }
  }
  ui->verticalLayout_analog->addStretch();

  UpdateSensorValues();
}

void SensorStatusView::UpdateSensorValues() {
  DeviceStatusMonitor* monitor = DeviceStatusMonitorSinglton::GetInstance();

  for (const QString& sensor_id : input_sensor_ids_) {
    QLineEdit* value_edit = input_line_edits_.value(sensor_id, nullptr);
    if (!value_edit) {
      continue;
    }

    uint8_t value = 0;
    bool got_value =
        monitor && monitor->GetInputIoValue(sensor_id.toStdString(), value);
    if (got_value) {
      value_edit->setText(value ? tr("ON") : tr("OFF"));
      ApplySensorValueStyle(value_edit, true, value != 0);
    } else {
      value_edit->setText(tr("--"));
      ApplySensorValueStyle(value_edit, false, false);
    }
  }

  for (const QString& sensor_id : axis_sensor_ids_) {
    QLineEdit* value_edit = axis_line_edits_.value(sensor_id, nullptr);
    if (!value_edit) {
      continue;
    }

    uint8_t value = 0;
    bool got_value =
        monitor && monitor->GetInputIoValue(sensor_id.toStdString(), value);
    if (got_value) {
      value_edit->setText(value ? tr("ON") : tr("OFF"));
      ApplySensorValueStyle(value_edit, true, value != 0);
    } else {
      value_edit->setText(tr("--"));
      ApplySensorValueStyle(value_edit, false, false);
    }
  }

  for (const QString& sensor_id : analog_sensor_ids_) {
    QLineEdit* value_edit = analog_line_edits_.value(sensor_id, nullptr);
    if (!value_edit) {
      continue;
    }

    double value = 0.0;
    bool got_value =
        monitor && monitor->GetAnalogIoValue(sensor_id.toStdString(), value);
    if (got_value) {
      value_edit->setText(QString::number(value, 'f', 2));
      auto* ps_edit = qobject_cast<PsLineEdit*>(value_edit);
      if (ps_edit) {
        ps_edit->Color().SetBaseColor(PsLineEdit::kDefaultColor);
        ps_edit->update();
      }
    } else {
      value_edit->setText(tr("--"));
      auto* ps_edit = qobject_cast<PsLineEdit*>(value_edit);
      if (ps_edit) {
        ps_edit->Color().SetBaseColor(PsLineEdit::kDefaultColor);
        ps_edit->update();
      }
    }
  }
}
