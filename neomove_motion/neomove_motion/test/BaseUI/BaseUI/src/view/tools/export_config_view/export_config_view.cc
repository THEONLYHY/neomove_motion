// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/04/09 16:35

#include "export_config_view.h"

#include <common/path/path_utils.h>

#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include "view/tools/popup_dialog/popup_dialog.h"

namespace {

// 获取当前 config 目录路径（与可执行文件同级的 config/）
QString GetConfigDirectoryPath() {
  return QDir::cleanPath(QString::fromStdWString(
      path_utils::GetFullPathFromCurrentModule(L"config")));
}

// 获取 config_new 暂存目录路径（启动时待加载的新配置）
QString GetConfigNewDirectoryPath() {
  QFileInfo config_info(GetConfigDirectoryPath());
  return QDir::cleanPath(
      QDir(config_info.dir().absolutePath()).filePath("config_new"));
}

// 获取 config_old 备份目录路径
QString GetConfigOldDirectoryPath() {
  QFileInfo config_info(GetConfigDirectoryPath());
  return QDir::cleanPath(
      QDir(config_info.dir().absolutePath()).filePath("config_old"));
}

// 在 parent_path 下生成带时间戳的子目录，如 config_20260410_143000
QString BuildTimestampedDirectoryPath(const QString& parent_path,
                                      const QString& base_name) {
  QString suffix = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
  return QDir::cleanPath(QDir(parent_path).filePath(base_name + "_" + suffix));
}

// 判断 path 是否等于 parent_path 或是其子目录
bool IsSameOrChildPath(const QString& path, const QString& parent_path) {
  QString normalized_path = QDir::cleanPath(path);
  QString normalized_parent_path = QDir::cleanPath(parent_path);
  if (normalized_path.compare(normalized_parent_path, Qt::CaseInsensitive) ==
      0) {
    return true;
  }

  QString parent_with_separator = normalized_parent_path;
  if (!parent_with_separator.endsWith('/')) {
    parent_with_separator += '/';
  }

  return normalized_path.startsWith(parent_with_separator, Qt::CaseInsensitive);
}

// 递归复制整个目录（包括隐藏文件和系统文件）
bool CopyDirectoryRecursively(const QString& source_path,
                              const QString& target_path) {
  QDir source_dir(source_path);
  if (!source_dir.exists()) {
    return false;
  }

  QDir target_dir;
  if (!target_dir.mkpath(target_path)) {
    return false;
  }

  QFileInfoList entries = source_dir.entryInfoList(
      QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden | QDir::System);
  for (const QFileInfo& entry : entries) {
    QString target_entry_path = QDir(target_path).filePath(entry.fileName());
    if (entry.isDir()) {
      if (!CopyDirectoryRecursively(entry.absoluteFilePath(),
                                    target_entry_path)) {
        return false;
      }
      continue;
    }

    QFile::remove(target_entry_path);
    if (!QFile::copy(entry.absoluteFilePath(), target_entry_path)) {
      return false;
    }
  }

  return true;
}

// 递归移动整个目录（复制 + 删除源，兼容跨磁盘）
bool MoveDirectoryRecursively(const QString& source_path,
                              const QString& target_path) {
  if (!CopyDirectoryRecursively(source_path, target_path)) {
    return false;
  }

  return QDir(source_path).removeRecursively();
}

// 将当前 config 导出到用户选择的目录
bool ExportConfigToDirectory(const QString& selected_dir,
                             QString* error_message) {
  QString config_dir = GetConfigDirectoryPath();
  QFileInfo config_info(config_dir);
  if (!config_info.exists() || !config_info.isDir()) {
    if (error_message) {
      *error_message = "当前配置文件夹不存在";
    }
    return false;
  }

  QString normalized_selected_dir = QDir::cleanPath(selected_dir);
  if (IsSameOrChildPath(normalized_selected_dir, config_dir)) {
    if (error_message) {
      *error_message = "导出文件夹不能是当前配置文件夹或其子文件夹";
    }
    return false;
  }

  QString export_path =
      BuildTimestampedDirectoryPath(normalized_selected_dir, "config");
  if (!CopyDirectoryRecursively(config_dir, export_path)) {
    if (error_message) {
      *error_message = "导出配置文件夹失败";
    }
    return false;
  }

  return true;
}

}  // namespace

ExportConfigView::ExportConfigView(QWidget* parent)
    : QWidget(parent), ui(new Ui::ExportConfigView) {
  ui->setupUi(this);

  ui->button_load->Color().SetBaseColor(PsColor::Color::kPurple);
  ui->button_upload->Color().SetBaseColor(PsColor::Color::kGreen);
  ui->button_export_log->Color().SetBaseColor(PsColor::Color::kGreen);

  connect(ui->button_load, &QPushButton::clicked, this,
          &ExportConfigView::OnLoadButtonClicked);
  connect(ui->button_upload, &QPushButton::clicked, this,
          &ExportConfigView::OnUploadButtonClicked);
  connect(ui->button_export_log, &QPushButton::clicked, this,
          &ExportConfigView::OnExportLogButtonClicked);
}

ExportConfigView::~ExportConfigView() { delete ui; }

/**
 * @brief 启动时检查并应用待加载的配置
 *
 * 检查 config_new/ 暂存区是否存在：
 * - 不存在：直接返回，正常启动
 * - 存在且有内容：删除当前 config/，将 config_new/ 内容移入 config/，删除
 * config_new/
 * - 备份已在用户点击加载时完成（config_old/），此处不再重复备份
 *
 * 此函数在 main() 中调用，早于所有单例初始化，此时 DB 未打开，文件无锁。
 */
