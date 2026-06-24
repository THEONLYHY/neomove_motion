#include "view/tools/shared_manager/shared_manager.h"

// MotionTestHost only provides a minimal SharedManager implementation.
// It keeps extracted PointSetting code linkable, but does not mount
// ImageDisplay or ManualControl because this host does not include those widgets.
SharedManager* SharedManager::GetInstance() {
  static SharedManager instance;
  return &instance;
}

void SharedManager::SetWidgets(ImageDisplay* image, ManualControl* ctrl) {
  Q_UNUSED(image);
  Q_UNUSED(ctrl);
}

void SharedManager::RequestImageMount(QWidget* requester, QLayout* layout) {
  Q_UNUSED(requester);
  Q_UNUSED(layout);
}

void SharedManager::RequestControlMount(QWidget* requester, QLayout* layout) {
  Q_UNUSED(requester);
  Q_UNUSED(layout);
}

void SharedManager::Release() {}
