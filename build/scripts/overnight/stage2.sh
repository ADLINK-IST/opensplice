#!/bin/sh

PATH=/usr/bin:/bin:/usr/local/bin:$PATH
export PATH

Assert setupBuildInfo
ArchiveLogs

if [ "$BUILD" = "no" ]
then
    echo "BUILD=SKIPPED" >> $RESFILE
    SetState "Complete"
    ArchiveLogs
    exit 0
fi


#Logs to use when there is a fail
cat > $LOGDIR/LOGFILES <<EOF
BUILD=build.txt
BUILD_DBT=build-dbt-tests.txt
BUILD_RBT=build-rbt-tests.txt
RUN_DBT=perform-dbt-tests.txt
RUN_RBT=perform-rbt-tests.txt
BUILD_EXAMPLES=examples/
RUN_EXAMPLES=examples/
BUILD_DIST=build-dist.txt
KEEP_DIST=archive-dist.txt
EOF

#Logs to use when pass
cat > $LOGDIR/LOGFILES_PASSED <<EOF
BUILD=build.txt
BUILD_DBT=build-dbt-tests.txt
BUILD_RBT=build-rbt-tests.txt
RUN_DBT=perform-dbt-tests.txt
RUN_RBT=perform-rbt-tests.txt
BUILD_EXAMPLES=examples/
RUN_EXAMPLES=examples/
BUILD_DIST=build-dist.txt
KEEP_DIST=../distro
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
if [ $RUN_DBT = "yes" ]
then
    echo "BUILD/DBT=TODO" >> $RESFILE
    echo "RUN/DBT=TODO" >> $RESFILE
else
    echo "BUILD/DBT=SKIP" >> $RESFILE
    echo "RUN/DBT=SKIP" >> $RESFILE
fi
if [ $RUN_RBT = "yes" ]
then
    echo "BUILD/RBT=TODO" >> $RESFILE
    echo "RUN/RBT=TODO" >> $RESFILE
else
    echo "BUILD/RBT=SKIP" >> $RESFILE
    echo "RUN/RBT=SKIP" >> $RESFILE
fi
if [ $RUN_EXAMPLES = "yes" ]
then
    echo "BUILD/EXAMPLES=TODO" >> $RESFILE
    echo "RUN/EXAMPLES=TODO" >> $RESFILE
else
    echo "BUILD/EXAMPLES=SKIP" >> $RESFILE
    echo "RUN/EXAMPLES=SKIP" >> $RESFILE
fi
ArchiveLogs

$IBSDIR/dcps_build $ARGS > $LOGDIR/build.txt 2>&1
BUILD_STAGE_WORKED=$?
if test_build $LOGDIR/build.txt $BUILD_STAGE_WORKED
then
    echo "BUILD=PASS" >> $RESFILE
else
    echo "BUILD=FAIL" >> $RESFILE
    BUILD_STAGE_WORKED=1
fi
ArchiveLogs

