#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: location of results"
fi

# Don't process common arguments.
. "$(dirname "$0")/tssettings.shsrc" || exit 0

savelogs $1 || exit 1
