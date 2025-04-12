#!/bin/sh
# ----------------------------------------------------------
# Build script for BEEP-8 applications under /sdk/app/
# This script builds each application by entering its directory,
# rebuilding its libraries, helper modules, and finally the main binary.
# Currently, it handles the "hello" app as an example.
# ----------------------------------------------------------

set -e  # Exit immediately if any command fails

cd ./hello

# Clean and build static/shared libraries
make cleanlib
make lib

# Clean and build helper tools or utilities
make cleanhelper
make helper

# Return to the root of the app directory
cd ../

# Build the main application binary
make
