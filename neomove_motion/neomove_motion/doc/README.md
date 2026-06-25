# NeoMove Motion Runtime Documents

本目录保存可提交的运行资料和样例配置，避免把 `x64/Debug` 这类编译输出目录加入 Git。

## MotionTestHost 配置包

`config/MotionTestHost/` 是 MotionTestHost 的样例运行配置。MotionTestHost
工程构建后会自动把该目录下的文件复制到可执行文件同级的 `config` 目录。
如果需要手动恢复本地运行目录，也可以执行：

```powershell
$target = "neomove_motion\neomove_motion\test\MotionTestHost\x64\Debug\config"
New-Item -ItemType Directory -Force $target | Out-Null
Copy-Item -Recurse -Force `
  neomove_motion\neomove_motion\doc\config\MotionTestHost\* `
  $target
```

目标目录结构应为：

```text
neomove_motion/neomove_motion/test/MotionTestHost/x64/Debug/config/
```

MotionTestHost 启动时会把 `<exe_dir>/config` 和 `neomove.json` 传给 `ModuleMgr::Init`。测试页面还会通过公共配置系统读取 `axis.json`、`io.json`、`unit.json` 等配置，所以本配置包不只包含 `neomove.json`。

## MotionTestHost 运动调试注意事项

- JOG 速度来自测试页的 JOG 速度输入框，不来自速度下拉框的 `velocity`。
- 速度下拉框对 JOG 只保留 `acc`、`dec` 和 profile type；定点运动和两段运动使用速度配置中的 `velocity`、`acc`、`dec`。
- `home_type=35` 表示当前位置回零，成功时不一定能看到机械移动，重点看回零返回值和当前位置是否被置零。
- 如果轴处于报警、正/负限位、软限位或伺服未上状态，先人工确认现场状态，再手动点击“清除报警”和“当前轴上伺服”。运动按钮不会隐式清报警或隐式上伺服。

## 已包含文件

最小必需配置：

- `neomove.json`
- `axis.json`
- `io.json`
- `module.json`
- `unit.json`
- `unit_point.json`
- `limit.json`
- `axis_compensation.json`

随包提供的宿主初始化配置：

- `task.json`
- `step.json`
- `algorithm.json`
- `algorithm_step.json`
- `calibration.json`
- `camera.json`
- `camera_config.json`
- `probe_configuration.json`
- `point.db`

## 现场参数

`neomove.json` 中的以下字段是样例值，真实设备运行前必须按现场控制器和轴配置确认：

- `controller_index`
- `controller_type`
- `controller_ip`，如果现场控制器需要指定 IP，可按插件支持的字段补充
- `home` 下各轴的 `home_type`、`velocity_fast`、`velocity_slow`、`acc`、`dec`

本配置包的目标是让 MotionTestHost 可以启动并加载页面配置，不承诺真实设备开箱即跑。

## 不提交的文件

以下内容属于构建产物、本地运行状态或重复快照，不应随配置包上传：

- `x64/Debug/`、`x64/Debug/qt/`
- `*.obj`、`*.pdb`、`*.exe`、`*.dll`、`*.mot`
- `point.db-shm`、`point.db-wal`
- `task-backup.json`、`step-backup.json`
- `local_config/`
- `ProbeStation.db`
- `.vs/`、`*.user`、`.agents/`、`.codex/`
