#!/bin/sh
BASEDIR=$(pwd)

echo "RUN HAMCAST MIDDLEWARE"
cd "$BASEDIR/bin"
if test $(uname) = "Darwin" ; then
    # For MacOS X:
    export DYLD_LIBRARY_PATH="$BASEDIR/lib:$DYLD_LIBRARY_PATH"
else
    # For Linux:
    export LD_LIBRARY_PATH="$BASEDIR/lib:$LD_LIBRARY_PATH"
fi

if [ -e "$BASEDIR/bin/middleware.ini" ] ; then 
    echo " found configuration"
else
    echo "ERROR, no configuration, missing middleware.ini!"
    exit 1
fi

if test -e "$BASEDIR/bin/middleware" ; then
    ./middleware
else
    echo "ERROR, middleware binary not found!"
    exit 1
fi
