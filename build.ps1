<#
.SYNOPSIS
    Automated build & test script for Project Grimoire Game Engine.

.DESCRIPTION
    Configures and compiles the Grimoire C++20 atmospheric pixel-art writing engine.
    Supports Clean, Release/Debug configurations, test suite execution, and instant launching.

.PARAMETER Clean
    Cleans previous build directory before configuring.

.PARAMETER Config
    Build configuration: "Release" (default) or "Debug".

.PARAMETER Run
    Immediately launches grimoire.exe upon successful build.

.PARAMETER Test
    Compiles and executes the automated test suite.
#>

[CmdletBinding()]
param (
    [switch]$Clean,
    [ValidateSet("Release", "Debug")]
    [string]$Config = "Release",
    [switch]$Run,
    [switch]$Test
)

$ErrorActionPreference = "Stop"
$sw = [System.Diagnostics.Stopwatch]::StartNew()

Write-Host "=========================================================" -ForegroundColor Cyan
Write-Host "      Project Grimoire: Pixel-Art Writing Engine         " -ForegroundColor Yellow
Write-Host "=========================================================" -ForegroundColor Cyan

$projectRoot = $PSScriptRoot
$buildDir = Join-Path $projectRoot "build"

if ($Clean -and (Test-Path $buildDir)) {
    Write-Host "`n[*] Cleaning previous build directory: $buildDir" -ForegroundColor Yellow
    Remove-Item -Recurse -Force $buildDir
}

if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

# 1. CMake Configure
Write-Host "`n[*] Configuring CMake project..." -ForegroundColor Yellow
$configArgs = @("-B", "build", "-S", ".")
if ($Test) {
    $configArgs += "-DGRIMOIRE_BUILD_TESTS=ON"
}
& cmake @configArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed!"
    exit 1
}

# 2. CMake Build
Write-Host "`n[*] Compiling Grimoire Engine ($Config)..." -ForegroundColor Yellow
$buildArgs = @("--build", "build", "--config", $Config, "--parallel")
& cmake @buildArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "Compilation failed!"
    exit 1
}

$sw.Stop()
Write-Host "`n[+] Build completed successfully in $([math]::Round($sw.Elapsed.TotalSeconds, 2))s!" -ForegroundColor Green

# 3. Run Test Suite if requested
if ($Test) {
    Write-Host "`n[*] Executing Grimoire Test Suite..." -ForegroundColor Yellow
    $testExe = Join-Path $buildDir "$Config\test_grimoire.exe"
    if (Test-Path $testExe) {
        & $testExe
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Test suite execution failed!"
            exit 1
        }
    } else {
        Write-Error "Test executable not found at: $testExe"
        exit 1
    }
}

# 4. Run Application if requested
if ($Run) {
    Write-Host "`n[*] Launching Grimoire..." -ForegroundColor Green
    $appExe = Join-Path $buildDir "$Config\grimoire.exe"
    if (Test-Path $appExe) {
        Start-Process -FilePath $appExe
    } else {
        Write-Error "Grimoire executable not found at: $appExe"
        exit 1
    }
}
