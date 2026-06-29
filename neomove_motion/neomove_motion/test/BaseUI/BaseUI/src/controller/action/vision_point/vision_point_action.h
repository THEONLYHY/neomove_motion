// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 22:29

#ifndef BASE_UI_SRC_CONTROLLER_ACTION_VISION_POINT_VISION_POINT_ACTION_H_
#define BASE_UI_SRC_CONTROLLER_ACTION_VISION_POINT_VISION_POINT_ACTION_H_

#include <action/impl/action_base.h>
#include <action/impl/action_base_with_model.h>
#include <action/impl/action_factory_impl.h>
#include <action/model/action_model_impl.h>
#include <config/axis/axis_config.h>
#include <config/axis/axis_config_item.h>
#include <config/point/point_config_item.h>
#include <glog/glog_helper.h>
#include <limit_motion/impl/acc_dec_profile_impl.h>

//生成视觉点位,将当前坐标值保存为点位
//{
//  "type" : "creact_vision_point",
//      "param" : {
//          "point_ids" :"视觉点位1",
//           "vision_point_axis" :
//              [
//                 {"axis_ids" : ""},
//                 {"axis_ids" : ""}
//              ]
//  }
//}
class CreactVisionPointAction
    : public action::ActionBaseWithModel<ActionModelImpl<action::ActionModel>> {
 public:
  CreactVisionPointAction(const char *type_name,
                          action::ActionFramework *framework)
      : ActionBaseWithModel(type_name, framework) {
    LOG(INFO) << "CreactVisionPointAction constructed, type: "
              << (type_name ? type_name : "null");
  }
  // virtual ~CreactVisionPointAction() {}
  int SetActionModel(action::ActionModelPtr action_model) override {
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
      point_ids_ = param_json["point_ids"];
      for (const nlohmann::json &axis_ids_item : param_json["vision_point_axis"]) {
        std::string axis_ids = axis_ids_item["axis_ids"];
        axis_ids_.push_back(axis_ids);
      }
    } catch (const std::exception &e) {
      LOG(ERROR) << "parse json error, msg=" << e.what();
      return 2;
    }
    return 0;
  }
  int SyncFromModel() override { return 0; }
  int RunImpl(action::RunDataSource data_src) override {
    try {
      yotta::PointConfigItemPtr point_item =
          point_config_->GetPointByIds(point_ids_.c_str());
      if (!point_item) {
        point_item = point_config_->AddPoint(point_ids_.c_str());
      }
      if (!point_item) {
        LOG(ERROR) << "AddPoint: " << point_ids_.c_str() << " error";
        return 7;
      }
      for (size_t i = 0; i < axis_ids_.size(); i++) {
        yotta::Axis *target_axis =
            limit_motion_mgr_helper_->GetAxisByIds(axis_ids_[i].c_str());
        if (!target_axis) {
          LOG(ERROR) << "target_axis: " << axis_ids_[i] << " not found";
          return 7;
        }
        double axis_pos = 0;
        target_axis->GetActualPosition(&axis_pos);

        yotta::PointAxisConfigItemPtr point_axis_item =
            point_item->GetPointAxisItemByIds(axis_ids_[i].c_str());
        if (!point_axis_item) {
          point_axis_item = point_item->AddPointAxisItem(axis_ids_[i].c_str());
        }
        if (!point_axis_item) {
          LOG(ERROR) << "AddPointAxisItem: " << axis_ids_[i] << " not found";
          return 7;
        }
        point_axis_item->SetPosition(axis_pos);
      }
      point_config_->Save();
    } catch (const std::exception &e) {
      LOG(ERROR) << "CalibrationDataProcessAction::Run error:" << e.what();
      return 1;
    }

    return 0;
  }

 private:
  std::string point_ids_;
  std::vector<std::string> axis_ids_;
};

