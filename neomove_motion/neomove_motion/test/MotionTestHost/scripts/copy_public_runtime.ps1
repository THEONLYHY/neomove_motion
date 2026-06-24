param(
  [Parameter(Mandatory = $true)]
  [string]$LibraryPath,

  [Parameter(Mandatory = $true)]
  [string]$TargetDir
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($TargetDir)) {
  throw "TargetDir is empty."
}

$targetFullPath = [System.IO.Path]::GetFullPath($TargetDir)
New-Item -ItemType Directory -Force -Path $targetFullPath | Out-Null

$publicBin = $null
foreach ($entry in ($LibraryPath -split ";")) {
  $libPath = $entry.Trim().Trim('"')
  if ([string]::IsNullOrWhiteSpace($libPath)) {
    continue
  }

  if ($libPath.StartsWith('$(')) {
    continue
  }

  $leaf = Split-Path -Leaf $libPath
  if ($leaf -ine "lib_2019" -and $leaf -ine "lib") {
    continue
  }

  $candidate = Join-Path (Split-Path -Parent $libPath) "bin"
  $debugRuntime = Join-Path $candidate "module_runtime_d.dll"
  $releaseRuntime = Join-Path $candidate "module_runtime.dll"
  if ((Test-Path -LiteralPath $debugRuntime) -or
      (Test-Path -LiteralPath $releaseRuntime)) {
    $publicBin = $candidate
    break
  }
}

if ([string]::IsNullOrWhiteSpace($publicBin)) {
  throw 'Could not find public/bin from LibraryPath. Add the public lib_2019 directory to Microsoft.Cpp.$(Platform).user.props LibraryPath.'
}

$dlls = Get-ChildItem -LiteralPath $publicBin -Filter "*.dll" -File
if ($dlls.Count -eq 0) {
  throw "No public runtime DLLs found in $publicBin."
}

foreach ($dll in $dlls) {
  Copy-Item -LiteralPath $dll.FullName -Destination $targetFullPath -Force
}

Write-Host "Copied $($dlls.Count) public runtime DLLs from $publicBin to $targetFullPath."
