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

function Copy-FileIfChanged($sourcePath, $destinationPath) {
  $sourceItem = Get-Item -LiteralPath $sourcePath
  if (Test-Path -LiteralPath $destinationPath) {
    $destinationItem = Get-Item -LiteralPath $destinationPath
    if ($sourceItem.Length -eq $destinationItem.Length -and
        $sourceItem.LastWriteTimeUtc -eq $destinationItem.LastWriteTimeUtc) {
      return $false
    }
  }

  New-Item -ItemType Directory -Force -Path (Split-Path -Parent $destinationPath) |
    Out-Null
  Copy-Item -LiteralPath $sourcePath -Destination $destinationPath -Force
  return $true
}

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

$copiedDllCount = 0
foreach ($dll in $dlls) {
  $destination = Join-Path $targetFullPath $dll.Name
  if (Copy-FileIfChanged $dll.FullName $destination) {
    $copiedDllCount += 1
  }
}

$actionSource = Join-Path $publicBin "action"
if (!(Test-Path -LiteralPath $actionSource)) {
  throw "No public action runtime directory found in $publicBin."
}

$actionFiles = Get-ChildItem -LiteralPath $actionSource -Recurse -File
if ($actionFiles.Count -eq 0) {
  throw "No public action runtime files found in $actionSource."
}

$actionTarget = Join-Path $targetFullPath "action"
$copiedActionCount = 0
foreach ($file in $actionFiles) {
  $relativePath = $file.FullName.Substring($actionSource.Length).TrimStart(
    [char[]]@([System.IO.Path]::DirectorySeparatorChar,
              [System.IO.Path]::AltDirectorySeparatorChar))
  $destination = Join-Path $actionTarget $relativePath
  if (Copy-FileIfChanged $file.FullName $destination) {
    $copiedActionCount += 1
  }
}

Write-Host "Copied $copiedDllCount/$($dlls.Count) public runtime DLLs and $copiedActionCount/$($actionFiles.Count) action files from $publicBin to $targetFullPath."