//点位计算:生成视觉点位,一个点位+偏移得到一个新的视觉点位
//{
//  "type" : "add_offset",
//      "param" : {
//          "old_point_ids" :"基板拍照位",
//          "new_point_ids" :"耦合位",
//           "offset" :
//              [
//                 {"axis_ids" : "AxisY","offset" : -60}
//              ]
//  }
//}
class CalcVisionPointAction
    : public action::ActionBaseWithModel<ActionModelImpl<action::ActionModel>> {
 public:
  CalcVisionPointAction(const char *type_name,
                        action::ActionFramework *framework)
      : ActionBaseWithModel(type_name, framework) {
    LOG(INFO) << "CalcVisionPointAction constructed, type: "
              << (type_name ? type_name : "null");
  }
  // virtual ~CreactVisionPointAction() {}
  int SetActionModel(action::ActionModelPtr action_model) override {
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
      axis_ids_list_.clear();
      offset_list_.clear();
      const nlohmann::json &json_data = action_model_->GetJsonData();
      const nlohmann::json &param_json = json_data.at("param");
      old_point_ids_ = param_json["old_point_ids"];
      new_point_ids_ = param_json["new_point_ids"];
      for (const nlohmann::json &offset_item : param_json["offset"]) {
        std::string axis_ids = offset_item["axis_ids"];
        double offset = offset_item["offset"];
        axis_ids_list_.push_back(axis_ids);
        offset_list_.push_back(offset);
      }

    } catch (const std::exception &e) {
      LOG(ERROR) << "parse json error, msg=" << e.what();
      return 2;
    }
    return 0;
  }
  int SyncFromModel() override { return 0; }
  int RunImpl(action::RunDataSource data_src) override {
    try {
      yotta::PointConfigItemPtr old_point_item =
          point_config_->GetPointByIds(old_point_ids_.c_str());
      if (!old_point_item) {
        LOG(ERROR) << "GetPoint: " << old_point_ids_.c_str() << " error";
        return 6;
      }
      yotta::PointConfigItemPtr new_vision_point_item =
          point_config_->GetPointByIds(new_point_ids_.c_str());
      if (!new_vision_point_item) {
        new_vision_point_item = point_config_->AddPoint(new_point_ids_.c_str());
      }
      if (!new_vision_point_item) {
        LOG(ERROR) << "AddPoint: " << new_point_ids_.c_str() << " error";
        return 7;
      }

      for (size_t i = 0; i < old_point_item->GetPointAxisItemCount(); i++) {
        yotta::PointAxisConfigItemPtr point_axis_config_item =
            old_point_item->GetPointAxisItem(i);
        std::string axis_ids;
        double axis_position = 0;
        char axis_ids_char[256];
        point_axis_config_item->GetAxisIds(axis_ids_char, 256);
        axis_ids = axis_ids_char;
        point_axis_config_item->GetPosition(&axis_position);

        for (int j = 0; j < axis_ids_list_.size(); j++) {
          if (axis_ids == axis_ids_list_[j]) {
            axis_position += offset_list_[j];
          }
        }

        yotta::PointAxisConfigItemPtr new_vision_point_axis_config_item =
            new_vision_point_item->GetPointAxisItemByIds(axis_ids.c_str());
        new_vision_point_item->GetPointAxisItemByIds(axis_ids.c_str());
        if (!new_vision_point_axis_config_item) {
          new_vision_point_axis_config_item =
              new_vision_point_item->AddPointAxisItem(axis_ids.c_str());
        }
        if (!new_vision_point_axis_config_item) {
          LOG(ERROR) << "AddPointAxisItem: " << axis_ids << " error";
          return 7;
        }
        new_vision_point_axis_config_item->SetPosition(axis_position);
      }

      point_config_->Save();
    } catch (const std::exception &e) {
      LOG(ERROR) << "CalcVisionPointAction::Run error:" << e.what();
      return 1;
    }

    return 0;
  }

 private:
  std::string old_point_ids_;
  std::string new_point_ids_;
  std::vector<std::string> axis_ids_list_;
  std::vector<double> offset_list_;
};

