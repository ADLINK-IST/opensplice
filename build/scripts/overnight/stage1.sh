#!/bin/sh

cd `dirname $0` || exit 1

IBSDIR=`pwd`

######## MIN NEEDED FOR OSPLI HACK HACK REMOVE
T_UNAME=`uname`
if [ "$T_UNAME" = "Linux" ]
then
    . /opt/ospldev/gcc/v3.2.3/release.com
    export JAVA_HOME=/home/dds/INSTALLED_FOR_DDS/JAVA
    export TAO_ROOT=/home/dds/INSTALLED_FOR_DDS/TAO
    export EORBHOME=/home/dds/INSTALLED_FOR_DDS/eorb
fi
#############################

. buildsys/functions
. ./dcps_functions

DATE=`date +%Y/%m/%d`
export DATE

ProcessArgs $*
ARGS=$*
Assert LoadConfigs
Assert SetupLogDir
Assert SetupResFile

# List the fields we want the display page to show.
echo "BUILD_DISPLAY=/SETUP_TYPE,/BUILD_DIST,/KEEP_DIST,RUN/EXAMPLES" > $LOGDIR/BUILDINFO

. ./stage2.sh
