## Context

`NeoMoveIoMonitorThread` already owns the periodic loop that polls IO and Axis
state. Axis objects already have `UpdateCachedStatus()`, `ReadCachedStatus()`,
and a `Wait()` condition variable, but Axis objects are only registered with the
monitor when `StartWatching()` is called. IO objects already declare cache fields
in `NeoMoveInputIo`, but `ReadValue()` still performs a direct PDO read, and
OutputIO has its own direct SDK read implementation.

The NeoMove SDK exposed in this repository is a flat `NM_*` C API keyed by an
integer controller index. It does not expose a WMX3-style per-thread SDK object
model. The existing `neomove_sdk_guard` is therefore still the process-level SDK
serialization boundary for direct SDK calls.

## Goals / Non-Goals

**Goals:**

- Make monitor registration independent from watcher registration.
- Make Axis getter, Axis `Wait()`, and IO `ReadValue()` status reads consume
  monitor caches instead of directly polling the SDK.
- Keep SDK status polling concentrated in `NeoMoveIoMonitorThread` for those
  read paths.
- Keep Qt manual motion slots responsive by dispatching motion work to
  `common::kMotion`.
- Keep yotta public API signatures stable.

**Non-Goals:**

- Do not replace every command-path SDK interaction in this change.
- Do not redesign NeoMove SDK initialization or introduce a WMX3-style per-thread
  SDK context, because the current NeoMove SDK headers do not expose that model.
- Do not change AnalogIO read/write behavior.
- Do not change BaseUI `DeviceStatusMonitor`'s public API.

## Decisions

### Decision: Register objects with null watchers at creation time

`NeoMoveMotionMgr::GetAxis()`, `GetInputIo()`, and `GetOutputIo()` will register
newly created objects with `NeoMoveIoMonitorThread` immediately after storing
the unique pointer in the manager cache. The watcher argument will be null.

Rationale: cache refresh must depend on object existence, not external watcher
registration. This keeps BaseUI `DeviceStatusMonitor` and other callers from
needing to register watchers just to make getters work.

Alternative considered: keep watcher-driven registration and make BaseUI register
watchers for every device. This leaves non-BaseUI yotta callers with the same
cache-not-refresh behavior and spreads the workaround across applications.

### Decision: Preserve watcher registration as watcher replacement

`StartWatching()` will continue to register the same object with a non-null
watcher. Re-registering will update the watcher entry and keep cache refresh
active.

Rationale: the monitor map already uses the IO key or axis id as the entry key.
Replacing `{nullptr, object}` with `{watcher, object}` is the smallest behavior
change.

Alternative considered: maintain separate cache object maps and watcher maps.
That reduces coupling but adds more state and migration risk for this focused
change.

### Decision: IO cache update lives on `NeoMoveInputIo`

`NeoMoveInputIo::UpdateCachedValue(unsigned char)` will update `cached_value_`
and `cache_valid_`. `NeoMoveOutputIo` will reuse the same cache storage through
inheritance, and `NeoMoveOutputIo::ReadValue()` will forward to
`NeoMoveInputIo::ReadValue()`.

Rationale: the output wrapper already inherits from `NeoMoveInputIo`, and both
input and output logical reads need the same cached byte/bit behavior.

Alternative considered: duplicate cache fields in `NeoMoveOutputIo`. That makes
input and output cache semantics easier to diverge accidentally.

### Decision: Monitor callbacks must not run while holding registration locks

`MonitorInputIo()`, `MonitorOutputIo()`, and `MonitorAxis()` will copy registered
entries to local snapshots under the relevant mutex, release the mutex, perform
SDK polling and object cache updates, then briefly re-lock only to update the
`last_*` state maps. Watcher callbacks run outside the mutex.

Rationale: after objects are registered automatically, the monitor map can grow
to include every configured object. Holding the map mutex across SDK calls or
watcher callbacks increases the chance of stalls and lock inversions with
`StartWatching()`, `GetAxis()`, and UI refresh code.

Alternative considered: keep the current lock scope and only add null checks.
That is smaller but preserves the existing lock contention risk.

### Decision: OutputIO bit writes remain read-modify-write command operations

For `bit >= 0`, `NeoMoveOutputIo::WriteValue()` may continue to call the SDK
read-modify-write helpers needed to preserve other bits in the same PDO word.
After a successful write, the object's logical cache is updated to the requested
bit value. The monitor later reconciles the cache with actual output state.

Rationale: removing the write-path read safely requires a PDO-word shadow cache,
which is larger than the requested getter/read cache change.

Alternative considered: write a single bit without reading the existing word.
That risks clobbering other outputs in the same PDO word.

### Decision: Manual motion slots post copied motion work

`RobotManual` slots will compute or copy transient UI/config values before
posting. The posted lambda will not capture references to slot parameters. Motion
object lookup and calls to `GetActualPosition()`, `AsyncMoveTo()`, `StartJog()`,
and `Stop()` execute on `common::kMotion`.

Rationale: this keeps Qt UI slots short and prevents UI-thread motion SDK access.

Alternative considered: post the whole slot body with `this`. That is smaller
but risks cross-thread access to Qt-owned object state and object lifetime.

## Risks / Trade-offs

- Cache first-frame gap -> getters and `ReadValue()` can return
  `cache_not_ready` immediately after object creation. Mitigation: document the
  behavior in specs and rely on the monitor's next poll to populate caches.
- Monitor polling failure -> stale cache remains until a later successful poll.
  Mitigation: update caches only on successful SDK reads and preserve error logs
  for failed polls.
- Output write optimistic cache -> a successful SDK write updates local cache
  before the hardware state is re-polled. Mitigation: monitor polling reconciles
  the real value.
- Raw object pointers in monitor entries -> lifetime must remain owned by
  `NeoMoveMotionMgr`. Mitigation: stop the monitor before `ClearResources()` in
  `Finalize()` and keep registration limited to manager-owned objects.

## Migration Plan

1. Add/adjust cache update APIs and monitor entry types.
2. Register objects with null watchers at creation time.
3. Move IO `ReadValue()` paths to cache reads.
4. Dispatch BaseUI manual motion work to `common::kMotion`.
5. Run OpenSpec validation, source checks, MSVC build, and BaseUI smoke tests.

Rollback is a source revert of the files listed in the proposal. No persistent
data migration is required.

## Open Questions

- Whether command-path status reads in `SetServoOn()`, `SetServoOff()`, and
  `ClearAxisAlarm()` should be moved to monitor cache in a follow-up change.
- Whether OutputIO bit writes should later use a PDO-word shadow cache to remove
  write-path read-modify-write SDK reads.