if [ $BUILD_STAGE_WORKED = 0 ]
then
    $IBSDIR/dcps_build_dist $ARGS > $LOGDIR/build-dist.txt 2>&1
    BUILD_DIST_STAGE_WORKED=$?
    if test_build_dist $LOGDIR/build-dist.txt $BUILD_DIST_STAGE_WORKED
    then
        echo "BUILD_DIST=PASS" >> $RESFILE        
        $IBSDIR/dcps_archive_dist $ARGS > $LOGDIR/archive-dist.txt 2>&1
        ARCHIVE_STAGE_WORKED=$?
        if test_archive_dist $LOGDIR/archive-dist.txt $ARCHIVE_STAGE_WORKED
        then
            echo "KEEP_DIST=PASS" >> $RESFILE
        else
            echo "KEEP_DIST=FAIL" >> $RESFILE
            ARCHIVE_STAGE_WORKED=1
        fi
    else
        echo "BUILD_DIST=FAIL" >> $RESFILE
        BUILD_DIST_STAGE_WORKED=1
        echo "KEEP_DIST=ABORTED" >> $RESFILE
        ARCHIVE_STAGE_WORKED=1
    fi
    ArchiveLogs

    if [ "$RUN_DBT" != "yes" ]
    then
        echo "BUILD/DBT=SKIPPED" >> $RESFILE
        echo "RUN/DBT=SKIPPED" >> $RESFILE
        BUILD_DBT_STAGE_WORKED=0
        PERFORM_DBT_STAGE_WORKED=0
    else
        $IBSDIR/dcps_build_dbt_tests $ARGS > $LOGDIR/build-dbt-tests.txt 2>&1
        BUILD_DBT_STAGE_WORKED=$?
        if test_build_dbt_tests $LOGDIR/build-dbt-tests.txt $BUILD_DBT_STAGE_WORKED
        then
            echo "BUILD/DBT=PASS" >> $RESFILE
        else
            echo "BUILD/DBT=FAIL" >> $RESFILE
            BUILD_DBT_STAGE_WORKED=1
        fi
        ArchiveLogs
        
        $IBSDIR/dcps_perform_dbt_tests $ARGS > $LOGDIR/perform-dbt-tests.txt 2>&1
        PERFORM_DBT_STAGE_WORKED=$?
        if test_perform_dbt_tests $LOGDIR/DBT-Results $LOGDIR/perform-dbt-tests.txt $PERFORM_DBT_STAGE_WORKED
        then
            echo "RUN/DBT=PASS" >> $RESFILE
        else
            echo "RUN/DBT=$DBT_INFO" >> $RESFILE
            PERFORM_DBT_STAGE_WORKED=1
        fi
        ArchiveLogs
    fi

    if [ "$RUN_RBT" != "yes" ]
    then
        echo "BUILD/RBT=SKIPPED" >> $RESFILE
        echo "RUN/RBT=SKIPPED" >> $RESFILE
        BUILD_RBT_STAGE_WORKED=0
        PERFORM_RBT_STAGE_WORKED=0
    else
        $IBSDIR/dcps_build_rbt_tests $ARGS > $LOGDIR/build-rbt-tests.txt 2>&1
        BUILD_RBT_STAGE_WORKED=$?
        if test_build_rbt_tests $LOGDIR/build-rbt-tests.txt $BUILD_RBT_STAGE_WORKED
        then
            echo "BUILD/RBT=PASS" >> $RESFILE
        else
            echo "BUILD/RBT=FAIL" >> $RESFILE
            BUILD_RBT_STAGE_WORKED=1
        fi
        ArchiveLogs
        $IBSDIR/dcps_perform_rbt_tests $ARGS > $LOGDIR/perform-rbt-tests.txt 2>&1
        PERFORM_RBT_STAGE_WORKED=$?
        if test_perform_rbt_tests $LOGDIR/RBT-Results $LOGDIR/perform-rbt-tests.txt $PERFORM_RBT_STAGE_WORKED
        then
            echo "RUN/RBT=PASS" >> $RESFILE
        else
            echo "RUN/RBT=$RBT_INFO" >> $RESFILE
            PERFORM_RBT_STAGE_WORKED=1
        fi
        ArchiveLogs
    fi
    if [ "$RUN_EXAMPLES" != "yes" ]
    then
        echo "BUILD/EXAMPLES=SKIPPED" >> $RESFILE
        echo "RUN/EXAMPLES=SKIPPED" >> $RESFILE
        BUILD_RUN_EXAMPLES_STAGE_WORKED=0
    else
        $IBSDIR/dcps_build_run_examples $ARGS > $LOGDIR/build-run-examples.txt 2>&1
        BUILD_RUN_EXAMPLES_STAGE_WORKED=$?
        if $BUILD_RUN_EXAMPLES_STAGE_WORKED
        then
            echo "BUILD/EXAMPLES=PASS" >> $RESFILE
            echo "RUN/EXAMPLES=PASS" >> $RESFILE
        else
            echo "BUILD/EXAMPLES=FAIL" >> $RESFILE
            echo "RUN/EXAMPLES=FAIL" >> $RESFILE
            BUILD_RUN_EXAMPLES_STAGE_WORKED=1
        fi
        ArchiveLogs
    fi
fi

if [ "$BUILD_STAGE_WORKED" = 0 ]
then
    if [ "$BUILD_RUN_EXAMPLES_STAGE_WORKED" != 0 ]
    then
	SetState "ExamplesFailed"
    else
       if [ "$PERFORM_DBT_STAGE_WORKED" != 0 -o "$PERFORM_RBT_STAGE_WORKED" != 0 ]
       then
           SetState "TestsFailed"
       else
           if [ "$BUILD_DIST_STAGE_WORKED" = 0 -a "$ARCHIVE_STAGE_WORKED" = 0 -a \
               "$BUILD_DBT_STAGE_WORKED" = 0 -a "$BUILD_RBT_STAGE_WORKED" = 0 ]
           then
               SetState "Complete"
           else
               SetState "Failed"
           fi
       fi
    fi
else
   SetState "Failed"
fi
ArchiveLogs
