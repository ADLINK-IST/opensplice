package VCPropertyBase;

# ************************************************************
# Description   : A VC property base module
# Author        : Chad Elliott
# Create Date   : 3/9/2010
# $Id$
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;

use WinPropertyBase;

use vars qw(@ISA);
@ISA = qw(WinPropertyBase);

# ************************************************************
# Subroutine Section
# ************************************************************

sub get_properties {
  my $self = shift;

  ## Get the base class properties and add the properties that we
  ## support.
  my $props = $self->WinPropertyBase::get_properties();

  ## All projects that use this base class are for Microsoft compilers.
  $$props{'microsoft'} = 1;

  return $props;
}


1;
