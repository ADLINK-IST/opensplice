#!/bin/sh


cd `dirname $0` || exit 1

. buildsys/functions

DATE=`date +%Y/%m/%d`

setupBuildInfo()
{
   # Dump the stuff we want the overnight page to display to identify this build
   echo "PRODUCT=$PRODUCT" >> $LOGDIR/BUILDINFO
   echo "RUNNER=$RUNNER" >> $LOGDIR/BUILDINFO
   echo "LOG_HOST=$LOG_HOST" >> $LOGDIR/BUILDINFO
   echo "LOG_USER=$LOG_USER" >> $LOGDIR/BUILDINFO
   echo "RCP=$RCP" >> $LOGDIR/BUILDINFO
   echo "RSH=$RSH" >> $LOGDIR/BUILDINFO
   echo "REPOSITORY_URL=$REPOSITORY_URL" >> $LOGDIR/BUILDINFO
   echo "SETUP_TYPE=$SETUP_TYPE" >> $LOGDIR/BUILDINFO
   echo "BUILD_DIST=$BUILD_DIST" >> $LOGDIR/BUILDINFO
   echo "KEEP_DIST=$KEEP_DIST" >> $LOGDIR/BUILDINFO
   echo "RUN_DBT=$RUN_DBT" >> $LOGDIR/BUILDINFO
   echo "RUN_RBT=$RUN_RBT" >> $LOGDIR/BUILDINFO
   echo "RUN_EXAMPLES=$RUN_EXAMPLES" >> $LOGDIR/BUILDINFO
   echo "REV=$REV" >> $LOGDIR/BUILDINFO
   echo "VER=$VER" >> $LOGDIR/BUILDINFO
   echo "DEP=$DEP" >> $LOGDIR/BUILDINFO
   # List the fields we want the display page to show.
   echo "BUILD_DISPLAY=/SETUP_TYPE,/BUILD_DIST,/KEEP_DIST,BUILD/DBT,BUILD/RBT,RUN/DBT,RUN/RBT,RUN/EXAMPLES" >> $LOGDIR/BUILDINFO
}

ProcessArgs $*
LoadConfigs
Assert SetupLogDir
Assert setupBuildInfo
Assert SetupRemoteLogDir
Assert SetupResFile

echo "BUILD=SKIP" >> $RESFILE
SetState "Complete"
ArchiveLogs
