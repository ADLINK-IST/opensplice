#!/bin/sh
export OSPL_HOME=`dirname $0`
export OSPL_TARGET=
export OSPL_URI="file://./ishapes-eu.xml"
echo -n "Enter your email address (the same used to log on the vortex demo): "; read PARTITION
./ishapes $PARTITION
