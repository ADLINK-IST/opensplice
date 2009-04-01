#!/bin/sh

cd `dirname $0` || exit 1

IBSDIR=`pwd`

. buildsys/functions
. ./dcps_functions

ProcessArgs $*
ARGS=$*
Assert LoadConfigs
Assert SetupLogDir
Assert SetupResFile
Assert setupBuildInfo

ArchiveLogs
. ./stage2.sh
