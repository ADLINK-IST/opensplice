#!/bin/bash

. "$(dirname "$0")/ospldistutils.shsrc" "$@" || exit 0

# Start bash subprocess with the previously set environment.
if [ "$(uname -o)" = 'Cygwin' ]; then
    source_ospldev_env_for_windows || exit 1
    source_osplsrc_env || exit 1

    /bin/bash
else
    source_osplsrc_env || exit 1

    if [ "$(uname -m)" = 'x86_64' ]; then
        # setarch with -RL options will setup environment useful for
        # development. In particular it will turn off address randomization
        # which simplifies debugging.
        setarch x86_64 -RL /bin/bash
    elif [ "$(uname -m)" = 'i686' ]; then
        setarch i686 -RL /bin/bash
    else
        echo "ERROR: Unknown environment"
        exit 1
    fi
fi
