#!/bin/bash

. "$(dirname "$0")/../ospldevutils.shsrc" "$@"
. "$(dirname "$0")/../ospldistutils.shsrc" "$@"
. "$(dirname "$0")/exsettings.shsrc" "$@" || exit 0

# Setup dev environment.
source_tao_runtime || exit 1
source_java_runtime || exit 1

# Setup dist environment.
source_ospldist_env || exit 1

check_ex_prereqs || exit 1

if [ "$(uname -o)" = 'Cygwin' ]; then
    cmd /C "devenv $OSPL_HOME/examples/All_Standalone_C_and_CPlusPlus.sln /Upgrade"
    cmd /C "devenv $OSPL_HOME/examples/All_Standalone_C_and_CPlusPlus.sln /Clean"
    cmd /C "devenv $OSPL_HOME/examples/All_Standalone_C_and_CPlusPlus.sln /Rebuild"

    cmd /C "devenv $OSPL_HOME/examples/CORBA_CPlusPlus.sln /Upgrade"
    cmd /C "devenv $OSPL_HOME/examples/CORBA_CPlusPlus.sln /Clean"
    cmd /C "devenv $OSPL_HOME/examples/CORBA_CPlusPlus.sln /Rebuild"

    cmd /C "devenv $OSPL_HOME/examples/CSharp.sln /Upgrade"
    cmd /C "devenv $OSPL_HOME/examples/CSharp.sln /Clean"
    cmd /C "devenv $OSPL_HOME/examples/CSharp.sln  /Rebuild"

    cmd /C "devenv $OSPL_HOME/examples/DCPS_ISO_CPlusPlus.sln /Upgrade"
    cmd /C "devenv $OSPL_HOME/examples/DCPS_ISO_CPlusPlus.sln /Clean"
    cmd /C "devenv $OSPL_HOME/examples/DCPS_ISO_CPlusPlus.sln /Rebuild"

    cd "$OSPL_HOME/examples/"
    cmd /C "call BUILD_Standalone_Java.bat"
    cmd /C "call BUILD_CORBA_Java.bat"
else
    for i in \
        'Makefile' \
        'Makefile.Standalone_Java' \
        'Makefile.CORBA_CPlusPlus' \
        'Makefile.CORBA_Java'
    do
        echo -e "\nBuilding $OSPL_HOME/examples/${i}..."
        cd "$OSPL_HOME/examples"
        make -f "$i" >> build.log 2>&1
        if [ $? -ne 0 ]
        then
            echo "Error calling make on ${i}"
        fi
    done

    if [ "$(uname)" != 'SunOS' ]; then
        cd "$OSPL_HOME/examples"
        make -f Makefile.DCPS_ISO_CPlusPlus >> build.log 2>&1
        if [ $? -ne 0 ]
        then
            echo "Error calling make on Makefile.DCPS_ISO_CPlusPlus"
        fi
    fi
fi
