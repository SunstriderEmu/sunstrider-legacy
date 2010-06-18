#! /bin/bash

TC_DIR="/opt/trinitycore"

cd "${TC_DIR}/build"

[[ -n "$1" ]] && MAKEOPTS="-j$1" || MAKEOPTS="-j1"

make ${MAKEOPTS}
make install
