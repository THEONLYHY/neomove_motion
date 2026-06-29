#ifndef BASE_UI_SRC_VIEW_MAIN_WINDOW_BASE_MAIN_WINDOW_H_
#define BASE_UI_SRC_VIEW_MAIN_WINDOW_BASE_MAIN_WINDOW_H_

#include <QDialog>
#include <QPointer>
#include <QStackedWidget>
#include <QWidget>

#include "controller/algorithm_process/algorithm_process_callback_impl.h"
#include "ui_base_main_window.h"
#include "view/components/ps_button/ps_button.h"
#include "view/pages/main_page/main_page.h"
#include "view/pages/start_page/start_page.h"
#include "view/pages/system_setting_page/system_setting_page.h"
#include "view/pages/title_bar_view/title_bar_view.h"

class BaseMainWindow : public QDialog {
  Q_OBJECT

 public:
  explicit BaseMainWindow(QWidget* parent = nullptr);
  ~BaseMainWindow() override;
  BaseMainWindow(const BaseMainWindow&) = delete;
  BaseMainWindow& operator=(const BaseMainWindow&) = delete;

 private slots:
  void OnSystemSettingClicked();
  void OnCalibrationClicked();
  void OnProbeInitEnd();
  void OnStopActionClicked();
  void OnBackPageClicked();

 private:
  bool InitModuleMgr();
  void InitPage();
  void FloatImageView(bool is_float, bool show = true);
  void FloatRobotXYRZView(bool is_float, bool show = true);

  Ui::BaseMainWindowClass* ui_ = nullptr;
  TitleBarView* title_bar_ = nullptr;
  QStackedWidget* main_stack_ = nullptr;
  PsButton* system_setting_button_ = nullptr;
  PsButton* stop_action_ = nullptr;
  PsButton* back_page_ = nullptr;
  QWidget* last_widget_ = nullptr;
  bool module_mgr_initialized_ = false;
  bool motion_init_complete_ = false;
  QPointer<StartPageView> start_page_;
  QPointer<SystemSettingPage> system_setting_page_;
  QPointer<QWidget> calibration_view_;
  QPointer<MainPage> main_page_;
  AlgorithmProcessCallbackImplPtr alg_process_callback_{
      new AlgorithmProcessCallbackImpl()};  // 算法
};

#endif  // BASE_UI_SRC_VIEW_MAIN_WINDOW_BASE_MAIN_WINDOW_H_
