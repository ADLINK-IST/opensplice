package WinPropertyBase;

# ************************************************************
# Description   : A Windows base module for properties
# Author        : Chad Elliott
# Create Date   : 3/9/2010
# $Id$
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;

# ************************************************************
# Subroutine Section
# ************************************************************

sub get_properties {
  my $self = shift;

  ## Get the base class properties and add the properties that we
  ## support.
  my $props = $self->Creator::get_properties();

  ## All projects that use this base class are for Windows.
  $$props{'windows'} = 1;

  return $props;
}


1;
