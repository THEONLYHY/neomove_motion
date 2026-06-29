// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2025/05/07 14:46

#ifndef PROBE_STATION8_SRC_CONTROLLER_ROBOT_MANUAL_ROBOT_DEFINE_H_
#define PROBE_STATION8_SRC_CONTROLLER_ROBOT_MANUAL_ROBOT_DEFINE_H_

#include <memory>
#include <string>
#include <vector>

struct MoveProperty {
  int move_model = 1;  // 0:JOG, 1:SCAN, 2:INDEX(一次走一个INDEX间隔)
  double step = 1;     // 运动步长 (微米)
};

//功能模块
struct UnitInfo {
  std::string unit_ids;
  std::vector<std::string> axis_ids;  // 轴名称
  // std::vector<int> axis_index;  // 轴序号,外部序号,非软银内部序号
  std::vector<int> axis_types;  // 1:X, 2:Y, 3:R1, 4:R2, 5:Z1,6:Z2,7:TX,8:TY

  std::vector<std::string> io_input_names;   // 输入IO
  std::vector<std::string> io_output_names;  // 输出IO
};

// 对外都使用shared_ptr避免拷贝
using UnitInfoPtr = std::shared_ptr<UnitInfo>;

#endif  // PROBE_STATION8_SRC_CONTROLLER_ROBOT_MANUAL_ROBOT_DEFINE_H_
