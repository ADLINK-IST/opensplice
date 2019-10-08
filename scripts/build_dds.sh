#!/bin/bash
cd $(dirname $0)/..

export SQLITE_HOME="$(cygpath -u "${SQLITE_HOME_WIN}")"
export NODEJS_HOME="$(cygpath -u "${NODEJS_HOME_WIN}")"
source configure $1
make install
