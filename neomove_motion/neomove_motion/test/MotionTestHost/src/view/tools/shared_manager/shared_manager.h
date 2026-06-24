#ifndef SRC_VIEW_TOOLS_SHARED_MANAGER_SHARED_MANAGER_H_
#define SRC_VIEW_TOOLS_SHARED_MANAGER_SHARED_MANAGER_H_

#include <QLayout>
#include <QPointer>
#include <QWidget>

class ImageDisplay;
class ManualControl;

// 共享控件管理器：单例
// 用于在多个页面之间流转 ImageDisplay 和 ManualControl 控件
class SharedManager {
 public:
  // 获取单例实例
  static SharedManager* GetInstance();

  // 设置控件引用（由 MainWindow 构造时调用）
  void SetWidgets(ImageDisplay* image, ManualControl* ctrl);

  // 申请挂载 ImageDisplay
  // layout 为 nullptr 时不操作，非 nullptr 时从上一个所有者卸载并挂载到新布局
  void RequestImageMount(QWidget* requester = nullptr, QLayout* layout = nullptr);

  // 申请挂载 ManualControl
  // layout 为 nullptr 时不操作，非 nullptr 时从上一个所有者卸载并挂载到新布局
  void RequestControlMount(QWidget* requester = nullptr, QLayout* layout = nullptr);

  // 释放所有控件所有权
  void Release();

 private:
  SharedManager() = default;
  ~SharedManager() = default;
  SharedManager(const SharedManager&) = delete;
  SharedManager& operator=(const SharedManager&) = delete;

  QPointer<ImageDisplay> image_display_;
  QPointer<ManualControl> manual_control_;
  QPointer<QWidget> current_image_owner_;
  QPointer<QWidget> current_control_owner_;
  QPointer<QLayout> current_image_layout_;
  QPointer<QLayout> current_control_layout_;
};

#endif  // SRC_VIEW_TOOLS_SHARED_MANAGER_SHARED_MANAGER_H_