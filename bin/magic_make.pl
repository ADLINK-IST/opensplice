#!/usr/bin/env perl
eval '(exit $?0)' && eval 'exec perl -w -S $0 ${1+"$@"}'
    & eval 'exec perl -w -S $0 $argv:q'
    if 0;

require 5.006;

use strict;
use FindBin;
use File::Spec;
use File::Find;
use File::Basename;
use Getopt::Long;
use File::stat;
use Pod::Usage;

my $scriptname = basename($0);

# Getopt::Long::Configure ("bundling_override");
Getopt::Long::Configure ('pass_through');

my $man = 0;
my $help = 0;
my $make = '';
my $check_mpc = '';
my $clean = '';
my $debug_override = '';
my $carryon = '';
my $exhaustive = 0;
my $squeaky = 0;
my $left_over_args;
my $ret;
my $type = "make";
my $ospl_home = '';
my $is_gcov = 0;
($ret, $left_over_args) = GetOptions('clean!' => \$clean,
                                     'check-mpc!' => \$check_mpc,
                                     'carryon!' => \$carryon,
                                     'debug!' => \$debug_override,
                                     'exhaustive' => \$exhaustive,
                                     'make!' => \$make,
                                     'ospl-home=s' => \$ospl_home,
                                     'squeaky' => \$squeaky,
                                     'type=s' => \$type,
                                     'help|?' => \$help,
                                     'man' => \$man) or pod2usage(2);

my $config = 'Release';

my $splice_target = $ENV{SPLICE_TARGET};
if ($debug_override == '')
{
  $config = 'Release';
  $config = 'Debug' if ($splice_target =~ 'dev' || $splice_target =~ 'debug');
}
else
{
  $config = ($debug_override ? 'Debug' : 'Release');
}

$is_gcov = 1 if $splice_target =~ 'gcov';

pod2usage(1) if $help;
pod2usage(-exitstatus => 0, -verbose => 2) if $man;

$check_mpc = 1 if ($make && $check_mpc eq '');
#$clean = 1 if $squeaky;
$make = 1 if ($make eq '' && !$clean);

if ($clean && !$make)
{
    $carryon = 1 if ($carryon eq '');
}

my($basePath) = (defined $FindBin::RealBin ? $FindBin::RealBin :
                                             File::Spec->rel2abs(dirname($0)));
if ($^O eq 'VMS') {
  $basePath = File::Spec->rel2abs(dirname($0)) if ($basePath eq '');
  $basePath = VMS::Filespec::unixify($basePath);
}

# See check_file_date - variables used to work out if
# we need to regenerate build files using MPC
my $oldest_build_file;
my $newest_mpc_file;

# Used on windows only.
my $wincmd_shell = $ENV{COMSPEC};
my $visual_studio;
if ($type =~ /^vc/)
{
  my @ext = split /;/, $ENV{PATHEXT} || '.exe;.com;.bat';
  # I *would* put MSBuild first but we're pathing the wrong one in
  # in configure
  my @exes = ( 'devenv', 'VCExpress', 'MSBuild' );
  studio:
  {
    foreach my $exe (@exes)
    {
      foreach my $p (File::Spec->path)
      {
        if (grep { -f and -x } map File::Spec->catfile($p, "$exe$_"), '', @ext)
        {
          $visual_studio = $exe;
          last studio;
        }
      }
    }
  }
  if (! defined $wincmd_shell)
  {
    $wincmd_shell = 'cmd.exe';
  }
  if (! defined $visual_studio)
  {
    $visual_studio = $ENV{OSPL_DEVENV};
  }
  if (! defined $visual_studio)
  {
    $visual_studio = 'devenv.com';
  }
}

