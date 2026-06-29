// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/01/09 22:29

#include "controller/action/custom_total_action.h"

#include <action/impl/action_factory_impl.h>

#include "controller/action/open_camera/open_camera_action.h"
#include "controller/action/vision_point/vision_point_action.h"
#include "controller/action/wait_condition/wait_condition_action.h"

// 注册到MotionFactory
REGISTER_ACTION(OpenCameraAction, action::kOpenCameraAction)
REGISTER_ACTION(WaitConditionAction, kWaitConditionAction)

REGISTER_ACTION(CreactVisionPointAction, kCreactVisionPointAction)
REGISTER_ACTION(CalcVisionPointByAddAction, kCalcVisionPointByAddAction)
REGISTER_ACTION(CalcVisionPointBySubAction, kCalcVisionPointBySubAction)
REGISTER_ACTION(CalcVisionPointAction, kCalcVisionPointAction)
