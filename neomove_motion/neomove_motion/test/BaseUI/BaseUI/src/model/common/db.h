// copyright 2025 YottaImage. All rights reserved.
// author panming
// date 2025/05/09 14:35
#ifndef BASE_UI_SRC_MODEL_COMMON_DB_H_
#define BASE_UI_SRC_MODEL_COMMON_DB_H_

#include <sqlite3/CppSQLite3.h>
#include <sqlite3/sqlite3_helper.h>

#include <memory>

using DBPtr = std::shared_ptr<CppSQLite3DB>;
using DBWeakPtr = std::weak_ptr<CppSQLite3DB>;

#endif  // BASE_UI_SRC_MODEL_COMMON_DB_H_