# Match a top-level build file on $_. e.g. a solution or
# a Makefile. These are what you would normally invoke to build
# some shizzle.
sub is_workspace_file
{
  my $match = 0;

  if ($type =~ /^vc/)
  {
    $match = (/^.*\.dsw\z/s ||
              /^.*\.sln\z/s);
  }
  else # ($type =~ /^make/)
  {
    $match = ( /^Makefile\z/s);
  }
  return $match;
}

# Match a 'lesser' build file on $_ e.g. a *.vcproj
# or Makefile.foo_lib. These would normally only be called from
# an encompassing 'top-level' build file as they specify
# dependency order.
sub is_project_file
{
  my $match = 0;
  if ($type =~ /^vc/)
  {
    $match = (/^.*\.dsp\z/s ||
              /^.*\.vcproj\z/s ||
              /^.*\.vcxproj\z/s);
  }
  else # ($type =~ /^make/)
  {
    $match = (/^Makefile\..*\z/s);

  }
  return $match;
}


# Match an even 'lesser' build file on $_ e.g. a *.vcproj.filter or *.suo
# or .depend file or anything tenuously related. Used with --exhaustive &
# --squeaky
sub is_anything
{
  my $match = 0;

  if ($type =~ /^vc/)
  {
    $match = (/^.*\.ncb\z/s ||
              /^.*\.ilk\z/s ||
              /^.*\.sdf\z/s ||
              /^.*\.suo\z/s ||
              /^.*\.vcproj\..*\z/s ||
              /^.*\.vcxproj\..*\z/s);
  }
  else # ($type =~ /^make/)
  {
    $match = (/^.depend\..*\z/s);
  }
  return $match;
}


# Record the modification time of $_ if
# it is the oldest generated file or the most recently
# modified mpc meta build file.
sub check_file_date
{
  #print "checking $File::Find::name\n";
  # my $file = $_;
  my $match = 0;

  $match = (is_workspace_file ||
                is_project_file);

  if ($match)
  {
    my $sb = stat($_);
    my $file_date = $sb->mtime;
    if ($oldest_build_file == 0 ||
        $file_date < $oldest_build_file)
    {
      print "$scriptname: Makefile date : $file_date $_\n";
      $oldest_build_file = $file_date;
    }
  }

  $match = 0;

  $match = (/^.*\.mpc\z/s ||
            /^.*\.mwc\z/s ||
            /^.*\.mpb\z/s ||
            /^.*\.mwb\z/s);

  if ($match)
  {
    my $sb = stat($_);
    my $file_date = $sb->mtime;
    if ($file_date > $newest_mpc_file)
    {
      print "$scriptname: MPC File date : $file_date $_\n";
      $newest_mpc_file = $file_date;
    }
  }
}

# Recursively check the passed directory to see if its
# generated build files are in need of regeneration
sub check_mpc_up_todate
{
  my $dir = shift(@_);
  $oldest_build_file = 0;
  $newest_mpc_file = 0;
  # print "check mpc $dir\n";

  find(\&check_file_date, "$dir");

  my $rebuild_required = $newest_mpc_file > $oldest_build_file;
  print "$scriptname: $newest_mpc_file > $oldest_build_file Detected MPC rebuild required...\n" if $rebuild_required;
  return $rebuild_required;
}

sub call_build_file
{
  my $mode = shift(@_);
  my $file = shift(@_);
  my $command;
  print "$scriptname: Proceeding to $mode $File::Find::name...\n";
  my $ret = 0;
  if ($type =~ /^vc/)
  {
    my $is_msbuild = (lc($visual_studio) =~ m/msbuild/);
    my $modeflag = $is_msbuild ? '/t:Build' : '/build';
    if (lc $mode eq 'clean')
    {
        $modeflag = $is_msbuild ? '/t:Clean' : '/clean';
    }
    my $conf_pre = $is_msbuild ? '/p:Configuration=' : '';
    $command = "\"$wincmd_shell\" /c $visual_studio $file $modeflag $conf_pre$config";
    if ($^O eq 'cygwin')
    {
        $command =~ s/\\/\//g;
    }
  }
  else # ($type =~ /^make/)
  {
    my $clean = '';
    if (lc $mode eq 'clean')
    {
        $clean = 'realclean';
    }
    $command = "make $clean -f $file CFG=$config";
  }
  print "$scriptname: calling - $command\n";
  $ret = system($command);
  if ($ret)
  {
    if (lc $mode ne 'clean' &&  !$carryon)
    {
        die "$scriptname: ERROR: return of $ret trying to $mode $File::Find::name !!! command used: $command\n";
    }
    else
    {
        print STDERR "$scriptname: ERROR/non zero return trying to $mode $File::Find::name. Keeping calm and carrying on.\n";
    }
  }
  else
  {
    print "$scriptname: $mode $File::Find::name succeeded.\n";
  }

}

