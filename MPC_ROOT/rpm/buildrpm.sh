#!/bin/sh

# ******************************************************************
#      Author: Chad Elliott
#        Date: 8/13/2009
# Description: Create an MPC rpm based on the current version number.
#         $Id$
# ******************************************************************

## First find out where this script is located
if [ "$0" != "`basename $0`" ]; then
  if [ "`echo $0 | cut -c1`" = "/" ]; then
    loc="`dirname $0`"
  else
    loc="`pwd`/`dirname $0`"
  fi
else
  ## Do my own 'which' here
  loc="."
  for i in `echo $PATH | tr ':' '\012'`; do
    if [ -x "$i/$0" -a ! -d "$i/$0" ]; then
      loc="$i"
      break
    fi
  done
fi

## Now, get back to where the main MPC script is located
while [ ! -x $loc/mpc.pl ]; do
  loc=`dirname $loc`
done

## Build up the packager name and email address
if [ -z "$REPLYTO" ]; then
  DOMAIN=`hostname | sed 's/[^\.][^\.]*\.//'`
  FULLDOMAIN=`echo $DOMAIN | grep '\.'`
  if [ -z "$DOMAIN" -o -z "$FULLDOMAIN" ]; then
    RESOLVDOMAIN=`grep '^search' /etc/resolv.conf | sed 's/.*\s//'`
    FULLDOMAIN=`echo $RESOLVDOMAIN | grep '\.'`
    if [ -z "$DOMAIN" -o -n "$FULLDOMAIN" ]; then
      DOMAIN=$RESOLVDOMAIN
    fi
  fi
  REPLYTO="$LOGNAME@$DOMAIN"
fi
PACKAGER=`getent passwd $LOGNAME | cut -d: -f5`
if [ -z "$PACKAGER" ]; then
  PACKAGER=$CL_USERNAME
fi
if [ -z "$PACKAGER" ]; then
  PACKAGER="<$REPLYTO>"
else
  PACKAGER="$PACKAGER <$REPLYTO>"
fi

## Save the MPC version
VERSION=`$loc/mpc.pl --version | sed 's/.*v//'`

## This is where we'll create the spec file and do the work
WDIR=/tmp/mpc.$$

## This is the directory name that RPM expects
MDIR=MPC-$VERSION

## This corresponds to BuildRoot in MPC.spec
BDIR=/tmp/mpc

## This is the final install directory and corresponds to the %files section
## of MPC.spec
FDIR=/usr/lib/MPC

##Check if build and work diretory already exist
if [ -d "$BDIR" -o -f "$BDIR" ]; then
  echo "Necessary directory $BDIR aleady exists."
  echo "Exiting."
  exit
fi

if [ -d "$WDIR" -o -f "$WDIR" ]; then
  echo "Necessary directory $WDIR aleady exists."
  echo "Exiting."
  exit
fi

## Create our working directory
mkdir -p $WDIR

## The directory where RPM will place the resulting file
if [ -x /usr/src/redhat -a -w /usr/src/redhat ]; then
  RPMLOC=/usr/src/redhat
elif [ -x /usr/src/packages -a -w /usr/src/packages ]; then
  RPMLOC=/usr/src/packages
else
  RPMLOC=$WDIR/rpmbuild
  mkdir -p $RPMLOC
  mkdir -p $RPMLOC/BUILD
  mkdir -p $RPMLOC/RPMS
  mkdir -p $RPMLOC/SOURCES
fi

## Make the spec file
cd $WDIR
sed "s/VERSION/$VERSION/; s/PACKAGER/$PACKAGER/; s!FINALDIR!$FDIR!" $loc/rpm/MPC.templ > MPC.spec

## Make a copy of the original MPC source to the new directory
mkdir -p $MDIR/$FDIR
cd $loc
tar --exclude=.svn -cf - . | (cd $WDIR/$MDIR/$FDIR; tar -xf -)

## Create the build source tar.bz2
cd $WDIR
tar --owner root --group root -cf $RPMLOC/SOURCES/$MDIR.tar $MDIR
bzip2 -9f $RPMLOC/SOURCES/$MDIR.tar

## Perform the RPM creation step
rm -rf $BDIR
mkdir -p $BDIR
rpmbuild --define "_topdir $RPMLOC" --define "_buildrootdir $BDIR" --define "buildroot $BDIR" --define "__arch_install_post %{nil}" -bb MPC.spec

if [ "$RPMLOC" = "$WDIR/rpmbuild" ]; then
  echo "Copying rpm to $loc/rpm"
  cp $RPMLOC/RPMS/*/*.rpm $loc/rpm
fi

## Clean everything up
cd ..
rm -rf $WDIR $BDIR
