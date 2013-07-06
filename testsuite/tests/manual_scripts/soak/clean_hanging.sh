#!/bin/bash

if [ $# -ne 0 ]; then
    echo "Usage: '$0'"
fi

function mykillall ()
{
    if [ $# -ne 2 ]; then
        echo "ERROR: Wrong number of arguments to ${FUNCNAME[0]}"
        return 1
    fi

    if [ "$(uname -o)" = 'Cygwin' ]; then
        ps -W | grep -v 'grep' | grep --ignore-case "$2" | gawk '{print $1}' | xargs --replace=_ /bin/kill "$1" _
    else
        killall "$1" "$2"
    fi
}

# We try to avoid killing other Java processes.
function mykillall_java ()
{
    if [ $# -ne 1 ]; then
        echo "ERROR: Wrong number of arguments to ${FUNCNAME[0]}"
        return 1
    fi

    if [ "$(uname -o)" = 'Cygwin' ]; then
        ps -W | grep -v 'grep' | grep 'cygdrive.*java' | gawk '{print $1}' | xargs --replace=_ /bin/kill "$1" _
    else
        ps ax | grep -v 'grep' | grep 'java.*touchstone.Main' | gawk '{print $1}' | xargs --replace=_ /bin/kill "$1" _
    fi
}

# Kill mmstats.
mykillall -9 mmstat &>/dev/null

# Kill watchers.
mykillall -9 watcher &>/dev/null

# Kill recorders.
mykillall -9 recorder &>/dev/null

# Kill touchstones.
mykillall -9 touchstone_cpp &>/dev/null
mykillall_java -9 &>/dev/null

# Kill splice daemons.
mykillall -9 spliced &>/dev/null
mykillall -9 networking &>/dev/null
mykillall -9 durability &>/dev/null

if [ "$(uname -o)" = 'Cygwin' ]; then
    # Clean temporary files.
    rm -f /tmp/osp*.DBF &>/dev/null
    rm -f /tmp/osp*.tmp &>/dev/null
else
    # Clean memory files.
    rm -f /tmp/spddskey_* &>/dev/null

    # Clean a left over shared memory. There is a heuristic that the size
    # of the shared memory allocated by OSPL is 10485760.
    ipcs -m | awk '$3=="'$USER'" && $5==10485760 {print "-m",$2}' | xargs ipcrm
fi