sub if_build_file_clean
{
  if (is_workspace_file ||
      ($exhaustive && is_project_file))
  {
    call_build_file('clean', $_);
  }
}

sub clean_dir
{
  my $dir = shift(@_);
  find(\&if_build_file_clean, "$dir");
}

sub if_build_file_make
{
  if (-d && $_ ne ".")
  {
    $File::Find::prune = 1;
  }
  if (is_workspace_file ||
      ($exhaustive && is_project_file))
  {
    call_build_file('make', $_);
  }
}

sub make_dir
{
  my $dir = shift(@_);

  find(\&if_build_file_make, "$dir");
}

sub if_build_file_delete
{
  if (is_workspace_file ||
      is_project_file ||
      ($exhaustive && is_anything))
  {
    print "$scriptname: deleting $File::Find::name\n";
    unlink $_;
  }
}

sub squeaky_dir
{
  my $dir = shift(@_);
  find(\&if_build_file_delete, "$dir");
}

# Run mwc.pl on either a file or a directory
# The difference don't mean nothing on a big ship
sub do_mpc
{
  my $file = shift(@_);
  my @mpc_args = @_;
  my $ret = 0;

  unshift(@mpc_args, '--type', "$type");
  if ($type =~ /^vc/)
  {
    unshift(@mpc_args, '--value_template', "configurations=$config");
  }
  # @todo See OSPL-2875 Covergae compilation disabled.
  # Uncomment to re-enable
  #if ($is_gcov)
  #{
  #  unshift(@mpc_args, '--value_template', 'coverage=1');
  #}
  if ($ospl_home ne '')
  {
    unshift(@mpc_args, '--ospl-home', "$ospl_home");
  }
  my $command = "mwc.pl @mpc_args";
  print STDERR "$scriptname: Generating MPC build file(s): $command\n";
  $ret = system($command);
  die "$scriptname: ERROR: Trying to run: $command !!!\n" if $ret;
}

my @ARGS_LEFT;
my $done_something;

foreach my $file (@ARGV) {
  # print "File is $file\n";
  if (-d $file)
  {
    $done_something = 1;
    print "$scriptname: Processing directory $file...\n";
    my $is_clean = 0;
    if ($clean)
    {
      clean_dir($file);
      $is_clean = 1;
    }
    if ($squeaky)
    {
      squeaky_dir($file);
    }
    if ($check_mpc && check_mpc_up_todate($file))
    {
        clean_dir($file) if ! $is_clean;
        do_mpc($file, @ARGV);
    }
    if ($make)
    {
      # @ARGS_LEFT = @ARGV;
      make_dir($file, @ARGV);
    }
  }
  elsif (-f $file)
  {
    $done_something = 1;
    if ($check_mpc && !$make && !$clean)
    {
      do_mpc($file, @ARGV);
    }
    else
    {
      print "$scriptname: Processing presumed buildfile $file...\n";
      if ($clean)
      {
        call_build_file('clean', $file);
      }
      if ($make)
      {
        call_build_file('build', $file);
      }
    }
  }
  else
  {

    #print "Unrecognised file / dir: $file\n";
    #podusage(2);
  }
}

