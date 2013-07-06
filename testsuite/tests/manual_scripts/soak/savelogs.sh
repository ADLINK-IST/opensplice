#!/bin/bash

if [ $# -ne 0 ]; then
    echo "Usage: '$0'"
fi

# Don't process common arguments.
. "$(dirname "$0")/../stafutils.shsrc"
. "$(dirname "$0")/soakutils.shsrc" --remote=unused || exit 0

# Setup STAF environment.
source_staf_env || exit 1

savelogs "$(plain_hostname)" || exit 1
