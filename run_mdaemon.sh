#!/bin/sh
BASEDIR=$(pwd)

echo "RUN monitoring daemon"
cd "$BASEDIR/bin"
if test $(uname) = "Darwin" ; then
    # For MacOS X:
    export DYLD_LIBRARY_PATH="$BASEDIR/lib:$LD_LIBRARY_PATH"
else
    # For Linux:
    export LD_LIBRARY_PATH="$BASEDIR/lib:$LD_LIBRARY_PATH"
fi

if test -e "$BASEDIR/bin/mdaemon" ; then
    if [ -z "$1" ] ; then
        ./mdaemon
    else
        ./mdaemon -d $1
    fi
else
    echo "ERROR, mdaemon binary not found!"
    exit 1
fi
