#! /usr/bin/perl
eval '(exit $?0)' && eval 'exec perl -w -S $0 ${1+"$@"}'
    & eval 'exec perl -w -S $0 $argv:q'
    if 0;

# ******************************************************************
#      Author: Chad Elliott
# Create Date: 3/09/2004
#         $Id$
# ******************************************************************

# ******************************************************************
# Pragma Section
# ******************************************************************

use strict;
use Cwd;
use FileHandle;
use File::Copy;
use File::Basename;
use File::Find;

# ******************************************************************
# Data Section
# ******************************************************************

my $insext   = 'ins';
my $version  = '2.0';
my %defaults = ('header_files'   => 1,
                'idl_files'      => 1,
                'inline_files'   => 1,
                'pidl_files'     => 1,
                'template_files' => 1,
                'mpb_files'      => 1,
               );

my %special  = ('exe_output' => 1,
                'lib_output' => 1,
               );
my %extended;
my %actual;
my %base;
my %override;
my $keepgoing = 0;
my $binarydir = '';

eval 'symlink("", "");';
my $hasSymlink = ($@ eq '');

# ******************************************************************
# Subroutine Section
# ******************************************************************

sub rm_updirs {
  my $path  = shift;
  my @parts = split(/[\/\\]/, $path);

  ## Split the path into parts and check for '..'.  If we find one
  ## and the previous entry wasn't one, then we can remove them both.
  for(my $i = 0; $i <= $#parts; $i++) {
    if ($i > 0 && $parts[$i] eq '..' && $parts[$i - 1] ne '..') {
      splice(@parts, $i - 1, 2);
      $i -= 2;
    }
  }
  return join('/', @parts);
}

sub get_dest {
  my($file, $insdir) = @_;
  return rm_updirs($insdir . '/' .
                   (defined $actual{$file}
                    ? "$actual{$file}/" . basename($file)
                    : $file));
}

sub copyFiles {
  my($files, $insdir, $symlink, $verbose) = @_;
  my $type = ($symlink ? 'link' : 'copy');
  my $cwd = getcwd();

  # If > 1 file in @$files is a symlink to the same target, create one of the
  # files (the shorter-named one) as a symlink in the destination tree.
  my %localLink; # keys: files to create as symlinks in the destination
                 # values: symlink target (used in the main loop, below)
  my %linkTargets; # keys: link targets in the source tree
                   # values: name of the symlink in the source tree
  foreach my $file (@$files) {
    if (-l $file) {
      my $dest = readlink($file);
      if (exists $linkTargets{$dest} && $file ne $linkTargets{$dest}) {
        if (length $file <= length $linkTargets{$dest}) {
          $localLink{$file} = $linkTargets{$dest};
        }
        else {
          $localLink{$linkTargets{$dest}} = $file;
          $linkTargets{$dest} = $file;
        }
      }
      else {
        $linkTargets{$dest} = $file;
      }
    }
  }

  foreach my $file (@$files) {
    my $dest = get_dest($file, $insdir);
    my $fulldir = dirname($dest);
    if (! -d $fulldir) {
      my $tmp = '';
      foreach my $part (split(/[\/\\]/, $fulldir)) {
        $tmp .= $part . '/';
        mkdir($tmp, 0755);
      }
    }

    if (! -e $dest || (-M $file) < (-M $dest)) {
      if ($verbose) {
        print '', ($symlink ? 'Linking' : 'Copying'), " to $dest\n";
      }
      my $status;
      if ($symlink) {
        unlink($dest);
        $status = symlink("$cwd/$file", $dest);
      }
      elsif (exists $localLink{$file}) {
        my $target = get_dest($localLink{$file}, $insdir);
        if ($fulldir eq dirname($target)) {
          $target = basename($target);
        }
        if ($verbose) {
          print "\tCreating local symlink to $target\n";
        }
        unlink($dest);
        $status = symlink($target, $dest);
      }
      else {
        $status = copy($file, $dest);
        chmod(0755, $dest) if ($status && -x $file);
      }
      if (!$status) {
        print STDERR "ERROR: Unable to $type $file to $dest\n";
        if (!$keepgoing) {
          return 0;
        }
      }
    }
    else {
      print "Skipping $file\n" if ($verbose);
    }
  }
  return 1;
}


