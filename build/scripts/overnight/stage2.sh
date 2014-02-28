#!/bin/bash

Assert setupBuildInfo
ArchiveLogs

if [ "$PRODUCT" = "ospli" ]
then
   OSPL_INNER_REV=`git rev-parse --short $REV`
else
   OSPL_INNER_REV=`cd $IBSDIR && git rev-parse --short HEAD`
   OSPL_OUTER_REV=`git rev-parse --short HEAD`
   export OSPL_OUTER_REV
fi
export OSPL_INNER_REV

if [ "$BUILD" = "no" ]
then
    echo "BUILD=SKIPPED" >> $RESFILE
    SetState "Complete"
    ArchiveLogs
    exit 0
fi

#Logs to use when there is a fail
cat > $LOGDIR/LOGFILES <<EOF
BUILD=build.txt_Totals.html
BUILD_DBT=build-dbt-tests.txt_Totals.html
BUILD_RBT=build-rbt-tests.txt_Totals.html
RUN_DBT=perform-dbt-tests.txt
RUN_RBT_SP=perform-rbt-tests-sp.txt
RUN_RBT_SHM=perform-rbt-tests-shm.txt
BUILD_EXAMPLES=examples/build/BuildResults_Totals.html
BUILD_SRC=build-src.txt
VALGRIND_SHM=valgrind_shm/vg_summary.html
VALGRIND_SP=valgrind_sp/vg_summary.html
COVERAGE_GCOV=coverage/index.html
COVERAGE_JACOCO=jcoverage/index.html
BUILD_DIST=build-dist.txt_Totals.html
KEEP_DIST=archive-dist.txt
EOF

vx_grep=`echo $SETUP_TYPE | grep vxworks`

if [ -n "$vx_grep" ]
then
echo "RUN_EXAMPLES_SP=examples/run_sp/vxworks_results.html" >> $LOGDIR/LOGFILES
echo "RUN_EXAMPLES_SHM=examples/run_shm/vxworks_results.html" >> $LOGDIR/LOGFILES
else
echo "RUN_EXAMPLES_SP=examples/run_sp/summary.html" >> $LOGDIR/LOGFILES
echo "RUN_EXAMPLES_SHM=examples/run_shm/summary.html" >> $LOGDIR/LOGFILES
fi


#Logs to use when pass
cat > $LOGDIR/LOGFILES_PASSED <<EOF
BUILD=build.txt_Totals.html
BUILD_DBT=build-dbt-tests.txt_Totals.html
BUILD_RBT=build-rbt-tests.txt_Totals.html
RUN_DBT=perform-dbt-tests.txt
RUN_RBT_SP=perform-rbt-tests-sp.txt
RUN_RBT_SHM=perform-rbt-tests-shm.txt
BUILD_EXAMPLES=examples/build/BuildResults_Totals.html
BUILD_SRC=build-src.txt
VALGRIND_SHM=valgrind_shm/vg_summary.html
VALGRIND_SP=valgrind_sp/vg_summary.html
COVERAGE_GCOV=coverage/index.html
COVERAGE_JACOCO=jcoverage/index.html
BUILD_DIST=build-dist.txt_Totals.html
KEEP_DIST=../distro
EOF

if [ -n "$vx_grep" ]
then
echo "RUN_EXAMPLES_SP=examples/run_sp/vxworks_results.html" >> $LOGDIR/LOGFILES_PASSED
echo "RUN_EXAMPLES_SHM=examples/run_shm/vxworks_results.html" >> $LOGDIR/LOGFILES_PASSED
else
echo "RUN_EXAMPLES_SP=examples/run_sp/summary.html" >> $LOGDIR/LOGFILES_PASSED
echo "RUN_EXAMPLES_SHM=examples/run_shm/summary.html" >> $LOGDIR/LOGFILES_PASSED
fi

cat > $RESFILE <<EOF
BUILD=TODO
EOF

if [ "$BUILD_DIST" = "yes" ]
then
    echo "BUILD_DIST=TODO" >> $RESFILE
else
    echo "BUILD_DIST=SKIP" >> $RESFILE
fi

if [ "$KEEP_DIST" = "yes" ]
then
    echo "KEEP_DIST=TODO" >> $RESFILE
else
    echo "KEEP_DIST=SKIP" >> $RESFILE
fi

