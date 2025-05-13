@echo off
setlocal enabledelayedexpansion

REM --- Determine project name (current directory name)
for %%I in (.) do set "PROJECT=%%~nxI"

REM --- Locate common script one level up
set "SCRIPT_DIR=%~dp0"
set "COMMON_SCRIPT=%SCRIPT_DIR%..\run_common.bat"

if not exist "%COMMON_SCRIPT%" (
  echo Error: common run script not found at %COMMON_SCRIPT% 1>&2
  exit /b 1
)

REM --- Execute common script with all forwarded arguments
call "%COMMON_SCRIPT%" %*
exit /b %ERRORLEVEL%