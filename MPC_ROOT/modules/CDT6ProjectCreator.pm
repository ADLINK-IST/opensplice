package CDT6ProjectCreator;

# ************************************************************
# Description   : Eclipse CDT 6 generator
# Author        : Chris Cleeland, Object Computing, Inc.
# Create Date   : 23-Apr-2010
# $Id$
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;

use ProjectCreator;

use vars qw(@ISA);
@ISA = qw(ProjectCreator);

# ************************************************************
# Data Section
# ************************************************************

my %templates = ('cdt6project'   => '.project',
                 'cdt6cproject'  => '.cproject' );

my @tkeys = sort keys %templates;

# ************************************************************
# Subroutine Section
# ************************************************************

sub crlf {
  #my $self = shift;
  return "\n";
}

sub project_file_name {
  my($self, $name, $template) = @_;

  ## Fill in the name and template if they weren't provided
  $name = $self->project_name() if (!defined $name);
  $template = 'cdt6project' if (!defined $template ||
                                !defined $templates{$template});

  if ($self->{'make_coexistence'}) {
    return $self->get_modified_project_file_name("cdt_$name",
                                                 '/' . $templates{$template});
  }
  else {
    return $templates{$template};
  }
}

sub fill_value {
  my($self, $name) = @_;

  if ($name eq 'platforms') {
    if (defined $ENV{'MPC_CDT_PLATFORMS'}) {
      return $ENV{'MPC_CDT_PLATFORMS'};
    }
    elsif ($^O eq 'darwin') {
      return 'macosx';
    }
    elsif ($^O eq 'MSWin32') {
      return 'win32';
    }
    else {
      return $^O; # cygwin, solaris, linux match what we expect
    }
  }
  elsif ($name eq 'nocross') {
    # return the value of the 'nocross' element from the project_info array
    return $self->get_project_info()->[ProjectCreator::NO_CROSS_COMPILE];
  }

  return $self->get_configurable($name);
}

sub get_configurable {
  #my($self, $name) = @_;
  return undef;
}

sub get_template {
  #my $self = shift;
  return @tkeys;
}

sub dependency_is_filename {
  #my $self = shift;
  return 0;
}

sub requires_forward_slashes {
  return 1;
}

sub file_visible {
  ## We only want the project file visible to the workspace creator.
  ## There can only be one and this is it.
  #my($self, $template) = @_;
  return $_[1] eq 'cdt6project';
}

sub get_dll_exe_template_input_file {
  #my $self = shift;
  return 'cdt6exe';
}

sub get_dll_template_input_file {
  #my $self = shift;
  #print "in get_dll_template_input_file\n";
  return 'cdt6dll';
}

sub get_lib_template_input_file {
  #my $self = shift;
  #print "in get_lib_template_input_file\n";
  return 'cdt6lib';
}

sub use_win_compatibility_commands {
  return (defined $ENV{'MPC_CDT_HOST_WIN32'})
      ? $ENV{'MPC_CDT_HOST_WIN32'} : ($^O eq 'MSWin32' || $^O eq 'cygwin');
}

sub pre_write_output_file {
  my $self = shift;
  if ($self->{'assign'}->{'custom_only'}) {
    return 1;
  }
  return $self->combine_custom_types();
}

1;