//点位计算:生成视觉点位,两个点位相加得到一个新的视觉点位
//{
//  "type" : "add_point",
//      "param" : {
//          "old_point_ids" :"基板_block点胶位",
//          "add_point_ids" : "Block点胶针头_补偿"
//          "new_point_ids" :"基板_block新点胶位",
//  }
//}
class CalcVisionPointByAddAction
    : public action::ActionBaseWithModel<ActionModelImpl<action::ActionModel>> {
 public:
  CalcVisionPointByAddAction(const char *type_name,
                             action::ActionFramework *framework)
      : ActionBaseWithModel(type_name, framework) {
    LOG(INFO) << "CalcVisionPointAction constructed, type: "
              << (type_name ? type_name : "null");
  }
  // virtual ~CreactVisionPointAction() {}
  int SetActionModel(action::ActionModelPtr action_model) override {
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
      old_point_ids_ = param_json["old_point_ids"];
      new_point_ids_ = param_json["new_point_ids"];
      offset_point_ids_ = param_json["add_point_ids"];
    } catch (const std::exception &e) {
      LOG(ERROR) << "parse json error, msg=" << e.what();
      return 2;
    }
    return 0;
  }
  int SyncFromModel() override { return 0; }
  int RunImpl(action::RunDataSource data_src) override {
    try {
      yotta::PointConfigItemPtr old_point_item =
          point_config_->GetPointByIds(old_point_ids_.c_str());
      if (!old_point_item) {
        LOG(ERROR) << "GetPoint: " << old_point_ids_.c_str() << " error";
        return 6;
      }
      yotta::PointConfigItemPtr offset_point_item =
          point_config_->GetPointByIds(offset_point_ids_.c_str());
      if (!offset_point_item) {
        LOG(ERROR) << "GetPoint: " << offset_point_ids_.c_str() << " error";
        return 6;
      }
      yotta::PointConfigItemPtr new_vision_point_item =
          point_config_->GetPointByIds(new_point_ids_.c_str());
      if (!new_vision_point_item) {
        new_vision_point_item = point_config_->AddPoint(new_point_ids_.c_str());
      }
      if (!new_vision_point_item) {
        LOG(ERROR) << "AddPoint: " << new_point_ids_.c_str() << " error";
        return 7;
      }

      for (size_t i = 0; i < old_point_item->GetPointAxisItemCount(); i++) {
        yotta::PointAxisConfigItemPtr point_axis_config_item =
            old_point_item->GetPointAxisItem(i);
        std::string axis_ids;
        double axis_position = 0;
        char axis_ids_char[256];
        point_axis_config_item->GetAxisIds(axis_ids_char, 256);
        axis_ids = axis_ids_char;
        point_axis_config_item->GetPosition(&axis_position);

        for (size_t j = 0; j < offset_point_item->GetPointAxisItemCount();
             j++) {
          yotta::PointAxisConfigItemPtr offset_point_axis_config_item =
              offset_point_item->GetPointAxisItem(j);
          std::string offset_axis_ids;
          double offset_axis_position = 0;
          char offset_axis_ids_char[256];
          offset_point_axis_config_item->GetAxisIds(offset_axis_ids_char, 256);
          offset_axis_ids = offset_axis_ids_char;
          offset_point_axis_config_item->GetPosition(&offset_axis_position);

          if (axis_ids == offset_axis_ids) {
            axis_position += offset_axis_position;
          }
        }

        yotta::PointAxisConfigItemPtr new_vision_point_axis_config_item =
            new_vision_point_item->GetPointAxisItemByIds(axis_ids.c_str());
        if (!new_vision_point_axis_config_item) {
          new_vision_point_axis_config_item =
              new_vision_point_item->AddPointAxisItem(axis_ids.c_str());
        }
        if (!new_vision_point_axis_config_item) {
          LOG(ERROR) << "AddPointAxisItem: " << axis_ids << " error";
          return 7;
        }
        new_vision_point_axis_config_item->SetPosition(axis_position);
      }

      point_config_->Save();
    } catch (const std::exception &e) {
      LOG(ERROR) << "CalcVisionPointAction::Run error:" << e.what();
      return 1;
    }

    return 0;
  }

 private:
  std::string old_point_ids_;
  std::string offset_point_ids_;
  std::string new_point_ids_;
};

