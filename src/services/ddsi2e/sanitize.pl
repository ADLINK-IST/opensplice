#!/usr/bin/env perl -w

#print "@ARGV\n";

die "usage: $0 SRCDIR... DSTDIR" unless @ARGV >= 2;
my ($DSTDIR) = pop @ARGV;
my (@SRCDIRS) = @ARGV;

die "$0: destination dir $DSTDIR exists" if -d $DSTDIR;
mkdir $DSTDIR or die "$0: failed to create destination dir $DSTDIR";

@sources = map { glob "$_/*.[ch] $_/*.odl" } @SRCDIRS;
die "$0: no files in source dir @SRCDIRS" unless @sources > 0;

my @stk = (); # stack of conditional nesting, for each: copy/discard/ignore
my $stkdc = 0; # discard count

# %isenh contains the macros that are considered to indicate DDSI2-E
# features.  Each macro maps to one of three values:
#
# - undef: copy as if it is not feature flag (for easy changing &
#   self-documenting);
#
# - 0: indicates a feature for inclusion in DDSI2;
#
# - 1: indicates a feature exclusive to DDSI2-E.
my %isenh = ('LITE' => 1,
             'DDSI_INCLUDE_ENCRYPTION' => 1,
             'DDSI_INCLUDE_NETWORK_CHANNELS' => 1,
             'DDSI_INCLUDE_BANDWIDTH_LIMITING' => 1,
             'DDSI_INCLUDE_NETWORK_PARTITIONS' => 1,
             'DDSI_INCLUDE_SSM' => 1,
             'DDSI2E_OR_NOT2E' => 1);

my %specials = ('q_main.c' => \&q_main_xforms, 'q_config.c' => \&q_config_xforms);

my $inner_lic = '/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */';
print STDOUT "Updating license header\n";

for my $src (@sources) {
  (my $src_notdir = $src) =~ s/^.*?\/([^\/]+)$/$1/g;
  my $dst = "$DSTDIR/$src_notdir";
  my $special_xform = $specials{$src_notdir};
  my $drop = 0, $copy = 0;

  my $comment = '';
  my $cline = 0;
  my $cfound = 0;

  die unless @stk == 0 && $stkdc == 0;

  #print STDERR "$src => $dst ... ";
  open SRC, "< $src" or die "$0: can't open $src";
  open DST, "> $dst" or die "$0: can't open $dst";
  while (<SRC>) {
    my $orig = $_;
    chomp;

    #detect and replace copyright header
    if (($cfound == 0) && (/^\s*\/\*$/)) {
      $comment .= $orig;
      $cline = 1;
      next;
    } elsif ($cline >= 1) {
      if (/^\s*\*\s*Vortex OpenSplice$/) {
        $cline++;
        $cfound = 1;
        next;
      } elsif ($cline == 1) {
        print DST $comment;
        $comment = '';
        $cline = 0;
      } elsif (/^\s*\*\/$/) {
        $cline = 0;
        print DST $inner_lic . "\n";
        next;
      } else {
        next;
      }
    }

    if (/^\s*\#\s*if(?:def)?\s+([A-Za-z_][A-Za-z_0-9]*)\s*(?:\/(?:\/.*|\*.*?\*\/)\s*)?$/) {
      if (not defined $isenh{$1}) {
        push @stk, 'ignore';
      } elsif ($isenh{$1}) {
        push @stk, 'discard';
        $stkdc++;
        next;
      } else {
        push @stk, 'copy';
        next;
      }
    } elsif (/^\s*\#\s*if(?:ndef\s+|\s+!\s*)([A-Za-z_][A-Za-z_0-9]*)\s*(?:\/(?:\/.*|\*.*?\*\/)\s*)?$/) {
      if (not defined $isenh{$1}) {
        push @stk, 'ignore';
      } elsif ($isenh{$1}) {
        push @stk, 'copy';
        next;
      } else {
        push @stk, 'discard';
        $stkdc++;
        next;
      }
    } elsif (/^\s*\#\s*if\s/) { # not relevant to DDSI-E
      push @stk, 'ignore';
    } elsif (/^\s*\#\s*else($|\s)/) {
      if ($stk[$#stk] eq 'discard') {
        $stk[$#stk] = 'copy';
        $stkdc--;
        next;
      } elsif ($stk[$#stk] eq 'copy') {
        $stk[$#stk] = 'discard';
        $stkdc++;
        next;
      }
    } elsif (/^\s*\#\s*endif($|\s)/) {
      my $x = pop @stk;
      $stkdc-- if $x eq 'discard';
      next unless $x eq 'ignore';
    }

    $orig = &$special_xform ($orig) if defined $special_xform;

    if ($stkdc == 0 && defined $orig) {
      print DST $orig;
      $copy++;
    } else {
      $drop++;
    }
  }
  close DST;
  close SRC;

  if ($copy == 0) {
    #print STDERR "deleted\n";
    unlink $dst or die "$0: can't delete $dst";
  } else {
    #printf STDERR "%.0f%% dropped\n", (100.0 * $drop / ($drop + $copy));
  }
}

# Currently the transforms below are good enough, but perhaps it would
# be better to be slightly more careful.

sub q_main_xforms {
  my ($line) = @_;
  $line =~ s/ddsi2e/ddsi2/g;
  $line =~ s/(?<!V_SERVICETYPE_)DDSI2E/DDSI2/g;
  return $line;
}

sub q_config_xforms {
  my ($line) = @_;
  $line =~ s/DDSI2EService\|\|?DDSI2Service/DDSI2Service/g;
  $line =~ s/DDSI2E/DDSI2/g;
  return $line;
}
