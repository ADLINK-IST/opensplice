#!/bin/bash

function print_install_help ()
{
    echo "Usage: '$0' source destination license_location"
    echo "    source - can be either a file with OSPL distribution"
    echo "             or a directory with existing OSPL installation"
    echo "    destination - a directory where OSPL has to be installed"
    echo "    license_location - where to obtain a license.lic file from"
}

if [ $# -ne 3 -o -z "$1" -o -z "$2" -o -z "$3" ]; then
    print_install_help
    exit 0
fi

# We don't show in this script any other help except for the one above.
. "$(dirname "$0")/osplconfig.shsrc" "$@"
. "$(dirname "$0")/ospldistutils.shsrc" "$@"

src="$(to_unix_path "$1")"
dst="$(to_unix_path "$2")"
lic_dir="$(to_unix_path "$3")"

# Use dst without trailing slashes.
dst="$(echo "$dst" | sed -e 's|/*$||')"

instmode='unattended'
if [ "$(uname)" != 'SunOS' ];then
    if [ "$(uname -o)" = 'Cygwin' ]; then
        instmode='win32'
    fi
fi

if [ -e "$dst" ]; then
    echo -e "\nDestination '$dst' already exists. Do you want to remove it (r|R), rename it (m|M), or uninstall it (u|U)?"

    read reply
    case "$reply" in
        r|R)
            echo -e "\nRemoving..."
            rm -rf "$dst" || exit 1
            ;;
        m|M)
            newname="${dst}_$(stat -c '%y' "$dst" | sed -e 's/ .*$//')"

            i=1
            attempt="$newname"
            until [ ! -e "$attempt" ]
            do
                attempt="${newname}_$i"
                let "++i"
            done
            newname="$attempt"

            echo -e "\nRenaming '$dst' to '$newname'..."
            mv -f "$dst" "$newname" || exit 1
            unset i attempt newname
            ;;
        u|U)
            echo -e "\nUnistalling..."

            uninstall="$(find "$dst" -maxdepth 3 -type f -regex '.*uninstall.*')"
            test -n "$uninstall" || error_help "ERROR: Uninstaller is not found" || exit 1

            chmod +x "$uninstall" &>/dev/null
            eval "\"$uninstall\" --mode $instmode" || exit 1
            # This waiting loop is necessary on Windows.
            while [ -e "$uninstall" ]; do sleep 1; done
            unset uninstall
            # And sleep some more...
            sleep 5

            echo -e "\nCleaning OSPL xml files..."
            clean_ospl_configs "$dst" || exit 1
            ;;
        *)
            print_install_help
            exit 1
            ;;
    esac
    unset reply
else
    # The directory doesn't exist. Attempt to create it.
    mkdir -p "$dst" || exit 1
    rm -rf "$dst" &>/dev/null
fi

if [ -f "$src" ]; then
    # Install from a distribution.
    echo -e "\nInstalling from a distribution ..."
    chmod +x "$src" &>/dev/null

    native_dst="$(to_native_path "$dst")"
    native_lic="$(to_native_path "$lic_dir/license.lic")"

    echo -e "License location is '$native_lic' "

    eval "\"$src\" --mode $instmode --prefix \"$native_dst\" --w_already_has_license 1 --nw_already_has_license 1 --providedLicenseFile \"$native_lic\""

    unset native_dst
    newdistr='yes'

    echo -e "\nCopying license file..."
    if [ "$(uname)" != 'SunOS' ]; then
       find "$dst" -type d -regex '.*/etc$' -exec "$(dirname "$0")/.cp_chmod" "$native_lic" '{}/' \;
    else
       licdirs=$(find "$dst" -type d -name 'etc') 
       for licdir in $licdirs
       do
           "$(dirname "$0")/.cp_chmod" "$native_lic" "$licdir" \; || exit 1
       done
    fi
else
    # Rename a directory with previously installed distribution to a dst.
    echo -e "\nRenaming '$src' to '$dst'..."
    mv -f "$src" "$dst" || exit 1

    echo -e "\nCopying license file..."
    if [ "$(uname)" != 'SunOS' ]; then
        find "$dst" -type d -regex '.*/etc$' -exec "$(dirname "$0")/.cp_chmod" "$native_lic" '{}/' \; || exit 1
    else
       licdir=$(find "$dst" -type d -name 'etc') 
       "$(dirname "$0")/.cp_chmod" "$native_lic" "$licdir" \; || exit 1
    fi
fi

if [ -n "$newdistr" ]; then
    unset newdistr
    echo -e "\nBacking up original OSPL config files..."
    backup_ospl_configs "$dst" || exit 1
fi

# This will update distribution related vars to newly installed OSPL.
update_distvar_env "$dst" || exit 1
