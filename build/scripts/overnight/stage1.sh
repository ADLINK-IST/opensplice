#!/bin/sh

cd `dirname $0` || exit 1

IBSDIR=`pwd`

. buildsys/functions
. ./dcps_functions

DATE=`date +%Y/%m/%d`
export DATE

ProcessArgs $*
ARGS=$*
Assert LoadConfigs
Assert SetupLogDir
Assert SetupResFile
Assert setupBuildInfo

ArchiveLogs
. ./stage2.sh
