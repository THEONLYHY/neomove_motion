# BaseUI Runtime Config

This config package is copied to `<BaseUI output>/config` by the BaseUI
post-build step.

The default axis and IO modules are `virtual_motion`, so BaseUI can start on a
local development machine without a NeoMove controller.

For real equipment, replace `axis.json` and `io.json` module settings with the
site-specific motion module and verify `neomove.json` before running hardware
operations.