pod2usage(2) if !$done_something;

exit (0);

__END__

=head1 NAME

magic_make.pl - Makes, cleans, or remakes your shizzle, whatever the weather.

=head1 SYNOPSIS

[perl] magic_make.pl [options] files/dirs

 Options:
  --check-mpc / --nocheck-mpc When making a directory check build files up to date first, clean & regenerate if not.
  --clean                     Clean the project file or directory
  --carryon / --nocarryon     If a build error is encountered, stop dead or keep going.
  --debug / --nodebug         Override the default for this build, whatever that is (--nodebug == Release)
  --exhaustive                Build or clean every damn thing in sight.
  --make                      Build the project file or directory.
  --squeaky                   Try and clean up all generated files for the build type. Exercise extreme caution.
  --type                      The type of build file e.g. vc8, vc9, make, javamake, javabat etc.. (see mwc.pl --help)
  --help                      Brief help message
  --man                       Full documentation

=head1 OPTIONS

=over 8

=item B<--check-mpc / --nocheck-mpc>

When making a directory check build files up-to-date first, clean & regenerate if not. Default is to B<--check-mpc> if action is B<--make>. Only works on directories.

=item B<--clean>

Clean the project file or directory. This action will occur automatically on directories if B<--check-mpc> is set, B<--make> is the action and the buildfiles are out of date. Can be combined with B<--make> to clean then make.

=item B<--carryon / --nocarryon>

If a build error is encountered making a particular file, stop dead (B<--nocarryon>) or ignore any errors & keep going (B<--carryon>) onto further buildfiles. Default B<--nocarryon> when making.

=item B<--exhaustive>

If the action is B<--make> (or B<--clean>) and we are processing a directory then find B<all> possible buildfiles thereunder & build (or clean) them. Build equivalent of take off and nuke the site from orbit. B<Warning>: may cause false positive errors, but sometimes it's the only way to be sure. Default: B<off>.

=item B<--make>

Build the project file or directory. This is the default action if none is specified. If a file: just gets on with it. If a directory will respect B<--check-mpc>

=item B<--squeaky>

Try and clean up all generated files for the build type. Exercise extreme caution. It is generally very very unwise to use this without --clean. Be careful of pointing this anywhere there might be hand maintained files.

=item B<--type>

Set the type of project to be built. Defaults to B<--type make> if not set. See B<mwc.pl --help>.

=item B<--man>

The full docs. If you can't see the 'DESCRIPTION' below this right now then this is for you...!

=back

=head1 DESCRIPTION

This tool lets you build files generated by MPC or directories containing MPC generated buildfiles generically. It
can detect updates to MPC meta-buildfiles and regenerate buildfiles if required on the hoof, although  note it can
only do this for one 'type' at a time thiough. This means If you are using this mode you shouldn't point it at root
that has subtrees of a different type under it.

E.g.:

mpc.make.pl --make --type make --nocheck-mpc ./examples/dcps

... is fine but:

mpc.make.pl --make --type make --check-mpc ./examples/dcps

... will probably not work as there are MPC files under there that can only be processed with the "--language chsharp"
flags, or the "--type javamake". Basically, if your tree has one language (C== C++ for this purpose) and type of build
file to be generated: it should all be aces.

=head2 EXAMPLES

=over 8

=item B<magic_make.pl .>

Find any buildfiles of the default type (Makefile) in this directory or those below, find also any MPC files.
If any MPC file is newer than the oldest buildfile (or there are no buildfiles) run clean on the existing buildfiles,
regenerate fresh Makefiles. In any case: then build all projects.

=item B<magic_make.pl --squeaky --clean --check-mpc --type vc10 --src-co --nomake isocpp/>

Clean any existing Visual Studio 2010 files then delete them. Regenerate Visual Studio 2010 files suitable for use in a
OpenSplice source checkout format (--src-co) but do not then make them.

=cut;
