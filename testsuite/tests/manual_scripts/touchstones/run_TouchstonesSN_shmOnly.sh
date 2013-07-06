#!/bin/bash

. "$(dirname "$0")/../ospldistutils.shsrc" "$@"
. "$(dirname "$0")/tssettings.shsrc" "$@" || exit 0

# Setup dist environment.
source_ospldist_env || exit 1

check_ts_dir || exit 1

check_ts_prereqs || exit 1

function run_single_touchstoneSN ()
{
    if [ $# -ne 12 ]; then
        error_trace "ERROR: Wrong number of arguments to ${FUNCNAME[0]}"
        return 1
    fi

    local master="$1" && test -n "$master" || error_trace "ERROR: Parameter \$1 is empty" || return 1
    local masterlog="$2" && test -n "$masterlog" || error_trace "ERROR: Parameter \$2 is empty" || return 1
    local masterospl="$3" && test -n "$masterospl" || error_trace "ERROR: Parameter \$3 is empty" || return 1
    local mastertsbin="$4" && test -n "$mastertsbin" || error_trace "ERROR: Parameter \$4 is empty" || return 1
    # Variables 5 through 8 are not used (They contain configuration for
    # slave which is not used in this test).
    local runtype="$9" && test -n "$runtype" || error_trace "ERROR: Parameter \$9 is empty" || return 1
    local transport="${10}" && test -n "$transport" || error_trace "ERROR: Parameter \$10 is empty" || return 1
    local multi="${11}" && test -n "$multi" || error_trace "ERROR: Parameter \$11 is empty" || return 1
    local impl="${12}" && test -n "$impl" || error_trace "ERROR: Parameter \$12 is empty" || return 1

    # We plainly hardcode some of the values. There is no sense to make these
    # parameters configurable as they change very rarely.
    local master_ospl_config="$masterospl/etc/config/ospl.xml"

    echo -e "\n-------------------------------"
    echo "<<< TouchStones SingleNode ($multi - $multi) in '$impl' between '$master' and '$master' using '$runtype' and '$transport' >>>"

    STAF local STAX EXECUTE FILE "$g_tsscript" JOBNAME DDSTouchStones FUNCTION touchstone_main ARGS "[$g_test_duration, '$master', '$master', '$masterospl', '$masterospl', '$masterlog', '$masterlog', '$mastertsbin', '$mastertsbin', '$impl', '$impl', $multi, $multi, $g_master_ts_group_id, $g_slave_ts_group_id, '$master_ospl_config', '$master_ospl_config', 'SN', '$g_master_recorder_script', $g_master_ts_start_app_id, $g_slave_ts_start_app_id, '$MY_STAF_PORT', '$MY_STAF_PORT']" WAIT RETURNRESULT DETAILS
    local result=$?

    echo "<<< STAF returned $result >>>"

    return $result
}

# Currently we assume that master host is always a localhost.
run_tests run_single_touchstoneSN "$(plain_hostname)" "$(plain_hostname)" "shm" "broadcast multicast" "1" "C C++ Java" 2>&1 | tee "$PWD/screen.log"

exit $g_touchstones_failed
