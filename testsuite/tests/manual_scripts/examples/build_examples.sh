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
    cmd /C "devenv $OSPL_HOME/examples/StandaloneCandCPlusPlus.sln /Upgrade"
    cmd /C "devenv $OSPL_HOME/examples/StandaloneCandCPlusPlus.sln /Clean"
    cmd /C "devenv $OSPL_HOME/examples/StandaloneCandCPlusPlus.sln /Rebuild"

    cmd /C "devenv $OSPL_HOME/examples/CORBACPlusPlus.sln /Upgrade"
    cmd /C "devenv $OSPL_HOME/examples/CORBACPlusPlus.sln /Clean"
    cmd /C "devenv $OSPL_HOME/examples/CORBACPlusPlus.sln /Rebuild"

    cmd /C "devenv $OSPL_HOME/examples/CSharp.sln /Upgrade"
    cmd /C "devenv $OSPL_HOME/examples/CSharp.sln /Clean"
    cmd /C "devenv $OSPL_HOME/examples/CSharp.sln  /Rebuild"

    cd "$OSPL_HOME/examples/"
    cmd /C "call Build_StandaloneJava.bat"
    cmd /C "call Build_CORBAJava.bat"

    # The dlrl java Tutorial is not included in the above so
    # we need to build it separately.
    cd "$OSPL_HOME/examples/dlrl/standalone/java/Tutorial"
    cmd /C "call BUILD.bat"
else
    for i in \
        'Makefile' \
        'Makefile.Standalone_Java' \
        'Makefile.CORBA_CPlusPlus' \
        'Makefile.CORBA_Java' \
        'Makefile.DCPS_ISO_CPlusPlus'
    do
        echo -e "\nBuilding $OSPL_HOME/examples/${i}..."
        cd "$OSPL_HOME/examples"
        make -f "$i" >> build.log 2>&1
        if [ $? -ne 0 ]
        then
            echo "Error calling make on ${i}"
        fi
    done

    cd "$OSPL_HOME/examples/dlrl/standalone/java/Tutorial"
    ./BUILD >> build.log 2>&1
    if [ $? -ne 0 ]
    then
        echo "Error building DLRL Java Tutorial"
    fi        
fi
