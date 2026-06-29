// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/04/09 16:35

#ifndef PROBE_STATION8_SRC_VIEW_PAGES_PROBER_CONFIGURATION_SETTING_EXPORT_CONFIG_EXPORT_CONFIG_VIEW_H_
#define PROBE_STATION8_SRC_VIEW_PAGES_PROBER_CONFIGURATION_SETTING_EXPORT_CONFIG_EXPORT_CONFIG_VIEW_H_

#include <QStringList>
#include <QWidget>

#include "ui_export_config_view.h"

void HandlePendingConfigLoad();

class ExportConfigView : public QWidget {
  Q_OBJECT

 public:
  explicit ExportConfigView(QWidget *parent = nullptr);
  ~ExportConfigView() override;

 private slots:
  void OnLoadButtonClicked();
  void OnUploadButtonClicked();
  void OnExportLogButtonClicked();

 private:
  Ui::ExportConfigView *ui;
};

#endif  // PROBE_STATION8_SRC_VIEW_PAGES_PROBER_CONFIGURATION_SETTING_EXPORT_CONFIG_EXPORT_CONFIG_VIEW_H_
