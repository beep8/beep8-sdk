@echo off
REM -----------------------------------------------------------------------------
REM beep8-env.bat — Prepend the SDK tool directories to PATH on Windows/MS-DOS
REM -----------------------------------------------------------------------------

REM Get the directory where this script is located (including trailing backslash)
SET "SCRIPT_DIR=%~dp0"

REM Prepend the tool\make and tool\busybox directories to the PATH
SET "PATH=%SCRIPT_DIR%tool\make;%SCRIPT_DIR%tool\busybox;%PATH%"

REM Inform the user
echo Updated PATH to include:
echo   %SCRIPT_DIR%tool\make
echo   %SCRIPT_DIR%tool\busybox
