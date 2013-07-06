package WorkspaceCreator;

# ************************************************************
# Description   : Base class for all workspace creators
# Author        : Chad Elliott
# Create Date   : 5/13/2002
# $Id$
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;
use FileHandle;
use File::Path;

use Creator;
use Options;
use WorkspaceHelper;

use IO::Socket;
use Data::Dumper;

use vars qw(@ISA);
@ISA = qw(Creator Options);

# ************************************************************
# Data Section
# ************************************************************

## process stuff
our $num_workers = 0;           # single-process
our $wdir;                      # tmp directory
our $wport;

my $wsext  = 'mwc';
my $wsbase = 'mwb';

## Valid names for assignments within a workspace
my %validNames = ('cmdline'  => 1,
                  'implicit' => 1,
                 );

## Singleton hash maps of project information
my %allprinfo;
my %allprojects;
my %allliblocs;

## Global previous workspace names
my %previous_workspace_name;

## Constant aggregated workspace type name
my $aggregated = 'aggregated_workspace';

my $onVMS = DirectoryManager::onVMS();

# ************************************************************
# Subroutine Section
# ************************************************************

sub new {
  my($class, $global, $inc, $template, $ti, $dynamic,
     $static, $relative, $addtemp, $addproj, $progress,
     $toplevel, $baseprojs, $gfeature, $relative_f, $feature,
     $features, $hierarchy, $exclude, $makeco, $nmod, $applypj,
     $genins, $into, $language, $use_env, $expandvars, $gendot,
     $comments, $foreclipse, $workers, $workers_dir,
     $workers_port) = @_;

  my $self = Creator::new($class, $global, $inc,
                          $template, $ti, $dynamic, $static,
                          $relative, $addtemp, $addproj,
                          $progress, $toplevel, $baseprojs,
                          $feature, $features,
                          $hierarchy, $nmod, $applypj,
                          $into, $language, $use_env, $expandvars,
                          'workspace');

  $self->{'pid'} = 'parent';

  # implicit dependency order counter. this is
  # incremented in the children.
  $self->{'imp_dep_ctr'};

  ## These need to be reset at the end of each
  ## workspace processed within a .mwc file
  $self->{'workspace_name'}      = undef;
  $self->{'projects'}            = [];
  $self->{'project_info'}        = {};
  $self->{'project_files'}       = [];
  $self->{'modified_count'}      = 0;
  $self->{'exclude'}             = {};
  $self->{'associated'}          = {};
  $self->{'scoped_assign'}       = {};
  $self->{'aggregated_mpc'}      = {};
  $self->{'aggregated_assign'}   = {};
  $self->{'mpc_to_output'}       = {};

  ## These are maintained/modified throughout processing
  $self->{$self->{'type_check'}} = 0;
  $self->{'cacheok'}             = $self->default_cacheok();
  $self->{'lib_locations'}       = {};
  $self->{'reading_parent'}      = [];
  $self->{'global_feature_file'} = $gfeature;
  $self->{'relative_file'}       = $relative_f;
  $self->{'project_file_list'}   = {};
  $self->{'ordering_cache'}      = {};
  $self->{'handled_scopes'}      = {};
  $self->{'scoped_basedir'}      = undef;
  $self->{'current_aggregated'}  = undef;

  ## These are static throughout processing
  $self->{'coexistence'}         = $self->requires_make_coexistence() ? 1 : $makeco;
  $self->{'for_eclipse'}         = $foreclipse;
  $self->{'workers'}             = $workers;
  $self->{'generate_dot'}        = $gendot;
  $self->{'generate_ins'}        = $genins;
  $self->{'verbose_ordering'}    = $self->default_verbose_ordering();
  $self->{'wctype'}              = $self->extractType("$self");
  $self->{'workspace_comments'}  = $comments;

  if (defined $$exclude[0]) {
    my $type = $self->{'wctype'};
    if (!defined $self->{'exclude'}->{$type}) {
      $self->{'exclude'}->{$type} = [];
    }
    push(@{$self->{'exclude'}->{$type}}, @$exclude);
    $self->{'orig_exclude'} = $self->{'exclude'};
  } else {
    $self->{'orig_exclude'} = {};
  }

  ## Add a hash reference for our workspace type
  if (!defined $previous_workspace_name{$self->{'wctype'}}) {
    $previous_workspace_name{$self->{'wctype'}} = {};
  }

  ## Warn users about unnecessary options
  if ($self->get_hierarchy() && $self->workspace_per_project()) {
    $self->warning("The -hierarchy option is unnecessary " .
                   "for the " . $self->{'wctype'} . " type.");
  }
  if ($self->{'coexistence'} && !$self->supports_make_coexistence()) {
    $self->warning("Using the -make_coexistence option has " .
                   "no effect on the " . $self->{'wctype'} . " type.");
  }

  ## multi-process config
  $num_workers = $workers if $workers > $num_workers;
  $wdir = $workers_dir;
  $wport = $workers_port;

  return $self;
}


sub default_cacheok {
  return 1;
}

sub set_verbose_ordering {
  my($self, $value) = @_;
  $self->{'verbose_ordering'} = $value;
}


sub modify_assignment_value {
  ## Workspace assignments do not need modification.
  return $_[2];
}


sub parse_line {
  my($self, $ih, $line, $flags) = @_;
  my($status, $error, @values) = $self->parse_known($line, $ih);

  ## Was the line recognized?
  if ($status && defined $values[0]) {
    if ($values[0] eq $self->{'grammar_type'}) {
      my $name = $values[1];
      if (defined $name && $name eq '}') {
        if (!defined $self->{'reading_parent'}->[0]) {
          ## Fill in all the default values
          $self->generate_defaults();

          ## End of workspace; Have subclass write out the file
          ## Generate the project files
          my($gstat, $creator, $err);
          if ($num_workers > 0) {
            if (!defined ($wport)) {
              ## use temp files for multiprocess mpc
              ## Lock the temp directory before generating project files.
              my $lock = 'mpc-worker.lock';

              ## check for valid temp directory
              if (!$wdir) {
                if ($^O eq 'MSWin32') {
                  $wdir = $ENV{TEMP};
                }
                else {
                  $wdir = '/tmp/mpc';
                }
              }

              ## shouldn't happen
              if (!$wdir) {
                die "Error: No temporary directory found. Supply one with \"-worker_dir\" option.\n";
              }

              $self->diagnostic("Multiprocess MPC using \"$wdir\" for temporary files.");

              unless (-d $wdir) {
                mkdir $wdir || die "Error: Can't find or create directory $wdir\n"
              }

              ## lock the directory
              if (-e "$wdir/$lock") {
		die "Error: Another instance of MPC is using $wdir, or a previous session failed to remove the lock file $lock\n";
              }
              else {
		open (FDL, ">$wdir/$lock") || die "Error reating lock file $lock in $wdir\n";
		print FDL "File generated by MPC process ", $$, " on ", scalar (localtime(time())), "\n";
                close FDL;

                $self->diagnostic("Multiprocess MPC created lock file $wdir/$lock");
              }

              ## generate the project files
              ($gstat, $creator, $err) = $self->generate_project_files_fork();

              ## Release temp directory lock;
              if (!unlink("$wdir/$lock")) {
                $self->error("Multiprocess MPC unable to remove lock file $wdir/$lock");
              }
              else {
                $self->diagnostic("Multiprocess MPC removed $wdir/$lock");
              }

            } else {
              ## Socket-based Multiprocess MPC
              ($gstat, $creator, $err) = $self->generate_project_files_fork_socket();
            }
          } else {
            ($gstat, $creator, $err) = $self->generate_project_files();
          }
          if ($gstat) {
            #exit(1);
            ($status, $error) = $self->write_workspace($creator, 1);
            $self->{'assign'} = {};
          } else {
            $error = $err;
            $status = 0;
          }

          $self->{'modified_count'} = 0;
          $self->{'workspace_name'} = undef;
          $self->{'projects'}       = [];
          $self->{'project_info'}   = {};
          $self->{'project_files'}  = [];
          $self->{'exclude'}        = $self->{'orig_exclude'};
          $self->{'associated'}     = {};
          $self->{'scoped_assign'}  = {};
          $self->{'aggregated_mpc'} = {};
          $self->{'aggregated_assign'} = {};
          $self->{'mpc_to_output'}  = {};
        }
        $self->{$self->{'type_check'}} = 0;
      } else {
        ## Workspace Beginning
        ## Deal with the inheritance hierarchy first
        if (defined $values[2]) {
          foreach my $parent (@{$values[2]}) {
            ## Read in the parent onto ourself
            my $file = $self->search_include_path("$parent.$wsbase");
            if (!defined $file) {
              $file = $self->search_include_path("$parent.$wsext");
            }

            if (defined $file) {
              push(@{$self->{'reading_parent'}}, 1);
              $status = $self->parse_file($file);
              pop(@{$self->{'reading_parent'}});

              $error = "Invalid parent: $parent" if (!$status);
            } else {
              $status = 0;
              $error = "Unable to locate parent: $parent";
            }
          }
        }

        ## Set up some initial values
        if (defined $name) {
          if ($name =~ /[\/\\]/) {
            $status = 0;
            $error = 'Workspaces can not have a slash ' .
              'or a back slash in the name';
          } else {
            $name =~ s/^\(\s*//;
            $name =~ s/\s*\)$//;

            ## Replace any *'s with the default name
            if (index($name, '*') >= 0) {
              $name = $self->fill_type_name(
                                            $name,
                                            $self->get_default_workspace_name());
            }

            $self->{'workspace_name'} = $name;
          }
        }
        $self->{$self->{'type_check'}} = 1;
      }
    } elsif ($values[0] eq '0') {
      if (defined $validNames{$values[1]}) {
        $self->process_assignment($values[1], $values[2], $flags);
      } else {
        $error = "Invalid assignment name: '$values[1]'";
        $status = 0;
      }
    } elsif ($values[0] eq '1') {
      if (defined $validNames{$values[1]}) {
        ## This code only runs when there is a non-scoped assignment.  As
        ## such, we can safely replace all environment variables here so
        ## that they are not incorrectly handled in aggregated
        ## workspaces.
        $self->replace_env_vars(\$values[2]) if ($values[2] =~ /\$/);
        $self->process_assignment_add($values[1], $values[2], $flags);
      } else {
        $error = "Invalid addition name: $values[1]";
        $status = 0;
      }
    } elsif ($values[0] eq '-1') {
      if (defined $validNames{$values[1]}) {
        $self->process_assignment_sub($values[1], $values[2], $flags);
      } else {
        $error = "Invalid subtraction name: $values[1]";
        $status = 0;
      }
    } elsif ($values[0] eq 'component') {
      my %copy = %{defined $flags ? $flags : $self->get_assignment_hash()};
      ($status, $error) = $self->parse_scope($ih,
                                             $values[1],
                                             $values[2],
                                             \%validNames,
                                             \%copy);
    } else {
      $error = "Unrecognized line: $line";
      $status = 0;
    }
  } elsif ($status == -1) {
    ## If the line contains a variable, try to replace it with an actual
    ## value.
    $line = $self->relative($line) if (index($line, '$') >= 0);

    foreach my $expfile ($line =~ /[\?\*\[\]]/ ? $self->mpc_glob($line) :
                         $line) {
      if ($expfile =~ /\.$wsext$/) {
        my %copy = %{defined $flags ? $flags : $self->get_assignment_hash()};
        ($status, $error) = $self->aggregated_workspace($expfile, \%copy);
        last if (!$status);
      } else {
        push(@{$self->{'project_files'}}, $expfile);
        $status = 1;
      }
    }
  }

  return $status, $error;
}


