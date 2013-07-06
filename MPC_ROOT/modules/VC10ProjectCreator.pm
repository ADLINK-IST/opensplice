package VC10ProjectCreator;

# ************************************************************
# Description   : A VC10 Project Creator
# Author        : Johnny Willemsen
# Create Date   : 11/10/2008
# $Id$
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;

use VC9ProjectCreator;

use vars qw(@ISA);
@ISA = qw(VC9ProjectCreator);

# ************************************************************
# Data Section
# ************************************************************

## NOTE: We call the constant as a function to support Perl 5.6.
my %info = (Creator::cplusplus() => {'ext'      => '.vcxproj',
                                     'dllexe'   => 'vc10exe',
                                     'libexe'   => 'vc10libexe',
                                     'dll'      => 'vc10dll',
                                     'lib'      => 'vc10lib',
                                     'template' => [ 'vc10', 'vc10filters' ],
                                    },
           );

my %config = ('vcversion' => '10.00',
              'prversion' => '10.0.30319.1',
              'toolsversion' => '4.0',
              'targetframeworkversion' => '4.0',
              'xmlheader' => 1,
              );

# ************************************************************
# Subroutine Section
# ************************************************************

sub get_info_hash {
  my($self, $key) = @_;

  ## If we have the setting in our information map, the use it.
  return $info{$key} if (defined $info{$key});

  ## Otherwise, see if our parent type can take care of it.
  return $self->SUPER::get_info_hash($key);
}

sub get_configurable {
  my($self, $name) = @_;
  return $config{$name};
}

## Because VC10 puts file filters in a different file
## that starts with the project file name, and ends
## with .filters extension. So we need to return two
## templates.
sub get_template {
  my $self = shift;
  my $templates = $self->SUPER::get_template();

  return (UNIVERSAL::isa($templates, 'ARRAY') ? @$templates : $templates);
}

sub file_visible {
  my($self, $template) = @_;
  my $templates = $self->SUPER::get_template();

  if (UNIVERSAL::isa($templates, 'ARRAY')) {
    return ($template eq $$templates[0]);
  }

  return 1;
}

## If the template is one of the additional templates,
## we need to append the proper extension to the file name.
sub project_file_name {
  my($self, $name, $template) = @_;

  my $project_file_name = $self->SUPER::project_file_name($name, $template);
  if (!$self->file_visible($template)) {
    $project_file_name .= '.filters';
  }

  return $project_file_name;
}

sub pre_write_output_file {
  my($self, $webapp) = @_;
  return $self->combine_custom_types();
}

1;
