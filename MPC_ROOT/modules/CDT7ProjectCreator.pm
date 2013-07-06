package CDT7ProjectCreator;

# ************************************************************
# Description   : A CDT7 Project Creator (Eclipse 3.6)
# Author        : Adam Mitz, Object Computing, Inc.
# Create Date   : 10/04/2010
# $Id$
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;
use CDT6ProjectCreator;

use vars qw(@ISA);
@ISA = qw(CDT6ProjectCreator);

# ************************************************************
# Data Section
# ************************************************************

my %config = ('scanner_config_builder_triggers' => 'full,incremental,',
              'additional_storage_modules' =>
                'org.eclipse.cdt.core.language.mapping ' .
                'org.eclipse.cdt.internal.ui.text.commentOwnerProjectMappings',
              'additional_error_parsers' =>
                'org.eclipse.cdt.core.GmakeErrorParser ' .
                'org.eclipse.cdt.core.CWDLocator'
             );

# ************************************************************
# Subroutine Section
# ************************************************************

sub get_configurable {
  my($self, $name) = @_;
  return $config{$name};
}

1;