sub determineSpecialName {
  my($tag, $dir, $info) = @_;

  my($insdir, $name) = split(/\s+/, $info);
  if (defined $name) {
    $insdir .= '/';
  }
  else {
    $name = $insdir;
    $insdir = '';
  }

  my $odir = ($insdir =~ /^\// ? $insdir : ($dir . $insdir)) . $binarydir;
  $odir = '.' if $odir eq '';
  if ($tag eq 'exe_output') {
    my @exes;
    my $fh   = new FileHandle();
    if (opendir($fh, $odir)) {
      foreach my $file (grep(!/^\.\.?$/, readdir($fh))) {
        if ($file =~ /^$name$/ ||
            $file =~ /^$name.*\.exe$/i) {
          push(@exes, "$dir$insdir$binarydir$file");
        }
      }
      closedir($fh);
    }
    return @exes;
  }
  elsif ($tag eq 'lib_output') {
    my @libs;
    my $fh   = new FileHandle();
    if (opendir($fh, $odir)) {
      foreach my $file (grep(!/^\.\.?$/, readdir($fh))) {
        if ($file =~ /^lib$name\.(a|so|sl|dylib)/ ||
            $file =~ /^(lib)?$name.*\.(dll|lib)$/i) {
          push(@libs, "$dir$insdir$binarydir$file");
        }
      }
      closedir($fh);
    }
    return @libs;
  }

  return "$dir$name";
}


sub replaceVariables {
  my $line = shift;
  while($line =~ /(\$\(([^)]+)\))/) {
    my $whole = $1;
    my $name = $2;
    my $val = (defined $ENV{$name} ? $ENV{$name} : '');
    $line =~ s/\$\([^)]+\)/$val/;
  }
  return $line;
}


sub loadInsFiles {
  my($files, $tags, $verbose) = @_;
  my $fh = new FileHandle();
  my @copy;

  foreach my $file (@$files) {
    if (open($fh, $file)) {
      if ($verbose) {
        print "Loading $file\n";
      }
      my $base = dirname($file);
      if ($base eq '.') {
        $base = '';
      }
      else {
        $base =~ s/^\.[\/\\]+//;
        $base .= '/';
      }

      my $current;
      while(<$fh>) {
        my $line = $_;
        $line =~ s/^\s+//;
        $line =~ s/\s+$//;

        if ($line ne '') {
          if ($line =~ /^(\w+):$/) {
            if (defined $$tags{$1}) {
              $current = $1;
            }
            else {
              $current = undef;
            }
          }
          elsif (defined $current) {
            $line = replaceVariables($line);
            my $perFileOverride = undef;
            my $start = $#copy + 1;
            if (defined $special{$current}) {
              push(@copy, determineSpecialName($current, $base, $line));
            }
            else {
              my($src, $dst);
              if ($line =~ /^\"([^"]+)\" (.*)/ || $line =~ /^(\S+) (\S+)/) {
                ($src, $dst) = ($1, $2);
              }
              else {
                ($src, $dst) = ($line, '');
              }
              if (defined $extended{$current}) {
                push(@copy, "$base$src");
                $perFileOverride = $dst;
              }
              else {
                push(@copy, "$base$src");
              }
            }
            if (-d $copy[-1]) {
              my $dir = pop @copy;
              my $replace = $perFileOverride
                  ? ($perFileOverride . '/' . basename($dir)) : '';
              find({no_chdir => 1,
                    wanted => sub {
                      if (!(-d || /\.svn\// || /~$/)) {
                        push @copy, $_;
                        my $rel = $_;
                        $rel =~ s/^$dir/$replace/;
                        $actual{$_} =
                            (defined $override{$current} ? $override{$current}
                             : $base{$current}) . '/' . dirname($rel);
                      }
                    }
                   }, $dir);
            }
            elsif (defined $override{$current}) {
              for(my $i = $start; $i <= $#copy; ++$i) {
                $actual{$copy[$i]} = $override{$current};
              }
            }
            elsif (defined $base{$current}) {
              for(my $i = $start; $i <= $#copy; ++$i) {
                $actual{$copy[$i]} = $base{$current} . '/' .
                  ($perFileOverride ? $perFileOverride : dirname($copy[$i]));
              }
            }
          }
        }
      }
      close($fh);
    }
    else {
      print STDERR "Unable to open $file\n";
      return ();
    }
  }

  return @copy;
}


sub getInsFiles {
  my $file  = shift;
  my @files;

  if (-d $file) {
    my $fh = new FileHandle();
    if (opendir($fh, $file)) {
      foreach my $f (grep(!/^\.\.?$/, readdir($fh))) {
        push(@files, getInsFiles("$file/$f"));
      }
      closedir($fh);
    }
  }
  elsif ($file =~ /\.$insext$/) {
    push(@files, $file);
  }
  return @files;
}