void HandlePendingConfigLoad() {
  QString config_new_path = GetConfigNewDirectoryPath();
  QDir config_new_dir(config_new_path);
  if (!config_new_dir.exists()) {
    return;
  }

  // 检查 config_new 是否有实际内容
  QFileInfoList entries = config_new_dir.entryInfoList(
      QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden | QDir::System);
  if (entries.isEmpty()) {
    config_new_dir.removeRecursively();
    return;
  }

  QString config_path = GetConfigDirectoryPath();

  // 删除当前 config 内容（备份已在 OnLoadButtonClicked 中完成）
  QDir config_dir(config_path);
  if (config_dir.exists()) {
    config_dir.removeRecursively();
  }

  // 将 config_new 复制到 config
  if (CopyDirectoryRecursively(config_new_path, config_path)) {
    config_new_dir.removeRecursively();
  }
}

/**
 * @brief 加载按钮点击槽函数
 *
 * 流程：
 * 1. 选择目标配置文件夹并校验路径合法性
 * 2. 备份当前 config → config_old/config_<timestamp>
 * 3. 复制目标配置 → config_new/（暂存区）
 * 4. 提示用户下次启动时生效
 */
void ExportConfigView::OnLoadButtonClicked() {
  QString selected_dir = QFileDialog::getExistingDirectory(
      this, tr("Select Config Folder"), QString(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (selected_dir.isEmpty()) {
    return;
  }

  QString config_dir = GetConfigDirectoryPath();
  QString normalized_selected = QDir::cleanPath(selected_dir);

  if (normalized_selected == QDir::cleanPath(config_dir)) {
    PopupDialogSingleton::GetInstance()->ShowPage();
    PopupDialogSingleton::GetInstance()->PopupInfo("加载失败：所选文件夹已是当前配置文件夹，无需加载");
    return;
  }
  if (IsSameOrChildPath(normalized_selected, config_dir)) {
    PopupDialogSingleton::GetInstance()->ShowPage();
    PopupDialogSingleton::GetInstance()->PopupInfo("加载失败：所选文件夹不能是当前配置文件夹或其子文件夹");
    return;
  }

  // 备份当前 config 到 config_old
  QString config_old_path = GetConfigOldDirectoryPath();
  QDir().mkpath(config_old_path);
  QString backup_path =
      BuildTimestampedDirectoryPath(config_old_path, "config");

  if (!CopyDirectoryRecursively(config_dir, backup_path)) {
    PopupDialogSingleton::GetInstance()->ShowPage();
    PopupDialogSingleton::GetInstance()->PopupInfo("加载失败：备份当前配置失败");
    return;
  }

  // 复制目标配置到 config_new 暂存区
  QString config_new_path = GetConfigNewDirectoryPath();
  QDir(config_new_path).removeRecursively();

  if (!CopyDirectoryRecursively(normalized_selected, config_new_path)) {
    PopupDialogSingleton::GetInstance()->ShowPage();
    PopupDialogSingleton::GetInstance()->PopupInfo("加载失败：准备配置文件失败");
    return;
  }

  PopupDialogSingleton::GetInstance()->ShowPage();
  PopupDialogSingleton::GetInstance()->PopupInfo("加载成功：配置将在下次启动时生效");
}

/**
 * @brief 导出按钮点击槽函数
 *
 * 将当前 config 复制到用户选择的目录，无需重启。
 */
void ExportConfigView::OnUploadButtonClicked() {
  QString selected_dir = QFileDialog::getExistingDirectory(
      this, tr("Select Export Folder"), QString(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (selected_dir.isEmpty()) {
    return;
  }

  QString error_message;
  if (ExportConfigToDirectory(selected_dir, &error_message)) {
    PopupDialogSingleton::GetInstance()->ShowPage();
    PopupDialogSingleton::GetInstance()->PopupInfo("导出成功");
  } else {
    PopupDialogSingleton::GetInstance()->ShowPage();
    PopupDialogSingleton::GetInstance()->PopupInfo("导出失败：" + error_message);
  }
}

/**
 * @brief 导出日志按钮点击槽函数
 *
 * 将当前 log 目录复制到用户选择的目录，带时间戳命名。
 */
void ExportConfigView::OnExportLogButtonClicked() {
  QString log_dir = QDir::cleanPath(QString::fromStdWString(
      path_utils::GetFullPathFromCurrentModule(L"log")));

  QFileInfo log_info(log_dir);
  if (!log_info.exists() || !log_info.isDir()) {
    PopupDialogSingleton::GetInstance()->ShowPage();
    PopupDialogSingleton::GetInstance()->PopupInfo("导出失败：日志文件夹不存在");
    return;
  }

  QString selected_dir = QFileDialog::getExistingDirectory(
      this, tr("Select Export Folder"), QString(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (selected_dir.isEmpty()) {
    return;
  }

  if (IsSameOrChildPath(selected_dir, log_dir)) {
    PopupDialogSingleton::GetInstance()->ShowPage();
    PopupDialogSingleton::GetInstance()->PopupInfo("导出失败：导出文件夹不能在日志文件夹内");
    return;
  }

  QString export_path = BuildTimestampedDirectoryPath(selected_dir, "log");
  if (CopyDirectoryRecursively(log_dir, export_path)) {
    PopupDialogSingleton::GetInstance()->ShowPage();
    PopupDialogSingleton::GetInstance()->PopupInfo("导出成功");
  } else {
    PopupDialogSingleton::GetInstance()->ShowPage();
    PopupDialogSingleton::GetInstance()->PopupInfo("导出失败：导出日志文件夹失败");
  }
}
