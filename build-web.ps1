<#
.SYNOPSIS
    Automated WebGPU build and packaging script for Project Grimoire.

.DESCRIPTION
    Configures and compiles the Grimoire C++20 engine to WebAssembly using Emscripten
    with the modern WebGPU (SOKOL_WGPU) backend and size-minimized flags.

.PARAMETER Clean
    Cleans previous web build directory before configuring.

.PARAMETER Config
    Build configuration: "Release" (default) or "Debug".

.PARAMETER Serve
    Launches a local development web server to test in browser.

.PARAMETER Port
    Port number for the development server (default: 8080).
#>

[CmdletBinding()]
param (
    [switch]$Clean,
    [ValidateSet("Release", "Debug")]
    [string]$Config = "Release",
    [switch]$Serve,
    [int]$Port = 8080
)

$ErrorActionPreference = "Stop"
$sw = [System.Diagnostics.Stopwatch]::StartNew()

Write-Host "=========================================================" -ForegroundColor Cyan
Write-Host "     Project Grimoire: WebGPU WebAssembly Build          " -ForegroundColor Yellow
Write-Host "=========================================================" -ForegroundColor Cyan

# 0. Check for Emscripten SDK and auto-activate if found
$emccCmd = Get-Command "emcc" -ErrorAction SilentlyContinue
if (-not $emccCmd) {
    $knownEmsdkRoots = @("C:\Dev\emsdk", "C:\git\eatsbits\emsdk", "$env:USERPROFILE\emsdk")
    foreach ($cand in $knownEmsdkRoots) {
        $envScript = Join-Path $cand "emsdk_env.ps1"
        if (Test-Path $envScript) {
            Write-Host "[*] Auto-activating Emscripten SDK from $cand..." -ForegroundColor Cyan
            $env:EMSDK_QUIET = "1"
            . $envScript
            break
        }
    }
    $emccCmd = Get-Command "emcc" -ErrorAction SilentlyContinue
}

if (-not $emccCmd) {
    Write-Host "`n[!] Emscripten SDK ('emcc') was not found in your PATH." -ForegroundColor Red
    Write-Host "    To set up Emscripten:" -ForegroundColor Yellow
    Write-Host "      1. git clone https://github.com/emscripten-core/emsdk.git"
    Write-Host "      2. cd emsdk"
    Write-Host "      3. .\emsdk.bat install latest"
    Write-Host "      4. .\emsdk.bat activate latest"
    Write-Host "      5. .\emsdk_env.ps1"
    Write-Host ""
    exit 1
}

$projectRoot = $PSScriptRoot
$buildDir = Join-Path $projectRoot "build-web"

if ($Clean -and (Test-Path $buildDir)) {
    Write-Host "`n[*] Cleaning previous web build directory: $buildDir" -ForegroundColor Yellow
    Remove-Item -Recurse -Force $buildDir
}

if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

# 1. Configure with Emscripten CMake wrapper
Write-Host "`n[*] Configuring CMake project with WebGPU backend..." -ForegroundColor Yellow
$configArgs = @("-B", "build-web", "-S", ".", "-DCMAKE_BUILD_TYPE=$Config", "-DGRIMOIRE_BUILD_TESTS=OFF")
& emcmake cmake @configArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "Emscripten CMake configuration failed!"
    exit 1
}

# 2. Compile WebAssembly bundle
Write-Host "`n[*] Compiling Grimoire WebGPU Wasm binary ($Config)..." -ForegroundColor Yellow
$buildArgs = @("--build", "build-web", "--config", $Config, "--parallel")
& cmake @buildArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "Web compilation failed!"
    exit 1
}

$sw.Stop()
Write-Host "`n[+] WebGPU compilation succeeded in $([math]::Round($sw.Elapsed.TotalSeconds, 2))s!" -ForegroundColor Green

# 3. Analyze and Report Payload Sizes
$wasmFile = Join-Path $buildDir "grimoire.wasm"
$jsFile   = Join-Path $buildDir "grimoire.js"
$htmlFile = Join-Path $buildDir "grimoire.html"

if (Test-Path $wasmFile) {
    $wasmSize = (Get-Item $wasmFile).Length / 1KB
    Write-Host "`n--- Payload Footprint ---" -ForegroundColor Cyan
    Write-Host ("  grimoire.wasm : {0:N1} KB (uncompressed)" -f $wasmSize) -ForegroundColor Yellow
    if (Test-Path $jsFile) {
        $jsSize = (Get-Item $jsFile).Length / 1KB
        Write-Host ("  grimoire.js   : {0:N1} KB (JS glue)" -f $jsSize) -ForegroundColor Yellow
    }
    if (Test-Path $htmlFile) {
        $htmlSize = (Get-Item $htmlFile).Length / 1KB
        Write-Host ("  grimoire.html : {0:N1} KB" -f $htmlSize) -ForegroundColor Yellow
    }
    Write-Host ("  Estimated Brotli payload : ~{0:N1} KB" -f ($wasmSize * 0.28)) -ForegroundColor Green
    Write-Host "-------------------------" -ForegroundColor Cyan
}

# 4. Optional Local Server
if ($Serve) {
    Write-Host "`n[*] Starting local WebGPU server on http://localhost:$Port/grimoire.html" -ForegroundColor Green
    Write-Host "    Press Ctrl+C to terminate the server.`n" -ForegroundColor DarkGray
    
    $pythonCmd = Get-Command "python" -ErrorAction SilentlyContinue
    if ($pythonCmd) {
        python -m http.server $Port --directory $buildDir
    } else {
        npx -y serve $buildDir -l $Port
    }
}
