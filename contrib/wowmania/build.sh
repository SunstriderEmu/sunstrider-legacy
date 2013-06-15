#! /bin/bash

TC_DIR="/home/trinitycore"

[[ -n "$1" ]] && MAKEOPTS="-j$1" || MAKEOPTS="-j1"

cd "${TC_DIR}/build"

echo 1 > ${TC_DIR}/etc/maintenance.conf

FLAGS="-O2 -march=native -pipe -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -fno-delete-null-pointer-checks -fno-strict-aliasing -frename-registers -fno-omit-frame-pointer -ggdb -minline-all-stringops"

cmake \
    -DPREFIX="${TC_DIR}/" \
    -DCONF_DIR="${TC_DIR}/etc" \
    -DCMAKE_C_COMPILER="/usr/bin/gcc-4.3.6" \
    -DCMAKE_CXX_COMPILER="/usr/bin/g++-4.3.6" \
    -DCMAKE_C_FLAGS="${FLAGS}" \
    -DCMAKE_CXX_FLAGS="${FLAGS}" \
    -DACE_LIBRARY="/usr/lib/libACE.so" \
    -DACE_INCLUDE_DIR="/usr/include" \
    -DDO_CLI=0 \
    -DDO_RA=0 \
    -DDO_DEBUG=1 \
    -DSHORT_SLEEP=1 \
    -DLARGE_CELL=0 \
    -DWITH_UNIT_CRASHFIX=1 \
    -DPROFILING=1 \
    ${TC_DIR}/sources

make ${MAKEOPTS}
make install

echo 0 > ${TC_DIR}/etc/maintenance.conf
