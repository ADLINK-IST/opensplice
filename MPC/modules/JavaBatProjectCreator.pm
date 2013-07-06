package JavaBatProjectCreator;

use strict;

use ProjectCreator;
use WinProjectBase;
use MakeProjectBase;
use VCPropertyBase;

use vars qw(@ISA);
@ISA = qw(MakeProjectBase WinProjectBase ProjectCreator VCPropertyBase);

sub languageSupported {
  return (Creator::java()  eq $_[0]->get_language());
}

sub project_file_prefix {
  my $self = shift;
  return 'BUILD_';
}

sub project_file_extension {
  #my $self = shift;
  return '.bat';
}

sub get_dll_exe_template_input_file {
  #my $self = shift;
  return 0;
}


sub get_dll_template_input_file {
  #my $self = shift;
  return 0;
}

sub get_template {

  #my $self = shift;
  return 'javabat';
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
