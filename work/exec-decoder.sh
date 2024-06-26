#!/bin/bash
# Script to instantly execute the Main-DAQ decoder.
#
# This script sources `/data2/e1039/this-e1039.sh` via `../setup.sh`.
# You might use '-v' or '-V' options to select a non-standard e1039-core version.
DIR_SCRIPT=$(dirname $(readlink -f $0))

if [ $(hostname -s) != 'e1039prod1' -o $USER != 'kenichi' ] ; then
    echo "!!ERROR!!  This script must be run on e1039prod1 by kenichi.  Abort."
    exit
fi

E1039_CORE_VERSION=default
E1039_CORE_DIR=
IS_ONLINE=false
DECO_MODE=devel
DECO_VERB=2
LAUNCHER=no
N_EVT=0

OPTIND=1
while getopts ":v:V:osxdle:" OPT ; do
    case $OPT in
        v ) E1039_CORE_VERSION=$OPTARG
            echo "  E1039_CORE version: $E1039_CORE_VERSION"
            ;;
        V ) E1039_CORE_DIR=$OPTARG
            echo "  E1039_CORE directory: $E1039_CORE_DIR"
            ;;
        o ) IS_ONLINE=true
            echo "  Online mode: $IS_ONLINE"
            ;;
        s ) DECO_MODE=std
            echo "  Decoder mode: $DECO_MODE"
            ;;
        x ) (( DECO_VERB++ ))
            ;;
        X ) (( DECO_VERB-- ))
            ;;
        d ) DECO_MODE=devel
            echo "  Decoder mode: $DECO_MODE"
            ;;
        l ) LAUNCHER=yes
            echo "  Launcher mode: $LAUNCHER"
            ;;
	e ) N_EVT=$OPTARG
            echo "  N of events: $N_EVT"
            ;;
    esac
done
shift $((OPTIND - 1))
echo "  Decoder verbosity: $DECO_VERB"

source $DIR_SCRIPT/../setup.sh

umask 0002
export E1039_DECODER_MODE=$DECO_MODE
export E1039_DECODER_VERBOSITY=$DECO_VERB

if [ $LAUNCHER = yes ] ; then
    FN_LOG=/data4/e1039_data/online/decoder_maindaq/log_decoder_daemon.txt
    echo "Launch a daemon process."
    echo "  Log file = $FN_LOG"
    root.exe -b -q "$E1039_ONLMON/work/Daemon4MainDaq.C" &>$FN_LOG
else
    if [ -z "$1" ] ; then
	echo "The 1st argument must be a run number.  Abort."
	exit
    fi
    RUN=$1
    echo "Single-run decoding for run = $RUN."
    mkdir -p /data4/e1039_data/online/decoder_maindaq
    FN_LOG=$(printf '/data4/e1039_data/online/decoder_maindaq/log_%06d.txt' $RUN)
    if [ -e $FN_LOG ] ; then
	for (( II = 1 ;  ; II++ )) ; do
	    test ! -e $FN_LOG.$II && mv $FN_LOG $FN_LOG.$II && break
	done
    fi
    echo "  Log file = $FN_LOG"
    root.exe -b -l -q "$E1039_ONLMON/work/Fun4MainDaq.C($RUN, $N_EVT, $IS_ONLINE)" &>$FN_LOG
    RET=$?
    SQMS_SEND=/data2/users/kenichi/local/SQMS/SQMS-sender.py
    if [ $RET -ne 0 -a $IS_ONLINE = true -a -e $SQMS_SEND ] ; then
	MSG="RUN: $RUN"$'\n'
	MSG+="RET: $RET"$'\n'
	MSG+="LOG: $FN_LOG"
	$SQMS_SEND -t 'exec-decoder.sh' -p 'E' -A 'Online Software Alarm,bhy7tf@virginia.edu' -m "$MSG"
    fi
fi
