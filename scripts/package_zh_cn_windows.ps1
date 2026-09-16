param(
    [Parameter(Mandatory = $true)][string] $BuildDir,
    [Parameter(Mandatory = $true)][string] $QtDir
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repoDir = Split-Path $PSScriptRoot -Parent
$buildRoot = (Resolve-Path -LiteralPath $BuildDir).Path
$qtRoot = (Resolve-Path -LiteralPath $QtDir).Path
$binaryDir = Join-Path $buildRoot 'output'
$packageDir = Join-Path $buildRoot 'package'

# A fresh destination prevents stale DLLs from hiding deployment errors.
if (Test-Path -LiteralPath $packageDir) {
    throw "Package directory already exists: $packageDir. Use a clean build directory."
}
New-Item -ItemType Directory -Path $packageDir | Out-Null

foreach ($name in @('klogg.exe', 'klogg_portable.exe', 'klogg_grep.exe',
                     'klogg_crashpad_handler.exe', 'klogg_minidump_dump.exe', 'tbb12.dll')) {
    Copy-Item -LiteralPath (Join-Path $binaryDir $name) -Destination $packageDir
}

# Use the actual build outputs instead of hardcoding an MSVC version or build type.
Get-ChildItem -LiteralPath $binaryDir -Filter '*.dll' -File |
    Copy-Item -Destination $packageDir -Force

$deployTool = Join-Path $qtRoot 'bin/windeployqt.exe'
& $deployTool --release --no-compiler-runtime --no-translations `
    --dir $packageDir (Join-Path $packageDir 'klogg.exe')
if ($LASTEXITCODE -ne 0) { throw 'windeployqt failed' }

# App and Qt translations are embedded by src/app/CMakeLists.txt. Use the Windows
# TLS backend so networking does not depend on an external OpenSSL installation.
$tlsDir = Join-Path $packageDir 'tls'
New-Item -ItemType Directory -Force -Path $tlsDir | Out-Null
Copy-Item -LiteralPath (Join-Path $qtRoot 'plugins/tls/qschannelbackend.dll') -Destination $tlsDir

# Deploy the redistributable CRT files from the official Visual Studio redist
# directory, matching the x64 compiler, so the zip needs no separate installer.
if (!$env:VCToolsRedistDir) { throw 'VCToolsRedistDir is not set; initialize MSVC x64 first' }
$crtDirs = @(Get-ChildItem -LiteralPath (Join-Path $env:VCToolsRedistDir 'x64') -Directory |
    Where-Object { $_.Name -match '^Microsoft\.VC\d+\.CRT$' })
if ($crtDirs.Count -ne 1) { throw 'Cannot uniquely locate the x64 Visual C++ redistributable CRT' }
Get-ChildItem -LiteralPath $crtDirs[0].FullName -Filter '*.dll' -File |
    Copy-Item -Destination $packageDir -Force

foreach ($name in @('COPYING', 'NOTICE', 'README.md', 'DOCUMENTATION.md')) {
    Copy-Item -LiteralPath (Join-Path $repoDir $name) -Destination $packageDir
}
Copy-Item -LiteralPath (Join-Path $buildRoot 'generated/documentation.html') -Destination $packageDir

$qtLicenses = Join-Path $qtRoot 'licenses'
if (Test-Path -LiteralPath $qtLicenses) {
    Copy-Item -LiteralPath $qtLicenses -Destination (Join-Path $packageDir 'qt-licenses') -Recurse
}

@'
klogg Windows x64 / Qt 6 / 简体中文

解压整个目录后运行 klogg.exe。
如需将设置保存在解压目录中，运行 klogg_portable.exe（该目录须可写）。
首次启动会根据系统语言选择界面语言；已有语言设置会保留。
可在“文件 → 设置”中选择 English 或中文（简体）。
应用和 Qt 中文翻译已内置，无需复制外部 .qm 文件。

此包仅支持 Windows x64，保留 Hyperscan 搜索引擎。
自动测试和构建记录见对应 GitHub Actions 运行页面。
'@ | Set-Content -LiteralPath (Join-Path $packageDir '使用说明.txt') -Encoding utf8

$qmPath = Join-Path $buildRoot 'src/app/zh_CN.qm'
if (!(Test-Path -LiteralPath $qmPath)) { throw 'The compiled Simplified Chinese QM resource is missing' }

Write-Output "Portable package ready: $packageDir"
