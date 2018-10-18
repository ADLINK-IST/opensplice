#!/bin/bash

if [ $# -lt 3 ] ; then
    echo "usage: $0 SCRIPT.PL SRC... DST" >&2
    exit 1
fi
script=$1 ; shift
srcs=""
while [ $# -gt 1 ] ; do
    srcs="$srcs $1" ; shift
done
dst=$1

trap "rm -rf $dst.tmp" EXIT

rm -rf $dst.tmp
perl -w $script $srcs $dst.tmp || { r=$? ; echo "error: $script" >&2 ; exit $r ; }

if [ ! -d $dst ] ; then
    mkdir $dst || { r=$? ; echo "error: mkdir" >&2 ; exit $r ; }
fi

for x in $dst.tmp/* ; do
    echo "" >> $x
    if [ "$OSPL_OUTER_REV" != "" ]; then
        echo "/* SHA1 used $OSPL_OUTER_REV */" >> $x
    else
        echo "/* SHA1 not available (unoffical build.) */" >> $x
    fi

    bx=`basename $x`
    if [ ! -f $dst/$bx ] ; then
        cp=true
    else
        diff $x $dst/$bx >/dev/null 2>/dev/null
        case $? in
            0) # no change
                cp=false
                ;;
            1)
                cp=true
                ;;
            *)
                r=$?
                echo "error: diff $x failed" >&2
                exit $r
                ;;
        esac
    fi

    if $cp ; then
        #echo "changed: $bx"
        cp -p $x $dst/$bx || { r=$? ; echo "error: cp" >&2 ; exit $r ; }
    fi
done
