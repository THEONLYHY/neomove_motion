#ifndef BASE_UI_SRC_VIEW_PAGES_SYSTEM_SETTING_PAGE_SYSTEM_SETTING_PAGE_H_
#define BASE_UI_SRC_VIEW_PAGES_SYSTEM_SETTING_PAGE_SYSTEM_SETTING_PAGE_H_

#include <QButtonGroup>
#include <QPointer>
#include <QShowEvent>
#include <QWidget>

#include "ui_system_setting_page.h"
#include "view/tools/axis_setting_view/axis_setting_view.h"
#include "view/tools/error_list_view/error_list_view.h"
#include "view/tools/export_config_view/export_config_view.h"
#include "view/tools/io_setting_view/io_setting_view.h"
#include "view/tools/point_setting_view/point_setting_view.h"

class SystemSettingPage : public QWidget {
  Q_OBJECT

 public:
  explicit SystemSettingPage(QWidget* parent = nullptr);
  ~SystemSettingPage() override;

  void ReTranslate();

 protected:
  void showEvent(QShowEvent* event) override;

 private slots:
  void OnSetButtonClicked(int id);
  void FloatDockWidget(bool imageview_float, bool robotxyrz_float);

 private:
  QPointer<PointSettingView> point_setting_view_;
  QPointer<IoSettingView> io_setting_view_;
  QPointer<AxisSettingView> motion_axis_setting_;
  QPointer<ErrorListView> error_list_view_;
  QPointer<ExportConfigView> export_config_view_;

  QButtonGroup btn_group_;
  Ui::system_setting_pageClass* ui = nullptr;
};

#endif  // BASE_UI_SRC_VIEW_PAGES_SYSTEM_SETTING_PAGE_SYSTEM_SETTING_PAGE_H_
