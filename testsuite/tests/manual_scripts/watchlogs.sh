#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: '$0' <log-director>"
    exit 0
fi

place="$1"
if [ ! -d "$place" ]; then
    echo "ERROR: Directory '$place' doesn't exist"
    exit 1
fi

function catfiles ()
{
    local curdir=''
    while read fl
    do
        local newdir="$(dirname "$fl")"

        if [ "$curdir" != "$newdir" ]; then
            curdir="$newdir"
            echo -e "\n\n\n##### Looking in a new dir '$newdir'..."
        fi

        echo "***** Logging file '$fl'..."
        cat "$fl"
        echo -e "\n"
    done
}

echo "Log files from '$place'"
find "$place" -type f | catfiles
