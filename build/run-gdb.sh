#!/bin/sh
#< run-gmb.sh
BN=`basename $0`

TMPEXE="tidy-gui2"

if [ ! -x "$TMPEXE" ]; then
    echo "$BN: Can NOT locate EXE! *** FIX ME ***"
    exit 1
fi

echo "$BN: At gdb promot use 'run' to run $TMPEXE"
echo "$BN: If segfault 'bt' at gdb prompt to see stack..."
echo "$BN: Doing: 'gdb --args $TMPEXE'"
gdb --args $TMPEXE

# eof

