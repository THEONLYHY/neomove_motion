// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/07/08 16:35
#ifndef BASE_UI_SRC_CONTROLLER_ACTION_WAIT_CONDITION_WAIT_CONDITION_ACTION_H_
#define BASE_UI_SRC_CONTROLLER_ACTION_WAIT_CONDITION_WAIT_CONDITION_ACTION_H_

#include <action/impl/action_base.h>
#include <action/impl/action_base_with_model.h>
#include <action/impl/action_factory_impl.h>
#include <action/model/action_model_impl.h>
#include <glog/glog_helper.h>
#include <limit_motion/limit_motion_helper.h>

#include <QElapsedTimer>

#include "controller/real_time_data/real_time_data.h"

//等待条件满足, io和自定义条件2选1
//{
//  "type" : "wait_condition",
//    "param" : {
//        "io_ids": "桥气缸到位",
//        "condition_ids": "自定义状态",
//        "value": true,
//        "millisecond": 10,
//        "timeout": 3000
//}

class WaitConditionAction
    : public action::ActionBaseWithModel<ActionModelImpl<action::ActionModel>> {
 public:
  WaitConditionAction(const char *type_name, action::ActionFramework *framework)
      : ActionBaseWithModel(type_name, framework) {
    LOG(INFO) << "WaitConditionAction constructed, type: "
              << (type_name ? type_name : "null");
  }
  int SetActionModel(action::ActionModelPtr action_model) override {
    io_ids_.clear();
    condition_ids_.clear();

    int ret = ActionBaseWithModel::SetActionModel(action_model);
    if (ret != 0) {
      LOG(ERROR) << "SetActionModel failed, ret=" << ret;
      return ret;
    }
    DCHECK(axis_config_);
    DCHECK(point_config_);
    if (!axis_config_ || !point_config_ || !action_model_) {
      LOG(ERROR) << "axis_config_ ||point_config_ is null";
      return 1;
    }
    try {
      const nlohmann::json &json_data = action_model_->GetJsonData();
      const nlohmann::json &param_json = json_data.at("param");

      if (param_json.contains("io_ids")) {
        io_ids_ = param_json["io_ids"];
        LOG(INFO) << "WaitConditionAction::InitData - io_ids=" << io_ids_;
      }
      if (param_json.contains("condition_ids")) {
        condition_ids_ = param_json["condition_ids"];
        LOG(INFO) << "WaitConditionAction::InitData - condition_ids="
                  << condition_ids_;
      }
      value_ = param_json["value"];
      millisecond_ = param_json["millisecond"];
      timeout_ = param_json["timeout"];
    } catch (const std::exception &e) {
      LOG(ERROR) << "parse json error, msg=" << e.what();
      return 2;
    }
    return 0;
  }
  int SyncFromModel() override { return 0; }
  int RunImpl(action::RunDataSource data_src) override {
    LOG(INFO) << "WaitCondition : ";
    if (io_ids_.empty()) {
      LOG(INFO) << "  io_ids : " << io_ids_;
    }
    QElapsedTimer timer;
    timer.start();
    while (true) {
      bool not_expected = false;
      if (!io_ids_.empty()) {
        yotta::InputIO *condition_io =
            limit_motion_mgr_helper_->GetInputIoByIds(io_ids_.c_str());
        if (condition_io) {
          unsigned char value = 0;
          condition_io->ReadValue(&value);
          if (static_cast<bool>(value) != value_) {
            not_expected = true;
          }
        } else {
          LOG(ERROR) << "WaitCondition Faile : InputIO is Null !" << io_ids_;
          return 1;
        }
      }
      if (!condition_ids_.empty()) {
        if (RealTimeDataSinglton::GetInstance()->get_condition_value(
                condition_ids_) != value_) {
          not_expected = true;
        }
      }

      if (io_ids_.empty() && condition_ids_.empty()) {
        LOG(ERROR) << "WaitCondition failed: io_ids and condition_ids are empty";
        return 1;
      }
      int time_spend = timer.elapsed();  // 毫秒
      if (time_spend > timeout_) {
        LOG(ERROR) << "WaitCondition TimeOut ! ";
        return 1;
      }
      if (not_expected) {
        std::this_thread::sleep_for(std::chrono::milliseconds(millisecond_));
        continue;
      } else {
        LOG(INFO) << "WaitCondition Satisfy : " << io_ids_ << condition_ids_
                  << ":" << value_;
        return 0;
      }
    }

    return 0;
  }

 private:
  std::string io_ids_;
  std::string condition_ids_;
  bool value_ = true;
  int millisecond_ = 10;
  int timeout_ = 3000;
};

#endif  // BASE_UI_SRC_CONTROLLER_ACTION_WAIT_CONDITION_WAIT_CONDITION_ACTION_H_
