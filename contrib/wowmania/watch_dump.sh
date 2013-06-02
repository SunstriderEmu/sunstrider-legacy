#! /bin/bash

trinitycore_dir="/opt/trinitycore"
dump_dir="${trinitycore_dir}/dump"
trinitycore_bin="${trinitycore_dir}/bin/trinity-core"
gdb="/usr/bin/gdb"

j=1

for i in $(ls -r $dump_dir) ; do
    if [ $j == "$1" ] ; then
        $gdb -q $trinitycore_bin -c "$dump_dir/$i"
        exit
    fi

    j=$(($j+1))
done
