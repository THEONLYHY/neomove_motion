## Why

NeoMove currently has several UI and yotta API paths that can read motion state
directly from the NeoMove SDK. This makes state refresh depend on watcher
registration in some cases, and it leaves room for SDK access contention and UI
thread stalls when getters, `ReadValue()`, `Wait()`, or manual motion slots run
outside the intended motion/monitor execution paths.

## What Changes

- Register Axis/InputIO/OutputIO objects with `NeoMoveIoMonitorThread` as soon
  as they are created by `NeoMoveMotionMgr`, with watcher pointers allowed to be
  null.
- Refresh Axis/InputIO/OutputIO object caches from `NeoMoveIoMonitorThread`
  regardless of whether an external watcher is registered.
- Serve Axis state getters, Axis position/velocity getters, Axis `Wait()`, and
  InputIO/OutputIO `ReadValue()` from monitor-updated caches.
- Return failure with `cache_not_ready` when a cached getter/read is called
  before the monitor has produced its first sample.
- Update an OutputIO object's cached logical value immediately after successful
  `WriteValue()`, while allowing the monitor to reconcile the value with real
  hardware state on later polls.
- Move BaseUI manual robot motion work from Qt slots onto the
  `common::kMotion` message loop.
- Keep yotta public interface signatures unchanged.

## Capabilities

### New Capabilities

- `neomove-monitor-cache`: NeoMove yotta plugin state reads are served from
  monitor-owned caches and no longer depend on watcher registration.
- `baseui-manual-motion-dispatch`: BaseUI manual motion slots dispatch motion
  object lookup and motion commands to the motion message loop.

### Modified Capabilities

- None. This repository currently has no existing OpenSpec capability specs to
  modify.

## Impact

- Affected plugin files:
  - `neomove_motion/neomove_motion/src/neomove_motion_mgr.cc`
  - `neomove_motion/neomove_motion/src/neomove_io_monitor_thread.h`
  - `neomove_motion/neomove_motion/src/neomove_io_monitor_thread.cc`
  - `neomove_motion/neomove_motion/src/neomove_axis.h`
  - `neomove_motion/neomove_motion/src/neomove_axis.cc`
  - `neomove_motion/neomove_motion/src/neomove_input_io.h`
  - `neomove_motion/neomove_motion/src/neomove_input_io.cc`
  - `neomove_motion/neomove_motion/src/neomove_output_io.h`
  - `neomove_motion/neomove_motion/src/neomove_output_io.cc`
- Affected BaseUI file:
  - `neomove_motion/neomove_motion/test/BaseUI/BaseUI/src/controller/robot_manual/robot_manual.cc`
- API compatibility:
  - No yotta public method signature changes.
  - Cache-not-ready behavior changes from opportunistic SDK reads to explicit
    read failure with `cache_not_ready`.
- Explicit scope boundary:
  - This change centralizes status reads used by getters, `ReadValue()`, and
    `Wait()`.
  - Command paths may still perform SDK operations required to execute commands
    or perform guarded command validation, such as OutputIO bit read-modify-write
    and existing servo/alarm command checks, unless a task below explicitly
    moves that read to cache.
