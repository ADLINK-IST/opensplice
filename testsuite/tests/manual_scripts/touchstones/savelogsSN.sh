#!/bin/bash

if [ $# -ne 0 ]; then
    echo "Usage: '$0'"
fi

# Don't process common arguments.
. "$(dirname "$0")/tssettings.shsrc" || exit 0

savelogs "$1" || exit 1
