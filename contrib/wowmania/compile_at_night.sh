#! /bin/bash

TC_DIR="/opt/trinitycore"
COMPILATIONFILE="${TC_DIR}/config/compilation_planned"

if [[ ( -e $COMPILATIONFILE ) && ( -r $COMPILATIONFILE ) && $(cat $COMPILATIONFILE) == 1 ]] ; then
    cd "${TC_DIR}/build"

    rm -rf * 

    echo 1 > ${TC_DIR}/config/maintenance

    source ${TC_DIR}/config/compilation_flags

    export CFLAGS="${CFLAGS}"
    export CXXFLAGS="${CXXFLAGS}"

    cmake \
        -DPREFIX="${TC_DIR}/" \
        -DDATA_DIR="${TC_DIR}/data" \
        -DCONFIG_DIR="${TC_DIR}/config" \
        -DCMAKE_C_FLAGS="${CFLAGS}" \
        -DCMAKE_CXX_FLAGS="${CXXFLAGS}" \
        -DCMAKE_EXE_LINKER_FLAGS="-Wl,--rpath -Wl,LIBDIR" \
        -DACE_LIBRARY="${TC_DIR}/lib/libACE.so" \
        -DACE_INCLUDE_DIR="${TC_DIR}/sources/dep/ACE_wrappers" \
        -DDO_CLI=0 \
        -DDO_DEBUG=1 \
        -DSHORT_SLEEP=1 \
        -DLARGE_CELL=0 \
        ${TC_DIR}/sources > /dev/null 2>&1

    make $MAKEOPTS
    make install

    echo 0 > ${TC_DIR}/config/maintenance
fi

echo 0 > ${TC_DIR}/config/compilation_planned
