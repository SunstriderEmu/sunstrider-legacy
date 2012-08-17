#! /bin/bash

TRINITYDIR="/opt/trinitycore"

REALMBIN="trinity-realm"
WORLDBIN="trinity-core"

REALMSTARTER="${TRINITYDIR}/bin/${REALMBIN}"
WORLDSTARTER="${TRINITYDIR}/bin/${WORLDBIN}"

REALMPID=$(cat ${TRINITYDIR}/run/realm.pid)
WORLDPID=$(cat ${TRINITYDIR}/run/world.pid)

LOGSDIR="${TRINITYDIR}/log"
LOGSBACKUPSDIR="${TRINITYDIR}/backup/log"
DUMPDIR="${TRINITYDIR}/dump"

RESTARTERLOG="${LOGSDIR}/restarter.log"
MAINTENANCEFILE="${TRINITYDIR}/config/maintenance.conf"
FIXGUIDTRIGGER="${TRINITYDIR}/config/do_fixguid"

DATE=$(date +"[%d/%m/%Y %R]")
MAXDUMPTOKEEP=3

STDERRFILE="stderr.$(date +%Y_%m_%d__%H_%M_%S).log"

export LIBC_FATAL_STDERR_=1

start_realm()
{
  cd ${DUMPDIR}

  if $($REALMSTARTER > /dev/null 2>&1 &) ; then
    echo "$DATE Le serveur Realm a redémarré."
  else
    echo "$DATE Le serveur Realm n'a pas redémarré."
  fi
}

if [[ ( -e $MAINTENANCEFILE ) && ( -r $MAINTENANCEFILE ) && $(cat $MAINTENANCEFILE) == 0 ]] ; then
  if [[ -z "$(ps -p $REALMPID | grep $REALMPID)" ]] ; then
    echo "$DATE Le serveur Realm est arrêté."

    start_realm
  fi

  if [[ -z "$(ps -p $WORLDPID | grep $WORLDPID)" ]] ; then
    echo "$DATE Redémarrage du serveur Realm"
    kill -9 $REALMPID

    start_realm

    echo "$DATE Le serveur World est arrêté."
    echo -n "$DATE Sauvegarde des logs characters... "
    mv $LOGSDIR/characters_*.log $LOGSBACKUPSDIR/characters/ && echo "done." || echo " failed"

    if [[ -z "$(ls $LOGSDIR/gm_commands*.log)" ]] ; then
      echo -n "$DATE Sauvegarde des logs gms... "
      mv $LOGSDIR/gm_commands*.log $LOGSBACKUPSDIR/gm_commands/ && echo "done." || echo " failed"
    fi

    echo -n "$DATE Sauvegarde des logs server..."
    mv $LOGSDIR/server_*.log $LOGSBACKUPSDIR/server/ && echo " done." || echo " failed"

    if [ -e $FIXGUIDTRIGGER ]; then
	$TRINITYDIR/fixguid > $LOGSDIR/fixguid.log 2>&1
	rm -f $FIXGUIDTRIGGER
    fi

    cd "${TRINITYDIR}/dump"

    if $($WORLDSTARTER > /dev/null 2>$LOGSDIR/$STDERRFILE &) ; then
      echo "$DATE Le serveur World a redémarré."
    else
      echo "$DATE Le serveur World n'a pas redémarré."
    fi
  fi
else
  echo "$DATE Le serveur est en maintenance."
fi

i=0

# Nettoyage des dumps
for DUMP in $(ls -r $DUMPDIR) ; do
  i=$(($i+1))

  if [ $i -gt $MAXDUMPTOKEEP ] ; then
    echo "$DATE Suppression du dump $DUMP"
    rm $DUMPDIR/$DUMP
  fi
done