sub aggregated_workspace {
  my($self, $file, $flags) = @_;
  my $fh = new FileHandle();

  if (open($fh, $file)) {
    my $oline = $self->get_line_number();
    my $tc    = $self->{$self->{'type_check'}};
    my $ag    = $self->{'handled_scopes'}->{$aggregated};
    my $pca   = $self->{'current_aggregated'};
    my $psbd  = $self->{'scoped_basedir'};
    my $prev_assign = $self->clone($self->get_assignment_hash());
    my($status, $error, @values) = (0, 'No recognizable lines');

    $self->{'handled_scopes'}->{$aggregated} = undef;
    $self->set_line_number(0);
    $self->{$self->{'type_check'}} = 0;
    $self->{'current_aggregated'} = $file;
    $self->{'scoped_basedir'} = $self->mpc_dirname($file);

    ## If the directory name for the file is the current directory, we
    ## need to empty it out.  If we don't, it will cause the file name to
    ## not match up with itself later on where scoped_basedir is used.
    $self->{'scoped_basedir'} = undef if ($self->{'scoped_basedir'} eq '.');

    while (<$fh>) {
      my $line = $self->preprocess_line($fh, $_);
      ($status, $error, @values) = $self->parse_known($line, $fh);

      ## Was the line recognized?
      if ($status) {
        if (defined $values[0]) {
          if ($values[0] eq $self->{'grammar_type'}) {
            if (defined $values[2]) {
              my $name = $self->mpc_basename($file);
              $name =~ s/\.[^\.]+$//;
              $status = 0;
              $error  = 'Aggregated workspace (' . $name .
                ') can not inherit from another workspace';
            } else {
              ($status, $error) = $self->parse_scope($fh,
                                                     '',
                                                     $aggregated,
                                                     \%validNames,
                                                     $flags);
            }
          } else {
            $status = 0;
            $error = 'Unable to aggregate ' . $file;
          }
          last;
        }
      } else {
        last;
      }
    }
    close($fh);

    if ($status) {
      $self->{'aggregated_assign'}->{$file} =
        $self->clone($self->get_assignment_hash());
      $self->{'assign'} = $prev_assign;
    }

    $self->{'scoped_basedir'} = $psbd;
    $self->{'current_aggregated'} = $pca;
    $self->{'handled_scopes'}->{$aggregated} = $ag;
    $self->{$self->{'type_check'}} = $tc;
    $self->set_line_number($oline);

    return $status, $error;
  }

  return 0, 'Unable to open ' . $file;
}


sub parse_scope {
  my($self, $fh, $name, $type, $validNames, $flags, $elseflags) = @_;

  if ($type eq $self->get_default_component_name()) {
    $type = $self->{'wctype'};
  }

  if ($name eq 'exclude') {
    return $self->parse_exclude($fh, $type, $flags);
  } elsif ($name eq 'associate') {
    return $self->parse_associate($fh, $type);
  } elsif ($name eq 'specific') {
    return $self->parse_specific($fh, $type, $validNames, $flags, $elseflags);
  } else {
    return $self->SUPER::parse_scope($fh, $name, $type,
                                     $validNames, $flags, $elseflags);
  }
}

sub process_types {
  my($self, $typestr) = @_;
  my $wcprops         = $self->get_properties();
  my %types;
  my %props;
  @types{split(/\s*,\s*/, $typestr)} = ();

  ## If there is a property in the typestr, i.e., prop:, then
  ## we need to extract it into its own collection while removing
  ## it from the types collection.
  if (index($typestr, 'prop:') >= 0) {
    foreach my $key (keys %types) {
      if ($key =~ /^prop:\s*(\w+)/) {
        ## Add the property to the prop hash.
        $props{$1} = 1;

        ## Remove the original property from the types.
        delete $types{$key};
      } elsif ($key =~ /^!prop:\s*(\w+)/) {
        ## Negate the property.
        $props{$1} = 0;

        ## Remove the original property from the types.
        delete $types{$key};
      }
    }
  }

  ## Now, process the properties and determine if this project
  ## type should be excluded. This will be the case if the property
  ## is valid and there exists a match between the listed properties
  ## and the workspace properties.
  while (my ($key, $val) = each %props) {
    if (exists $$wcprops{$key}) {
      if ($$wcprops{$key} == 1 and $$wcprops{$key} == $val) {
        $types{$self->{wctype}} = 1;
      } else {
        delete $types{$self->{wctype}};
      }
    } elsif ($val == 0) {
      $types{$self->{wctype}} = 1;
    }
  }

  ## Remove all negated types from the collection.
  foreach my $key (keys %types) {
    if ($key =~ /^!\s*(\w+)/) {
      if ($1 eq $self->{wctype}) {
        ## Remove the negated key
        delete $types{$key};

        ## Then delete the key that was negated in the exclusion
        delete $types{$1};
      }
    }
  }

  return \%types;
}

sub parse_exclude {
  my($self, $fh, $typestr, $flags) = @_;
  my $status          = 0;
  my $errorString     = 'Unable to process exclude';
  my $negated         = (index($typestr, '!') >= 0);
  my $types           = $self->process_types($typestr);
  my $count           = 1;
  my @exclude;

  if (exists $$types{$self->{wctype}}) {
    while (<$fh>) {
      my $line = $self->preprocess_line($fh, $_);

      if ($line eq '') {
      } elsif ($line =~ /^}(.*)$/) {
        --$count;
        if (defined $1 && $1 ne '') {
          $status = 0;
          $errorString = "Trailing characters found: '$1'";
        } else {
          $status = 1;
          $errorString = undef;
        }
        last if ($count == 0);
      } else {
        if ($line =~ /^(\w+)\s*(\([^\)]+\))?\s*{$/) {
          ++$count;
        } elsif ($self->parse_assignment($line, [], $fh)) {
          ## Ignore all assignments
        } else {
          if ($line =~ /^"([^"]+)"$/) {
            $line = $1;
          }

          ## If the line contains a variable, try to replace it with an
          ## actual value.
          $line = $self->relative($line) if (index($line, '$') >= 0);

          if (defined $self->{'scoped_basedir'} &&
              $self->path_is_relative($line)) {
            $line = $self->{'scoped_basedir'} . '/' . $line;
          }
          if ($line =~ /[\?\*\[\]]/) {
            push(@exclude, $self->mpc_glob($line));
          } else {
            push(@exclude, $line);
          }
        }
      }
    }

    foreach my $type (keys %$types) {
      if (!defined $self->{'exclude'}->{$type}) {
        $self->{'exclude'}->{$type} = [];
      }
      push(@{$self->{'exclude'}->{$type}}, @exclude);
    }
  } else {
    if ($negated) {
      ($status, $errorString) = $self->SUPER::parse_scope($fh,
                                                          'exclude',
                                                          $typestr,
                                                          \%validNames,
                                                          $flags);
    } else {
      ## If this exclude block didn't match the current type and the
      ## exclude wasn't negated, we need to eat the exclude block so that
      ## these lines don't get included into the workspace.
      while (<$fh>) {
        my $line = $self->preprocess_line($fh, $_);

        if ($line =~ /^(\w+)\s*(\([^\)]+\))?\s*{$/) {
          ++$count;
        } elsif ($line =~ /^}(.*)$/) {
          --$count;
          if (defined $1 && $1 ne '') {
            $status = 0;
            $errorString = "Trailing characters found: '$1'";
          } else {
            $status = 1;
            $errorString = undef;
          }
          last if ($count == 0);
        }
      }
    }
  }

  return $status, $errorString;
}


sub parse_associate {
  my($self, $fh, $assoc_key) = @_;
  my $status      = 0;
  my $errorString = 'Unable to process associate';
  my $count       = 1;
  my @projects;

  if (!defined $self->{'associated'}->{$assoc_key}) {
    $self->{'associated'}->{$assoc_key} = {};
  }

  while (<$fh>) {
    my $line = $self->preprocess_line($fh, $_);

    if ($line eq '') {
    } elsif ($line =~ /^}(.*)$/) {
      --$count;
      if (defined $1 && $1 ne '') {
        $errorString = "Trailing characters found: '$1'";
        last;
      } else {
        $status = 1;
        $errorString = undef;
      }
      last if ($count == 0);
    } else {
      if ($line =~ /^(\w+)\s*(\([^\)]+\))?\s*{$/) {
        ++$count;
      } elsif ($self->parse_assignment($line, [], $fh)) {
        $errorString = 'Assignments are not ' .
          'allowed within an associate scope';
        last;
      } else {
        if ($line =~ /^"([^"]+)"$/) {
          $line = $1;
        }

        ## If the line contains a variable, try to replace it with an
        ## actual value.
        $line = $self->relative($line) if (index($line, '$') >= 0);

        if (defined $self->{'scoped_basedir'} &&
            $self->path_is_relative($line)) {
          $line = $self->{'scoped_basedir'} . '/' . $line;
        }
        if ($line =~ /[\?\*\[\]]/) {
          foreach my $file ($self->mpc_glob($line)) {
            $self->{'associated'}->{$assoc_key}->{$file} = 1;
          }
        } else {
          $self->{'associated'}->{$assoc_key}->{$line} = 1;
        }
      }
    }
  }

  return $status, $errorString;
}


sub parse_specific {
  my($self, $fh, $typestr, $validNames, $flags, $elseflags) = @_;
  my $types   = $self->process_types($typestr);
  my $wctype  = $self->{'wctype'};
  my $matches = exists $types->{$wctype};

  # $elseflags needs to be defined for Creator::parse_scope to allow "} else {"
  $elseflags = {} unless defined $elseflags;

  # Assignments within 'specific' always go to the workspace-level assignment
  # hash table instead of the $flags bound to the scope.
  my $assign = $self->get_assignment_hash();

  return $self->SUPER::parse_scope($fh, 'specific', $matches ? $wctype : undef,
                                   $validNames, $matches ? ($assign, $elseflags)
                                   : (undef, $assign));
}


sub handle_unknown_assignment {
  my $self   = shift;
  my $type   = shift;
  my @values = @_;

  if (defined $type) {
    $self->process_any_assignment(undef, @values);
  }

  return 1, undef;
}


sub excluded {
  my($self, $file) = @_;

  foreach my $excluded (@{$self->{'exclude'}->{$self->{'wctype'}}}) {
    return 1 if ($excluded eq $file || index($file, "$excluded/") == 0);
  }

  return 0;
}


sub handle_scoped_end {
  my($self, $type, $flags) = @_;
  my $status = 1;
  my $error;

  ## Replace instances of $PWD with the current directory plus the
  ## scoped_basedir.  We have to do it now otherwise, $PWD will be the
  ## wrong directory if it's done later.
  if (defined $$flags{'cmdline'} && defined $self->{'scoped_basedir'} &&
      index($$flags{'cmdline'}, '$PWD') >= 0) {
    my $dir = $self->getcwd() . '/' . $self->{'scoped_basedir'};
    $$flags{'cmdline'} =~ s/\$PWD(\W)/$dir$1/g;
    $$flags{'cmdline'} =~ s/\$PWD$/$dir/;
  }

  if ($type eq $aggregated && !defined $self->{'handled_scopes'}->{$type}) {
    ## Go back to the previous directory and add the directory contents
    ($status, $error) = $self->handle_scoped_unknown(undef, $type, $flags, '.');
  }

  $self->{'handled_scopes'}->{$type} = undef;
  return $status, $error;
}


