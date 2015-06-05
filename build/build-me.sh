#!/bin/sh
#< build-me.sh - qt-gui - 20150531 - 20150524
# 20150531 - add some command options
BN=`basename $0`
TMPPRJ="qt-gui"
TMPLOG="bldlog-1.txt"
TMPOPTS=""
TMPOPTS="-DCMAKE_INSTALL_PREFIX=$HOME"
# Possible options to help with problems
# TMPOPTS="$TMPOPTS -DCMAKE_VERBOSE_MAKEFILE=ON"

VERBOSE=0
DBGSYMS=0
DOPAUSE=0

# option like EXTRA=-DCMAKE_BUILD_TYPE=DEBUG
add_extra_cmopt()
{   
    if [ "$#" -gt "0" ]; then
        LEN1=`expr length $1`
        if [ "$LEN1" -gt "7" ]; then
            XOPT=`echo $1 | cut -b7-$LEN1`
            TMPOPTS="$TMPOPTS $XOPT"
            echo "$BN: Add EXTRA option [$XOPT]"
        else
            echo "$BN: ERROR: Length $LEN! less than/equals 6"
            exit 1
        fi
    else
        echo "$BN: ERROR: No option passed!"
        exit 1
    fi
}

give_help()
{
    echo "$BN [OPTIONS]"
    echo ""
    echo "OPTIONS"
    echo " VERBOSE = Use verbose build (def=$VERBOSE)"
    echo " DEBUG   = Enable DEBUG symbols (-g)."
    echo " EXTRA=CMOPT = Add a extra CMake option."
#    echo " NOPAUSE = Skip the pausing before each step."
    #echo " PROFILING = Enable PROFILING (-pg)."
    echo ""
    exit 1
}


for arg in $@; do
      case $arg in
         VERBOSE) VERBOSE=1 ;;
         DEBUG) DBGSYMS=1 ;;
#         NOPAUSE) DOPAUSE=0 ;;
         EXTRA=*) add_extra_cmopt $arg ;;
         --help) give_help ;;
         -h) give_help ;;
         -\?) give_help ;;
         *)
            echo "$BN: ERROR: Invalid argument [$arg]"
            exit 1
            ;;
      esac
done

if [ "$VERBOSE" = "1" ]; then
    TMPOPTS="$TMPOPTS -DCMAKE_VERBOSE_MAKEFILE=TRUE"
    echo "$BN: Enabling VERBOSE make"
fi

if [ "$DBGSYMS" = "1" ]; then
    TMPOPTS="$TMPOPTS -DCMAKE_BUILD_TYPE=Debug -DENABLE_DEBUG_SYMBOLS:BOOL=TRUE"
    echo "$BN: Enabling DEBUG symbols"
fi

if [ -f "$TMPLOG" ]; then
    rm -f $TMPLOG
fi

echo "Begin $TMPPRJ project build" > $TMPLOG

echo "Doing 'cmake .. $TMPOPTS'"
echo "Doing 'cmake .. $TMPOPTS'" >> $TMPLOG
cmake .. $TMPOPTS >> $TMPLOG 2>&1
if [ ! "$?" =  "0" ]; then
	echo "cmake config, gen error $?"
	echo "See $TMPLOG for details..."
	exit 1
fi

echo "Doing 'make'"
echo "Doing 'make'" >>$TMPLOG
make >>$TMPLOG 2>&1
if [ ! "$?" =  "0" ]; then
	echo "make error $?"
	echo "See $TMPLOG for details..."
	exit 1
fi

echo "Appears a successful build..."
echo ""
echo "Perhaps follow with 'make install', to install to $HOME/bin unless changed..."
echo ""

# eof

