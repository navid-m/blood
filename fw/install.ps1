# Build and install Blood on Windows. Requires cl.exe on PATH.
[CmdletBinding()]
param(
    [string]$OutDir = "$env:LOCALAPPDATA\Programs\blood"
)

$ErrorActionPreference = "Stop"

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    throw "cl.exe was not found. Run this from a Visual Studio Developer PowerShell prompt."
}

$Root = Split-Path -Parent $PSScriptRoot
$Source = Join-Path $Root "generated\blood_windows.c"
$Compat = Join-Path $Root "src\windows_compat.c"

if (-not (Test-Path $Source)) {
    throw "generated\blood_windows.c is missing. Generate it with 'spectre build sw'."
}
if (-not (Test-Path $Compat)) {
    throw "src\windows_compat.c is missing."
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
$ObjDir = Join-Path $env:TEMP "blood-build"
New-Item -ItemType Directory -Force -Path $ObjDir | Out-Null
$Output = Join-Path $OutDir "blood.exe"

Push-Location $Root
try {
    & cl.exe /nologo /O2 /W3 /D_CRT_SECURE_NO_WARNINGS /utf-8 `
        "/Fo:$ObjDir\" "/Fd:$ObjDir\blood.pdb" "/Fe:$Output" `
        $Source $Compat `
        /link /SUBSYSTEM:CONSOLE ws2_32.lib
    if ($LASTEXITCODE -ne 0) {
        throw "cl.exe failed with exit code $LASTEXITCODE"
    }
}
finally {
    Pop-Location
}

& $Output -v
Write-Host ""
Write-Host "Blood installed at $Output"
Write-Host "Add '$OutDir' to PATH to run 'blood' from any terminal."
