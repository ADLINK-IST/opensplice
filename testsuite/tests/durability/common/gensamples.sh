#!/bin/bash
#
# $1 : Sample count
# $2 : Start value 

[[ $1 && $2 ]] || {
    echo "Usage: $0 [count] [start-value]"
    exit 1
}

START="$2"
END="$(( $2 + $1 ))"

for (( i=$START; i<$END; i++ )); do
    echo "$i,$(( $i + 2 )),$(( $i * 2))" 
done;
