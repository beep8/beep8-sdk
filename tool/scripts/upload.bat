@echo off
REM ==== BEEP-8 ROM Image Upload & Browser Launch Script ====
REM This BAT file uploads the pre-built BEEP-8 ROM image to beep8.org and launches it in the browser.
REM This script uses curl, installed by default on Windows, for the upload.

setlocal enabledelayedexpansion


REM Default ports
set http_port=443
set api_port=8082

REM Get the directory of the current script
set "script_dir=%~dp0"

REM Set the path to busybox relative to the script directory
set "busybox_path=%script_dir%..\busybox\busybox.exe"

REM Check if command line arguments are provided
if "%~2" neq "" (
    set http_port=%~2
)
if "%~3" neq "" (
    set api_port=%~3
)

set b8path=%~1

REM Check if argument contains non-ASCII characters, allowing ':'
set "cleaned_path=%b8path::=%"
echo %cleaned_path% | %busybox_path% grep -q "[^[:print:]]" && (
    echo %b8path% contains non-ASCII characters.
    exit /b 1
)

REM Validate the file size
for %%A in ("%b8path%") do set "filesize=%%~zA"
if %filesize% gtr 4194304 (
    echo Error: File size exceeds 4MB.
    exit /b 1
)

set "b8path=%~dpfnx1"
set "converted=%b8path:/=_%"
set "converted=%converted::=_%"
set "converted=%converted:\=_%"

set command=curl -s --retry 10 --max-time 10 -X POST -H "Content-Type: application/octet-stream" -H "Content-Disposition: attachment; filename=%converted%" --data-binary @%b8path% https://beep8.org:%api_port%/api/upload

for /f %%i in ('echo prompt $E ^| cmd') do set "ESC=%%i"
set RESET=%ESC%[0m
set BLUE=%ESC%[34m
set CYAN=%ESC%[36m
set YELLOW=%ESC%[33m

for /f "tokens=*" %%i in ('%command% 2^>^&1 ^| %busybox_path% grep -E "^[0-9a-f]{32}\.b8$"') do (
  set "b8file=%%i"
)

if "%b8file%"=="" (
  echo %YELLOW%
  echo Hint:
  echo If the browser does not update automatically, please manually press the browser's refresh button.
  echo %RESET%
  exit /b
)


set "url=https://beep8.org:%http_port%/b8/beep8.html?b8rom=%b8file%&vterm0=1&wdt=1&api_port=%api_port%"

start "" "%url%"

echo %YELLOW%
echo By manually copying and pasting the URL
echo %CYAN% "%url%"
echo %YELLOW%into your web browser, you can run the built program. 
echo It is executable on any web browser, whether on iPhone, Android, Mac, Windows PC, or Linux PC.
echo %RESET%
endlocal