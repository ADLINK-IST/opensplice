package VCProjectBase;

# ************************************************************
# Description   : A VC Project base module
# Author        : Chad Elliott
# Create Date   : 1/4/2005
# $Id$
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;

use VCPropertyBase;
use WinProjectBase;

use vars qw(@ISA);
@ISA = qw(VCPropertyBase WinProjectBase);

# ************************************************************
# Subroutine Section
# ************************************************************

sub compare_output {
  #my $self = shift;
  return 1;
}


sub require_dependencies {
  my $self = shift;

  ## Only write dependencies for non-static projects
  ## and static exe projects, unless the user wants the
  ## dependency combined static library.
  return ($self->get_static() == 0 || $self->exe_target() ||
          $self->dependency_combined_static_library());
}


sub dependency_is_filename {
  #my $self = shift;
  return 0;
}


1;
