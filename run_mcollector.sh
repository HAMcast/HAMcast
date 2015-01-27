#!/bin/sh
BASEDIR=$(pwd)

echo "RUN monitoring collector"
cd "$BASEDIR/bin"
if test $(uname) = "Darwin" ; then
    # For MacOS X:
    export DYLD_LIBRARY_PATH="$BASEDIR/lib:$LD_LIBRARY_PATH"
else
    # For Linux:
    export LD_LIBRARY_PATH="$BASEDIR/lib:$LD_LIBRARY_PATH"
fi

if test -e "$BASEDIR/bin/mcollector" ; then
    ./mcollector
else
    echo "ERROR, mdaemon binary not found!"
    exit 1
fi
