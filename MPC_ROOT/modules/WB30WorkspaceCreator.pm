package WB30WorkspaceCreator;

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

use WB26WorkspaceCreator;
use WB30ProjectCreator;

use vars qw(@ISA);
@ISA = qw(WB26WorkspaceCreator);

# ************************************************************
# Subroutine Section
# ************************************************************

sub get_id_for_comment {
  return '$Id$';
}

sub get_project_prefix {
  return 'wb_';
}

sub get_additional_output {
  ## Create the accompanying list file.  It always goes in the same
  ## directory as the first workspace output file.  See
  ## WorkspaceCreator.pm for a description of the array elements.
  my($self) = @_;
  my $name = 'wb30projects';
  if ($self->get_apply_project()) {
     my $nmod = $self->get_name_modifier();
     if (defined $nmod) {
        $nmod =~ s/\*/$name/g;
        $name = $nmod;
     }
  }
  $name = $name . '.lst';

  return [[undef, "$name", \&WB26WorkspaceCreator::list_file_body]];
}


1;