if [ "$BUILD_RBT" = "yes" ]
then
    echo "BUILD/RBT=TODO" >> $RESFILE
else
    echo "BUILD/RBT=SKIP" >> $RESFILE
fi

if [ "$BUILD_DBT" = "yes" ]
then
    echo "BUILD/DBT=TODO" >> $RESFILE
else
    echo "BUILD/DBT=SKIP" >> $RESFILE
fi

if [ "$BUILD_SRC" = "yes"  ]
then
    echo "BUILD/SRC=TODO" >> $RESFILE
else
    echo "BUILD/SRC=SKIP" >> $RESFILE
fi

if [ "$BUILD_EXAMPLES" = "yes"  ]
then
    echo "BUILD/EXAMPLES=TODO" >> $RESFILE
else
    echo "BUILD/EXAMPLES=SKIP" >> $RESFILE
fi

if [ "$RUN_EXAMPLES_SP" = "yes" ]
then
    echo "RUN_EXAMPLES/SP=TODO" >> $RESFILE
else
    echo "RUN_EXAMPLES/SP=SKIP" >> $RESFILE
fi

if [ "$RUN_EXAMPLES_SHM" = "yes" ]
then
    echo "RUN_EXAMPLES/SHM=TODO" >> $RESFILE
else
    echo "RUN_EXAMPLES/SHM=SKIP" >> $RESFILE
fi

if [ "$RUN_RBT_SP" = "yes" ]
then
    echo "RUN_RBT/SP=TODO" >> $RESFILE
else
    echo "RUN_RBT/SP=SKIP" >> $RESFILE
fi

if [ "$RUN_RBT_SP" = "yes" ]
then
    echo "RUN_RBT/SHM=TODO" >> $RESFILE
else
    echo "RUN_RBT/SHM=SKIP" >> $RESFILE
fi

if [ "$RUN_DBT" = "yes" ]
then
    echo "RUN/DBT=TODO" >> $RESFILE
else
    echo "RUN/DBT=SKIP" >> $RESFILE
fi

if [ "$VALGRIND" = "yes" -a "$RUN_EXAMPLES_SP" = "yes" ]
then
    echo "VALGRIND/SP=TODO" >> $RESFILE
else
    echo "VALGRIND/SP=SKIP" >> $RESFILE
fi

if [ "$VALGRIND" = "yes" -a "$RUN_EXAMPLES_SHM" = "yes" ]
then
    echo "VALGRIND/SHM=TODO" >> $RESFILE
else
    echo "VALGRIND/SHM=SKIP" >> $RESFILE
fi

if [ "$TEST_GCOV" = "yes" ]
then
    echo "COVERAGE/GCOV=TODO" >> $RESFILE
else
    echo "COVERAGE/GCOV=SKIP" >> $RESFILE
fi

if [ "$TEST_JACOCO" = "yes" ]
then
    echo "COVERAGE/JACOCO=TODO" >> $RESFILE
else
    echo "COVERAGE/JACOCO=SKIP" >> $RESFILE
fi

ArchiveLogs

