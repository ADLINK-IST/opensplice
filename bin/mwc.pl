#!/usr/bin/env perl
eval '(exit $?0)' && eval 'exec perl -w -S $0 ${1+"$@"}'
    & eval 'exec perl -w -S $0 $argv:q'
    if 0;

# ******************************************************************
#         $Id:$
# ******************************************************************

# ******************************************************************
# Pragma Section
# ******************************************************************

require 5.006;

use strict;
use FindBin;
use File::Spec;
use File::Basename;
use Getopt::Long;


# Getopt::Long::Configure ("bundling_override");
Getopt::Long::Configure ('pass_through');

my $openfusion_tao_root = $ENV{TAO_ROOT_NORMALIZED};

if (defined $openfusion_tao_root)
{
    $ENV{TAO_ROOT} = $openfusion_tao_root;
    $ENV{ACE_ROOT} = $openfusion_tao_root;
}

my $src_checkout = 0;
my $java_only = 0;
my $left_over_args;
my $ret;
my $ospl_home_hack;
($ret, $left_over_args) = GetOptions('src-co' => \$src_checkout,
                                     'java-only' => \$java_only,
                                     'ospl-home=s' => \$ospl_home_hack);

my($basePath) = (defined $FindBin::RealBin ? $FindBin::RealBin :
                                             File::Spec->rel2abs(dirname($0)));
if ($^O eq 'VMS') {
  $basePath = File::Spec->rel2abs(dirname($0)) if ($basePath eq '');
  $basePath = VMS::Filespec::unixify($basePath);
}

$basePath .= '/../MPC';

my($mpcroot) = $ENV{MPC_ROOT};
my($mpcpath) = (defined $mpcroot ? $mpcroot :
                                   dirname(dirname($basePath)) . '/MPC');
unshift(@INC, $mpcpath . '/modules');
unshift(@INC, $basePath . '/modules');
$ENV{OSPLI_BASE_PATH} = $basePath;

my $ospl_home_norm = $ENV{OSPL_HOME_NORMALIZED};
my $os_header_dir = 'linux';
my $os_arch = 'x86';
my $os_compiler;
my $just_os;
my $just_os_ver;
my $just_compiler;

my $splice_target = $ENV{SPLICE_TARGET};
my $splice_host = $ENV{SPLICE_HOST};

if ($splice_host ne $splice_target)
{
    unshift(@ARGV, '--features', 'cross_compile=1');
}

# splice targets are generally of form <arch>.<os+ver>-<somenonsense>
if ($splice_target =~ /^([^\.]+)\.(.+)-.*/)
{
    $os_header_dir = $2;
    $os_arch = $1;
    unshift(@ARGV, '--features', 'ospl_arch_'. $os_arch . '=1');
    # Set the target as an MPC feature so we can hang last ditch config on it
    my $target = $os_arch . '_' . $os_header_dir;
    # The now remove any other .s
    $target =~ s/\./_/g;
    unshift(@ARGV, '--features', $target . '=1');
}

# ... except some of them where <os+ver> is actually <os+ver>_<compiler>
if ($os_header_dir =~ /^([^_]+)_(.+)/)
{
    $os_header_dir = $1;
    $os_compiler = $2;
}

my $os_and_version = $os_header_dir;
# Remove any other .s
$os_and_version =~ s/\./_/g;

# ... we separate <os+ver>, except sometimes there isn't a ver
if ($os_header_dir =~  /^([[:alpha:]]+)([[:digit:]\.]+)$/) {
    $just_os = $1;
    $just_os_ver = $2;
    $just_os_ver =~ s/\./_/g;
    unshift(@ARGV, '--features', 'ospl_os_ver_' . $just_os_ver . '=1');
}
else {
    $just_os = $os_header_dir;
}

# ... & sometimes the compiler has a version, sometimes not...
if (defined $os_compiler) {
    if ($os_compiler =~  /^([[:alpha:]]+)([[:digit:]\.]+)$/) {
        $just_compiler = $1;
    }
    else {
        $just_compiler = $os_compiler;
    }
}

if ($java_only == 1)
{
    $just_compiler = "java";
    if ($just_os =~ /^win/ )
    {
    }
    else
    {
        $just_os = "jvm";
    }
}

unshift(@ARGV, '--include', $ENV{TAO_ROOT} . '/bin/MakeProjectCreator/config/TAO');
unshift(@ARGV, '--include', $ENV{TAO_ROOT} . '/bin/MakeProjectCreator/config');

if ($src_checkout == 1)
{
    # Dealing with our wandering OS abstraction header directory loactions
    my $headers = $ENV{OSPL_TARGET_HEADERS};

    # Features can't have .'s in
    $headers =~ s/\./_/g;

    unshift(@ARGV, '--features', 'src_co=1');
    unshift(@ARGV, '--features', "os_headers_$headers=1");
}
else
{
    if (defined $ospl_home_hack)
    {
        $ENV{OSPL_HOME} = $ospl_home_hack;
    }
    delete $ENV{TAO_ROOT};
    delete $ENV{ACE_ROOT};
    delete $ENV{ODBCHOME};
    print "OSPL_HOME is $ENV{OSPL_HOME}\n";
}

my $build_bit_64 = 0;

if ($os_arch eq 'x86_64')
{
    $build_bit_64 = 1;
}

unshift(@ARGV, '--value_template', 'lib_modifier=');
unshift(@ARGV, '--features', 'ospl_64_bit=' . $build_bit_64);

if (defined $just_os) {
    unshift(@ARGV, '--features', "ospl_os_$just_os=1");
    if ($just_os =~ /^win/ ) {
        if ($java_only == 1) {
        } else {
            unshift(@ARGV, '--features', 'ospl_dcpssacs=1');
        }
    } else {
        if (defined $just_compiler) {
            unshift(@ARGV, '--features', "ospl_compiler_$just_compiler=1");
        }
    }
}

# I would expect, and deserve, to go to hell for the below repetition
# but it's planned to be refactored and replaced in the next cycle
# @see http://jira.prismtech.com:8080/browse/OSPL-2024

# splice hosts are generally of form <arch>.<os+ver>-<somenonsense>
my $host_header_dir;
if ($splice_host =~ /^([^\.]+)\.(.+)-.*/)
{
    $host_header_dir = $2;
}

# ... except some of them where <os+ver> is actually <os+ver>_<compiler>
if ($host_header_dir =~ /^([^_]+)_(.+)/)
{
    $host_header_dir = $1;
}

my $just_host;
# ... we separate <os+ver>, except sometimes there isn't a ver
if ($host_header_dir =~  /^([[:alpha:]]+)([[:digit:]\.]+)$/) {
    $just_host = $1;
}
else {
    $just_host = $host_header_dir;
}

if (defined $just_host) {
    unshift(@ARGV, '--features', "ospl_os_host_$just_host=1");
}

if (defined $mpcroot) {
  print "MPC_ROOT was set to $mpcroot.\n";
}

if (! -d "$mpcpath/modules") {
  print STDERR "ERROR: Unable to find the MPC modules in $mpcpath.\n";
  if (defined $mpcroot) {
    print STDERR "Your MPC_ROOT environment variable does not point to a ",
                 "valid MPC location.\n";
  }
  else {
    print STDERR "You can set the MPC_ROOT environment variable to the ",
                 "location of MPC.\n";
  }
  exit(255);
}

require Driver;

sub getBasePath {
  return $mpcpath;
}

my($driver) = new Driver($basePath, Driver::workspaces());
exit($driver->run(@ARGV));
