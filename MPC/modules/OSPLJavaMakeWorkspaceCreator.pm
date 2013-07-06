package OSPLJavaMakeWorkspaceCreator;

use strict;

use OSPLJavaMakeProjectCreator;
use MakeWorkspaceBase;
use WorkspaceCreator;

use vars qw(@ISA);
@ISA = qw(MakeWorkspaceBase WorkspaceCreator OSPLJavaMakeProjectCreator);

# ************************************************************
# Data Section
# ************************************************************

my $targets = 'clean depend generated realclean check-syntax $(CUSTOM_TARGETS)';

# ************************************************************
# Subroutine Section
# ************************************************************

sub write_project_targets {
  my($self, $fh, $crlf, $target, $list) = @_;

  ## Print out a make command for each project
  foreach my $project (@$list) {
    my $dname = $self->mpc_dirname($project);
    my $chdir = ($dname ne '.');
    print $fh "\t\@",
              ($chdir ? "cd $dname && " : ''),
              "\$(MAKE) -f ",
              ($chdir ? $self->mpc_basename($project) : $project),
              " $target$crlf";
  }
}

sub pre_workspace {
  my($self, $fh) = @_;
  $self->workspace_preamble($fh, $self->crlf(), 'Make Workspace',
                            '$Id$');
}


sub write_comps {
  my($self, $fh) = @_;
  my %targnum;
  my @list = $self->number_target_deps($self->get_projects(),
                                       $self->get_project_info(),
                                       \%targnum, 0);

  ## Send all the information to our base class method
  $self->write_named_targets($fh, $self->crlf(), \%targnum, \@list,
                             ($self->languageIs(Creator::csharp) ?
                               'bundle ' : '') . $targets, '', '',
                             $self->project_target_translation(1), 1);
}




1;
