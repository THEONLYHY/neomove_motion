#include "view/pages/main_page/main_page.h"

#include <QLabel>
#include <QVBoxLayout>

MainPage::MainPage(QWidget* parent) : QWidget(parent) {
  setStyleSheet("QWidget { background:#E5E7EB; }");
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  auto* label = new QLabel("BaseUI", this);
  label->setAlignment(Qt::AlignCenter);
  label->setStyleSheet("QLabel { color:#374151; font-size:48px; }");
  layout->addWidget(label);
}