run_examples()
{
    EXRUNTYPE=$1
    export EXRUNTYPE
    EXRUNTYPE_UPPER=`echo $1 | tr "[:lower:]" "[:upper:]"`

    STAGE_WORKED=RUN_EXAMPLES_${EXRUNTYPE_UPPER}_STAGE_WORKED
    VG_STAGE_RES=VALGRIND_${EXRUNTYPE_UPPER}_STAGE_OK

    eval FLAG=\$RUN_EXAMPLES_${EXRUNTYPE_UPPER}

    if [ "$FLAG" = "yes" ]
    then
        if [ "$BUILD_EXAMPLES" = "yes" ]
        then
            if [ "$VALGRIND" = "yes" ]
            then
                mkdir $LOGDIR/valgrind_${EXRUNTYPE}
            fi
            mkdir $LOGDIR/examples/run_$EXRUNTYPE
            AUTOMATION_DIR=$WORKDIR/build/testsuite/automation \
               VXWORKS_KERNEL_MODE=$VXWORKS_KERNEL_MODE \
               OSPL_EXAMPLE_TEST_KERNEL=$OSPL_EXAMPLE_TEST_KERNEL \
               OSPL_OUTER_HOME=$WORKDIR/build \
               $IBSDIR/dcps_run_examples $ARGS > $LOGDIR/examples/run_$EXRUNTYPE/run_results.txt 2>&1

            eval $STAGE_WORKED=$?
            eval RES=\$${STAGE_WORKED}
            if [ $RES = 0 ]
            then
                echo "RUN_EXAMPLES/${EXRUNTYPE_UPPER}=PASS" >> $RESFILE
            else
                echo "RUN_EXAMPLES/${EXRUNTYPE_UPPER}=FAIL" >> $RESFILE
            fi

            if [ "$VALGRIND" = "yes" ]
            then
                create_valgrind_summary $LOGDIR/valgrind_$EXRUNTYPE/
                eval $VG_STAGE_RES=$?
                eval RES=\$$VG_STAGE_RES
                if [ "$RES" = 0 ]
                then
                    echo "VALGRIND/${EXRUNTYPE_UPPER}=PASS" >> $RESFILE
                else
                    echo "VALGRIND/${EXRUNTYPE_UPPER}=FAIL" >> $RESFILE
                fi
                ArchiveLogs
            else
                echo "VALGRIND/${EXRUNTYPE_UPPER}=SKIPPED" >> $RESFILE
                eval $VG_STAGE_RES=0
            fi
            # Remove valgrind generated files
            SHORTSETUP=`echo $SETUP_TYPE | sed 's/-release//'`
            find "$WORKDIR/$EXAMPLE_INSTALL_DIR/HDE/$SHORTSETUP/" \
                -type f \( -name 'vg_*.txt' -o -name '*.log' \) -exec rm {} \;
        fi
    else
        echo "VALGRIND/${EXRUNTYPE_UPPER}=SKIPPED" >> $RESFILE
        echo "RUN_EXAMPLES/${EXRUNTYPE_UPPER}=SKIPPED" >> $RESFILE
        eval $STAGE_WORKED=0
        eval $VG_STAGE_RES=0
    fi
}

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
    if [ "$BUILD_DIST" != "yes" ]
    then
        echo "BUILD_DIST=SKIPPED" >> $RESFILE
        BUILD_DIST_STAGE_WORKED=0
        if [ "$KEEP_DIST" != "yes" ]
        then
            echo "KEEP_DIST=SKIPPED" >> $RESFILE
            ARCHIVE_STAGE_WORKED=0
        else
            echo "KEEP_DIST=ABORTED" >> $RESFILE
            ARCHIVE_STAGE_WORKED=1
        fi
    else
        $IBSDIR/dcps_build_dist $ARGS > $LOGDIR/build-dist.txt 2>&1
        BUILD_DIST_STAGE_WORKED=$?
        if test_build_dist $LOGDIR/build-dist.txt $BUILD_DIST_STAGE_WORKED
        then
            echo "BUILD_DIST=PASS" >> $RESFILE
            if [ "$KEEP_DIST" != "yes" ]
            then
                echo "KEEP_DIST=SKIPPED" >> $RESFILE
                ARCHIVE_STAGE_WORKED=0
            else
                $IBSDIR/dcps_archive_dist $ARGS > $LOGDIR/archive-dist.txt 2>&1
                ARCHIVE_STAGE_WORKED=$?
                if test_archive_dist $LOGDIR/archive-dist.txt $ARCHIVE_STAGE_WORKED
                then
                    echo "KEEP_DIST=PASS" >> $RESFILE
                else
                    echo "KEEP_DIST=FAIL" >> $RESFILE
                    ARCHIVE_STAGE_WORKED=1
                fi
            fi
        else
            echo "BUILD_DIST=FAIL" >> $RESFILE
            BUILD_DIST_STAGE_WORKED=1
            if [ "$KEEP_DIST" != "yes" ]
            then
                echo "KEEP_DIST=SKIPPED" >> $RESFILE
                ARCHIVE_STAGE_WORKED=0
            else
                echo "KEEP_DIST=ABORTED" >> $RESFILE
                ARCHIVE_STAGE_WORKED=1
            fi
        fi
    fi
    ArchiveLogs

    if [ $BUILD_DIST_STAGE_WORKED != 0 ]
    then
        echo "BUILD/SRC=SKIPPED" >> $RESFILE
        BUILD_SRC_STAGE_WORKED=0
    else
        if [ "$BUILD_SRC" != "yes" ]
        then
            echo "BUILD/SRC=SKIPPED" >> $RESFILE
            BUILD_SRC_STAGE_WORKED=0
        else
            $IBSDIR/dcps_build_src $ARGS > $LOGDIR/build-src.txt 2>&1
            BUILD_SRC_STAGE_WORKED=$?
            if test_build $LOGDIR/build-src.txt $BUILD_SRC_STAGE_WORKED
            then
                echo "BUILD/SRC=PASS" >> $RESFILE
            else
                echo "BUILD/SRC=FAIL" >> $RESFILE
                BUILD_SRC_STAGE_WORKED=1
            fi
        fi
    fi
    ArchiveLogs

    if [ "$BUILD_RBT" != "yes" ]
    then
        echo "BUILD/RBT=SKIPPED" >> $RESFILE
        BUILD_RBT_STAGE_WORKED=0
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
    fi
    ArchiveLogs

    if [  "$BUILD_RBT" != "yes" -o "$RUN_RBT_SP" != "yes" ]
    then
        echo "RUN_RBT/SP=SKIPPED" >> $RESFILE
        PERFORM_RBT_SP_STAGE_WORKED=0
    else
        RBTLOGDIR=$LOGDIR/RBT-Results-SP
        export RBTLOGDIR
        RBTTYPE=SP
        export RBTTYPE
        TIMED_TESTS=$LOGDIR/RBT-TimedTests-SP.log
        export TIMED_TESTS
        $IBSDIR/dcps_perform_rbt_tests $ARGS > $LOGDIR/perform-rbt-tests-sp.txt 2>&1
        PERFORM_RBT_SP_STAGE_WORKED=$?
        if test_perform_rbt_tests $LOGDIR/RBT-Results-SP $LOGDIR/perform-rbt-tests-sp.txt $PERFORM_RBT_SP_STAGE_WORKED
        then
            echo "RUN_RBT/SP=PASS" >> $RESFILE
        else
            echo "RUN_RBT/SP=$RBT_INFO" >> $RESFILE
            PERFORM_RBT_SP_STAGE_WORKED=1
        fi
    fi
    ArchiveLogs

    if [  "$BUILD_RBT" != "yes" -o "$RUN_RBT_SHM" != "yes" ]
    then
        echo "RUN_RBT/SHM=SKIPPED" >> $RESFILE
        PERFORM_RBT_SHM_STAGE_WORKED=0
    else
        RBTLOGDIR=$LOGDIR/RBT-Results-SHM
        export RBTLOGDIR
        RBTTYPE=SHM
        export RBTTYPE
        TIMED_TESTS=$LOGDIR/RBT-TimedTests-SHM.log
        export TIMED_TESTS
        $IBSDIR/dcps_perform_rbt_tests $ARGS > $LOGDIR/perform-rbt-tests-shm.txt 2>&1
        PERFORM_RBT_SHM_STAGE_WORKED=$?
        if test_perform_rbt_tests $LOGDIR/RBT-Results-SHM $LOGDIR/perform-rbt-tests-shm.txt $PERFORM_RBT_SHM_STAGE_WORKED
        then
            echo "RUN_RBT/SHM=PASS" >> $RESFILE
        else
            echo "RUN_RBT/SHM=$RBT_INFO" >> $RESFILE
            PERFORM_RBT_SHM_STAGE_WORKED=1
        fi
    fi
    ArchiveLogs

    if [ "$BUILD_DBT" != "yes" ]
    then
        echo "BUILD/DBT=SKIPPED" >> $RESFILE
        BUILD_DBT_STAGE_WORKED=0
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
    fi
    ArchiveLogs

    if [ "$BUILD_DBT" != "yes" -o "$RUN_DBT" != "yes" ]
    then
        echo "RUN/DBT=SKIPPED" >> $RESFILE
        PERFORM_DBT_STAGE_WORKED=0
    else
        TIMED_TESTS=$LOGDIR/DBT-TimedTests.log
        export TIMED_TESTS
        $IBSDIR/dcps_perform_dbt_tests $ARGS > $LOGDIR/perform-dbt-tests.txt 2>&1
        PERFORM_DBT_STAGE_WORKED=$?
        if test_perform_dbt_tests $LOGDIR/DBT-Results $LOGDIR/perform-dbt-tests.txt $PERFORM_DBT_STAGE_WORKED
        then
            echo "RUN/DBT=PASS" >> $RESFILE
        else
            echo "RUN/DBT=$DBT_INFO" >> $RESFILE
            PERFORM_DBT_STAGE_WORKED=1
        fi
    fi
    ArchiveLogs

    if [ "$TEST_GCOV" = "yes" -o "$TEST_JACOCO" = "yes" ]
    then
        $IBSDIR/coverage_collect_results $ARGS > $LOGDIR/collect-coverage-statistics.txt 2>&1
    else
        echo "COVERAGE/GCOV=SKIPPED" >> $RESFILE
        echo "COVERAGE/JACOCO=SKIPPED" >> $RESFILE
    fi

    ArchiveLogs

    #Do examples first as they rarely hang and are quite quick
    if [ "$BUILD_EXAMPLES" = "yes" ]
    then
        CURRENT_PL_CYGWIN=`uname | grep CYGWIN`

        # For windows platforms we need to test a directory with spaces.  This
        # is not so important for posix platforms as generally spaces are not
        # used in directories
        if [ "$CURRENT_PL_CYGWIN" != "" ];
        then
            EXAMPLE_INSTALL_DIR="test inst"
        else
            EXAMPLE_INSTALL_DIR=installed
        fi

        export EXAMPLE_INSTALL_DIR

        RTS_INSTALL_DIR=rtsinstall
        export RTS_INSTALL_DIR

        mkdir $LOGDIR/examples
        mkdir $LOGDIR/examples/build
        $IBSDIR/dcps_build_examples $ARGS > $LOGDIR/examples/build/build_results.txt 2>&1
        BUILD_EXAMPLES_STAGE_WORKED=$?
        test_example_build $LOGDIR/examples/build
        BUILD_EXAMPLES_LOGS_OK=$?
        if [ $BUILD_EXAMPLES_STAGE_WORKED = 0 -a $BUILD_EXAMPLES_LOGS_OK = 0 ]
        then
            echo "BUILD/EXAMPLES=PASS" >> $RESFILE
        else
            echo "BUILD/EXAMPLES=FAIL" >> $RESFILE
            BUILD_EXAMPLES_STAGE_WORKED=1
        fi
    else
        echo "BUILD/EXAMPLES_SHM=SKIPPED" >> $RESFILE
        BUILD_EXAMPLES_STAGE_WORKED=0
    fi
    ArchiveLogs

    run_examples sp
    run_examples shm

    if [ "$BUILD_EXAMPLES" = "yes"  ]
    then
        # Uninstall the distribution that was unpacked during dcps_build_examples
        $IBSDIR/dcps_uninstall_distro $ARGS > $LOGDIR/examples/build/uninstall_distro.txt 2>&1
        UNINSTALL_DISTRO_STAGE_WORKED=$?
    fi

    ArchiveLogs
