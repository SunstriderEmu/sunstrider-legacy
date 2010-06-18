#! /bin/bash

USER="wowmania_save"
PASSWORD="muLStveDPCwfrLjp"

TC_DIR="/opt/trinitycore"
BACKUPSDIR="${TC_DIR}/backup/wowmania/db"
MAINTENANCEFILE="${TC_DIR}/config/maintenance"

DUMPER="mysqldump"
CHECKER="mysqlcheck"

TIME=$(date +"%Y-%m-%d-%H-%M")
SAVENAME=$(date +"%N")

if [[ ( -e $MAINTENANCEFILE ) && ( -r $MAINTENANCEFILE ) && $(cat $MAINTENANCEFILE) == 0 ]] ; then
    echo -n "[${TIME}] Sauvegarde de la base de données $2..."

    if [[ ! -d $BACKUPSDIR/$2 ]] ; then
        mkdir -p $BACKUPSDIR/$2;
    fi

    if $DUMPER --quick --skip-extended-insert --single-transaction --user=$USER --password=$PASSWORD $1 > ${BACKUPSDIR}/$1/$SAVENAME.sql ; then
        echo "done."
    else
        echo "failed."
    fi

    echo -n "[${TIME}] Opérations d'optimisation et de réparation de la base de données $1..."

    if $CHECKER --analyze --optimize --auto-repair --user=$USER --password=$PASSWORD $1 > /dev/null ; then
        echo "done."
    else
        echo "failed."
    fi
fi
