package GHSPropertyBase;

# ************************************************************
# Description   : A GHS property base module.
# Author        : Chad Elliott
# Create Date   : 3/9/2010
# $Id$
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;

# ************************************************************
# Data Section
# ************************************************************

our $ghsunix = 'MPC_GHS_UNIX';

# ************************************************************
# Subroutine Section
# ************************************************************

sub get_properties {
  my $self = shift;

  ## Get the base class properties and add the properties that we
  ## support.
  my $props = $self->Creator::get_properties();

  ## This project creator can work for UNIX and Windows.  Set the
  ## property based on the environment variable.
  $$props{'windows'} = 1 if (!defined $ENV{$ghsunix});

  return $props;
}

1;
