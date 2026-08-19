@echo off
setlocal
set "UE_DIR="
set "UE_VERSION=5.8"
set "BASE_DIR=%~dp0"
set "RELEASE_DIR=%BASE_DIR%Release\FSRUE"
set "REG_KEY=HKEY_LOCAL_MACHINE\SOFTWARE\Epic Games\Unreal Engine\%UE_VERSION%"
echo Looking For Unreal Engine %UE_VERSION%...
for /F "tokens=2*" %%A in ('reg query "%REG_KEY%" /v "InstalledDirectory" 2^>nul') do (
    set "UE_DIR=%%B"
)
if "%UE_DIR%"=="" (
    echo [ERROR] Unreal Engine %UE_VERSION% Not Found!
    pause
    exit /b 1
)
echo Found At: %UE_DIR%
set "MARKETPLACE_DIR=%UE_DIR%\Engine\Plugins\Marketplace"
echo Marketplace Directory: %MARKETPLACE_DIR%
if not exist "%MARKETPLACE_DIR%" (
    mkdir "%MARKETPLACE_DIR%"
)
rmdir "%BASE_DIR%Release" /s /q 2>nul
echo.
set "RUN_UAT=%UE_DIR%\Engine\Build\BatchFiles\RunUAT.bat"
echo ========================================================
echo Building Plugin: FSR
echo ========================================================
call "%RUN_UAT%" BuildPlugin -plugin="%BASE_DIR%FSR\FSR.uplugin" -package="%RELEASE_DIR%\FSR"
echo.
echo ========================================================
echo Building Plugin: FSRMRP
echo ========================================================
mklink /J "%MARKETPLACE_DIR%\FSR" "%RELEASE_DIR%\FSR" >nul
call "%RUN_UAT%" BuildPlugin -plugin="%BASE_DIR%FSRMRP\FSRMRP.uplugin" -package="%RELEASE_DIR%\FSRMRP"
rmdir "%MARKETPLACE_DIR%\FSR"
rmdir "%MARKETPLACE_DIR%" 2>nul
echo.
echo ========================================================
echo Packaging Plugins
echo ========================================================
copy /Y "%BASE_DIR%License.txt" "%RELEASE_DIR%\" >nul
copy /Y "%BASE_DIR%Readme.md" "%RELEASE_DIR%\" >nul
copy /Y "%BASE_DIR%Engine.patch" "%RELEASE_DIR%\" >nul
powershell -command "Compress-Archive -Path '%RELEASE_DIR%' -DestinationPath '%BASE_DIR%FSRUE-%UE_VERSION%.zip' -Force"
rmdir "%BASE_DIR%Release" /s /q 2>nul
echo.
echo ========================================================
echo Process Completed!
echo ========================================================
pause