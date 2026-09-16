param([Parameter(Mandatory = $true)][string] $PackageDir)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$packageRoot = (Resolve-Path -LiteralPath $PackageDir).Path
$reportPath = Join-Path (Split-Path $packageRoot -Parent) 'package-validation.txt'
$report = [System.Collections.Generic.List[string]]::new()

foreach ($name in @('klogg.exe', 'klogg_portable.exe', 'klogg_grep.exe', 'Qt6Core.dll', 'Qt6Gui.dll',
                     'Qt6Widgets.dll', 'Qt6Network.dll', 'Qt6Core5Compat.dll',
                     'tbb12.dll', 'vcruntime140.dll', 'msvcp140.dll',
                     'platforms/qwindows.dll', 'tls/qschannelbackend.dll')) {
    if (!(Test-Path -LiteralPath (Join-Path $packageRoot $name))) {
        throw "Missing runtime dependency: $name"
    }
}
$report.Add('PASS: required application, Qt6, platform, TLS and MSVC runtime files are present')

# Inspect every deployed PE file, including plugins, to reject accidental x86 DLLs.
foreach ($file in Get-ChildItem -LiteralPath $packageRoot -Recurse -File |
    Where-Object { $_.Extension -in @('.exe', '.dll') }) {
    $reader = [System.IO.BinaryReader]::new([System.IO.File]::OpenRead($file.FullName))
    try {
        if ($reader.ReadUInt16() -ne 0x5A4D) { throw "Invalid PE file: $($file.Name)" }
        $reader.BaseStream.Position = 0x3C
        $peOffset = $reader.ReadInt32()
        $reader.BaseStream.Position = $peOffset
        if ($reader.ReadUInt32() -ne 0x00004550 -or $reader.ReadUInt16() -ne 0x8664) {
            throw "The package contains a non-x64 binary: $($file.Name)"
        }
    }
    finally { $reader.Dispose() }
}
$report.Add('PASS: every deployed executable and DLL uses the x64 PE machine type')

$report | Set-Content -LiteralPath $reportPath -Encoding utf8
$report | Write-Output

# A temporary copy keeps portable settings and generated logs out of the package.
python "$PSScriptRoot/test_zh_cn_windows_runtime.py" --package-dir $packageRoot --report $reportPath
if ($LASTEXITCODE -ne 0) { throw 'Packaged runtime tests failed' }
