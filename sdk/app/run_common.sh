#!/usr/bin/env bash
set -euo pipefail

################################################################################
# run_common.sh – BEEP-8 SDK Shell Build Script
#
# Usage:
#   ./run_common.sh clean    # remove generated files
#
# (no arguments or "all"/"run" – clean, build, generate ROM, and upload)
################################################################################

# Project name: current directory name
PROJECT=$(basename "$PWD")

# Directory layout
OBJDIR=./obj
EXPORTDIR=./data/export
IMPORTDIR=./data/import

# Locate tool directories
TOOL_TOP=$(cd ../../../tool && pwd)
B8LIB_TOP=$(cd ../../b8lib && pwd)
B8HELPER_TOP=$(cd ../../b8helper && pwd)

# Detect OS
UNAME_S=$(uname -s)
case "$UNAME_S" in
  Linux)  OS=linux ;;
  Darwin) OS=osx   ;;
  *)       OS=windows ;;
esac

# Detect hardware architecture
HW=$(uname -m)

# Select gcc version per environment
case "$OS" in
  windows) GCCVER=gcc10.3 ;; 
  linux)   GCCVER=gcc10.2 ;; 
  osx)     GCCVER=gcc11.3 ;; 
  *)       GCCVER=gcc11.3 ;; 
esac

# Cross-compiler commands
GNUARM_TOP=$(cd ../../../gnuarm && pwd)
CROSS_PREFIX="$GNUARM_TOP/$OS/$GCCVER/$HW/bin/arm-none-eabi-"
GCC_CMD="${CROSS_PREFIX}gcc"
GXX_CMD="${CROSS_PREFIX}g++"
AS_CMD="${CROSS_PREFIX}gcc"
LD_CMD="${CROSS_PREFIX}g++"
OBJCOPY_CMD="${CROSS_PREFIX}objcopy"
ODUMP_CMD="${CROSS_PREFIX}objdump"

# Use ccache if available
if command -v ccache >/dev/null 2>&1; then
  CCACHE_CMD=ccache
else
  CCACHE_CMD=
fi

# BEEP-8 helper tools
PNG2C="$TOOL_TOP/png2c/$OS/$HW/png2c"
GENB8ROM="$TOOL_TOP/genb8rom/$OS/$HW/genb8rom"
RELB8ROM="$TOOL_TOP/relb8rom/$OS/$HW/relb8rom"

# Compilation flags
CFLAGS=(
  -fmax-errors=5
  -I.
  -I"$B8LIB_TOP/include"
  -I"$B8HELPER_TOP/include"
  -O2
  -fno-builtin
  -ffunction-sections
  -fdata-sections
  -fno-common
  -Wall
  -Wextra
  -Wno-missing-field-initializers
  -MD
  -MP
  -Wshadow
  -Wundef
  -fno-strength-reduce
  -fno-strict-aliasing
  -fomit-frame-pointer
  -nostartfiles
  -mno-thumb-interwork
  -marm
  -mfloat-abi=soft
  -march=armv4
  -mcpu=strongarm
  -fno-lto
)
CXXFLAGS=(
  -std=c++2a
  -fno-threadsafe-statics
  -fno-exceptions
  -Wno-deprecated-copy
)
LDFLAGS=(
  -T"$B8LIB_TOP/src/crt/beep8.ld"
  -static
  -Wl,--gc-sections,--no-undefined
  -Wl,-Map="$OBJDIR/$PROJECT.map"
  -marm
  -fno-threadsafe-statics
  -fno-lto
)

# Output filenames
ELF="$OBJDIR/$PROJECT.out"
BIN="$OBJDIR/$PROJECT.bin"
ROM="$OBJDIR/$PROJECT.b8"
LST="$OBJDIR/$PROJECT.lst"

# Default ports
HTTP_PORT=${HTTP_PORT:-443}
API_PORT=${API_PORT:-8082}

# Print usage instructions
usage() {
  cat <<EOF
Usage: $0 [all|clean|run]
  clean        : remove generated files
  all/run      : clean, build, generate B8 ROM, and upload
EOF
  exit 1
}

# Clean generated files with verbose output
clean() {
  echo "=== clean ==="
  if [ -d "$OBJDIR" ] || [ -d "$EXPORTDIR" ]; then
    echo "Removing generated files and directories:"
    rm -rfv "$OBJDIR" "$EXPORTDIR"
  else
    echo "No generated files to remove."
  fi
}

