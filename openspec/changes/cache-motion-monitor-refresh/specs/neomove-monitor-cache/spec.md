## ADDED Requirements

### Requirement: Monitor registration is independent from watchers

NeoMove motion objects SHALL be registered with `NeoMoveIoMonitorThread` when
created by `NeoMoveMotionMgr`, even when no external watcher has been registered.

#### Scenario: Axis created without watcher

- **WHEN** `NeoMoveMotionMgr::GetAxis(id)` creates a new Axis object
- **THEN** the Axis object is registered with the monitor using a null watcher
- **AND** the monitor can refresh the Axis cache on later polls

#### Scenario: InputIO created without watcher

- **WHEN** `NeoMoveMotionMgr::GetInputIo(addr, bit)` creates a new InputIO object
- **THEN** the InputIO object is registered with the monitor using a null watcher
- **AND** the monitor can refresh the InputIO cache on later polls

#### Scenario: OutputIO created without watcher

- **WHEN** `NeoMoveMotionMgr::GetOutputIo(addr, bit)` creates a new OutputIO object
- **THEN** the OutputIO object is registered with the monitor using a null watcher
- **AND** the monitor can refresh the OutputIO cache on later polls

#### Scenario: Watcher registered later

- **WHEN** a caller later calls `StartWatching()` on a monitored object
- **THEN** monitor cache refresh continues for the same object
- **AND** watcher callbacks are invoked only when a non-null watcher is present

### Requirement: Axis read APIs use monitor cache

Axis read APIs SHALL return values from the latest monitor-updated cached
`NM_AXISSTATUS` instead of directly polling the NeoMove SDK.

#### Scenario: Axis cache is ready

- **WHEN** a caller requests servo state, home state, operation state, actual position, actual velocity, or target position after monitor sampling
- **THEN** the API returns a value computed from cached `NM_AXISSTATUS`

#### Scenario: Axis cache is not ready

- **WHEN** a caller requests an Axis cached value before the monitor has produced a successful sample
- **THEN** the API returns failure
- **AND** the Axis error contains `cache_not_ready`

#### Scenario: Axis Wait completes from monitor update

- **WHEN** `Wait()` is pending and the monitor observes `motionComplete` and `inPos`
- **THEN** `UpdateCachedStatus()` wakes `Wait()`
- **AND** `Wait()` returns success

#### Scenario: Axis Wait fails from monitor update

- **WHEN** `Wait()` is pending and the monitor observes servo off, a hard fault, or motion toward a triggered limit
- **THEN** `UpdateCachedStatus()` wakes `Wait()`
- **AND** `Wait()` returns failure with motion error details

### Requirement: IO ReadValue uses monitor cache

InputIO and OutputIO `ReadValue()` SHALL return cached logical IO values instead
of directly reading PDO state from the NeoMove SDK.

#### Scenario: InputIO cache is ready

- **WHEN** a caller reads an InputIO after the monitor has sampled it
- **THEN** `ReadValue()` returns the cached logical input value
- **AND** `ReadValue()` does not call `NM_EtherCATReadPDO`

#### Scenario: OutputIO cache is ready

- **WHEN** a caller reads an OutputIO after the monitor has sampled it
- **THEN** `ReadValue()` returns the cached logical output value
- **AND** `ReadValue()` does not call `NM_EtherCATReadPDO`

#### Scenario: IO cache is not ready

- **WHEN** a caller reads InputIO or OutputIO before the monitor has produced a successful sample
- **THEN** `ReadValue()` returns failure
- **AND** the IO error contains `cache_not_ready`

### Requirement: Output writes update local logical cache

OutputIO `WriteValue()` SHALL update the object's local cached logical value
after a successful SDK write, while allowing the monitor to correct the cache
with later hardware samples.

#### Scenario: Output byte write succeeds

- **WHEN** `WriteValue(byte_val)` succeeds for an OutputIO configured as a byte output
- **THEN** the OutputIO object's cache is immediately updated to `byte_val`

#### Scenario: Output bit write succeeds

- **WHEN** `WriteValue(byte_val)` succeeds for an OutputIO configured as a bit output
- **THEN** the OutputIO object's cache is immediately updated to logical `1` for non-zero values and `0` for zero values

#### Scenario: Output write fails

- **WHEN** `WriteValue(byte_val)` fails
- **THEN** the OutputIO object's cache is not overwritten by the requested value

### Requirement: Monitor polling does not hold registration locks during SDK calls

`NeoMoveIoMonitorThread` SHALL avoid holding registration map mutexes while
calling NeoMove SDK read functions or external watcher callbacks.

#### Scenario: InputIO monitor poll

- **WHEN** `MonitorInputIo()` polls registered input entries
- **THEN** it performs SDK reads and watcher callbacks outside `input_io_mutex_`

#### Scenario: OutputIO monitor poll

- **WHEN** `MonitorOutputIo()` polls registered output entries
- **THEN** it performs SDK reads and watcher callbacks outside `output_io_mutex_`

#### Scenario: Axis monitor poll

- **WHEN** `MonitorAxis()` polls registered Axis entries
- **THEN** it performs SDK reads and watcher callbacks outside `axis_mutex_`
