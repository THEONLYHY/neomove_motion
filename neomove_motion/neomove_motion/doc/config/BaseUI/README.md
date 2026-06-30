# BaseUI Runtime Config

This config package is copied to `<BaseUI output>/config` by the BaseUI
post-build step.

This package is currently configured for real NeoMove hardware. Both
`axis.json` and `io.json` must keep `default_module` set to `neomove_motion`,
and `neomove.json` must match the site controller and axis parameters before
hardware operations.

Use `test/BaseUI/scripts/verify_real_neomove_runtime_contract.ps1` to verify the
post-build runtime directory. The existing
`test/BaseUI/scripts/verify_motion_runtime_contract.ps1` is the local virtual
machine contract and must not be used to approve real equipment configuration.
