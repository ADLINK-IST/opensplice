package WB30ProjectCreator;

# ************************************************************
# Description   : Wind River Workbench 3.0 generator
# Author        : Adam Mitz (Object Computing, Inc.)
# Create Date   : 07/21/2010
# $Id$
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;

use WB26ProjectCreator;
use XMLProjectBase;

use vars qw(@ISA);
@ISA = qw(XMLProjectBase WB26ProjectCreator);

# ************************************************************
# Data Section
# ************************************************************

my %templates = ('wb26'           => '.project',
                 'wb26wrproject'  => '.wrproject',
                 'wb26wrmakefile' => '.wrmakefile',
                 'wb30cproject'   => '.cproject');

my @tkeys = sort keys %templates;

# ************************************************************
# Subroutine Section
# ************************************************************

sub project_file_name {
  my($self, $name, $template) = @_;

  ## Fill in the name and template if they weren't provided
  $name = $self->project_name() if (!defined $name);
  $template = 'wb26' if (!defined $template || !defined $templates{$template});

  if ($self->{'make_coexistence'}) {
    return $self->get_modified_project_file_name("wb_$name",
                                                 '/' . $templates{$template});
  }
  else {
    return $templates{$template};
  }
}

sub post_file_creation {
  my($self, $file) = @_;
  if ($file =~ /$templates{'wb26wrmakefile'}$/) {
    my @lines;
    if (open(IN, $file)) {
      while(<IN>) {
        s/\\&quot;/\\"/g;
        s/&quot;/"/g;
        s/&gt;/>/g;
        s/&lt;/</g;
        s/&amp;/&/g;
        push(@lines, $_);
      }
      close(IN);
    }
    else {
      return "Can't open $file for post-processing input.";
    }
    if (open(OUT, ">$file")) {
      print OUT @lines;
      close(OUT);
    }
    else {
      return "Can't open $file for post-processing output.";
    }
  }
  return undef;
}

sub get_template {
  #my $self = shift;
  return @tkeys;
}

sub get_dll_exe_template_input_file {
  #my $self = shift;
  return 'wb30exe';
}

sub get_dll_template_input_file {
  #my $self = shift;
  return 'wb30dll';
}

1;
