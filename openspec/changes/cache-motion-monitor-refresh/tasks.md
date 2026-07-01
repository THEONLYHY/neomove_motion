## 1. OpenSpec and preflight

- [ ] 1.1 Confirm this change validates as an OpenSpec change after all artifacts are written with `openspec validate cache-motion-monitor-refresh --strict`.
- [ ] 1.2 Record the current direct SDK read locations with `rg -n "NM_GetAxisStatus|ReadWord\\(" neomove_motion/neomove_motion/src/neomove_axis.cc neomove_motion/neomove_motion/src/neomove_input_io.cc neomove_motion/neomove_motion/src/neomove_output_io.cc neomove_motion/neomove_motion/src/neomove_io_monitor_thread.cc`.
- [ ] 1.3 Treat direct reads inside Axis getters, Axis `Wait()`, InputIO `ReadValue()`, and OutputIO `ReadValue()` as in-scope removals; treat OutputIO bit read-modify-write and command-path Axis status validation as explicit exceptions unless a later task changes them.

## 2. Axis monitor registration and cache reads

- [ ] 2.1 In `neomove_motion/neomove_motion/src/neomove_motion_mgr.cc`, register a newly created `NeoMoveAxis` with `io_monitor_thread_->RegisterAxis(id, nullptr, axis)` immediately after storing it in `axes_`.
- [ ] 2.2 Preserve existing `NeoMoveAxis::StartWatching()` behavior so calling it later re-registers the same axis with a non-null watcher.
- [ ] 2.3 Confirm `state()`, `home_state()`, `operation_state()`, `GetActualPosition()`, `GetActualVelocity()`, and `GetTargetPosition()` all call `ReadCachedStatus()` and return `cache_not_ready` if the cache is invalid.
- [ ] 2.4 Confirm `Wait()` waits only on `wait_cv_` and is only completed by `UpdateCachedStatus()` changing `wait_result_`.

## 3. IO cache API and read behavior

- [ ] 3.1 In `neomove_motion/neomove_motion/src/neomove_input_io.cc`, implement `NeoMoveInputIo::UpdateCachedValue(unsigned char value)` to store `cached_value_` and set `cache_valid_`.
- [ ] 3.2 Change `NeoMoveInputIo::ReadValue()` to validate `byte_val`, fail with `cache_not_ready` when `cache_valid_` is false, and otherwise return `cached_value_` without calling `neomove_pdo::ReadWord()`.
- [ ] 3.3 Change `NeoMoveOutputIo::ReadValue()` to forward to `NeoMoveInputIo::ReadValue()` or remove the override if MSVC accepts inherited resolution.
- [ ] 3.4 After successful `NeoMoveOutputIo::WriteValue()`, call `UpdateCachedValue()` with the requested logical output value.
- [ ] 3.5 Keep OutputIO bit write helpers as guarded SDK read-modify-write command operations, and do not classify those helper reads as `ReadValue()` reads.

## 4. Monitor entry ownership and polling

- [ ] 4.1 In `neomove_motion/neomove_motion/src/neomove_io_monitor_thread.h`, change IO monitor entries from `std::pair<yotta::IO::IOWatcher*, yotta::IO*>` to a typed entry that stores `yotta::IO::IOWatcher* watcher` and `NeoMoveInputIo* io`.
- [ ] 4.2 Update `RegisterInputIo()` and `RegisterOutputIo()` signatures and callers to pass `NeoMoveInputIo*` or `NeoMoveOutputIo*` as cache-capable objects.
- [ ] 4.3 In `MonitorInputIo()`, copy registered entries to a local snapshot under `input_io_mutex_`, release the mutex, poll PDO values, call `UpdateCachedValue(new_state)` on every successful poll, update `last_input_io_state_` under a short mutex lock, and invoke watcher callbacks outside the mutex.
- [ ] 4.4 Apply the same snapshot, cache update, short-lock state comparison, and out-of-lock watcher callback pattern to `MonitorOutputIo()`.
- [ ] 4.5 Apply the same lock-scope pattern to `MonitorAxis()` so SDK calls and watcher callbacks do not run while holding `axis_mutex_`.
- [ ] 4.6 Preserve `Finalize()` ordering so `io_monitor_thread_->Stop()` happens before `ClearResources()` releases Axis/IO objects.

## 5. Motion manager object creation

- [ ] 5.1 Remove redundant `SetAddress(addr, bit)` calls on cached `GetInputIo()` hits because the cache key already matches the object address.
- [ ] 5.2 Remove redundant `SetAddress(addr, bit)` calls on cached `GetOutputIo()` hits for the same reason.
- [ ] 5.3 After creating and storing a new `NeoMoveInputIo`, register it with `RegisterInputIo(addr, bit, nullptr, raw_ptr)`.
- [ ] 5.4 After creating and storing a new `NeoMoveOutputIo`, register it with `RegisterOutputIo(addr, bit, nullptr, raw_ptr)`.

## 6. BaseUI manual motion dispatch

- [ ] 6.1 In `neomove_motion/neomove_motion/test/BaseUI/BaseUI/src/controller/robot_manual/robot_manual.cc`, add the required include for `common::MessageLoop` if it is not already available through existing headers.
- [ ] 6.2 In `RobotManual::OnButtonClicked()`, copy `axis_ids`, `direction`, `move_property_`, configured speed values, and the resolved index step before posting; execute `GetActualPosition()` and `AsyncMoveTo()` inside a `common::kMotion` task.
- [ ] 6.3 In `RobotManual::OnButtonLongPressed()`, copy `axis_ids`, `direction`, `move_property_`, and configured speed values before posting; execute `StartJog()` inside a `common::kMotion` task.
- [ ] 6.4 In `RobotManual::OnButtonLongPressReleased()`, copy `axis_ids` before posting; execute axis lookup and `Stop()` inside a `common::kMotion` task.
- [ ] 6.5 Do not capture references to slot parameters or mutable UI-owned state in posted motion lambdas.

## 7. Verification

- [ ] 7.1 Run `openspec validate cache-motion-monitor-refresh --strict` and fix any artifact validation errors.
- [ ] 7.2 Run `rg -n "ReadWord\\(" neomove_motion/neomove_motion/src/neomove_input_io.cc neomove_motion/neomove_motion/src/neomove_output_io.cc` and confirm `ReadValue()` no longer performs direct PDO reads.
- [ ] 7.3 Run `rg -n "NM_GetAxisStatus" neomove_motion/neomove_motion/src/neomove_axis.cc neomove_motion/neomove_motion/src/neomove_io_monitor_thread.cc` and confirm Axis getter/`Wait()` paths do not directly call the SDK.
- [ ] 7.4 Build `neomove_motion/neomove_motion.sln` in `Debug|x64` from a Visual Studio developer shell with `msbuild neomove_motion/neomove_motion.sln /p:Configuration=Debug /p:Platform=x64`.
- [ ] 7.5 Smoke test BaseUI device status refresh after startup, manual scan/index move, jog press/release, and output write/read display.
