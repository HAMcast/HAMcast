#!/bin/bash
CMD=$1
if [ -z $CMD ]; then
    echo "Missing Parameter!"
    echo "USAGE: $0 send|recv"
    exit 1;
fi
echo "Command: $CMD"
PAYLOADS=( 100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400 ) 
for i in {0..13} ; do 
    size=${PAYLOADS[$i]}
    rm $CMD"_native_"$size".dat"
    rm $CMD"_hamcast_"$size".dat"
    rm $CMD"_scribe_"$size".dat"
    FILES=$CMD"_native_"$size"_*"
    for j in $FILES; do
        cat $j | grep -e "^[0-9]" >> $CMD"_native_"$size".dat";
    done
    FILES=$CMD"_hamcast_"$size"_*"
    for j in $FILES; do
        cat $j | grep -e "^[0-9]" >> $CMD"_hamcast_"$size".dat";
    done
    FILES=$CMD"_scribe_"$size"_*"
    for j in $FILES; do
        cat $j | grep -e "^[0-9]" >> $CMD"_scribe_"$size".dat";
    done
done
