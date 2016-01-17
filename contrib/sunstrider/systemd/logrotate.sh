#!/bin/sh -
/usr/sbin/logrotate --force -s $1 $2 || true
