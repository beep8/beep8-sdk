#!/bin/sh
set -e
cd ./hello
make cleanlib
make lib
make cleanhelper
make helper
cd ../
make
