$ErrorActionPreference = "Stop"

cmake -S . -B build -A x64
cmake --build build --config Release --target package_release

$dist = "build/dist/F2一键断网_v1.0"
if (!(Test-Path "$dist/F2一键断网.exe")) { throw "Missing exe: $dist/F2一键断网.exe" }
Compress-Archive -Path $dist -DestinationPath "F2_NetworkSwitch_v1.0.zip" -Force
Write-Host "Done: F2_NetworkSwitch_v1.0.zip"
