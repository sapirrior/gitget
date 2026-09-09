# gitget Windows Installer (PowerShell)
[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"

$Repo = "sapirrior/gitget"
$BinaryName = "gitget.exe"

Write-Host "=== gitget Windows Installer ===" -ForegroundColor Cyan
Write-Host ""

# 1. Architecture Detection
$Arch = [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture
switch ($Arch) {
    "X64"   { $ArchType = "amd64" }
    "Arm64" { $ArchType = "arm64" }
    Default {
        Write-Error "Unsupported architecture: $Arch"
        exit 1
    }
}

$AssetName = "gitget-windows-$ArchType.exe"
Write-Host "Detected Architecture: $Arch ($ArchType)" -ForegroundColor Green
Write-Host "Target Artifact:      $AssetName" -ForegroundColor Green
Write-Host ""

# 2. Ask permission to proceed with downloading
$ConfirmDownload = Read-Host "Do you want to proceed with downloading '$AssetName'? [Y/n]"
if ($ConfirmDownload -and $ConfirmDownload -notmatch '^[Yy]$') {
    Write-Host "Installation cancelled by user." -ForegroundColor Yellow
    exit 0
}

$DownloadUrl = "https://github.com/$Repo/releases/latest/download/$AssetName"
$TempFile = [System.IO.Path]::Combine([System.IO.Path]::GetTempPath(), "gitget_download.exe")

Write-Host "`nDownloading from $DownloadUrl..." -ForegroundColor Cyan
try {
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12 -bor [System.Net.SecurityProtocolType]::Tls13
    Invoke-WebRequest -Uri $DownloadUrl -OutFile $TempFile -UseBasicParsing
    Write-Host "✓ Download complete!" -ForegroundColor Green
} catch {
    Write-Error "Failed to download $AssetName : $_"
    exit 1
}

# 3. Prompt destination options
Write-Host "`nWhere would you like to install gitget?" -ForegroundColor Cyan
Write-Host "  1) User Local AppData ($env:LOCALAPPDATA\gitget)"
Write-Host "  2) Current Directory ($((Get-Location).Path)) as '$BinaryName'"
Write-Host "  3) Custom Directory"
$DestChoice = Read-Host "Select an option [1-3] (default: 1)"
if (-not $DestChoice) { $DestChoice = "1" }

switch ($DestChoice) {
    "1" {
        $TargetDir = Join-Path $env:LOCALAPPDATA "gitget"
    }
    "2" {
        $TargetDir = (Get-Location).Path
    }
    "3" {
        $TargetDir = Read-Host "Enter custom directory path"
        if (-not $TargetDir) {
            Write-Host "Invalid directory. Defaulting to $env:LOCALAPPDATA\gitget."
            $TargetDir = Join-Path $env:LOCALAPPDATA "gitget"
        }
    }
    Default {
        $TargetDir = Join-Path $env:LOCALAPPDATA "gitget"
    }
}

$TargetPath = Join-Path $TargetDir $BinaryName

# 4. Ask permission before copying
$ConfirmInstall = Read-Host "Install '$BinaryName' to '$TargetPath'? [Y/n]"
if ($ConfirmInstall -and $ConfirmInstall -notmatch '^[Yy]$') {
    Write-Host "Installation aborted. Temporary file cleaned up." -ForegroundColor Yellow
    Remove-Item -Path $TempFile -Force -ErrorAction SilentlyContinue
    exit 0
}

if (-not (Test-Path $TargetDir)) {
    New-Item -ItemType Directory -Path $TargetDir -Force | Out-Null
}

Move-Item -Path $TempFile -Destination $TargetPath -Force
Write-Host "`n✓ Binary successfully installed to: $TargetPath" -ForegroundColor Green

# 5. Check if directory is in PATH and request permission to add it
$CurrentPath = [Environment]::GetEnvironmentVariable("Path", [EnvironmentVariableTarget]::User)
$PathList = $CurrentPath -split ';' | ForEach-Object { $_.TrimEnd('\') }
$NormalizedTargetDir = $TargetDir.TrimEnd('\')

if ($PathList -notcontains $NormalizedTargetDir) {
    Write-Host "`nNotice: '$TargetDir' is not currently in your user PATH." -ForegroundColor Yellow
    $ConfirmPath = Read-Host "Would you like to add '$TargetDir' to your user PATH environment variable? [Y/n]"
    if (-not $ConfirmPath -or $ConfirmPath -match '^[Yy]$') {
        try {
            $NewPath = "$CurrentPath;$TargetDir"
            [Environment]::SetEnvironmentVariable("Path", $NewPath, [EnvironmentVariableTarget]::User)
            $env:Path = "$env:Path;$TargetDir"
            Write-Host "✓ Added '$TargetDir' to user PATH." -ForegroundColor Green
            Write-Host "Please restart your PowerShell terminal for PATH changes to take effect across all sessions." -ForegroundColor Cyan
        } catch {
            Write-Warning "Could not update user PATH: $_"
        }
    } else {
        Write-Host "Skipped modifying user PATH."
    }
}

Write-Host "`nAll set! Run 'gitget --help' to verify installation.`n" -ForegroundColor Green
