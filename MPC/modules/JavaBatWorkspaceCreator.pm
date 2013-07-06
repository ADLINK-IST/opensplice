package JavaBatWorkspaceCreator;

use strict;

#use MakeWorkspaceBase;
use WinWorkspaceBase;
use WorkspaceCreator;
use JavaBatProjectCreator;

use vars qw(@ISA);
@ISA = qw(WinWorkspaceBase WorkspaceCreator JavaBatProjectCreator);

sub workspace_file_prefix {
  #my $self = shift;
  return 'BUILD_';
}

sub workspace_file_extension {
  #my $self = shift;
  return '.bat';
}


sub pre_workspace {
  #my($self, $fh) = @_;

}

sub workspace_per_project {
  #my $self = shift;
  return 1;
}

sub workspace_file_name {
  my $self = shift;

  if ($self->{'per_project_workspace_name'})
  {
    return "BUILD_java.bat";
  }

  return $self->workspace_file_prefix() . $self->get_workspace_name() . $self->workspace_file_extension();
}

sub write_project_targets {
  #my($self, $fh, $crlf, $target, $list) = @_;
}


sub write_comps {
  my($self, $fh, $gen) = @_;
   my $projects = $self->get_projects();

  my @list = $self->sort_dependencies($projects);

  my $crlf = $self->crlf();
  print $fh "\@echo off$crlf";
  foreach my $project (@list) {
    print $fh "echo call $project %*$crlf";
    print $fh "call $project %*$crlf";
    print $fh "IF NOT %ERRORLEVEL% == 0 ($crlf";
    print $fh "ECHO:$crlf";
    print $fh "ECHO *** Error building $project$crlf";
    print $fh "ECHO:$crlf";
    print $fh "GOTO error$crlf";
    print $fh ")$crlf";
    print $fh "cd %~dp0$crlf";
  }
  print $fh "GOTO end$crlf";
  print $fh ":error$crlf";
  print $fh "ECHO An error occurred, exiting now$crlf";
  print $fh ":end$crlf";
}


sub get_properties {
  my $self = shift;

  ## Create the map of properties that we support.
  my $props = {};

  ## Merge in properties from all base projects
  foreach my $base (@ISA) {
    my $func = $base . '::get_properties';
    my $p = $self->$func();
    foreach my $key (keys %$p) {
      $$props{$key} = $$p{$key};
    }
  }

  return $props;
}


1;
