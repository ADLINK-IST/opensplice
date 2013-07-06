#!/bin/bash

. "$(dirname "$0")/../ospldevutils.shsrc" "$@"
. "$(dirname "$0")/../ospldistutils.shsrc" "$@"
. "$(dirname "$0")/tssettings.shsrc" "$@" || exit 0

# Setup dev environment.
source_java_runtime || exit 1

# Setup dist environment.
source_ospldist_env || exit 1

if [ "$(hostname)" = 'v240a' ]; then
   source_studio12_runtime || exit 1
fi

# Add dcpssaj.jar to classpath as this is
# no longer done by the release script
source_ospl_classpath || exit 1

check_ts_prereqs || exit 1

g_scriptdir="$PWD/$(dirname "$0")"

# Remove any previously installed touchstone.
g_unix_ts_root="$(to_unix_path "$TS_ROOT")"
    
echo -e "\nchecking if dds-touchstone directory exists... at $g_unix_ts_root"
if [ -d "$g_unix_ts_root" ]; then
    echo -e "\nremoving dds-touchstone directory ..."
    rm -rf  "$g_unix_ts_root"
fi

#Checkout touchstone from PrismTech git repository as it is up to date
cd "$SRC_ROOT"
git clone ssh://git@repository2.prismtech.com/dds-touchstone.git || exit 1    

if [ "$(echo "$LOCAL_OSPL_DISTVER" | sed -e 's|\..*||')" = 'V5' ]; then
   cd "$TS_ROOT"
   git checkout -b PreOpenSpliceV6.1-branch origin/PreOpenSpliceV6.1-branch
fi

cd "$g_unix_ts_root" || exit 1

# On Solaris we need to use gpatch instead of patch
if [ "$(uname)" != 'SunOS' ]; then
   patch=patch
else
   patch=gpatch
fi

if [ "$(uname -o)" = 'Cygwin' ]; then
    # Build on windows is done differently.
    echo -e "\nPatching for Windows build..."
    patch -p0 <"$g_scriptdir/diffs/makefiles+MakeWin.tools.bat.diff" || exit 1

    echo -e "\nCleaning previous TS build..."
    rm -rf "$g_unix_ts_root/bin/windows" "$g_unix_ts_root/objs" &>/dev/null

    echo -e "\nDoing TS build..."
    cmd /C "call MakeWin.bat" || exit 1
else
    echo -e "\nCleaning previous TS build..."
    make clean &>/dev/null

    if [ -d "$g_unix_ts_root/makefiles/generated" ]; then
        echo -e "\nDo you want to reconfigure previous build (y/N)?"
        read yesno
        test "$yesno" = 'y' -o "$yesno" = 'Y' && . configure.sh
        unset yesno
    else
        echo -e "\nDoing TS configure..."
        . configure.sh
    fi

    echo -e "\nDoing TS build..."
    make || exit 1
fi

