#!/usr/bin/env bash
set -euo pipefail

################################################################################
# pico8_example/run.sh – Wrapper for common BEEP-8 build script
#
# This script delegates all commands to the centralized sdk/app/run.sh.
################################################################################

# Determine project name (directory name)
export PROJECT=$(basename "$PWD")

# Locate the common build script (one level up)
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
COMMON_SCRIPT="$SCRIPT_DIR/../run_common.sh"

# Verify common script exists
if [ ! -f "$COMMON_SCRIPT" ]; then
  echo "Error: common run script not found at $COMMON_SCRIPT" >&2
  exit 1
fi

# Execute the common script, forwarding all arguments
exec "$COMMON_SCRIPT" "$@"
