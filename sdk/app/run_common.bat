:: run_common.bat - BEEP-8 SDK Build Script for Windows
@echo off
setlocal EnableDelayedExpansion

for /f %%i in ('echo prompt $E ^| cmd') do set "ESC=%%i"
set RESET=%ESC%[0m
set BLUE=%ESC%[34m
set CYAN=%ESC%[36m
set YELLOW=%ESC%[33m

REM --- Determine project name
for %%I in (.) do set "PROJECT=%%~nxI"

REM --- Verbose flag: if first argument is -v, enable verbose mode
set "VERBOSE=0"
if /I "%~1"=="-v" (
    set "VERBOSE=1"
    echo [VERBOSE mode enabled]
    echo on
    shift
)

REM --- Directory layout
set "OBJDIR=.\obj"
set "EXPORTDIR=.\data\export"
set "IMPORTDIR=.\data\import"

REM --- Locate tool and library roots
set "SCRIPT_DIR=%~dp0"
for %%D in ("%SCRIPT_DIR%..\..") do set "ROOT_DIR=%%~fD"
set "TOOL_ROOT=%ROOT_DIR%\tool"
set "B8LIB_ROOT=%ROOT_DIR%\sdk\b8lib"
set "B8HELPER_ROOT=%ROOT_DIR%\sdk\b8helper"
set "GNUARM_ROOT=%ROOT_DIR%\gnuarm"

REM --- Environment (tool directories use Windows_NT, HW fixed)
set "OS=Windows_NT"
set "HW=x86_64"
set "GCCVER=gcc10.3"
set "CROSS_PREFIX=%GNUARM_ROOT%\%OS%\%GCCVER%\%HW%\bin\arm-none-eabi-"
set "GCC_CMD=%CROSS_PREFIX%gcc"
set "GXX_CMD=%CROSS_PREFIX%g++"
set "AS_CMD=%CROSS_PREFIX%gcc"
set "LD_CMD=%CROSS_PREFIX%g++"
set "OBJCOPY_CMD=%CROSS_PREFIX%objcopy"
set "ODUMP_CMD=%CROSS_PREFIX%objdump"

REM --- Use ccache if available
where ccache >nul 2>&1
if %ERRORLEVEL%==0 (
    set "CCACHE=ccache"
) else (
    set "CCACHE="
)
if defined CCACHE (
    set "CC_CMD=%CCACHE% %GCC_CMD%"
    set "CXX_CMD=%CCACHE% %GXX_CMD%"
) else (
    set "CC_CMD=%GCC_CMD%"
    set "CXX_CMD=%GXX_CMD%"
)

REM --- Helper tools
set "PNG2C=%TOOL_ROOT%\png2c\%OS%\%HW%\png2c.exe"
set "GENB8ROM=%TOOL_ROOT%\genb8rom\%OS%\%HW%\genb8rom.exe"
set "RELB8ROM=%TOOL_ROOT%\relb8rom\%OS%\%HW%\relb8rom.exe"

REM --- Compilation and link flags
set "CFLAGS=-fmax-errors=5 -I. -I%B8LIB_ROOT%\include -I%B8HELPER_ROOT%\include -O2 -fno-builtin -ffunction-sections -fdata-sections -fno-common -Wall -Wextra -Wno-missing-field-initializers -MD -MP -Wshadow -Wundef -fno-strength-reduce -fno-strict-aliasing -fomit-frame-pointer -nostartfiles -mno-thumb-interwork -marm -mfloat-abi=soft -march=armv4 -mcpu=strongarm -fno-lto"
set "CXXFLAGS=-std=c++2a -fno-threadsafe-statics -fno-exceptions -Wno-deprecated-copy"
set "LDFLAGS=-T%B8LIB_ROOT%\src\crt\beep8.ld -static -Wl,--gc-sections,--no-undefined -Wl,-Map=%OBJDIR%\%PROJECT%.map -marm -fno-threadsafe-statics -fno-lto"

REM --- Output names
set "ELF=%OBJDIR%\%PROJECT%.out"
set "BIN=%OBJDIR%\%PROJECT%.bin"
set "ROM=%OBJDIR%\%PROJECT%.b8"
set "LST=%OBJDIR%\%PROJECT%.lst"

REM --- Default ports
if not defined HTTP_PORT set "HTTP_PORT=443"
if not defined API_PORT  set "API_PORT=8082"

REM --- Argument dispatch (default to run/upload)
if "%~1"==""        goto run
if /I "%~1"=="clean" goto clean
if /I "%~1"=="all"   goto all
if /I "%~1"=="run"   goto run

echo Usage: %~n0 [-v] [run ^| all ^| clean]
exit /b 1

:clean
echo === clean ===
if exist "%OBJDIR%"    rd /s /q "%OBJDIR%"
if exist "%EXPORTDIR%" rd /s /q "%EXPORTDIR%"
echo done.
exit /b 0

:gen_pngs
echo === convert PNG -> C++ source ===
if not exist "%EXPORTDIR%" mkdir "%EXPORTDIR%"
for %%F in ("%IMPORTDIR%\*.png") do (
    if exist "%%F" (
        set "PNG=%%F"
        set "OUT=%EXPORTDIR%\%%~nF.png.cpp"
        "%PNG2C%" "!PNG!" > "!OUT!"
    )
)
exit /b 0

:compile
echo === compile sources ===
if not exist "%OBJDIR%" mkdir "%OBJDIR%"

REM Assemble bootloader
if not exist "%OBJDIR%\bootloader.o" (
    echo %YELLOW%Assembling bootloader... %RESET%
    echo "%AS_CMD%" -c %CFLAGS% "%B8LIB_ROOT%\src\crt\bootloader.S" -o "%OBJDIR%\bootloader.o"
    "%AS_CMD%" -c %CFLAGS% "%B8LIB_ROOT%\src\crt\bootloader.S" -o "%OBJDIR%\bootloader.o"
)

REM Compile crt0
if not exist "%OBJDIR%\crt0.o" (
    echo %YELLOW%Compiling crt0.c... %RESET%
    echo "%CC_CMD%" -c %CFLAGS% "%B8LIB_ROOT%\src\crt\crt0.c" -o "%OBJDIR%\crt0.o"
    "%CC_CMD%" -c %CFLAGS% "%B8LIB_ROOT%\src\crt\crt0.c" -o "%OBJDIR%\crt0.o"
)

REM Compile C sources
for %%S in (*.c) do (
    if exist "%%S" (
        echo ---
        echo Compiling C   %%S... 
        echo "%CC_CMD%" -c %CFLAGS% "%%S" -o "%OBJDIR%\%%~nS.o"
        "%CC_CMD%" -c %CFLAGS% "%%S" -o "%OBJDIR%\%%~nS.o"
    )
)

echo %RESET%
REM Compile C++ and PNG-generated sources
for %%S in (*.cpp) do (
    if exist "%%S" (
        echo ---
        echo Compiling C++ %%S... 
        echo "%CXX_CMD%" -c %CFLAGS% %CXXFLAGS% "%%S" -o "%OBJDIR%\%%~nS.o"
        "%CXX_CMD%" -c %CFLAGS% %CXXFLAGS% "%%S" -o "%OBJDIR%\%%~nS.o"
    )
)
for %%P in ("%EXPORTDIR%\*.png.cpp") do (
    if exist "%%P" (
        echo ---
        echo Compiling C++ %%~nP... 
        echo "%CXX_CMD%" -c %CFLAGS% %CXXFLAGS% "%%P" -o "%OBJDIR%\%%~nP.o"
        "%CXX_CMD%" -c %CFLAGS% %CXXFLAGS% "%%P" -o "%OBJDIR%\%%~nP.o"
    )
)
exit /b 0

:link_and_rom
echo === linking -> %ELF% ===
set "OBJS="
for %%O in ("%OBJDIR%\*.o") do set "OBJS=!OBJS! %%O"

echo ---
echo "%LD_CMD%" %LDFLAGS% !OBJS! -lnosys -lm -L"%B8LIB_ROOT%\lib" -lb8 -L"%B8HELPER_ROOT%\lib" -lb8helper -o "%ELF%"
"%LD_CMD%" %LDFLAGS% !OBJS! -lnosys -lm -L"%B8LIB_ROOT%\lib" -lb8 -L"%B8HELPER_ROOT%\lib" -lb8helper -o "%ELF%"

echo ---
echo === objcopy -> %BIN% ===
echo "%OBJCOPY_CMD%" -R .noinit -R .comment --strip-debug -O binary "%ELF%" "%BIN%"
"%OBJCOPY_CMD%" -R .noinit -R .comment --strip-debug -O binary "%ELF%" "%BIN%"

echo === generating B8 ROM -> %ROM% ===
if not exist romfs mkdir romfs
pushd romfs >nul
echo "%GENB8ROM%" -i "*" -o "..\%OBJDIR%\romfs.bin"
"%GENB8ROM%" -i "*" -o "..\%OBJDIR%\romfs.bin"
popd >nul
echo "%RELB8ROM%" -i "%BIN%" -r "%OBJDIR%\romfs.bin" -o "%ROM%"
"%RELB8ROM%" -i "%BIN%" -r "%OBJDIR%\romfs.bin" -o "%ROM%"

echo Build complete: %ROM%
exit /b 0

:all
call :clean
call :gen_pngs
call :compile
call :link_and_rom
exit /b 0

:run
call :all
echo === uploading to device ===
call "%TOOL_ROOT%\scripts\upload.bat" "%ROM%" "%HTTP_PORT%" "%API_PORT%"
exit /b 0
