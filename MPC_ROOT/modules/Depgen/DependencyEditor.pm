package DependencyEditor;

# ************************************************************
# Description   : Edits existing dependencies.
# Author        : Chad Elliott
# Create Date   : 2/10/2002
# $Id$
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;
use FileHandle;

use DependencyGenerator;

# ************************************************************
# Subroutine Section
# ************************************************************

sub new {
  return bless {}, $_[0];
}


sub process {
  my($self, $output, $type, $noinline, $macros,
     $ipaths, $replace, $exclude, $files,
     $append) = @_;

  ## Back up the original file and receive the contents
  my $contents;
  if (-s $output) {
    $contents = [];
    if (!$self->backup($output, $contents, $append)) {
      print STDERR "ERROR: Unable to backup $output\n";
      return 1;
    }
  }

  ## Write out the contents of the file
  my $fh = new FileHandle();
  if (open($fh, ">$output")) {
    if (defined $contents) {
      foreach my $line (@$contents) {
        print $fh $line;
      }
    }

    if (!$append) {
      ## Write out the new dependency marker
      print $fh "# DO NOT DELETE THIS LINE -- depgen.pl uses it.\n",
	        "# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.\n\n";
    }
    else {
      ## Write start append comment
      print $fh "# DO NOT DELETE THIS LINE -- depgen.pl appended ",
	        "the following.\n",
	        "# APPENDED DEPENDENCY RULES " ,
		"by depgen.pl.\n\n";
    }

    ## Generate the new dependencies and write them to the file
    my $dep = new DependencyGenerator($macros, $ipaths, $replace,
                                      $type, $noinline,
                                      $exclude);

    ## Sort the files so the dependencies are reproducible
    foreach my $file (sort @$files) {
      ## In some situations we may be passed a directory as part of an
      ## option.  If it is an unknown option, we may think the directory
      ## needs to be part of the dependencies when it should not.
      print $fh $dep->process($file), "\n" if (!-d $file);
    }

    ## Write out the end of the block warning
    print $fh "# IF YOU PUT ANYTHING HERE IT WILL GO AWAY\n";
    close($fh);
  }
  else {
    print STDERR "ERROR: Unable to open $output for output\n";
    return 1;
  }

  return 0;
}


sub backup {
  my($self, $source, $contents, $append) = @_;
  my $status;
  my $fh     = new FileHandle();
  my $backup = "$source.bak";

  ## Back up the file.  While doing so, keep track of the contents of the
  ## file and keep everything except the old dependencies or keep
  ## everything if appending.
  my $search_string;
  if (!$append) {
    $search_string = 'DO NOT DELETE';
  }
  else {
    $search_string = 'IF YOU PUT ANYTHING HERE IT WILL GO AWAY';
  }

  if (open($fh, $source)) {
    my $oh = new FileHandle();
    if (open($oh, ">$backup")) {
      my $record = 1;
      $status = 1;
      while(<$fh>) {
        print $oh $_;
        if ($record) {
          if (index($_, $search_string) >= 0) {
            $record = undef;
          }
          else {
            push(@$contents, $_);
          }
        }
      }
      close($oh);

      ## Set file permission so that the backup has the same permissions
      ## as the original file.
      my @buf = stat($source);
      if (defined $buf[8] && defined $buf[9]) {
        utime($buf[8], $buf[9], $backup);
      }
    }
    close($fh);
  }
  return $status;
}


1;