//点位计算:生成视觉点位,两个点位相减得到一个新的视觉点位
//{
//  "type" : "sub_point",
//      "param" : {
//          "old_point_ids" :"耦合位",
//          "sub_point_ids" : "基板定位OK位"
//          "new_point_ids" :"基板定位OK位到耦合偏移",
//  }
//}
class CalcVisionPointBySubAction
    : public action::ActionBaseWithModel<ActionModelImpl<action::ActionModel>> {
 public:
  CalcVisionPointBySubAction(const char *type_name,
                             action::ActionFramework *framework)
      : ActionBaseWithModel(type_name, framework) {
    LOG(INFO) << "CalcVisionPointAction constructed, type: "
              << (type_name ? type_name : "null");
  }
  // virtual ~CreactVisionPointAction() {}
  int SetActionModel(action::ActionModelPtr action_model) override {
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
      old_point_ids_ = param_json["old_point_ids"];
      sub_point_ids_ = param_json["sub_point_ids"];
      new_point_ids_ = param_json["new_point_ids"];
    } catch (const std::exception &e) {
      LOG(ERROR) << "parse json error, msg=" << e.what();
      return 2;
    }
    return 0;
  }
  int SyncFromModel() override { return 0; }
  int RunImpl(action::RunDataSource data_src) override {
    try {
      yotta::PointConfigItemPtr old_point_item =
          point_config_->GetPointByIds(old_point_ids_.c_str());
      if (!old_point_item) {
        LOG(ERROR) << "GetPoint: " << old_point_ids_.c_str() << " error";
        return 6;
      }
      yotta::PointConfigItemPtr sub_point_item =
          point_config_->GetPointByIds(sub_point_ids_.c_str());
      if (!sub_point_item) {
        LOG(ERROR) << "GetPoint: " << sub_point_ids_.c_str() << " error";
        return 6;
      }
      yotta::PointConfigItemPtr new_vision_point_item =
          point_config_->GetPointByIds(new_point_ids_.c_str());
      if (!new_vision_point_item) {
        new_vision_point_item = point_config_->AddPoint(new_point_ids_.c_str());
      }
      if (!new_vision_point_item) {
        LOG(ERROR) << "AddPoint: " << new_point_ids_.c_str() << " error";
        return 7;
      }

      for (size_t i = 0; i < old_point_item->GetPointAxisItemCount(); i++) {
        yotta::PointAxisConfigItemPtr point_axis_config_item =
            old_point_item->GetPointAxisItem(i);
        std::string axis_ids;
        double axis_position = 0;
        char axis_ids_char[256];
        point_axis_config_item->GetAxisIds(axis_ids_char, 256);
        axis_ids = axis_ids_char;
        point_axis_config_item->GetPosition(&axis_position);

        for (size_t j = 0; j < sub_point_item->GetPointAxisItemCount(); j++) {
          yotta::PointAxisConfigItemPtr offset_point_axis_config_item =
              sub_point_item->GetPointAxisItem(j);
          std::string offset_axis_ids;
          double offset_axis_position = 0;
          char offset_axis_ids_char[256];
          offset_point_axis_config_item->GetAxisIds(offset_axis_ids_char, 256);
          offset_axis_ids = offset_axis_ids_char;
          offset_point_axis_config_item->GetPosition(&offset_axis_position);

          if (axis_ids == offset_axis_ids) {
            axis_position -= offset_axis_position;
          }
        }
        yotta::PointAxisConfigItemPtr new_vision_point_axis_config_item =
            new_vision_point_item->GetPointAxisItemByIds(axis_ids.c_str());
        if (!new_vision_point_axis_config_item) {
          new_vision_point_axis_config_item =
              new_vision_point_item->AddPointAxisItem(axis_ids.c_str());
        }
        if (!new_vision_point_axis_config_item) {
          LOG(ERROR) << "AddPointAxisItem: " << axis_ids << " error";
          return 7;
        }
        new_vision_point_axis_config_item->SetPosition(axis_position);
      }

      point_config_->Save();
    } catch (const std::exception &e) {
      LOG(ERROR) << "CalcVisionPointAction::Run error:" << e.what();
      return 1;
    }

    return 0;
  }

 private:
  std::string old_point_ids_;
  std::string sub_point_ids_;
  std::string new_point_ids_;
};

#endif  // BASE_UI_SRC_CONTROLLER_ACTION_VISION_POINT_VISION_POINT_ACTION_H_
