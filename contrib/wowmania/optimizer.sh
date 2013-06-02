#! /bin/bash

TC_DIR="/opt/trinitycore"
MAINTENANCEFILE="${TC_DIR}/config/maintenance"

CHECKER="mysqlcheck"

TIME=$(date +"%Y-%m-%d-%H-%M")

if [[ ( -e $MAINTENANCEFILE ) && ( -r $MAINTENANCEFILE ) && $(cat $MAINTENANCEFILE) == 0 ]] ; then
    echo -n "[${TIME}] Opérations d'optimisation et de réparation de la base de données $1..."

    $CHECKER --analyze --optimize --auto-repair $1 > /dev/null 2>&1 && echo "done." || echo "failed."
fi
