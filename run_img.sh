#!/bin/sh
BASEDIR=$(pwd)

echo "RUN HAMCAST IMG"
cd "$BASEDIR/bin"
if test $(uname) = "Darwin" ; then
    # For MacOS X:
    export DYLD_LIBRARY_PATH="$BASEDIR/lib:$LD_LIBRARY_PATH"
else
    # For Linux:
    export LD_LIBRARY_PATH="$BASEDIR/lib:$LD_LIBRARY_PATH"
fi
if test -e "$BASEDIR/bin/img" ; then
    if test -e "$BASEDIR/bin/img.cfg" ; then
        ./img -f img.cfg -d 1 -u 2
    else
        echo "ERROR, img.cfg file not found"
        exit 1
    fi
else
    echo "ERROR, img binary not found"
    exit 1
fi
