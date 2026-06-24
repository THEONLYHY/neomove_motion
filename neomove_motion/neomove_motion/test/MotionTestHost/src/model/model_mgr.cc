#include "model/model_mgr.h"

#include <QCoreApplication>
#include <QDir>

namespace {

std::wstring ConfigPath(const QString& file_name) {
  const QString path = QDir::toNativeSeparators(
      QCoreApplication::applicationDirPath() + "/config/" + file_name);
  return path.toStdWString();
}

}  // namespace

bool ModelMgr::Init(ModelMgrReadyCallback callback) {
  const bool unit_ready = unit_info_mgr_->Init(ConfigPath("unit.json"));
  const bool point_ready =
      unit_point_mgr_->Init(ConfigPath("unit_point.json"));
  const bool ready = unit_ready && point_ready;

  if (callback) {
    callback(ModelType::kModelModuleMgr, ready);
  }
  return ready;
}
