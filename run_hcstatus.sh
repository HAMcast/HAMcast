#!/bin/sh
BASEDIR=$(pwd)

echo "RUN HAMCAST STATUS"
cd "$BASEDIR/bin"
if test $(uname) = "Darwin" ; then
    # For MacOS X:
    export DYLD_LIBRARY_PATH="$BASEDIR/lib:$LD_LIBRARY_PATH"
else
    # For Linux:
    export LD_LIBRARY_PATH="$BASEDIR/lib:$LD_LIBRARY_PATH"
fi
if test -e "$BASEDIR/bin/hc_status" ; then
    ./hc_status
else
    echo "ERROR, hc_status binary not found"
    exit 1
fi
