#!/bin/bash

# Define this before common routines are loaded.
g_impl='cpp'

. "$(dirname "$0")/../ospldistutils.shsrc" "$@"
. "$(dirname "$0")/soakutils.shsrc" "$@" || exit 0

# Setup dist environment.
source_ospldist_env || exit 1

# After logging is setup we can dump screen log to log directory.
setup_logging || exit 1

run_soak_test 2>&1 | tee "$g_unix_logdir/screen.log"

exit $g_soak_test_failed
