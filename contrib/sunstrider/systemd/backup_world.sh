#!/bin/bash

#Usage: create_backup <world db> <backup dir> <backup db>

BACKUPS_LIMIT=30
WORLD_DB=$1
BACKUP_DIR=$2
BACKUP_WORLD_DB=$3

LOG_FILE="$BACKUP_DIR/backups.log"

DATE=`date "+%F"`
FILENAME="world_$DATE.sql"

log()
{
    if [ -n "$1" ]
    then
        IN="$1"
    else
        read IN # This reads a string from stdin and stores it in a variable called IN
    fi


    MSG=$DATE' '$IN
    echo $MSG
    echo $MSG >> $LOG_FILE
}

create_backup()
{
    log "Creating new backup $BACKUP_DIR/$FILENAME"

    pushd $BACKUP_DIR &> /dev/null
        mysqldump $WORLD_DB > $FILENAME
        mysql $BACKUP_WORLD_DB < $FILENAME
        gzip $FILENAME
    popd > /dev/null
}

clean_backups()
{
    log "Cleaning old backups in $BACKUP_DIR"

    i=0
    for BACKUP in $(ls -r $BACKUP_DIR/world_*.gz) ; do
        i=$(($i+1))

        if [ $i -gt $BACKUPS_LIMIT ] ; then
            log "Deleting backup $BACKUP"
            rm $BACKUP
        fi
    done
}

create_backup
clean_backups
