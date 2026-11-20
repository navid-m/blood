# Build and install Blood on Windows. Uses cl.exe when available, otherwise
# falls back to clang.exe.
[CmdletBinding()]
param(
    [string]$OutDir = "$env:LOCALAPPDATA\Programs\blood"
)

$ErrorActionPreference = "Stop"

$Cl = Get-Command cl.exe -ErrorAction SilentlyContinue
$Clang = Get-Command clang.exe -ErrorAction SilentlyContinue
if (-not $Cl -and -not $Clang) {
    throw "Neither cl.exe nor clang.exe was found. Run this from a Visual Studio Developer PowerShell prompt or install LLVM."
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
    if ($Cl) {
        & cl.exe /nologo /O2 /W3 /D_CRT_SECURE_NO_WARNINGS /utf-8 `
            "/Fo:$ObjDir\" "/Fd:$ObjDir\blood.pdb" "/Fe:$Output" `
            $Source $Compat `
            /link /SUBSYSTEM:CONSOLE ws2_32.lib
        if ($LASTEXITCODE -ne 0) {
            throw "cl.exe failed with exit code $LASTEXITCODE"
        }
    } else {
        # clang.exe uses the MSVC toolchain headers/libs when run from a
        # Developer PowerShell prompt, but also works with an LLVM/MinGW setup.
        & clang.exe -O2 -w -std=gnu17 -D_CRT_SECURE_NO_WARNINGS `
            -Wno-error=int-conversion `
            -Wno-error=implicit-function-declaration `
            -o $Output $Source $Compat -lws2_32
        if ($LASTEXITCODE -ne 0) {
            throw "clang.exe failed with exit code $LASTEXITCODE"
        }
    }
}
finally {
    Pop-Location
}

& $Output -v
Write-Host ""
Write-Host "Blood installed at $Output"
Write-Host "Add '$OutDir' to PATH to run 'blood' from any terminal."