fi

if [ "$BUILD_STAGE_WORKED" = 0 ]
then
    if [ "$BUILD_DIST_STAGE_WORKED" = 0 -a "$ARCHIVE_STAGE_WORKED" = 0 -a \
        "$BUILD_DBT_STAGE_WORKED" = 0 -a "$BUILD_RBT_STAGE_WORKED" = 0 -a \
        "$BUILD_SRC_STAGE_WORKED" = 0 ]
    then
        if [ "$PERFORM_DBT_STAGE_WORKED" != 0 -o "$PERFORM_RBT_SP_STAGE_WORKED" != 0 -o "$PERFORM_RBT_SHM_STAGE_WORKED" != 0 ]
        then
            SetState "TestsFailed"
        else
            if [ "$RUN_EXAMPLES_SHM_STAGE_WORKED" != 0 -o "$RUN_EXAMPLES_SP_STAGE_WORKED" != 0 -o "$BUILD_EXAMPLES_STAGE_WORKED" != 0 ]
            then
                SetState "ExamplesFailed"
            else
                if [ "$VALGRIND_SHM_STAGE_OK" = 1 -o "$VALGRIND_SP_STAGE_OK" = 1 ]
                then
                    SetState "ValgrindFail"
                else
                    if [ "$VALGRIND_SHM_STAGE_OK" = 2 -o "$VALGRIND_SP_STAGE_OK" = 2 ]
                    then
                        SetState "ValgrindLeak"
                    else
                        SetState "Complete"
                    fi
                fi
            fi
        fi
    else
        SetState "Failed"
    fi
else
   SetState "Failed"
fi
ArchiveLogs
