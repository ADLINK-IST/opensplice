#!/bin/sh


cd `dirname $0` || exit 1

. buildsys/functions

DATE=`date +%Y/%m/%d`
export DATE

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
   echo "RUN_TESTS=$RUN_TESTS" >> $LOGDIR/BUILDINFO
   echo "RUN_EXAMPLES=$RUN_EXAMPLES" >> $LOGDIR/BUILDINFO
   echo "REV=$REV" >> $LOGDIR/BUILDINFO
   echo "VER=$VER" >> $LOGDIR/BUILDINFO
   echo "DEP=$DEP" >> $LOGDIR/BUILDINFO

   # List the fields we want the display page to show.
   echo "BUILD_DISPLAY=/SETUP_TYPE,/KEEP_DIST,RUN/TESTS,RUN/EXAMPLES" >> $LOGDIR/BUILDINFO
}

ProcessArgs $*
ARGS=$*
Assert LoadConfigs
Assert SetupLogDir
Assert SetupResFile
Assert setupBuildInfo

ArchiveLogs

WaitForParentBuild

cat > $LOGDIR/LOGFILES <<EOF
BUILD=build.txt
TESTS=dbt.txt
EOF

cat > $RESFILE <<EOF
BUILD=TODO
EOF

if [ $BUILD_DIST = "yes" ]
then
    echo "BUILD_DIST=TODO" >> $RESFILE
else
    echo "BUILD_DIST=SKIP" >> $RESFILE
fi
if [ $KEEP_DIST = "yes" ]
then
    echo "KEEP_DIST=TODO" >> $RESFILE
else
    echo "KEEP_DIST=SKIP" >> $RESFILE
fi
if [ $RUN_TESTS = "yes" ]
then
    echo "TESTS/DBT=TODO" >> $RESFILE
else
    echo "TESTS/DBT=SKIP" >> $RESFILE
fi
ArchiveLogs

./dcps_build $ARGS

BUILD_WORKED=$?

ArchiveLogs

if [ $BUILD_WORKED = 0 ]
then
    ./dcps_build_dist $ARGS
    BUILD_DIST_WORKED=$?
    ArchiveLogs
fi

if [ "$BUILD_DIST_WORKED" = 0 ]
then
    ./dcps_archive_dist $ARGS
    ArchiveLogs
fi

if [ $BUILD_WORKED = 0 ]
then
    ./dcps_perform_tests $ARGS
    BUILD_WORKED=$?
    ArchiveLogs
fi

if [ $BUILD_WORKED = 0 ]
then
   SetState "Complete"
else
   SetState "Failed"
fi
ArchiveLogs
