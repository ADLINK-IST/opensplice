#!/bin/bash

cd `dirname $0` || exit 1

IBSDIR=`pwd`

. buildsys/functions
. ./dcps_functions

DATE=`date +%Y/%m/%d`
export DATE

ProcessArgs $*
ARGS=$*
Assert LoadConfigs
Assert PreRunSetup
Assert SetupLogDir
Assert SetupResFile

# List the fields we want the display page to show.
echo "BUILD_DISPLAY=/SETUP_TYPE,/BUILD_DIST,/KEEP_DIST,RUN/EXAMPLES_SP" > $LOGDIR/BUILDINFO

. ./stage2.sh

Assert PostRunSetup
