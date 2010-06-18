#! /bin/bash

TC_DIR="/opt/trinitycore"

[[ -n "$1" ]] && MAKEOPTS="-j$1" || MAKEOPTS="-j1"

cd "${TC_DIR}/build"

rm -rf * 

echo 1 > ${TC_DIR}/config/maintenance

#FLAGS="-O3 -march=nocona -ggdb3 -pipe -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -fno-strength-reduce -fno-delete-null-pointer-checks -fno-strict-aliasing -msse2 -ftree-vectorize -fomit-frame-pointer -funroll-loops -m3dnow"
FLAGS="-O2 -march=nocona -pipe -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -fno-strength-reduce -fno-delete-null-pointer-checks -fno-strict-aliasing -msse2 -m3dnow"
export CFLAGS="${FLAGS}"
export CXXFLAGS="${FLAGS}"

cmake \
    -DPREFIX="${TC_DIR}/" \
    -DCONF_DIR="${TC_DIR}/config" \
    -DCMAKE_C_COMPILER="/usr/bin/gcc-4.2" \
    -DCMAKE_CXX_COMPILER="/usr/bin/g++-4.2" \
    -DCMAKE_C_FLAGS="${CFLAGS}" \
    -DCMAKE_CXX_FLAGS="${CXXFLAGS}" \
    -DACE_LIBRARY="/usr/local/lib/libACE.so" \
    -DACE_INCLUDE_DIR="/usr/local/include" \
    -DDO_CLI=0 \
    -DDO_RA=0 \
    -DDO_DEBUG=0 \
    -DSHORT_SLEEP=0 \
    -DLARGE_CELL=0 \
    ${TC_DIR}/sources

make ${MAKEOPTS}
make install

echo 0 > ${TC_DIR}/config/maintenance
