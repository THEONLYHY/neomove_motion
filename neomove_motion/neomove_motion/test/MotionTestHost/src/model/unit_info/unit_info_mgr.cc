#include "unit_info_mgr.h"

#include <common/json_config/json_config_helper.h>
#include <common/message_loop.h>
#include <common/path/path_utils.h>
#include <glog/glog_helper.h>

#include <QString>
#include <fstream>

#include "model/model_mgr.h"

bool UnitInfoMgr::Init(const std::wstring& file_path) {
	json_config_.reset(new NlohmanJsonConfig);
	bool success = json_config_->Init(file_path);
	if (!success) {
		LOG(ERROR) << L"打开axis_info文件失败: " << file_path;
		ready_ = false;
		return ready_;
	}

	std::lock_guard<std::mutex> lock(axis_list_mutex_);
	module_info_.clear();
	// 读取轴状态信息数组

	int module_size = json_config_->GetArraySize("unit", &success);
	for (size_t i = 0; i < module_size; ++i) {
		auto module_axis_info = std::make_shared<UnitInfo>();

		std::string base_key = "unit[" + std::to_string(i) + "]";
		std::string module_name =
			json_config_->GetString(base_key + "/ids", "", &success);

		module_axis_info->unit_ids =  module_name;
		int axis_size = json_config_->GetArraySize(base_key + "/axis", &success);
		if (success) {
			for (int j = 0; j < axis_size; ++j) {
				bool axis_success = true;
				std::string axis_key = base_key + "/axis[" + std::to_string(j) + "]";

				// 读取名字
				std::string axis_ids =
					json_config_->GetString(axis_key + "/ids", "", &axis_success);
				//// 读取轴索引
				//int axis_index =
				//    json_config_->GetInt(axis_key + "/index", 0, &axis_success);
				int axis_type =
					json_config_->GetInt(axis_key + "/type", 0, &axis_success);

				module_axis_info->axis_ids.push_back(axis_ids);
				//module_axis_info.axis_index.push_back(axis_index);
				module_axis_info->axis_types.push_back(axis_type);
			}
		}
		else {
			LOG(WARNING) << "读取module_axis_info数组失败";
		}

		int io_input_size =
			json_config_->GetArraySize(base_key + "/io_input", &success);
		if (success) {
			for (int j = 0; j < io_input_size; j++) {
				bool io_input_success = true;
				std::string io_input_key =
					base_key + "/io_input[" + std::to_string(j) + "]";
				// 读取名字
				std::string io_input_name = json_config_->GetString(
					io_input_key + "/ids", "", &io_input_success);
				module_axis_info->io_input_names.push_back(io_input_name);
			}
		}
		else {
			LOG(WARNING) << "读取module_io_input_info数组失败";
		}

		int io_output_size =
			json_config_->GetArraySize(base_key + "/io_output", &success);
		if (success) {
			for (int j = 0; j < io_output_size; j++) {
				bool io_output_success = true;
				std::string io_output_key =
					base_key + "/io_output[" + std::to_string(j) + "]";
				// 读取名字
				std::string io_output_name = json_config_->GetString(
					io_output_key + "/ids", "", &io_output_success);
				module_axis_info->io_output_names.push_back(io_output_name);
			}
		}
		else {
			LOG(WARNING) << "读取module_io_output_info数组失败";
		}

		module_info_.push_back(module_axis_info);
	}

	ready_ = success;
	return ready_;
}

int UnitInfoMgr::GetUnitCount() {
	std::lock_guard<std::mutex> lock(axis_list_mutex_);
	return static_cast<int>(module_info_.size());
}
UnitInfoPtr UnitInfoMgr::GetUnitInfo(int index) {
	std::lock_guard<std::mutex> lock(axis_list_mutex_);
	if (index < 0 || index >= module_info_.size()) {
		LOG(ERROR)
			<< "GetUnitInfo error : An out-of-bounds error has occurred";
		return nullptr;
	}
	return module_info_[index];  // 直接返回shared_ptr，无需拷贝
}
UnitInfoPtr UnitInfoMgr::GetUnitInfo(std::string unit_ids) {
	std::lock_guard<std::mutex> lock(axis_list_mutex_);
	for (int i = 0; i < module_info_.size(); i++) {
		if (unit_ids == module_info_[i]->unit_ids) {
			return module_info_[i];  // 直接返回shared_ptr，无需拷贝
		}
	}
	LOG(ERROR) << "GetUnitInfo error : No this Unit :"
		<< unit_ids;
	return nullptr;
}

std::string UnitInfoMgr::GetUnitIdByAxisIds(std::string axis_ids)
{
	std::lock_guard<std::mutex> lock(axis_list_mutex_);
	for (int i = 0; i < module_info_.size(); i++) {
		for (int j = 0; j < module_info_[i]->axis_ids.size(); j++)
		{
			if (axis_ids == module_info_[i]->axis_ids[j]) {
				return module_info_[i]->unit_ids;
			}
		}
	}
	LOG(ERROR) << "GetUnitId error : No this Unit have :"
		<< axis_ids;
	return "";
}