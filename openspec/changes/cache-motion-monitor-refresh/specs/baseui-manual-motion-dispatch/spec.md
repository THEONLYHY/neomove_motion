## ADDED Requirements

### Requirement: Manual robot motion runs on motion loop

BaseUI manual robot motion slots SHALL dispatch motion object lookup and motion
API calls to `common::kMotion` instead of executing those calls directly on the
Qt UI slot call stack.

#### Scenario: User clicks manual scan or index move

- **WHEN** `RobotManual::OnButtonClicked()` is triggered from the UI
- **THEN** transient UI/config values needed for the move are copied before posting
- **AND** axis lookup, `GetActualPosition()`, and `AsyncMoveTo()` run on `common::kMotion`

#### Scenario: User starts manual jog

- **WHEN** `RobotManual::OnButtonLongPressed()` is triggered from the UI
- **THEN** transient UI/config values needed for the jog are copied before posting
- **AND** axis lookup and `StartJog()` run on `common::kMotion`

#### Scenario: User releases manual jog

- **WHEN** `RobotManual::OnButtonLongPressReleased()` is triggered from the UI
- **THEN** the axis id is copied before posting
- **AND** axis lookup and `Stop()` run on `common::kMotion`

#### Scenario: Posted task captures data

- **WHEN** a manual motion task is posted to `common::kMotion`
- **THEN** the task does not capture references to slot parameters
- **AND** the task does not read mutable Qt UI state after it leaves the UI slot
