#!/bin/bash

# This will pull in a right STAF environment.
. "$(dirname "$0")/ospldevutils.shsrc" "$@"
. "$(dirname "$0")/stafutils.shsrc" "$@" || exit 0

echo -e "\nStopping STAF..."
stop_staf || exit 1
