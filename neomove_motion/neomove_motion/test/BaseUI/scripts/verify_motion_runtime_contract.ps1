param(
  [string]$RepoRoot = (Resolve-Path "$PSScriptRoot\..\..\..\..").Path
)

$sourcePath = Join-Path $RepoRoot "neomove_motion\test\BaseUI\BaseUI\src\view\main_window\base_main_window.cc"
if (-not (Test-Path -LiteralPath $sourcePath)) {
  Write-Error "base_main_window.cc not found: $sourcePath"
  exit 1
}

$source = Get-Content -LiteralPath $sourcePath -Raw
$setDllDirectoryIndex = $source.IndexOf("SetDllDirectoryW")
$moduleInitIndex = $source.IndexOf("main_process::GetModuleMgr()->Init")
if ($setDllDirectoryIndex -lt 0 -or $moduleInitIndex -lt 0 -or
    $setDllDirectoryIndex -gt $moduleInitIndex) {
  Write-Error "motion DLL search path must be configured before ModuleMgr::Init"
  exit 1
}

if ($source -notmatch "module_mgr_initialized_\s*=\s*InitModuleMgr\(\)") {
  Write-Error "ModuleMgr init result must be saved before starting motion-dependent services"
  exit 1
}

if ($source -notmatch "if\s*\(\s*module_mgr_initialized_\s*\)\s*\{[\s\S]*DeviceStatusMonitorSinglton::GetInstance\(\)->Start\(\)") {
  Write-Error "DeviceStatusMonitor must only start after ModuleMgr initialization succeeds"
  exit 1
}

$projectPath = Join-Path $RepoRoot "neomove_motion\test\BaseUI\BaseUI\BaseUI.vcxproj"
if (-not (Test-Path -LiteralPath $projectPath)) {
  Write-Error "BaseUI.vcxproj not found: $projectPath"
  exit 1
}

$project = Get-Content -LiteralPath $projectPath -Raw
if ($project -match '<PublicSdkDir[^>]*>[A-Za-z]:\\') {
  Write-Error "PublicSdkDir must not be committed as a machine-specific absolute path"
  exit 1
}

if ($project -notmatch '\.\.\\\.\.\\\.\.\\doc\\config\\BaseUI') {
  Write-Error "BaseUI project must copy tracked BaseUI config into output config directory"
  exit 1
}

if ($project -notmatch '\$\(PublicSdkDir\)bin\\motion') {
  Write-Error "BaseUI project must copy motion plugins from the public SDK into output motion directory"
  exit 1
}

$configRoot = Join-Path $RepoRoot "neomove_motion\doc\config\BaseUI"
$axisConfigPath = Join-Path $configRoot "axis.json"
$ioConfigPath = Join-Path $configRoot "io.json"
foreach ($configPath in @($axisConfigPath, $ioConfigPath)) {
  if (-not (Test-Path -LiteralPath $configPath)) {
    Write-Error "BaseUI runtime config not found: $configPath"
    exit 1
  }
}

$axisConfig = Get-Content -LiteralPath $axisConfigPath -Raw
$ioConfig = Get-Content -LiteralPath $ioConfigPath -Raw
if ($axisConfig -notmatch '"default_module"\s*:\s*"virtual_motion"' -or
    $axisConfig -match '"module"\s*:\s*"neomove_motion"') {
  Write-Error "BaseUI axis config must use virtual_motion by default for local development"
  exit 1
}

if ($ioConfig -notmatch '"default_module"\s*:\s*"virtual_motion"') {
  Write-Error "BaseUI IO config must use virtual_motion by default for local development"
  exit 1
}

Write-Output "motion runtime contract verified"