# Convert PNG assets into C++ sources
gen_pngs() {
  echo "=== convert PNG → C++ source ==="
  mkdir -p "$EXPORTDIR"
  for png in "$IMPORTDIR"/*.png; do
    [ -e "$png" ] || continue
    out="$EXPORTDIR/$(basename "${png%.*}").png.cpp"
    if [ ! -f "$out" ] || [ "$png" -nt "$out" ]; then
      echo "png2c $png → $out"
      "$PNG2C" "$png" > "$out"
    fi
  done
}

# Compile sources incrementally
compile() {
  echo "=== compile sources ==="
  mkdir -p "$OBJDIR"

  # Assemble bootloader
  local boot_src boot_obj
  boot_src="$B8LIB_TOP/src/crt/bootloader.S"
  boot_obj="$OBJDIR/bootloader.o"
  if [ ! -f "$boot_obj" ] || [ "$boot_src" -nt "$boot_obj" ]; then
    echo "Assembling $boot_src → $boot_obj"
    $AS_CMD -c "${CFLAGS[@]}" "$boot_src" -o "$boot_obj"
  fi

  # Compile crt0
  local crt_src crt_obj
  crt_src="$B8LIB_TOP/src/crt/crt0.c"
  crt_obj="$OBJDIR/crt0.o"
  if [ ! -f "$crt_obj" ] || [ "$crt_src" -nt "$crt_obj" ]; then
    echo "Compiling $crt_src → $crt_obj"
    if [ -n "$CCACHE_CMD" ]; then
      $CCACHE_CMD $GCC_CMD -c "${CFLAGS[@]}" "$crt_src" -o "$crt_obj"
    else
      $GCC_CMD -c "${CFLAGS[@]}" "$crt_src" -o "$crt_obj"
    fi
  fi

  # Compile application C files
  for src in *.c; do
    [ -e "$src" ] || continue
    obj="$OBJDIR/${src%.c}.o"
    if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ]; then
      echo "Compiling C   $src → $obj"
      if [ -n "$CCACHE_CMD" ]; then
        $CCACHE_CMD $GCC_CMD -c "${CFLAGS[@]}" "$src" -o "$obj"
      else
        $GCC_CMD -c "${CFLAGS[@]}" "$src" -o "$obj"
      fi
    fi
  done

  # Compile C++ and PNG-generated sources
  for src in *.cpp "$EXPORTDIR"/*.png.cpp; do
    [ -e "$src" ] || continue
    name=$(basename "$src")
    obj="$OBJDIR/${name%.cpp}.o"
    if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ]; then
      echo "Compiling C++ $src → $obj"
      if [ -n "$CCACHE_CMD" ]; then
        $CCACHE_CMD $GXX_CMD -c "${CFLAGS[@]}" "${CXXFLAGS[@]}" "$src" -o "$obj"
      else
        $GXX_CMD -c "${CFLAGS[@]}" "${CXXFLAGS[@]}" "$src" -o "$obj"
      fi
    fi
  done
}

# Link and generate ROM
link_and_rom() {
  echo "=== linking → $ELF ==="
  objs=( "$OBJDIR"/*.o )
  $LD_CMD "${LDFLAGS[@]}" "${objs[@]}" \
    -lnosys -lm \
    -L"$B8LIB_TOP/lib" -lb8 \
    -L"$B8HELPER_TOP/lib" -lb8helper \
    -o "$ELF"

  echo "=== objcopy → $BIN ==="
  $OBJCOPY_CMD -R .noinit -R .comment --strip-debug -O binary "$ELF" "$BIN"

  echo "=== generating B8 ROM → $ROM ==="
  mkdir -p romfs
  (cd romfs && "$GENB8ROM" -i "*" -o "../$OBJDIR/romfs.bin")
  "$RELB8ROM" -i "$BIN" -r "$OBJDIR/romfs.bin" -o "$ROM"

  if [ "${EXPORT_LIST:-0}" -eq 1 ]; then
    echo "=== generating listing → $LST ==="
    $ODUMP_CMD -Mreg-names-std -d -C "$ELF" > "$LST"
    echo "$LST"
  fi

  echo "Build complete: $ROM"
}

# Full build sequence
build() {
  clean
  gen_pngs
  compile
  link_and_rom
}

# Build and upload to actual device
run() {
  build
  echo "=== uploading to device ==="
  if [ "$OS" = "windows" ]; then
    "$TOOL_TOP/scripts/upload.bat" "$ROM" "$HTTP_PORT" "$API_PORT"
  else
    "$TOOL_TOP/scripts/upload.sh" "$ROM" "$HTTP_PORT" "$API_PORT"
  fi
}

# Dispatch based on argument
case "${1:-all}" in
  clean) clean ;;
  all|run) run   ;;
  *)     usage ;;
esac
