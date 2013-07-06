package RpmSpecProjectCreator;

# ************************************************************
# Description   : An RPM .spec file Project Creator
# Author        : Adam Mitz (OCI)
# Create Date   : 11/23/2010
# $Id$
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;
use File::Path;

use ProjectCreator;

use vars qw(@ISA);
@ISA = qw(ProjectCreator);

# ************************************************************
# Subroutine Section
# ************************************************************

sub project_file_extension {
  return '.dummy';
}

# Don't actually write anything, just keep MPC internal data structures
# up-to-date as if it had been written.  We don't want a .spec file for each
# MPC project because that is too fine-grained.  See the corresponding
# workspace creator for the actual .spec file creation.
sub write_output_file {
  my $self = shift;
  my $tover = $self->get_template_override();
  my @templates = $self->get_template();
  @templates = ($tover) if (defined $tover);

  if (scalar @templates != 1) {
    return 0, 'there should be only one template';
  }

  my $template = $templates[0];
  $self->{'current_template'} = $template;

  my $name = $self->transform_file_name($self->project_file_name(undef,
                                                                 $template));
  $self->process_assignment('project_file', $name);
  new TemplateParser($self)->collect_data();

  if (defined $self->{'source_callback'}) {
    my $cb     = $self->{'source_callback'};
    my $pjname = $self->get_assignment('project_name');
    my @list   = $self->get_component_list('source_files');
    if (UNIVERSAL::isa($cb, 'ARRAY')) {
      my @copy = @$cb;
      my $s = shift(@copy);
      &$s(@copy, $name, $pjname, \@list);
    }
    elsif (UNIVERSAL::isa($cb, 'CODE')) {
      &$cb($name, $pjname, \@list);
    }
    else {
      $self->warning("Ignoring callback: $cb.");
    }
  }

  # Still need outdir since ProjectCreator::write_install_file (or similar)
  # may depend on outdir existing before the WorkspaceCreator runs.
  my $outdir = $self->get_outdir();
  mkpath($outdir, 0, 0777) if ($outdir ne '.');

  $self->add_file_written($name);
  return 1, '';
}

1;
