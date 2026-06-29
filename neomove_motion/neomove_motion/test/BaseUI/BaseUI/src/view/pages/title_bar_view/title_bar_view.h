#ifndef BASE_UI_SRC_VIEW_PAGES_TITLE_BAR_VIEW_TITLE_BAR_VIEW_H_
#define BASE_UI_SRC_VIEW_PAGES_TITLE_BAR_VIEW_TITLE_BAR_VIEW_H_

#include <QColor>
#include <QHBoxLayout>
#include <QMap>
#include <QPixmap>
#include <QTimer>
#include <QVector>
#include <QWidget>

#include "model/user_manage/user_manage.h"
#include "ui_title_bar_view.h"
#include "view/components/ps_button/ps_button.h"
#include "view/components/ps_label/ps_label.h"
#include "view/tools/alarm_lamp_view/alarm_lamp_view.h"
#include "view/tools/log_view/log_view.h"
#include "view/tools/login_dialog/login_dialog.h"

struct TitleBarStatusItem {
  QString id;
  QString name;
  QString value;
  QColor color = QColor("#3B82F6");
  int width = 160;
  bool visible = true;
};

class TitleBarView : public QWidget {
  Q_OBJECT

 public:
  explicit TitleBarView(QWidget* parent = nullptr);
  ~TitleBarView() override;

  void ReTranslate();
  void SetStatusItems(const QVector<TitleBarStatusItem>& items);
  void UpdateStatusValue(const QString& id, const QString& value);
  void UpdateStatusColor(const QString& id, const QColor& color);
  void SetCustomWidget(QWidget* widget);

signals:
  void SigMinimizeRequested();
  void SigLoginSuccess();

 protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

 private slots:
  void OnClickedLogo();
  void UpdateTime();
  void OnPrintScreenClicked();
  void OnLoginSuccess(User* user);

 private:
  void SaveScreenShot(const QPixmap& screen_shot);
  PsLabel* CreateStatusLabel(const TitleBarStatusItem& item);
  void ReplaceLayoutWidget(QHBoxLayout* layout, QWidget** holder,
                           QWidget* widget);

  Ui::title_bar_viewClass* ui = nullptr;
  QHBoxLayout* status_layout_ = nullptr;
  QHBoxLayout* custom_layout_ = nullptr;
  QWidget* custom_widget_ = nullptr;
  LogView* log_view_ = nullptr;
  AlarmLampView* alarm_lamp_view_ = nullptr;
  LoginDialog* login_dialog_ = nullptr;
  UserManagerPtr user_manager_;
  QTimer* timer_ = nullptr;
  QMap<QString, PsLabel*> status_labels_;
};

#endif  // BASE_UI_SRC_VIEW_PAGES_TITLE_BAR_VIEW_TITLE_BAR_VIEW_H_
