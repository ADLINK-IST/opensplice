package MakePropertyBase;

# ************************************************************
# Description   : A Make property base module
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

  ## All projects that use this base class are 'make' based.
  $$props{'make'} = 1;

  return $props;
}


1;
