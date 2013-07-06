package BorlandPropertyBase;

# ************************************************************
# Description   : A Borland base module for properties.
# Author        : Chad Elliott
# Create Date   : 5/5/2009
# $Id$
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;
use WinPropertyBase;

our @ISA = qw(WinPropertyBase);

# ************************************************************
# Subroutine Section
# ************************************************************

sub get_properties {
  my $self = shift;

  ## Get the base class properties and add the properties that we
  ## support.
  my $props = $self->WinPropertyBase::get_properties();

  ## All projects that use this base class are for Borland compilers.
  $$props{'borland'} = 1;

  return $props;
}


1;
