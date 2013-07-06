#!/bin/bash

if [ $# -ne 0 ]; then
    echo "Usage: '$0'"
fi

# Don't process common arguments.
. "$(dirname "$0")/exsettings.shsrc" || exit 0

savelogs "$(plain_hostname)" || exit 1
