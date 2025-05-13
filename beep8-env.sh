#!/usr/bin/env sh

# Only run on macOS (Darwin)
if [ "$(uname)" = "Darwin" ]; then
  # Get the directory where this script is located
  SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
  # Prepend the tool directory to the PATH
  export PATH="${SCRIPT_DIR}/tool:${PATH}"
fi

# From here on, 'make' will be available
make --version
