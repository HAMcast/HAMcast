#!/bin/bash

SET_PERL5LIB=/opt/local/lib/perl5/site_perl/5.12.4/
if [ -n "${SET_PERL5LIB}" ]; then
	SET_PERL5LIB="${SET_PERL5LIB}":"${PERL5LIB}"
fi

CMD=$1
if [ -z $CMD ]; then
    echo "Missing Parameter!"
    echo "USAGE: $0 send|recv"
    exit 1;
fi
echo "Command: $CMD"
PAYLOADS=( 100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400 ) 
echo "Delete old files ..."
    DST=$CMD"_native.dat"
    rm $DST;
    DST=$CMD"_hamcast.dat"
    rm $DST;
    DST=$CMD"_scribe.dat"
    rm $DST;
echo "Generating stats ..."
for i in {0..13} ; do 
    size=${PAYLOADS[$i]}
    SRC=$CMD"_native_"$size".dat"
    DST=$CMD"_native.dat"
    echo -n "$size " >> $DST
    PERL5LIB="${SET_PERL5LIB}" ./file2stat.pl $SRC | sort -n | sed s/\\./,/g >> $DST
    SRC=$CMD"_hamcast_"$size".dat"
    DST=$CMD"_hamcast.dat"
    echo -n "$size " >> $DST
    PERL5LIB="${SET_PERL5LIB}" ./file2stat.pl $SRC | sort -n | sed s/\\./,/g >> $DST
    SRC=$CMD"_scribe_"$size".dat"
    DST=$CMD"_scribe.dat"
    echo -n "$size " >> $DST
    PERL5LIB="${SET_PERL5LIB}" ./file2stat.pl $SRC | sort -n | sed s/\\./,/g >> $DST
done
echo "Done ..."
