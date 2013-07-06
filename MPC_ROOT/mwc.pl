#! /usr/bin/perl
eval '(exit $?0)' && eval 'exec perl -w -S $0 ${1+"$@"}'
    & eval 'exec perl -w -S $0 $argv:q'
    if 0;

# ******************************************************************
#      Author: Chad Elliott
#        Date: 6/17/2002
#         $Id$
# ******************************************************************

# ******************************************************************
# Pragma Section
# ******************************************************************

require 5.006;

use strict;
use FindBin;
use File::Spec;
use File::Basename;

## Sometimes $FindBin::RealBin will end up undefined.  If it is, we need
## to use the directory of the built-in script name.  And, for VMS, we
## have to convert that into a UNIX path so that Perl can use it
## internally.
my $basePath = (defined $FindBin::RealBin && $FindBin::RealBin ne '' ?
                  $FindBin::RealBin : File::Spec->rel2abs(dirname($0)));
$basePath = VMS::Filespec::unixify($basePath) if ($^O eq 'VMS');

## Add the full path to the MPC modules to the Perl include path
my $mpcpath = $basePath;
unshift(@INC, $mpcpath . '/modules');

# Has to be a require because it's in the modules directory.
require mpc_debug;

## If the ACE_ROOT environment variable is defined and this version of
## MPC is located inside the directory to which ACE_ROOT points, we will
## assume that the user wanted the ACE specific version of this script.
## We will change the $basePath to what it would have been had the user
## run this script out of $ACE_ROOT/bin.
my $aceroot = $ENV{ACE_ROOT};
$aceroot =~ s!\\!/!g if (defined $aceroot);
$basePath = $aceroot . '/bin/MakeProjectCreator'
               if (defined $aceroot && $aceroot eq dirname($basePath));

require Driver;

# ************************************************************
# Subroutine Section
# ************************************************************

sub getBasePath {
  return $mpcpath;
}

# ************************************************************
# Main Section
# ************************************************************

my $driver = new Driver($basePath, Driver::workspaces());
exit($driver->run(@ARGV));
