#!/bin/bash

# This will pull in a right STAF environment.
. "$(dirname "$0")/ospldevutils.shsrc" "$@"
. "$(dirname "$0")/stafutils.shsrc" "$@" || exit 0

echo -e "\nStopping STAF..."
stop_staf || exit 1

source_tao_runtime || exit 1
source_java_runtime || exit 1
echo -e "\nStarting STAF..."
start_staf || exit 1
