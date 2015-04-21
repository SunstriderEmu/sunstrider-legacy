#!/bin/bash

# change the current directory to the location of this script
cd "$(dirname ${BASH_SOURCE[0]})"

for x in $(ls updates/auth*.sql); do
    mysql -u root -D auth < $x
done

for x in $(ls updates/char*.sql); do
    mysql -u root -D characters < $x
done

for x in $(ls updates/world*.sql); do
    mysql -u root -D world < $x
done