sub usageAndExit {
  my $msg = shift;

  print STDERR "$msg\n" if (defined $msg);

  my $base = basename($0);
  my $spc  = ' ' x (length($base) + 8);
  print STDERR "$base v$version\n",
               "Usage: $base [-a tag1[,tagN]] [-b tag=dir] [-d dir]",
               ($hasSymlink ? '[-l] ' : ''), "[-o tag=dir]\n",
               $spc, "[-s tag1[,tagN]] [-x tag1[,tagN]] [-v] [-k] [-i]\n",
               $spc, "[install directory] [$insext files or directories]\n\n",
               "Install files matching the tag specifications found ",
               "in $insext files.\n\n",
               "-a  Adds to the default set of tags that get copied.\n",
               "-b  Install tag into dir underneath the install directory.\n",
               "-d  Libs/executables are copied from this sub-directory.\n",
               "-i  Read standard input in place of the $insext file.\n",
               "-k  Keep going if a file to be copied is missing.\n",
               ($hasSymlink ? "-l  Use symbolic links instead of copying.\n" : ''),
               "-o  Install tag into dir.\n",
               "-s  Sets the tags that get copied.\n",
               "-v  Enables verbose mode.\n",
               "-x  Enable extended behavior for the given tags:\n",
               "      These tags will use the 'gendir' setting from the\n",
               "      project as a relative target directory.\n",
               "\n",
               "The default set of tags are:\n";
  my $first = 1;
  foreach my $key (sort keys %defaults) {
    print STDERR '', ($first ? '' : ', '), $key;
    $first = 0;
  }
  print STDERR "\n";

  exit(0);
}

# ******************************************************************
# Main Section
# ******************************************************************

my $verbose;
my $first = 1;
my $insdir;
my $symlink;
my @insfiles;
my %tags = %defaults;

for(my $i = 0; $i <= $#ARGV; ++$i) {
  my $arg = $ARGV[$i];
  if ($arg =~ /^-/) {
    if ($arg eq '-a') {
      ++$i;
      if (defined $ARGV[$i]) {
        foreach my $tag (split(',', $ARGV[$i])) {
          $tags{$tag} = 1;
        }
      }
      else {
        usageAndExit('-a requires a parameter.');
      }
    }
    elsif ($arg eq '-b') {
      ++$i;
      if (defined $ARGV[$i]) {
        if ($ARGV[$i] =~ /([^=]+)=(.*)/) {
          $base{$1} = $2;
        }
        else {
          usageAndExit("Invalid parameter to -b: $ARGV[$i]");
        }
      }
      else {
        usageAndExit('-b requires a parameter.');
      }
    }
    elsif ($arg eq '-d') {
      ++$i;
      if (defined $ARGV[$i]) {
        $binarydir = $ARGV[$i];
        $binarydir =~ s/\\/\//g;
        $binarydir .= '/' unless $binarydir =~ /\/$/;
      }
      else {
        usageAndExit('-d requires a parameter.');
      }
    }
    elsif ($arg eq '-i') {
      push(@insfiles, '-');
    }
    elsif ($arg eq '-k') {
      $keepgoing = 1;
    }
    elsif ($arg eq '-l') {
      $symlink = $hasSymlink;
    }
    elsif ($arg eq '-o') {
      ++$i;
      if (defined $ARGV[$i]) {
        if ($ARGV[$i] =~ /([^=]+)=(.*)/) {
          $override{$1} = $2;
        }
        else {
          usageAndExit("Invalid parameter to -o: $ARGV[$i]");
        }
      }
      else {
        usageAndExit('-o requires a parameter.');
      }
    }
    elsif ($arg eq '-s') {
      ++$i;
      if (defined $ARGV[$i]) {
        %tags = ();
        foreach my $tag (split(',', $ARGV[$i])) {
          $tags{$tag} = 1;
        }
      }
      else {
        usageAndExit('-s requires a parameter.');
      }
    }
    elsif ($arg eq '-v') {
      $verbose = 1;
    }
    elsif ($arg eq '-x') {
      ++$i;
      if (defined $ARGV[$i]) {
        foreach my $tag (split(',', $ARGV[$i])) {
          $extended{$tag} = 1;
        }
      }
      else {
        usageAndExit('-x requires a parameter.');
      }
    }
    else {
      usageAndExit('Unkown option: ' . $arg);
    }
  }
  elsif (!defined $insdir) {
    $arg =~ s/\\/\//g;
    $insdir = $arg;
  }
  else {
    if ($first) {
      $first = 0;
      if ($verbose) {
        print "Collecting $insext files...\n";
      }
    }
    $arg =~ s/\\/\//g;
    push(@insfiles, getInsFiles($arg));
  }
}

if (!defined $insdir) {
  usageAndExit();
}
elsif (!defined $insfiles[0]) {
  print "No $insext files were found.\n";
  exit(1);
}

my $status = 1;
my @files  = loadInsFiles(\@insfiles, \%tags, $verbose);
if (defined $files[0]) {
  $status = (copyFiles(\@files, $insdir, $symlink, $verbose) ? 0 : 1);
}

exit($status);