sub handle_scoped_unknown {
  my($self, $fh, $type, $flags, $line) = @_;
  my $status = 1;
  my $error;
  my $dupchk;

  ## If $type is undef, we are in a skipped part of a specific block
  return 1 unless defined $type;

  if ($line =~ /^\w+.*{/) {
    if (defined $fh) {
      my @values;
      my $tc = $self->{$self->{'type_check'}};
      $self->{$self->{'type_check'}} = 1;
      ($status, $error, @values) = $self->parse_line($fh, $line, $flags);
      $self->{$self->{'type_check'}} = $tc;
    } else {
      $status = 0;
      $error  = 'Unhandled line: ' . $line;
    }
    return $status, $error;
  }

  ## If the line contains a variable, try to replace it with an actual
  ## value.
  if (index($line, '$') >= 0) {
    $line = $self->relative($line);
  } elsif (defined $self->{'scoped_basedir'}) {
    if ($self->path_is_relative($line)) {
      if ($line eq '.') {
        $line = $self->{'scoped_basedir'};
      } else {
        ## This is a relative path and the project may have been added
        ## previously without a relative path.  We need to convert the
        ## relative path into an absolute path and, if possible, remove
        ## the current working directory from the front.  This will get
        ## it down to a path that's relative to the current directory and
        ## likely to match up with the addition of this file or directory
        ## from an upper workspace.
        my $cwd = $self->getcwd();
        $line = Cwd::abs_path($self->{'scoped_basedir'} . "/$line");
        if (index($line, $cwd) == 0) {
          $line = substr($line, length($cwd) + 1);
        }
      }
    }
  }

  ## We must build up the list of project files and use them as the
  ## keys in the duplicate hash check.  We need to call
  ## search_for_files() because the user may have just listed
  ## directories in the workspace and we need to deal with mpc files.
  my @files;
  $self->search_for_files($self->{'project_files'}, \@files);
  my %dup;
  @dup{@files} = ();
  $dupchk = \%dup;

  ## If the aggregated workspace contains a scope (other than exclude)
  ## it will be processed in the block above and we will eventually get
  ## here, but by that time $type will no longer be $aggregated.  So,
  ## we just need to set it here to ensure that we don't add everything
  ## in the scoped_basedir directory in handle_scoped_end()
  $self->{'handled_scopes'}->{$aggregated} = 1;

  if (-d $line) {
    my @files;
    $self->search_for_files([ $line ], \@files, $$flags{'implicit'});

    ## If we are generating implicit projects within a scope, then
    ## we need to remove directories and the parent directories for which
    ## there is an mpc file.  Otherwise, the projects will be added
    ## twice.
    if ($$flags{'implicit'}) {
      my %remove;
      foreach my $file (@files) {
        if ($file =~ /\.mpc$/) {
          my $exc = $file;
          do {
            $exc = $self->mpc_dirname($exc);
            $remove{$exc} = 1;
          } while ($exc ne '.' && $exc !~ /[a-z]:[\/\\]/i);
        }
      }

      my @acceptable;
      foreach my $file (@files) {
        push(@acceptable, $file) if (!defined $remove{$file});
      }
      @files = @acceptable;
    }

    foreach my $file (@files) {
      $self->add_aggregated_mpc($file, $dupchk, $flags);
    }
  } else {
    foreach my $expfile ($line =~ /[\?\*\[\]]/ ? $self->mpc_glob($line) :
                         $line) {
      if ($expfile =~ /\.$wsext$/) {
        ## An aggregated workspace within an aggregated workspace or scope.
        ($status, $error) = $self->aggregated_workspace($expfile, $flags);
        last if (!$status);
      } else {
        $self->add_aggregated_mpc($expfile, $dupchk, $flags);
      }
    }
  }
  $self->{'handled_scopes'}->{$type} = 1;

  return $status, $error;
}


sub add_aggregated_mpc {
  my($self, $file, $dupchk, $flags) = @_;
  if (!$self->excluded($file)) {
    if (defined $dupchk && exists $$dupchk{$file}) {
      $self->information("Duplicate mpc file ($file) added by an " .
                         'aggregate workspace.  It will be ignored.');
    } else {
      $self->{'scoped_assign'}->{$file} = $flags;
      push(@{$self->{'project_files'}}, $file);
      push(@{$self->{'aggregated_mpc'}->{$self->{'current_aggregated'}}},
           $file) if defined $self->{'current_aggregated'};
    }
  }
}


sub search_for_files {
  my($self, $files, $array, $impl) = @_;
  my $excluded = 0;

  foreach my $file (@$files) {
    if (-d $file) {
      my @f = $self->generate_default_file_list(
                                                $file,
                                                $self->{'exclude'}->{$self->{'wctype'}},
                                                \$excluded);
      $self->search_for_files(\@f, $array, $impl);
      if ($impl) {
        $file =~ s/^\.\///;

        # Strip out ^ symbols
        $file =~ s/\^//g if ($onVMS);

        unshift(@$array, $file);
      }
    } elsif ($file =~ /\.mpc$/) {
      $file =~ s/^\.\///;

      # Strip out ^ symbols
      $file =~ s/\^//g if ($onVMS);

      unshift(@$array, $file);
    }
  }

  return $excluded;
}


sub remove_duplicate_projects {
  my($self, $list) = @_;
  my $count = scalar(@$list);

  for (my $i = 0; $i < $count; ++$i) {
    my $file = $$list[$i];
    foreach my $inner (@$list) {
      if ($file ne $inner &&
          $file eq $self->mpc_dirname($inner) && ! -d $inner) {
        splice(@$list, $i, 1);
        --$count;
        --$i;
        last;
      }
    }
  }
}


sub generate_default_components {
  my($self, $files, $impl, $excluded) = @_;
  my $pjf = $self->{'project_files'};

  if (defined $$pjf[0]) {
    ## If we have files, then process directories
    my @built;
    foreach my $file (@$pjf) {
      if (!$self->excluded($file)) {
        if (-d $file) {
          my @found;
          my @gen = $self->generate_default_file_list(
                                                      $file,
                                                      $self->{'exclude'}->{$self->{'wctype'}});
          $self->search_for_files(\@gen, \@found, $impl);
          push(@built, @found);
          if ($impl || $self->{'scoped_assign'}->{$file}->{'implicit'}) {
            push(@built, $file);
          }
        } else {
          push(@built, $file);
        }
      }
    }

    ## If the workspace is set to implicit remove duplicates from this
    ## list.
    $self->remove_duplicate_projects(\@built) if ($impl);

    ## Set the project files
    $self->{'project_files'} = \@built;
  } else {
    ## Add all of the wanted files in this directory
    ## and in the subdirectories.
    $excluded |= $self->search_for_files($files, $pjf, $impl);

    ## If the workspace is set to implicit remove duplicates from this
    ## list.
    $self->remove_duplicate_projects($pjf) if ($impl);

    ## If no files were found, then we push the empty
    ## string, so the Project Creator will generate
    ## the default project file.
    push(@$pjf, '') if (!defined $$pjf[0] && !$excluded);
  }
}


sub get_default_workspace_name {
  my $self = shift;
  my $name = $self->{'current_input'};

  if ($name eq '') {
    $name = $self->base_directory();
  } else {
    ## Since files on UNIX can have back slashes, we transform them
    ## into underscores.
    $name =~ s/\\/_/g;

    ## Take off the extension
    $name =~ s/\.[^\.]+$//;
  }

  return $name;
}


sub generate_defaults {
  my $self = shift;

  ## Generate default workspace name
  if (!defined $self->{'workspace_name'}) {
    $self->{'workspace_name'} = $self->get_default_workspace_name();
  }

  ## Modify the exclude list if we have changed directory from the original
  ## starting directory.  Just take off the difference from the front.
  my @original;
  my $top   = $self->getcwd() . '/';
  my $start = $self->getstartdir() . '/';

  if ($start ne $top && $top =~ s/^$start//) {
    foreach my $exclude (@{$self->{'exclude'}->{$self->{'wctype'}}}) {
      push(@original, $exclude);
      $exclude =~ s/^$top//;
    }
  }

  my $excluded = 0;
  my @files = $self->generate_default_file_list(
                                                '.',
                                                $self->{'exclude'}->{$self->{'wctype'}},
                                                \$excluded);

  ## Generate default components
  $self->generate_default_components(\@files,
                                     $self->get_assignment('implicit'),
                                     $excluded);

  ## Return the actual exclude list of we modified it
  if (defined $original[0]) {
    $self->{'exclude'}->{$self->{'wctype'}} = \@original;
  }
}


sub get_workspace_name {
  return $_[0]->{'workspace_name'};
}


sub get_current_output_name {
  return $_[0]->{'current_output'};
}


sub write_and_compare_file {
  my($self, $outdir, $oname, $func, @params) = @_;
  my $fh    = new FileHandle();
  my $status = 1;
  my $errorString = undef;

  ## Set the output directory if one wasn't provided
  $outdir = $self->get_outdir() if (!defined $outdir);

  ## Create the full name and pull off the directory.  The directory
  ## portion may not be the same as $outdir, since $name could possibly
  ## contain a directory portion too.
  my $name = "$outdir/$oname";
  my $dir  = $self->mpc_dirname($name);

  ## Make the full path if necessary
  mkpath($dir, 0, 0777) if ($dir ne '.');

  ## Set the current output data member to our file's full name
  $self->{'current_output'} = $name;

  if ($self->compare_output()) {
    ## First write the output to a temporary file
    my $tmp = "$outdir/MWC$>.$$";
    my $different = 1;
    if (open($fh, ">$tmp")) {
      ($status, $errorString) = &$func($self, $fh, @params);
      close($fh);

      $different = 0 if ($status && !$self->files_are_different($name, $tmp));
    } else {
      $status = 0;
      $errorString = "Unable to open $tmp for output.";
    }

    if ($status) {
      if ($different) {
        unlink($name);

        if (!rename($tmp, $name)) {
          $status = 0;
          $errorString = "Unable to open $name for output";
        }
      } else {
        ## There is no need to rename, so remove our temp file.
        unlink($tmp);
      }
    }
  } else {
    if (open($fh, ">$name")) {
      &$func($self, $fh, @params);
      close($fh);
    } else {
      $status = 0;
      $errorString = "Unable to open $name for output.";
    }
  }

  return $status, $errorString;
}

sub write_workspace {

  my($self, $creator, $addfile) = @_;
  my $status = 1;
  my $errorString;
  my $duplicates = 0;

  if ($self->get_toplevel()) {

    ## There is usually a progress indicator callback provided, but if
    ## the output is being redirected, there will be no progress
    ## indicator.
    my $progress = $self->get_progress_callback();
    &$progress() if (defined $progress);

    if ($addfile) {

      ## To be consistent across multiple project types, we disallow
      ## duplicate project names for all types, not just VC6.
      ## Note that these name are handled case-insensitive by VC6
      my %names;
      foreach my $project (@{$self->{'projects'}}) {
        my $name = lc($self->{'project_info'}->{$project}->[ProjectCreator::PROJECT_NAME]);
        if (defined $names{$name}) {
          ++$duplicates;
          $self->error("Duplicate case-insensitive project '$name'. " .
                       "Look in " . $self->mpc_dirname($project) .
                       " and " . $self->mpc_dirname($names{$name}) .
                       " for project name conflicts.");
        } else {
          $names{$name} = $project;
        }
      }
    } else {
      $self->{'per_project_workspace_name'} = 1;
    }

    my $name = $self->transform_file_name($self->workspace_file_name());

    my $abort_creation = 0;
    if ($duplicates > 0) {
      $abort_creation = 1;
      $errorString = "Duplicate case-insensitive project names are " .
        "not allowed within a workspace.";
      $status = 0;
    } else {
      if (!defined $self->{'projects'}->[0]) {
        $self->information('No projects were created.');
        $abort_creation = 1;
      }
    }

    if (!$abort_creation) {
      ## Verify and possibly modify the dependencies
      if ($addfile) {
        $self->verify_build_ordering();
      }

      if ($addfile || !$self->file_written($name)) {
        ($status, $errorString) = $self->write_and_compare_file(
                                                                undef, $name,
                                                                sub {
                                                                  my($self, $fh) = @_;
                                                                  $self->pre_workspace($fh, $creator, $addfile);
                                                                  my($status, $errorString) = $self->write_comps($fh, $creator, $addfile);
                                                                  ## If write_comps() does't return a status, set status to true.
                                                                  $status = 1 if (!defined $status || $status eq "");
                                                                  if ($status) {
                                                                    my $wsHelper = WorkspaceHelper::get($self);
                                                                    $wsHelper->perform_custom_processing($fh, $creator, $addfile);

                                                                    $self->post_workspace($fh, $creator, $addfile);
                                                                  }
                                                                  return $status, $errorString;
                                                                });
        $self->add_file_written($name) if ($status && $addfile);
      }

      my $additional = $self->get_additional_output();
      foreach my $entry (@$additional) {
        ($status, $errorString) = $self->write_and_compare_file(@$entry);
        if (!$status) {
          last;
        }
      }

      if ($addfile && $self->{'generate_dot'}) {
        my $dh     = new FileHandle();
        my $wsname = $self->get_workspace_name();
        if (open($dh, ">$wsname.dot")) {
          my %targnum;
          my @list = $self->number_target_deps($self->{'projects'},
                                               $self->{'project_info'},
                                               \%targnum, 0);
          print $dh "digraph $wsname {\n";
          foreach my $project (@{$self->{'projects'}}) {
            if (defined $targnum{$project}) {
              my $pname = $self->{'project_info'}->{$project}->[ProjectCreator::PROJECT_NAME];
              foreach my $number (@{$targnum{$project}}) {
                print $dh "  $pname -> ",
                  $self->{'project_info'}->{$list[$number]}->[ProjectCreator::PROJECT_NAME],
                    ";\n";
              }
            }
          }
          print $dh "}\n";
          close($dh);
        } else {
          $self->warning("Unable to write to $wsname.dot.");
        }
      }
    }

    $self->{'per_project_workspace_name'} = undef if (!$addfile);
  }

  return $status, $errorString;
}


sub save_project_info {
  my($self, $gen, $gpi, $gll, $dir, $projects, $pi, $ll) = @_;
  my $c = 0;

  ## For each file written
  foreach my $pj (@$gen) {
    ## Save the full path to the project file in the array
    my $full = ($dir ne '.' ? "$dir/" : '') . $pj;
    push(@$projects, $full);

    ## Get the corresponding generated project info and save it
    ## in the hash map keyed on the full project file name
    $$pi{$full} = $$gpi[$c];
    $c++;
  }

  foreach my $key (keys %$gll) {
    $$ll{$key} = $$gll{$key};
  }
}


sub topname {
  my($self, $file) = @_;
  my $dir  = '.';
  my $rest = $file;
  if ($file =~ /^([^\/\\]+)[\/\\](.*)/) {
    $dir  = $1;
    $rest = $2;
    $dir .= '/' if ($dir =~ /^[a-z]:$/i);
  }
  return $dir, $rest;
}


sub generate_hierarchy {
  my($self, $creator, $origproj, $originfo) = @_;
  my $current;
  my @saved;
  my %sinfo;
  my $cwd = $self->getcwd();
  my $status = 1;
  my $errorString;

  ## Make a copy of these.  We will be modifying them.
  ## It is necessary to sort the projects to get the correct ordering.
  ## Projects in the current directory must come before projects in
  ## other directories.
  my @projects  = sort { return $self->sort_projects_by_directory($a, $b) + 0;
                       } @{$origproj};
  my %projinfo  = %{$originfo};

  foreach my $prj (@projects) {
    my($top, $rest) = $self->topname($prj);

    if (!defined $current) {
      $current = $top;
      push(@saved, $rest);
      $sinfo{$rest} = $projinfo{$prj};
    } elsif ($top ne $current) {
      if ($current ne '.') {
        ## Write out the hierachical workspace
        $self->cd($current);
        ($status, $errorString) = $self->generate_hierarchy($creator, \@saved, \%sinfo);

        $self->{'projects'}       = \@saved;
        $self->{'project_info'}   = \%sinfo;
        $self->{'workspace_name'} = $self->base_directory();
        ($status, $errorString) = $self->write_workspace($creator) if ($status);

        last if !$status;
        $self->cd($cwd);
      }

      ## Start the next one
      $current = $top;
      @saved = ($rest);
      %sinfo = ();
      $sinfo{$rest} = $projinfo{$prj};
    } else {
      push(@saved, $rest);
      $sinfo{$rest} = $projinfo{$prj};
    }
  }
  if ($status && defined $current && $current ne '.') {
    $self->cd($current);
    ($status, $errorString) = $self->generate_hierarchy($creator, \@saved, \%sinfo);

    $self->{'projects'}       = \@saved;
    $self->{'project_info'}   = \%sinfo;
    $self->{'workspace_name'} = $self->base_directory();
    ($status, $errorString) = $self->write_workspace($creator) if ($status);

    $self->cd($cwd);
  }

  return $status, $errorString;
}

sub generate_project_files {
  my $self      = shift;
  my $status = (scalar @{$self->{'project_files'}} == 0 ? 1 : 0);
  my @projects;
  my %pi;
  my %liblocs;
  my $creator = $self->project_creator();
  my $cwd = $self->getcwd();
  my $impl = $self->get_assignment('implicit');
  my $postkey = $creator->get_dynamic() .
    $creator->get_static() . "-$self";
  my $previmpl  = $impl;
  my $prevcache = $self->{'cacheok'};
  my %gstate    = $creator->save_state();
  my $genimpdep = $self->generate_implicit_project_dependencies();
  my $errorString;

  $Data::Dumper::Indent = 0;

  ## Save this project creator setting for later use in the
  ## number_target_deps() method.
  $self->{'dependency_is_filename'} = $creator->dependency_is_filename();

  ## Remove the address portion of the $self string
  $postkey =~ s/=.*//;

  ## Set the source file callback on our project creator
  $creator->set_source_listing_callback([\&source_listing_callback, $self]);

  foreach my $ofile (@{$self->{'project_files'}}) {
    if (!$self->excluded($ofile)) {
      my $file    = $ofile;
      my $dir     = $self->mpc_dirname($file);
      my $restore = 0;

      if (defined $self->{'scoped_assign'}->{$ofile}) {
        ## Handle the implicit assignment
        my $oi = $self->{'scoped_assign'}->{$ofile}->{'implicit'};
        if (defined $oi) {
          $previmpl = $impl;
          $impl     = $oi;
        }

        ## Handle the cmdline assignment
        my $cmdline = $self->{'scoped_assign'}->{$ofile}->{'cmdline'};
        if (defined $cmdline && $cmdline ne '') {
          ## Save the cacheok value
          $prevcache = $self->{'cacheok'};

          ## Get the current parameters and process the command line
          my %parameters = $self->current_parameters();
          $self->process_cmdline($cmdline, \%parameters);

          ## Set the parameters on the creator
          $creator->restore_state(\%parameters);
          $restore = 1;
        }
      }

      ## If we are generating implicit projects and the file is a
      ## directory, then we set the dir to the file and empty the file
      if ($impl && -d $file) {
        $dir  = $file;
        $file = '';

        ## If the implicit assignment value was not a number, then
        ## we will add this value to our base projects.
        if ($impl !~ /^\d+$/) {
          my $bps = $creator->get_baseprojs();
          push(@$bps, split(/\s+/, $impl));
          $restore = 1;
          $self->{'cacheok'} = 0;
        }
      }

      ## Generate the key for this project file
      my $prkey = $self->getcwd() . '/' .
        ($file eq '' ? $dir : $file) . "-$postkey";

      ## We must change to the subdirectory for
      ## which this project file is intended
      if ($self->cd($dir)) {
        my $files_written = [];
        my $gen_proj_info = [];
        my $gen_lib_locs  = {};
        if ($self->{'cacheok'} && defined $allprojects{$prkey}) {
          $files_written = $allprojects{$prkey};
          $gen_proj_info = $allprinfo{$prkey};
          $gen_lib_locs  = $allliblocs{$prkey};

          $status = 1;
        } else {
	  $status = $creator->generate($self->mpc_basename($file));

          ## If any one project file fails, then stop
          ## processing altogether.
          if (!$status) {
            ## We don't restore the state before we leave,
            ## but that's ok since we will be exiting right now.
            return $status, $creator,
              "Unable to process " . ($file eq '' ? " in $dir" : $file);
          }

          ## Get the individual project information and
          ## generated file name(s)
          $files_written = $creator->get_files_written();
          $gen_proj_info = $creator->get_project_info();
          $gen_lib_locs  = $creator->get_lib_locations();

          if ($self->{'cacheok'}) {
            $allprojects{$prkey} = $files_written;
            $allprinfo{$prkey}   = $gen_proj_info;
            $allliblocs{$prkey}  = $gen_lib_locs;
          }

          push(@{$self->{'mpc_to_output'}->{$ofile}}, @$files_written);
        }
        $self->cd($cwd);
        $self->save_project_info($files_written, $gen_proj_info,
                                 $gen_lib_locs, $dir,
                                 \@projects, \%pi, \%liblocs);
      } else {
        ## Unable to change to the directory.
        ## We don't restore the state before we leave,
        ## but that's ok since we will be exiting soon.
        return 0, $creator, "Unable to change directory to $dir";
      }

      ## Return things to the way they were
      $impl = $previmpl if (defined $self->{'scoped_assign'}->{$ofile});
      if ($restore) {
        $self->{'cacheok'} = $prevcache;
        $creator->restore_state(\%gstate);
      }
    } else {
      ## This one was excluded, so status is ok
      $status = 1;
    }

  }

  ## Add implict project dependencies based on source files
  ## that have been used by multiple projects.  If we do it here
  ## before we call generate_hierarchy(), we don't have to call it
  ## in generate_hierarchy() for each workspace.
  $self->{'projects'}     = \@projects;
  $self->{'project_info'} = \%pi;

  if ($status && $genimpdep) {
    $self->add_implicit_project_dependencies($creator, $cwd);
  }

  ## If we are generating the hierarchical workspaces, then do so
  $self->{'lib_locations'} = \%liblocs;
  if ($self->get_hierarchy() || $self->workspace_per_project()) {
    my $orig = $self->{'workspace_name'};
    ($status, $errorString) = $self->generate_hierarchy($creator, \@projects, \%pi);
    $self->{'workspace_name'} = $orig;
  }

  ## Reset the projects and project_info
  $self->{'projects'}      = \@projects;
  $self->{'project_info'}  = \%pi;

  return $status, $creator, $errorString;
}

sub generate_project_files_fork {
  my $self   = shift;
  my $status = (scalar @{$self->{'project_files'}} == 0 ? 1 : 0);

  my @projects;
  my %pi;
  my %liblocs;

  my $creator = $self->project_creator('child');
  my $cwd = $self->getcwd();
  my $impl = $self->get_assignment('implicit');
  my $postkey = $creator->get_dynamic() .
    $creator->get_static() . "-$self";
  my $previmpl  = $impl;
  my $prevcache = $self->{'cacheok'};
  my %gstate    = $creator->save_state();
  my $genimpdep = $self->generate_implicit_project_dependencies();
  my $errorString;

  my @save;
  my $VAR1;
  $Data::Dumper::Indent = 0;

  ## Save this project creator setting for later use in the
  ## number_target_deps() method.
  $self->{'dependency_is_filename'} = $creator->dependency_is_filename();

  ## Remove the address portion of the $self string
  $postkey =~ s/=.*//;

  ## Set the source file callback on our project creator
  $creator->set_source_listing_callback([\&source_listing_callback, $self]);

  my $pid;
  my @pids;
  my $tmp = 'mpctmp00000000';

  ## remove old temp files
  my @tmpfiles = glob "${wdir}/mpctmp*";
  for my $file (@tmpfiles) {
    unlink $file || die "Error: Unable to delete tmp file $file in directory $wdir";
  }

  my $num_tmp_files = scalar (@tmpfiles);

  $self->diagnostic("Multiprocess MPC removed $num_tmp_files existing files like \"mpctmp\*\" in $wdir.");

  foreach my $ofile (@{$self->{'project_files'}}) {
    if ($#pids + 1 >= $num_workers) {
      waitpid(shift @pids, 0);
    }

    ++$tmp;

    ## open the output file in parent so it can die if there's an error
    open (FD, ">${wdir}/$tmp") || die "Can't open $tmp for write";

    $pid = fork();
    if ($pid != 0) {
      push @pids, $pid;
    } else {
      $self->{'pid'} = 'child';

      if (!$self->excluded($ofile)) {
        my $file    = $ofile;
        my $dir     = $self->mpc_dirname($file);
        my $restore = 0;

        if (defined $self->{'scoped_assign'}->{$ofile}) {
          ## Handle the implicit assignment
          my $oi = $self->{'scoped_assign'}->{$ofile}->{'implicit'};
          if (defined $oi) {
            $previmpl = $impl;
            $impl     = $oi;
          }

          ## Handle the cmdline assignment
          my $cmdline = $self->{'scoped_assign'}->{$ofile}->{'cmdline'};
          if (defined $cmdline && $cmdline ne '') {
            ## Save the cacheok value
            $prevcache = $self->{'cacheok'};

            ## Get the current parameters and process the command line
            my %parameters = $self->current_parameters();
            $self->process_cmdline($cmdline, \%parameters);

            ## Set the parameters on the creator
            $creator->restore_state(\%parameters);
            $restore = 1;
          }
        }

        ## If we are generating implicit projects and the file is a
        ## directory, then we set the dir to the file and empty the file
        if ($impl && -d $file) {
          $dir  = $file;
          $file = '';

          ## If the implicit assignment value was not a number, then
          ## we will add this value to our base projects.
          if ($impl !~ /^\d+$/) {
            my $bps = $creator->get_baseprojs();
            push(@$bps, split(/\s+/, $impl));
            $restore = 1;
            $self->{'cacheok'} = 0;
          }
        }

        ## Generate the key for this project file
        my $prkey = $self->getcwd() . '/' .
          ($file eq '' ? $dir : $file) . "-$postkey";

        ## We must change to the subdirectory for
        ## which this project file is intended

        if ($self->cd($dir)) {
          my $files_written = [];
          my $gen_proj_info = [];
          my $gen_lib_locs  = {};

          if ($self->{'cacheok'} && defined $allprojects{$prkey}) {
            $files_written = $allprojects{$prkey};
            $gen_proj_info = $allprinfo{$prkey};
            $gen_lib_locs  = $allliblocs{$prkey};

            $status = 1;
          } else {
            $status = $creator->generate($self->mpc_basename($file));

            ## If any one project file fails, then stop
            ## processing altogether.
            if (!$status) {
              # save the status info and exit. the parent will
	      # see the error.
	      print FD "$status|Unable to process " .
		($file eq '' ? " in $dir" : $file) . "\n";

	      exit(1);          # child error
            }

            ## Get the individual project information and
            ## generated file name(s)
            $files_written = $creator->get_files_written();
            $gen_proj_info = $creator->get_project_info();
            $gen_lib_locs  = $creator->get_lib_locations();

          }


	  print FD "$status|''|$self->{'cacheok'}|$previmpl|$prevcache\n";
	  print FD "$ofile|$prkey|$dir|$cwd|$restore\n";

	  print FD Dumper ($files_written), "\n";
	  print FD Dumper ($gen_proj_info), "\n";
	  print FD Dumper ($gen_lib_locs), "\n";

	  # there's a callback that sets the project file list
	  # since we can't callback between processes we store
	  # the list for later
	  print FD Dumper ($self->{'project_file_list'}), "\n";

        } else {
          ## Unable to change to the directory.
          ## We don't restore the state before we leave,
          ## but that's ok since we will be exiting soon.
	  print FD "$status|Unable to change directory to $dir\n";

	  exit (1);             # child error
        }

      } else {
        ## This one was excluded, so status is ok
        ## no need to set though since the child will exit.
	#$status = 1;
      }

      exit(0);                  # child is finished
    }
  }

  for $pid (@pids) {
    # this will also reap any zombies
    waitpid($pid, 0);
  }

  my ($msg, $cacheok, $ofile, $prkey, $dir, $restore);

  # read the children's stored data
  my @kid_data = glob "${wdir}/mpctmp*";

  for my $kd (@kid_data) {
    open (FD, "<$kd") || die "Can't open $kd for read";

    ($status, $msg, $cacheok, $previmpl, $prevcache) = split /\|/, <FD>;

    if (!$status) {
      return $status, $creator, $msg;
    }

    ($ofile, $prkey, $dir, $cwd, $restore) = split /\|/, <FD>;

    eval (<FD>);
    my $files_written = $VAR1;

    eval (<FD>);
    my $gen_proj_info = $VAR1;

    # have to reconstitute gen_lib_locs in the same order it was
    # created or else multi-process implicit dependency may differ from
    # single process.
    eval (<FD>);
    my $gen_lib_locs;
    for my $k (sort { substr($a, 0 , index ($a, '|')) <=>
			substr ($b, 0, index ($b, '|')) } keys %$VAR1) {

      $gen_lib_locs->{substr ($k, index ($k, '|') + 1)} =
	$VAR1->{$k};
    }

    # have to reconstitute project_file_list in the same order it was
    # created or else multi-process implicit dependency may differ from
    # single process.
    eval (<FD>);
    for my $k (sort { substr($a, 0 , index ($a, '|')) <=>
			substr ($b, 0, index ($b, '|')) } keys %$VAR1) {

      $self->{'project_file_list'}->{substr ($k, index ($k, '|') + 1)} =
	$VAR1->{$k};
    }

    $self->{'cacheok'} = $cacheok;
    if ($self->cd($dir)) {
      if ($self->{'cacheok'} && defined $allprojects{$prkey}) {

	$files_written = $allprojects{$prkey};
	$gen_proj_info = $allprinfo{$prkey};
	$gen_lib_locs  = $allliblocs{$prkey};
	$status = 1;
      } else {
	# file is already generated. check status
	if (!$status) {

	  ## We don't restore the state before we leave,
	  ## but that's ok since we will be exiting right now.
	  return $status, $creator, $msg;
	}

	## Get the individual project information and
	## generated file name(s)
	if ($self->{'cacheok'}) {

	  $allprojects{$prkey} = $files_written;
	  $allprinfo{$prkey}   = $gen_proj_info;
	  $allliblocs{$prkey}  = $gen_lib_locs;
	}

	push(@{$self->{'mpc_to_output'}->{$ofile}}, @$files_written);
      }

      $self->cd($cwd);
      $self->save_project_info($files_written, $gen_proj_info,
			       $gen_lib_locs, $dir,
			       \@projects, \%pi, \%liblocs);
    } else {

      ## Unable to change to the directory.
      ## We don't restore the state before we leave,
      ## but that's ok since we will be exiting soon.
      return 0, $creator, $msg;

    }

    ## Return things to the way they were
    $impl = $previmpl if (defined $self->{'scoped_assign'}->{$ofile});
    if ($restore) {
      $self->{'cacheok'} = $prevcache;
      $creator->restore_state(\%gstate);
    }
  }

  ## Add implict project dependencies based on source files
  ## that have been used by multiple projects.  If we do it here
  ## before we call generate_hierarchy(), we don't have to call it
  ## in generate_hierarchy() for each workspace.
  $self->{'projects'}     = \@projects;
  $self->{'project_info'} = \%pi;

  if ($status && $genimpdep) {
    #print "aipd: $cwd\n", Dumper ($creator), "\n";
    $self->add_implicit_project_dependencies($creator, $cwd);
  }

  ## If we are generating the hierarchical workspaces, then do so
  $self->{'lib_locations'} = \%liblocs;
  if ($self->get_hierarchy() || $self->workspace_per_project()) {
    my $orig = $self->{'workspace_name'};
    ($status, $errorString) = $self->generate_hierarchy($creator, \@projects, \%pi);
    $self->{'workspace_name'} = $orig;
  }

  ## Reset the projects and project_info
  $self->{'projects'}      = \@projects;
  $self->{'project_info'}  = \%pi;

  return $status, $creator, $errorString;
}

sub send_to_parent {
  my $self = shift;
  my $arr = shift;

  # send the data
  my $sock = new IO::Socket::INET (
                                   PeerAddr => 'localhost',
                                   PeerPort => $wport,
                                   Proto => 'tcp',
                                  );

  if (!defined ($sock)) {
    die "Child could not create socket";
  }

  map { print $sock "$_\n"; } @$arr;
  $sock->close();
}

sub generate_project_files_fork_socket {
  my $self      = shift;
  my $status = (scalar @{$self->{'project_files'}} == 0 ? 1 : 0);

  my @projects;
  my %pi;
  my %liblocs;

  my $creator = $self->project_creator('child');
  my $cwd = $self->getcwd();
  my $impl = $self->get_assignment('implicit');
  my $postkey = $creator->get_dynamic() .
    $creator->get_static() . "-$self";
  my $previmpl  = $impl;
  my $prevcache = $self->{'cacheok'};
  my %gstate    = $creator->save_state();
  my $genimpdep = $self->generate_implicit_project_dependencies();
  my $errorString;

  my @save;
  my $VAR1;
  $Data::Dumper::Indent = 0;

  ## Save this project creator setting for later use in the
  ## number_target_deps() method.
  $self->{'dependency_is_filename'} = $creator->dependency_is_filename();

  ## Remove the address portion of the $self string
  $postkey =~ s/=.*//;

  ## Set the source file callback on our project creator
  $creator->set_source_listing_callback([\&source_listing_callback, $self]);

  my $pid;
  my @pids;
  my @pdata;                    # parents data sent from children.

  ## setup workers' data
  my @wdata;
  my $beg;
  my $fin;

  my $num_prj_files = $#{$self->{'project_files'}} + 1;

  ## reduce the number of workers if necessary
  ## what if $num_workers > SOMAXCONN?? (unlikely)
  if ($num_workers > SOMAXCONN) {
    $self->diagnostic("Multiprocess MPC reducing # workers from $num_workers to " . SOMAXCONN . ", the max # of queued connections");
    $num_workers = SOMAXCONN;
  }

  if ($num_workers > $num_prj_files) {
    # don't fork more workers than there are jobs
    $self->diagnostic("Multiprocess MPC reducing # workers from $num_workers to $num_prj_files, the number of project files.");
    $num_workers = $num_prj_files;
  }

  my $num_per_worker = int ($num_prj_files / $num_workers);
  my $num_lines_per_prj = 6;

  $self->diagnostic("Multiprocess MPC using $num_workers workers to process $num_prj_files project files.");

  for (my $wctr = 0; $wctr < $num_workers; ++$wctr) {
    $beg = $wctr * $num_per_worker;
    $fin = $beg + $num_per_worker - 1;

    @{$wdata[$wctr]} = @{$self->{'project_files'}}[$beg..$fin];
  }

  ## give any remaining data to last worker.
  if ($num_prj_files > $num_per_worker * $num_workers) {
    push @{$wdata[$num_workers - 1]} ,
      @{$self->{'project_files'}}[$num_per_worker
                                  * $num_workers..$#{$self->{'project_files'}}];

  }

  ## Setup listener. Do this before fork so that (in the rare case)
  ## when child tries to send data before the accept(), the socket
  ## is at least initialized.
  my $sock = new IO::Socket::INET (
                                   LocalHost => 'localhost',
                                   LocalPort => $wport,
                                   Proto     => 'tcp',
                                   Listen    => $num_workers,
                                   Reuse     => 1
                                  );
  if (!defined ($sock)) {
    die "Error setting up parent listener";
  }

  ## spawn the workers.
  my $id = 0;
  while ($id < $num_workers) {
    # use pipes as barrier
    $pid = fork();
    if ($pid != 0) {
      push @pids, $pid;
    } else {
      ## after fork, child knows its id and which data to use.
      $self->{'pid'} = 'child';
      last;
    }
    ++$id;
  }

  if ($self->{pid} eq 'parent') {
    $self->diagnostic("Multiprocess MPC using port $wport.");

    # read the data from the kids
    for (my $ctr = 0; $ctr < $num_workers; ++$ctr) {
      my $handle = $sock->accept();
      die "Accept error" if !$handle;
      my $id = <$handle>;
      @{$pdata[$id]} = <$handle>;

      # each project as 6 records
      if ((($#{$pdata[$id]} + 1) / $num_lines_per_prj) != $num_per_worker) {
        if ($#{$pdata[$id]} != 0) {
          # 0 indicates a failed status which will be delt with later
          if (($id == $num_workers - 1) && ((($#{$pdata[$id]} + 1) / $num_lines_per_prj) != $num_per_worker + $#{$self->{'project_files'}} + 1 - ($num_workers * $num_per_worker))) {
            # The last child may have more than num_per_worker records
            my $rec = $#{$pdata[$id]} + 1;
            my $exp = $num_per_worker * $num_lines_per_prj;
            die "There is an error in the child data. Expected $exp. Received $rec";
          }
        }
      }
    }
    # all data has been read
    $sock->close();

  } else {
    ## This is the code the workers run.
    undef $sock;
    ## generate projects
    my @cdata = ($id);
    foreach my $ofile (@{$wdata[$id]}) {
      if (!$self->excluded($ofile)) {
        my $file    = $ofile;
        my $dir     = $self->mpc_dirname($file);
        my $restore = 0;

        if (defined $self->{'scoped_assign'}->{$ofile}) {
          ## Handle the implicit assignment
          my $oi = $self->{'scoped_assign'}->{$ofile}->{'implicit'};
          if (defined $oi) {
            $previmpl = $impl;
            $impl     = $oi;
          }

          ## Handle the cmdline assignment
          my $cmdline = $self->{'scoped_assign'}->{$ofile}->{'cmdline'};
          if (defined $cmdline && $cmdline ne '') {
            ## Save the cacheok value
            $prevcache = $self->{'cacheok'};

            ## Get the current parameters and process the command line
            my %parameters = $self->current_parameters();
            $self->process_cmdline($cmdline, \%parameters);

            ## Set the parameters on the creator
            $creator->restore_state(\%parameters);
            $restore = 1;
          }
        }

        ## If we are generating implicit projects and the file is a
        ## directory, then we set the dir to the file and empty the file
        if ($impl && -d $file) {
          $dir  = $file;
          $file = '';

          ## If the implicit assignment value was not a number, then
          ## we will add this value to our base projects.
          if ($impl !~ /^\d+$/) {
            my $bps = $creator->get_baseprojs();
            push(@$bps, split(/\s+/, $impl));
            $restore = 1;
            $self->{'cacheok'} = 0;
          }
        }

        ## Generate the key for this project file
        my $prkey = $self->getcwd() . '/' .
          ($file eq '' ? $dir : $file) . "-$postkey";

        ## We must change to the subdirectory for
        ## which this project file is intended

        if ($self->cd($dir)) {
          my $files_written = [];
          my $gen_proj_info = [];
          my $gen_lib_locs  = {};

          if ($self->{'cacheok'} && defined $allprojects{$prkey}) {
            $files_written = $allprojects{$prkey};
            $gen_proj_info = $allprinfo{$prkey};
            $gen_lib_locs  = $allliblocs{$prkey};

            $status = 1;
          } else {
            $status = $creator->generate($self->mpc_basename($file));

            ## If any one project file fails, then stop
            ## processing altogether.
            if (!$status) {
              # save the status info and exit. the parent will
              # see the error.
              @cdata = ($id);
              push @cdata, "$status|Unable to process " .
                ($file eq '' ? " in $dir" : $file) . "\n";

              $self->send_to_parent(\@cdata);
              exit(1);          # child error
            }

            ## Get the individual project information and
            ## generated file name(s)
            $files_written = $creator->get_files_written();
            $gen_proj_info = $creator->get_project_info();
            $gen_lib_locs  = $creator->get_lib_locations();

          }

          push @cdata, "$status|''|$self->{'cacheok'}|$previmpl|$prevcache";
          push @cdata, "$ofile|$prkey|$dir|$cwd|$restore";
          push @cdata, Dumper ($files_written);
          push @cdata, Dumper ($gen_proj_info);
          push @cdata, Dumper ($gen_lib_locs);

          # there's a callback that sets the project file list
          # since we can't callback between processes we store
          # the list for later
          push @cdata, Dumper ($self->{'project_file_list'});

          $self->cd($cwd);

        } else {
          ## Unable to change to the directory.
          ## We don't restore the state before we leave,
          ## but that's ok since we will be exiting soon.
          @cdata = ($id);
          push @cdata, "$status|Unable to change directory to $dir\n";
          $self->send_to_parent(\@cdata);

          exit (1);             # child error
        }
      ## Return things to the way they were
      $impl = $previmpl if (defined $self->{'scoped_assign'}->{$ofile});
      if ($restore) {
        $self->{'cacheok'} = $prevcache;
        $creator->restore_state(\%gstate);
      }

      } else {
        ## This one was excluded, so status is ok
        ## no need to set though since the child will exit.
        #$status = 1;
      }

    }

    # send all the data at once.
    $self->send_to_parent(\@cdata);

    exit (0);

    # end of child
  }

  # This is the parent again.

  for $pid (@pids) {
    # this will reap any zombies
    waitpid($pid, 0);
  }

  my ($msg, $cacheok, $ofile, $prkey, $dir, $restore);

  # read the children's stored data
  for (my $i = 0; $i < $num_workers; ++$i) {
    for (my $j = 0; $j <  $#{$pdata[$i]} + 1; ++$j) {
      ($status, $msg, $cacheok, $previmpl, $prevcache) = split /\|/, ${$pdata[$i]}[$j++];

      # check that the child was successful
      if (!$status) {
        return $status, $creator, $msg;
      }

      ($ofile, $prkey, $dir, $cwd, $restore) = split /\|/, ${$pdata[$i]}[$j++];

      eval (${$pdata[$i]}[$j++]);
      my $files_written = $VAR1;

      eval (${$pdata[$i]}[$j++]);
      my $gen_proj_info = $VAR1;

      # have to reconstitute gen_lib_locs in the same order it was
      # created or else multi-process implicit dependency may differ from
      # single process.
      eval (${$pdata[$i]}[$j++]);
      my $gen_lib_locs;
      for my $k (sort { substr($a, 0 , index ($a, '|')) <=>
                          substr ($b, 0, index ($b, '|')) } keys %$VAR1) {

        $gen_lib_locs->{substr ($k, index ($k, '|') + 1)} =
          $VAR1->{$k};
      }

      # have to reconstitute project_file_list in the same order it was
      # created or else multi-process implicit dependency may differ from
      # single process.
      eval (${$pdata[$i]}[$j]);
      for my $k (sort { substr($a, 0 , index ($a, '|')) <=>
                          substr ($b, 0, index ($b, '|')) } keys %$VAR1) {

        $self->{'project_file_list'}->{substr ($k, index ($k, '|') + 1)} =
          $VAR1->{$k};
      }

      $self->{'cacheok'} = $cacheok;
      if ($self->cd($dir)) {
        if ($self->{'cacheok'} && defined $allprojects{$prkey}) {

          $files_written = $allprojects{$prkey};
          $gen_proj_info = $allprinfo{$prkey};
          $gen_lib_locs  = $allliblocs{$prkey};
          $status = 1;
        } else {
          # file is already generated. check status
          if (!$status) {

            ## We don't restore the state before we leave,
            ## but that's ok since we will be exiting right now.
            return $status, $creator, $msg;
          }

          ## Get the individual project information and
          ## generated file name(s)
          if ($self->{'cacheok'}) {

            $allprojects{$prkey} = $files_written;
            $allprinfo{$prkey}   = $gen_proj_info;
            $allliblocs{$prkey}  = $gen_lib_locs;
          }

          push(@{$self->{'mpc_to_output'}->{$ofile}}, @$files_written);
        }

        $self->cd($cwd);
        $self->save_project_info($files_written, $gen_proj_info,
                                 $gen_lib_locs, $dir,
                                 \@projects, \%pi, \%liblocs);
      } else {

        ## Unable to change to the directory.
        ## We don't restore the state before we leave,
        ## but that's ok since we will be exiting soon.
        return 0, $creator, $msg;

      }

      ## Return things to the way they were
      $impl = $previmpl if (defined $self->{'scoped_assign'}->{$ofile});
      if ($restore) {
        $self->{'cacheok'} = $prevcache;
        $creator->restore_state(\%gstate);
      }
    }
  }

  ## Add implict project dependencies based on source files
  ## that have been used by multiple projects.  If we do it here
  ## before we call generate_hierarchy(), we don't have to call it
  ## in generate_hierarchy() for each workspace.
  $self->{'projects'}     = \@projects;
  $self->{'project_info'} = \%pi;

  if ($status && $genimpdep) {
    #print "aipd: $cwd\n", Dumper ($creator), "\n";
    $self->add_implicit_project_dependencies($creator, $cwd);
  }

  ## If we are generating the hierarchical workspaces, then do so
  $self->{'lib_locations'} = \%liblocs;
  if ($self->get_hierarchy() || $self->workspace_per_project()) {
    my $orig = $self->{'workspace_name'};
    ($status, $errorString) = $self->generate_hierarchy($creator, \@projects, \%pi);
    $self->{'workspace_name'} = $orig;
  }

  ## Reset the projects and project_info
  $self->{'projects'}      = \@projects;
  $self->{'project_info'}  = \%pi;

  return $status, $creator, $errorString;
}


sub array_contains {
  my($self, $left, $right) = @_;
  my %check;

  ## Initialize the hash keys with the left side array
  @check{@$left} = ();

  ## Check each element on the right against the left.
  foreach my $r (@$right) {
    return 1 if (exists $check{$r});
  }

  return 0;
}


sub non_intersection {
  my($self, $left, $right, $over) = @_;
  my $status = 0;
  my %check;

  ## Initialize the hash keys with the left side array
  @check{@$left} = ();

  ## Check each element on the right against the left.
  ## Store anything that isn't in the left side in the over array.
  foreach my $r (@$right) {
    if (exists $check{$r}) {
      $status = 1;
    } else {
      push(@$over, $r);
    }
  }
  return $status;
}


sub indirect_dependency {
  my($self, $dir, $ccheck, $cfile) = @_;

  $self->{'indirect_checked'}->{$ccheck} = 1;
  if (index($self->{'project_info'}->{$ccheck}->[ProjectCreator::DEPENDENCIES], $cfile) >= 0) {
    return 1;
  } else {
    my $deps = $self->create_array(
                                   $self->{'project_info'}->{$ccheck}->[ProjectCreator::DEPENDENCIES]);
    foreach my $dep (@$deps) {
      if (defined $self->{'project_info'}->{"$dir$dep"} &&
          !defined $self->{'indirect_checked'}->{"$dir$dep"} &&
          $self->indirect_dependency($dir, "$dir$dep", $cfile)) {
        return 1;
      }
    }
  }

  return 0;
}


sub add_implicit_project_dependencies {
  my($self, $creator, $cwd) = @_;
  my %bidir;
  my %save;

  ## Take the current working directory and regular expression'ize it.
  $cwd = $self->escape_regex_special($cwd);

  ## Look at each projects file list and check it against all of the
  ## others.  If any of the other projects file lists contains anothers
  ## file, then they are dependent (due to build parallelism).  So, we
  ## append the dependency and remove the file in question from the
  ## project so that the next time around the foreach, we don't find it
  ## as a dependent on the one that we just modified.
  my @pflkeys = keys %{$self->{'project_file_list'}};

  foreach my $key (@pflkeys) {
    foreach my $ikey (@pflkeys) {
      ## Not the same project and
      ## The same directory and
      ## We've not already added a dependency to this project
      if ($key ne $ikey &&
          ($self->{'project_file_list'}->{$key}->[1] eq
           $self->{'project_file_list'}->{$ikey}->[1]) &&
          (!defined $bidir{$ikey} ||
           !$self->array_contains($bidir{$ikey}, [$key]))) {
        my @over;
        if ($self->non_intersection(
                                    $self->{'project_file_list'}->{$key}->[2],
                                    $self->{'project_file_list'}->{$ikey}->[2],
                                    \@over)) {
          ## The project contains shared source files, so we need to
          ## look into adding an implicit inter-project dependency.
          $save{$ikey} = $self->{'project_file_list'}->{$ikey}->[2];
          $self->{'project_file_list'}->{$ikey}->[2] = \@over;
          if (defined $bidir{$key}) {
            push(@{$bidir{$key}}, $ikey);
          } else {
            $bidir{$key} = [$ikey];
          }
          my $append = $creator->translate_value('after', $key);
          my $file   = $self->{'project_file_list'}->{$ikey}->[0];
          my $dir    = $self->{'project_file_list'}->{$ikey}->[1];
          my $cfile  = $creator->translate_value('after', $ikey);
          ## Remove our starting directory from the projects directory
          ## to get the right part of the directory to prepend.
          $dir =~ s/^$cwd[\/\\]*//;

          ## Turn the append value into a key for 'project_info' and
          ## prepend the directory to the file.
          my $ccheck = $append;
          $ccheck =~ s/"//g;
          if ($dir ne '') {
            $dir .= '/';
            $ccheck = "$dir$ccheck";
            $file = "$dir$file";
          }

          ## If the append value key contains a reference to the project
          ## that we were going to append the dependency value, then
          ## ignore the generated dependency.  It is redundant and
          ## quite possibly wrong.
          $self->{'indirect_checked'} = {};
          if (defined $self->{'project_info'}->{$file} &&
              (!defined $self->{'project_info'}->{$ccheck} ||
               !$self->indirect_dependency($dir, $ccheck, $cfile))) {
            ## Append the dependency
            $self->{'project_info'}->{$file}->[ProjectCreator::DEPENDENCIES] .= " $append";
          }
        }
      }
    }
  }

  ## Restore the modified values in case this method is called again
  ## which is the case when using the -hierarchy option.
  foreach my $skey (keys %save) {
    $self->{'project_file_list'}->{$skey}->[2] = $save{$skey};
  }
}


sub get_projects {
  return $_[0]->{'projects'};
}


sub get_project_info {
  return $_[0]->{'project_info'};
}


sub get_lib_locations {
  return $_[0]->{'lib_locations'};
}


sub get_first_level_directory {
  my($self, $file) = @_;

  if (($file =~ tr/\///) > 0) {
    my $dir = $file;
    $dir =~ s/^([^\/]+\/).*/$1/;
    $dir =~ s/\/+$//;
    return $dir;
  }

  return '.';
}


sub get_associated_projects {
  return $_[0]->{'associated'};
}


sub sort_within_group {
  my($self, $list, $start, $end) = @_;
  my $deps;
  my %seen;
  my $ccount  = 0;
  my $cmax    = ($end - $start) + 1;
  my $previ   = -1;
  my $prevpjs = [];
  my $movepjs = [];

  ## Put the projects in the order specified
  ## by the project dependencies.
  for (my $i = $start; $i <= $end; ++$i) {
    ## If our moved project equals our previously moved project then
    ## we count this as a possible circular dependency.
    my $key = "@$list";
    if ($seen{$key} ||
        (defined $$movepjs[0] && defined $$prevpjs[0] &&
         $$movepjs[0] == $$prevpjs[0] && $$movepjs[1] == $$prevpjs[1])) {
      ++$ccount;
    } else {
      $ccount = 0;
    }

    ## Detect circular dependencies
    if ($ccount > $cmax) {
      my @prjs;
      foreach my $mvgr (@$movepjs) {
        push(@prjs, $$list[$mvgr]);
      }
      my $other = $$movepjs[0] - 1;
      if ($other < $start || $other == $$movepjs[1] || !defined $$list[$other]) {
        $other = undef;
      }
      $self->warning('Circular dependency detected while processing the ' .
                     ($self->{'current_input'} eq '' ?
                      'default' : $self->{'current_input'}) .
                     ' workspace. ' .
                     'The following projects are involved: ' .
                     (defined $other ? "$$list[$other], " : '') .
                     join(' and ', @prjs));
      return;
    }

    ## Keep track of the previous project movement
    $seen{$key} = 1;
    $prevpjs = $movepjs;
    $movepjs = [] if ($previ < $i);
    $previ = $i;

    $deps = $self->get_validated_ordering($$list[$i]);
    if (defined $$deps[0]) {
      my $baseproj = ($self->{'dependency_is_filename'} ?
                      $self->mpc_basename($$list[$i]) :
                      $self->{'project_info'}->{$$list[$i]}->[ProjectCreator::PROJECT_NAME]);
      my $moved = 0;
      foreach my $dep (@$deps) {
        if ($baseproj ne $dep) {
          ## See if the dependency is listed after this project
          for (my $j = $i + 1; $j <= $end; ++$j) {
            my $ldep = ($self->{'dependency_is_filename'} ?
                        $self->mpc_basename($$list[$j]) :
                        $self->{'project_info'}->{$$list[$j]}->[ProjectCreator::PROJECT_NAME]);
            if ($ldep eq $dep) {
              $movepjs = [$i, $j];
              ## If so, move it in front of the current project.
              ## The original code, which had splices, didn't always
              ## work correctly (especially on AIX for some reason).
              my $save = $$list[$j];
              for (my $k = $j; $k > $i; --$k) {
                $$list[$k] = $$list[$k - 1];
              }
              $$list[$i] = $save;

              ## Mark that an entry has been moved
              $moved = 1;
              $j--;
            }
          }
        }
      }
      --$i if ($moved);
    }
  }
}


sub build_dependency_chain {
  my($self, $name, $len, $list, $ni, $glen, $groups, $map, $gdeps) = @_;
  my $deps = $self->get_validated_ordering($name);

  if (defined $$deps[0]) {
    foreach my $dep (@$deps) {
      ## Find the item in the list that matches our current dependency
      my $mapped = $$map{$dep};
      if (defined $mapped) {
        for (my $i = 0; $i < $len; $i++) {
          if ($$list[$i] eq $mapped) {

            ## Locate the group number to which the dependency belongs
            for (my $j = 0; $j < $glen; $j++) {
              if ($i >= $$groups[$j]->[0] && $i <= $$groups[$j]->[1]) {

                if ($j != $ni) {
                  ## Add every project in the group to the dependency chain
                  for (my $k = $$groups[$j]->[0]; $k <= $$groups[$j]->[1]; $k++) {
                    my $ldep = $self->mpc_basename($$list[$k]);
                    if (!exists $$gdeps{$ldep}) {
                      $$gdeps{$ldep} = 1;
                      $self->build_dependency_chain($$list[$k],
                                                    $len, $list, $j,
                                                    $glen, $groups,
                                                    $map, $gdeps);
                    }
                  }
                }
                last;
              }
            }
            last;
          }
        }
      }

      $$gdeps{$dep} = 1;
    }
  }
}


sub sort_by_groups {
  my($self, $list, $grindex) = @_;
  my @groups = @$grindex;
  my $llen   = scalar(@$list);

  ## Check for duplicates first before we attempt to sort the groups.
  ## If there is a duplicate, we quietly return immediately.  The
  ## duplicates will be flagged as an error when creating the main
  ## workspace.
  my %dupcheck;
  foreach my $proj (@$list) {
    my $base = $self->mpc_basename($proj);
    return undef if (defined $dupcheck{$base});
    $dupcheck{$base} = $proj;
  }

  my %circular_checked;
  for (my $gi = 0; $gi <= $#groups; ++$gi) {
    ## Detect circular dependencies
    if (!$circular_checked{$gi}) {
      $circular_checked{$gi} = 1;
      for (my $i = $groups[$gi]->[0]; $i <= $groups[$gi]->[1]; ++$i) {
        my %gdeps;
        $self->build_dependency_chain($$list[$i], $llen, $list, $gi,
                                      $#groups + 1, \@groups,
                                      \%dupcheck, \%gdeps);
        if (exists $gdeps{$self->mpc_basename($$list[$i])}) {
          ## There was a cirular dependency, get all of the directories
          ## involved.
          my %dirs;
          foreach my $gdep (keys %gdeps) {
            $dirs{$self->mpc_dirname($dupcheck{$gdep})} = 1;
          }

          ## If the current directory was involved, translate that into
          ## a directory relative to the start directory.
          if (defined $dirs{'.'}) {
            my $cwd = $self->getcwd();
            my $start = $self->getstartdir();
            if ($cwd ne $start) {
              my $startre = $self->escape_regex_special($start);
              delete $dirs{'.'};
              $cwd =~ s/^$startre[\\\/]//;
              $dirs{$cwd} = 1;
            }
          }

          ## Display a warining to the user
          my @keys = sort keys %dirs;
          $self->warning('Circular directory dependency detected in the ' .
                         ($self->{'current_input'} eq '' ?
                          'default' : $self->{'current_input'}) .
                         ' workspace. ' .
                         'The following director' .
                         ($#keys == 0 ? 'y is' : 'ies are') .
                         ' involved: ' . join(', ', @keys));
          return;
        }
      }
    }

    ## Build up the group dependencies
    my %gdeps;
    for (my $i = $groups[$gi]->[0]; $i <= $groups[$gi]->[1]; ++$i) {
      my $deps = $self->get_validated_ordering($$list[$i]);
      @gdeps{@$deps} = () if (defined $$deps[0]);
    }

    ## Search the rest of the groups for any of the group dependencies
    for (my $gj = $gi + 1; $gj <= $#groups; ++$gj) {
      for (my $i = $groups[$gj]->[0]; $i <= $groups[$gj]->[1]; ++$i) {
        if (exists $gdeps{$self->mpc_basename($$list[$i])}) {
          ## Move this group ($gj) in front of the current group ($gi)
          my @save;
          for (my $j = $groups[$gi]->[1] + 1; $j <= $groups[$gj]->[1]; ++$j) {
            push(@save, $$list[$j]);
          }
          my $offset = $groups[$gj]->[1] - $groups[$gi]->[1];
          for (my $j = $groups[$gi]->[1]; $j >= $groups[$gi]->[0]; --$j) {
            $$list[$j + $offset] = $$list[$j];
          }
          for (my $j = 0; $j <= $#save; ++$j) {
            $$list[$groups[$gi]->[0] + $j] = $save[$j];
          }

          ## Update the group indices
          my $shiftamt = ($groups[$gi]->[1] - $groups[$gi]->[0]) + 1;
          for (my $j = $gi + 1; $j <= $gj; ++$j) {
            $groups[$j]->[0] -= $shiftamt;
            $groups[$j]->[1] -= $shiftamt;
          }
          my @grsave = @{$groups[$gi]};
          $grsave[0] += $offset;
          $grsave[1] += $offset;
          for (my $j = $gi; $j < $gj; ++$j) {
            $groups[$j] = $groups[$j + 1];
            $circular_checked{$j} = $circular_checked{$j + 1};
          }
          $groups[$gj] = \@grsave;
          $circular_checked{$gj} = 1;

          ## Start over from the first group
          $gi = -1;

          ## Exit from the outter ($gj) loop
          $gj = $#groups;
          last;
        }
      }
    }
  }
}


sub sort_dependencies {
  my($self, $projects, $groups) = @_;
  my @list = sort { return $self->sort_projects_by_directory($a, $b) + 0;
                  } @$projects;
  ## The list above is sorted by directory in order to keep projects
  ## within the same directory together.  Otherwise, when groups are
  ## created we may get multiple groups for the same directory.

  ## Put the projects in the order specified
  ## by the project dependencies.  We only need to do
  ## this if there is more than one element in the array.
  if ($#list > 0) {
    ## If the parameter wasn't passed in or it was passed in
    ## and was true, sort with directory groups in mind
    if (!defined $groups || $groups) {
      ## First determine the individual groups
      my @grindex;
      my $previous = [0, undef];
      for (my $li = 0; $li <= $#list; ++$li) {
        my $dir = $self->get_first_level_directory($list[$li]);
        if (!defined $previous->[1]) {
          $previous = [$li, $dir];
        } elsif ($previous->[1] ne $dir) {
          push(@grindex, [$previous->[0], $li - 1]);
          $previous = [$li, $dir];
        }
      }
      push(@grindex, [$previous->[0], $#list]);

      ## Next, sort the individual groups
      foreach my $gr (@grindex) {
        $self->sort_within_group(\@list, @$gr) if ($$gr[0] != $$gr[1]);
      }

      ## Now sort the groups as single entities
      $self->sort_by_groups(\@list, \@grindex) if ($#grindex > 0);
    } else {
      $self->sort_within_group(\@list, 0, $#list);
    }
  }

  return @list;
}


sub number_target_deps {
  my($self, $projects, $pjs, $targets, $groups) = @_;
  my @list = $self->sort_dependencies($projects, $groups);

  ## This block of code must be done after the list of dependencies
  ## has been sorted in order to get the correct project numbers.
  for (my $i = 0; $i <= $#list; ++$i) {
    my $project = $list[$i];
    if (defined $$pjs{$project}) {
      my($name, $deps) = @{$$pjs{$project}};
      if (defined $deps && $deps ne '') {
        my @numbers;
        my %dhash;
        @dhash{@{$self->create_array($deps)}} = ();

        ## For each dependency, search in the sorted list
        ## up to the point of this project for the projects
        ## that this one depends on.  When the project is
        ## found, we put the target number in the numbers array.
        for (my $j = 0; $j < $i; ++$j) {
          ## If the dependency is a filename, then take the basename of
          ## the project file.  Otherwise, get the project name based on
          ## the project file from the "project_info".
          my $key = ($self->{'dependency_is_filename'} ?
                     $self->mpc_basename($list[$j]) :
                     $self->{'project_info'}->{$list[$j]}->[ProjectCreator::PROJECT_NAME]);
          push(@numbers, $j) if (exists $dhash{$key});
        }

        ## Store the array in the hash keyed on the project file.
        $$targets{$project} = \@numbers if (defined $numbers[0]);
      }
    }
  }

  return @list;
}


sub project_target_translation {
  my($self, $case) = @_;
  my %map;

  ## Translate project names to avoid target collision with
  ## some versions of make.
  foreach my $key (keys %{$self->{'project_info'}}) {
    my $dir  = $self->mpc_dirname($key);
    my $name = $self->{'project_info'}->{$key}->[ProjectCreator::PROJECT_NAME];

    ## We want to compare to the upper most directory.  This will be the
    ## one that may conflict with the project name.
    $dir =~ s/[\/\\].*//;
    if (($case && $dir eq $name) || (!$case && lc($dir) eq lc($name))) {
      $map{$key} = "$name-target";
    } else {
      $map{$key} = $name;
    }
  }
  return \%map;
}


sub optionError {
  my($self, $str) = @_;
  $self->warning("$self->{'current_input'}: $str.") if (defined $str);
}


sub process_cmdline {
  my($self, $cmdline, $parameters) = @_;

  ## Set cache use to default.
  $self->{'cacheok'} = $self->default_cacheok();

  if (defined $cmdline && $cmdline ne '') {
    my $args = $self->create_array($cmdline);

    ## Look for environment variables
    foreach my $arg (@$args) {
      $self->replace_env_vars(\$arg) if ($arg =~ /\$/);
    }

    my $options = $self->options('MWC', {}, 0, @$args);
    if (defined $options) {
      foreach my $key (keys %$options) {
        my $type = $self->is_set($key, $options);

        if (!defined $type) {
          ## This option was not used, so we ignore it
        } elsif ($type eq 'ARRAY') {
          push(@{$parameters->{$key}}, @{$options->{$key}});
        } elsif ($type eq 'HASH') {
          foreach my $hk (keys %{$options->{$key}}) {
            $parameters->{$key}->{$hk} = $options->{$key}->{$hk};
          }
        } elsif ($type eq 'SCALAR') {
          $parameters->{$key} = $options->{$key};
        }
      }

      ## Some option data members are named consistently with the MPC
      ## option name.  In this case, we can use this foreach loop.
      foreach my $consistent_opt ('exclude', 'for_eclipse', 'gendot',
                                  'gfeature_file', 'into',
                                  'make_coexistence', 'recurse') {
        ## Issue warnings for the options provided by the user
        if ($self->is_set($consistent_opt, $options)) {
          $self->optionError("-$consistent_opt is ignored");
        }
      }

      ## For those that are inconsistent, we have special code to deal
      ## with them.
      if ($self->is_set('reldefs', $options)) {
        $self->optionError('-noreldefs is ignored');
      }

      ## Make sure no input files were specified (we can't handle it).
      if (defined $options->{'input'}->[0]) {
        $self->optionError('Command line files ' .
                           'specified in a workspace are ignored');
      }

      ## Determine if it's ok to use the cache
      my @cacheInvalidating = ('global', 'include', 'baseprojs',
                               'template', 'ti', 'relative', 'language',
                               'addtemp', 'addproj', 'feature_file',
                               'features', 'use_env', 'expand_vars');
      foreach my $key (@cacheInvalidating) {
        if ($self->is_set($key, $options)) {
          $self->{'cacheok'} = 0;
          last;
        }
      }
    }
  }
}


sub current_parameters {
  my $self = shift;
  my %parameters = $self->save_state();

  ## We always want the project creator to generate a toplevel
  $parameters{'toplevel'} = 1;
  return %parameters;
}


sub project_creator {
  my $self = shift;
  my $pid = shift;
  if (not defined $pid) {
    $pid = 'parent';
  }

  my $str = "$self";

  ## NOTE: If the subclassed WorkspaceCreator name prefix does not
  ##       match the name prefix of the ProjectCreator, this code
  ##       will not work and the subclassed WorkspaceCreator will
  ##       need to override this method.

  $str =~ s/Workspace/Project/;
  $str =~ s/=HASH.*//;

  ## Set up values for each project creator
  ## If we have command line arguments in the workspace, then
  ## we process them before creating the project creator
  my $cmdline    = $self->get_assignment('cmdline');
  my %parameters = $self->current_parameters();
  $self->process_cmdline($cmdline, \%parameters);

  ## Create the new project creator with the updated parameters
  return $str->new($parameters{'global'},
                   $parameters{'include'},
                   $parameters{'template'},
                   $parameters{'ti'},
                   $parameters{'dynamic'},
                   $parameters{'static'},
                   $parameters{'relative'},
                   $parameters{'addtemp'},
                   $parameters{'addproj'},
                   $parameters{'progress'},
                   $parameters{'toplevel'},
                   $parameters{'baseprojs'},
                   $self->{'global_feature_file'},
                   $parameters{'relative_file'},
                   $parameters{'feature_file'},
                   $parameters{'features'},
                   $parameters{'hierarchy'},
                   $self->{'exclude'}->{$self->{'wctype'}},
                   $self->make_coexistence(),
                   $parameters{'name_modifier'},
                   $parameters{'apply_project'},
                   $self->{'generate_ins'} || $parameters{'genins'},
                   $self->get_into(),
                   $parameters{'language'},
                   $parameters{'use_env'},
                   $parameters{'expand_vars'},
                   $self->{'gendot'},
                   $parameters{'comments'},
                   $self->{'for_eclipse'},
                   $pid);
}


sub sort_files {
  #my $self = shift;
  return 0;
}


sub make_coexistence {
  return $_[0]->{'coexistence'};
}


sub get_modified_workspace_name {
  my($self, $name, $ext, $nows) = @_;
  my $nmod  = $self->get_name_modifier();
  my $oname = $name;

  if (defined $nmod) {
    $nmod =~ s/\*/$name/g;
    $name = $nmod;
  }

  ## If this is a per project workspace, then we should not
  ## modify the workspace name.  It may overwrite another workspace
  ## but that's ok, it will only be a per project workspace.
  ## Also, if we don't want the workspace name attached ($nows) then
  ## we just return the name plus the extension.
  return "$name$ext" if ($nows || $self->{'per_project_workspace_name'});

  my $pwd    = $self->getcwd();
  my $type   = $self->{'wctype'};
  my $wsname = $self->get_workspace_name();

  if (!defined $previous_workspace_name{$type}->{$pwd}) {
    $previous_workspace_name{$type}->{$pwd} = $wsname;
    $self->{'current_workspace_name'} = undef;
  } else {
    my $prefix = ($oname eq $wsname ? $name : "$name.$wsname");
    $previous_workspace_name{$type}->{$pwd} = $wsname;
    while ($self->file_written("$prefix" .
                               ($self->{'modified_count'} > 0 ?
                                ".$self->{'modified_count'}" : '') .
                               "$ext")) {
      ++$self->{'modified_count'};
    }
    $self->{'current_workspace_name'} =
      "$prefix" . ($self->{'modified_count'} > 0 ?
                   ".$self->{'modified_count'}" : '') . "$ext";
  }

  return (defined $self->{'current_workspace_name'} ?
          $self->{'current_workspace_name'} : "$name$ext");
}


sub generate_recursive_input_list {
  my($self, $dir, $exclude) = @_;
  return $self->extension_recursive_input_list($dir, $exclude, $wsext);
}


sub verify_build_ordering {
  my $self = shift;
  foreach my $project (@{$self->{'projects'}}) {
    $self->get_validated_ordering($project);
  }
}


sub get_validated_ordering {
  my($self, $project) = @_;
  my $deps;

  if (defined $self->{'ordering_cache'}->{$project}) {
    $deps = $self->{'ordering_cache'}->{$project};
  } else {
    $deps = [];
    if (defined $self->{'project_info'}->{$project}) {
      my($name, $dstr) = @{$self->{'project_info'}->{$project}};
      if (defined $dstr && $dstr ne '') {
        $deps = $self->create_array($dstr);
        my $dlen = scalar(@$deps);
        for (my $i = 0; $i < $dlen; $i++) {
          my $dep   = $$deps[$i];
          my $found = 0;
          ## Avoid circular dependencies
          if ($dep ne $name && $dep ne $self->mpc_basename($project)) {
            foreach my $p (@{$self->{'projects'}}) {
              if ($dep eq $self->{'project_info'}->{$p}->[ProjectCreator::PROJECT_NAME] ||
                  $dep eq $self->mpc_basename($p)) {
                $found = 1;
                last;
              }
            }
            if (!$found) {
              if ($self->{'verbose_ordering'}) {
                $self->warning("processing '$project' and '$name' references '$dep' which has " .
                               "not been processed.");
              }
              splice(@$deps, $i, 1);
              --$dlen;
              --$i;
            }
          } else {
            ## If a project references itself, we must remove it
            ## from the list of dependencies.
            splice(@$deps, $i, 1);
            --$dlen;
            --$i;
          }
        }
      }

      $self->{'ordering_cache'}->{$project} = $deps;
    }
  }

  return $deps;
}


sub source_listing_callback {
  my($self, $project_file, $project_name, $list) = @_;

  # have to keep projects in the the same order as if run in
  # single process. otherwise implicit dependencies produces
  # different output
  if ($self->{'pid'} ne 'parent') {
    $project_name = ++$self->{'imp_dep_ctr'} . '|' . $project_name;
  }

  $self->{'project_file_list'}->{$project_name} = [ $project_file,
                                                    $self->getcwd(), $list ];
}


sub sort_projects_by_directory {
  my($self, $left, $right) = @_;
  my $sa = index($left, '/');
  my $sb = index($right, '/');

  if ($sa >= 0 && $sb == -1) {
    return 1;
  } elsif ($sb >= 0 && $sa == -1) {
    return -1;
  }
  return $left cmp $right;
}


sub get_relative_dep_file {
  my($self, $creator, $project, $dep) = @_;

  ## If the dependency is a filename, we have to find the key that
  ## matches the project file.
  if ($creator->dependency_is_filename()) {
    foreach my $key (keys %{$self->{'project_file_list'}}) {
      if ($self->{'project_file_list'}->{$key}->[0] eq $dep) {
        $dep = $key;
        last;
      }
    }
  }

  if (defined $self->{'project_file_list'}->{$dep}) {
    my $base = $self->{'project_file_list'}->{$dep}->[1];
    my @dirs = grep(!/^$/, split('/', $base));
    my $last = -1;
    $project =~ s/^\///;
    for (my $i = 0; $i <= $#dirs; $i++) {
      my $dir = $dirs[$i];
      if ($project =~ s/^$dir\///) {
        $last = $i;
      } else {
        last;
      }
    }

    my $dependee = $self->{'project_file_list'}->{$dep}->[0];
    if ($last == -1) {
      return $base . '/' . $dependee;
    } else {
      my $built = '';
      for (my $i = $last + 1; $i <= $#dirs; $i++) {
        $built .= $dirs[$i] . '/';
      }
      $built .= $dependee;
      my $dircount = ($project =~ tr/\///);
      return ('../' x $dircount) . $built;
    }
  }
  return undef;
}


sub create_command_line_string {
  my $self = shift;
  my @args = @_;
  my $str;

  foreach my $arg (@args) {
    $arg =~ s/^\-\-/-/;
    if ($arg =~ /\$/ && $^O ne 'MSWin32') {
      ## If we're not running on Windows and the command line argument
      ## contains a dollar sign, we need to wrap the argument in single
      ## quotes so that the UNIX shell does not interpret it.
      $arg = "'$arg'";
    } else {
      ## Unfortunately, the Windows command line shell does not
      ## understand single quotes correctly.  So, we have the distinction
      ## above and handle dollar signs here too.
      $arg = "\"$arg\"" if ($arg =~ /[\s\*\$]/);
    }
    if (defined $str) {
      $str .= " $arg";
    } else {
      $str = $arg;
    }
  }
  return $str;
}


sub print_workspace_comment {
  my $self = shift;
  my $fh   = shift;

  if ($self->{'workspace_comments'}) {
    foreach my $line (@_) {
      print $fh $line;
    }
  }
}


sub get_initial_relative_values {
  my $self = shift;
  return $self->get_relative(), $self->get_expand_vars();
}


sub get_secondary_relative_values {
  return \%ENV, $_[0]->get_expand_vars();
}


sub convert_all_variables {
  #my $self = shift;
  return 1;
}


sub workspace_file_name {
  my $self = shift;
  return $self->get_modified_workspace_name($self->get_workspace_name(),
                                            $self->workspace_file_extension());
}


sub relative {
  my $self = shift;
  my $line = $self->SUPER::relative(shift);
  $line =~ s/\\/\//g;
  return $line;
}

# ************************************************************
# Virtual Methods To Be Overridden
# ************************************************************

sub requires_make_coexistence {
  #my $self = shift;
  return 0;
}


sub supports_make_coexistence {
  #my $self = shift;
  return 0;
}


sub generate_implicit_project_dependencies {
  #my $self = shift;
  return 0;
}


sub workspace_file_extension {
  #my $self = shift;
  return '';
}


sub workspace_per_project {
  #my $self = shift;
  return 0;
}


sub default_verbose_ordering {
  return 0;                     # Don't warning if there are missing dependencies.
}


sub pre_workspace {
  #my $self    = shift;
  #my $fh      = shift;
  #my $creator = shift;
  #my $top     = shift;
}


sub write_comps {
  #my $self    = shift;
  #my $fh      = shift;
  #my $creator = shift;
  #my $top     = shift;
}


sub post_workspace {
  #my $self    = shift;
  #my $fh      = shift;
  #my $creator = shift;
  #my $top     = shift;
}

sub requires_forward_slashes {
  #my $self = shift;
  return 0;
}

sub get_additional_output {
  #my $self = shift;

  ## This method should return an array reference of array references.
  ## For each entry, the array should be laid out as follows:
  ## [ <directory or undef to use the current output directory>,
  ##   <file name>,
  ##   <function to write body of file, $self and $fh are first params>,
  ##   <optional additional parameter 1>,
  ##   ...,
  ##   <optional additional parameter N>
  ## ]
  return [];
}

1;
