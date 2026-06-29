#ifndef BASE_UI_SRC_VIEW_PAGES_START_PAGE_START_PAGE_H_
#define BASE_UI_SRC_VIEW_PAGES_START_PAGE_START_PAGE_H_

#include <QWidget>

#include "ui_start_page.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class StartPageClass;
};
QT_END_NAMESPACE

class StartPageView : public QWidget {
  Q_OBJECT

 public:
  explicit StartPageView(QWidget* parent = nullptr);
  ~StartPageView() override;
  void ReTranslate();

 signals:
  void SystemSettingRequested();
  void CalibrationRequested();
  void SigGoSecondPage();
  void SigInitEnd();
  void InitSucceeded();
  void InitFailed(int result);
  void InitTaskFinished(int result);

 public slots:
  void OnLoginSuccess();
  void OnProbeConfigClicked();
  void OnMaintenanceClicked();
  void OnMotionInitClicked();

 private slots:
  void OnInitTaskFinished(int result);

 protected:
  void showEvent(QShowEvent* event) override;

 private:
  Ui::StartPageClass* ui = nullptr;
};

#endif  // BASE_UI_SRC_VIEW_PAGES_START_PAGE_START_PAGE_H_
