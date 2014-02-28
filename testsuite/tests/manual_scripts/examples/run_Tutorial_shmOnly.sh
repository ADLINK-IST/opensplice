#!/bin/bash

. "$(dirname "$0")/../ospldevutils.shsrc" "$@"
. "$(dirname "$0")/../ospldistutils.shsrc" "$@"
. "$(dirname "$0")/exsettings.shsrc" "$@" || exit 0

# Setup dev environment.
source_tao_runtime || exit 1
source_java_runtime || exit 1

# Setup dist environment.
source_ospldist_env || exit 1

check_ex_prereqs || exit 1

g_tscript="$SRC_ROOT/ospli/testsuite/tests/tutorial/run_tutorial_test.xml"

function run_single_dcps_tutorial ()
{
    if [ $# -ne 9 ]; then
        error_trace "ERROR: Wrong number of arguments to ${FUNCNAME[0]}"
        return 1
    fi

    local master="$1" && test -n "$master" || error_trace "ERROR: Parameter \$1 is empty" || return 1
    local masterlog="$2" && test -n "$masterlog" || error_trace "ERROR: Parameter \$2 is empty" || return 1
    local masterospl="$3" && test -n "$masterospl" || error_trace "ERROR: Parameter \$3 is empty" || return 1
    local slave="$4" && test -n "$slave" || error_trace "ERROR: Parameter \$4 is empty" || return 1
    local slavelog="$5" && test -n "$slavelog" || error_trace "ERROR: Parameter \$5 is empty" || return 1
    local slaveospl="$6" && test -n "$slaveospl" || error_trace "ERROR: Parameter \$6 is empty" || return 1
    local runtype="$7" && test -n "$runtype" || error_trace "ERROR: Parameter \$7 is empty" || return 1
    local transport="$8" && test -n "$transport" || error_trace "ERROR: Parameter \$8 is empty" || return 1
    local impl="$9" && test -n "$impl" || error_trace "ERROR: Parameter \$9 is empty" || return 1

    echo -e "\n-------------------------------"
    echo "<<< Tutorial DCPS in '$impl' between '$master' and '$slave' using '$runtype' and '$transport' >>>"

    STAF local STAX EXECUTE FILE "$g_tscript" JOBNAME DDSTutorial FUNCTION tutorial_main ARGS "['$runtype', '$master', '$masterospl', '$masterlog', '$impl', '$slave', '$slaveospl', '$slavelog', '$impl', 'dcps', '$MY_STAF_PORT', '$MY_STAF_PORT']" WAIT RETURNRESULT DETAILS
    local result=$?

    echo "<<< STAF returned $result >>>"

    return $result
}

# Currently we assume that master host is always a localhost.
run_tests run_single_dcps_tutorial "$(plain_hostname)" "shm" "broadcast multicast" "c cpp java corba_cpp corba_java" 2>&1 | tee "$PWD/screen.log"

exit $g_examples_failed
