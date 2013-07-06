package RpmSpecWorkspaceCreator;

# ************************************************************
# Description   : An RPM .spec file Workspace Creator
# Author        : Adam Mitz (OCI)
# Create Date   : 11/23/2010
# $Id$
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;
use File::Path;
use POSIX qw(strftime);

use RpmSpecProjectCreator;
use WorkspaceCreator;

use vars qw(@ISA);
@ISA = qw(WorkspaceCreator);

# ************************************************************
# Data Section
# ************************************************************

my $ext = '.spec'; # extension of files written by this WorkspaceCreator

# ************************************************************
# Subroutine Section
# ************************************************************

sub workspace_file_name {
  my $self = shift;
  return $self->get_modified_workspace_name($self->get_workspace_name(), $ext);
}

# Called by document_template.pl
sub documentation_info {
  shift; #ignore package name
  my $keywords = shift;
  %$keywords = ('apply' => \&interpret_keyword, 'cond' => \&interpret_keyword);
  return '^sub get_template', '^EOT$';
}

sub default_verbose_ordering {
  return 1; # Warn if there are missing dependencies.
}

# Called by document_template.pl
sub interpret_keyword {
  my $vname = shift;
  $vname = (split /,/, $vname)[0];
  return ($vname, $vname, $vname, undef);
}

# Don't actually write the .spec file for the workspace.  Instead just invoke
# the $func callback so that post_workspace() and other parts of the normal
# workspace processing are called.  We don't want a .spec file for each MPC
# workspace because that is too course-grained.  Instead, post_workspace() will
# create one .spec for each aggregated workspace inside the primary workspace.
# Using the workspace aggregation mechanism this way allows multiple .spec
# files per workspace with MPC deriving their dependencies based on the
# projects they contain.
sub write_and_compare_file {
  my($self, $outdir, $oname, $func, @params) = @_;
  &$func($self, undef, @params);
  return 1;
}

sub rpmname {
  my($self, $mwc, $rpm2mwc, $check_unique) = @_;
  my $outfile = $mwc;
  $outfile =~ s/\.mwc$//i;
  $outfile = $self->get_modified_workspace_name($outfile, $ext, 1);
  my $base = $self->mpc_basename($outfile);
  $base =~ tr/-/_/; # - is special for RPM, we translate it to _
  if ($check_unique && $rpm2mwc->{$base}) {
    die "ERROR: Can't create a duplicate RPM name: $base for mwc file $mwc\n" .
        "\tsee corresponding mwc file $rpm2mwc->{$base}\n";
  }
  $rpm2mwc->{$base} = $mwc;
  return $base;
}

## helper functions for the mini-template language

sub mtl_cond {
  my($vars, $pre, $rep) = @_;
  my @v;
  return (@v = grep {$_} map {$rep->{lc $_}} split(' ', $vars)) ? "$pre@v" : '';
}

sub mtl_apply {
  my($name, $subst, $rep) = @_;
  return join("\n", map {my $x = $subst; $x =~ s!\$_!$_!g; $x}
              split(' ', $rep->{lc $name}));
}

sub mtl_var {
  my($name, $default, $rep) = @_;
  return defined $rep->{lc $name} ? $rep->{lc $name} :
      (defined $default ? $default : ">>ERROR: no value for $name<<");
}

## end helper functions for the mini-template language


sub post_workspace {
  my($self, $fh, $prjc) = @_;

  my $prjext = '\\' . # regexp escape for the dot that begins the extension
      $prjc->project_file_extension();

  my %rpm2mwc;  # rpm name (basename of spec file) => aggregated mwc w/ path
  my %mwc2rpm;  # inverse of the above hash
  my %proj2rpm; # project name (output of mpc) => rpm name that it belongs to
  # first pass to build the hashes above
  foreach my $agg (keys %{$self->{'aggregated_mpc'}}) {
    my $rpm = $mwc2rpm{$agg} = $self->rpmname($agg, \%rpm2mwc, 1);
    foreach my $m (@{$self->{'aggregated_mpc'}->{$agg}}) {
      foreach my $p (@{$self->{'mpc_to_output'}->{$m}}) {
        $proj2rpm{$p} = $rpm;
      }
    }
  }

  if (0 == scalar keys %proj2rpm) {
    # nothing to generate (no aggregated workspaces)
    return;
  }

  my $outdir = $self->get_outdir();
  my $now = strftime '%a %b %d %Y %H:%M:%S', localtime;

  my %assign = %{$self->get_assignment_hash()};
  $assign{'rpm_description'} =~ s/\\n\s*/\n/g  # Allow the description to span
      if exists $assign{'rpm_description'};    # multiple lines in the output
  map {$_ = $self->process_special($_)} values %assign;

  # determine when this addtemp processing should actually occur
  while (my($key, $arr) = each %{$self->get_addtemp()}) {
    foreach my $val (@$arr) {
      my $v = $val->[1];
      $v =~ s/\\n\s*/\n/g if $key eq 'rpm_description';
      $v = $self->process_special($v);
      $self->process_any_assignment(\%assign, $val->[0], $key, $v);
    }
  }

  foreach my $agg (keys %{$self->{'aggregated_mpc'}}) {
    my $name = "$outdir/$agg"; # $agg may contain directory parts
    my $dir  = $self->mpc_dirname($name);
    my $base = $mwc2rpm{$agg};
    my $rpm = $base;
    $rpm =~ s/$ext$//;
    $name = "$dir/$base";
    mkpath($dir, 0, 0777) if ($dir ne '.');

    my %rpm_requires; # keys are RPMs that this RPM depends on
    my @projects;
    foreach my $m (@{$self->{'aggregated_mpc'}->{$agg}}) {
      my $projdir = $self->mpc_dirname($m);
      foreach my $p (@{$self->{'mpc_to_output'}->{$m}}) {
        my $proj = $p;
        $proj =~ s/$prjext$//;
        push @projects, $proj;
        my $deps = $self->get_validated_ordering("$projdir/$p");
        foreach my $d (@$deps) {
          my $rpmdep = $proj2rpm{$d};
          if (defined $rpmdep && $rpmdep ne $base) {
            $rpm_requires{$rpmdep} = 1;
          }
        }
      }
    }

    # The hash %rep has replacement values for the template .spec file text,
    # those values come from a few different sources, starting with the
    # workspace-wide assignments, then let RPM-specific ones (from aggregated
    # workspaces) override those, and finally add the ones known by MPC.
    # process_special() handles quotes and escape characters.

    my %rep = %assign;

    while (my($key, $val) = each %{$self->{'aggregated_assign'}->{$agg}}) {
      $val =~ s/\\n\s*/\n/g if $key eq 'rpm_description';
      $rep{$key} = $self->process_special($val);
    }

    $rep{'rpm_name'} = $rpm;
    $rep{'rpm_mpc_workspace'} = $self->mpc_basename($agg);
    $rep{'rpm_mpc_requires'} =
        join(' ', sort map {s/$ext$//; $_} keys %rpm_requires);

    my $fh = new FileHandle;
    open $fh, ">$name" or die "can't open $name";
    my $t = get_template();

    ## We have decided not to reuse the TemplateParser.pm, so this file has
    ## its own little template language which is a subset of that one.

    ## <%cond(var1 [var2...], prefix)%>
    ##   Output the prefix text followed by the concatenated, space separated,
    ##   values of the variables (var1, var2, etc) only if at least one of
    ##   said values is non-empty.
    $t =~ s/<%cond\(([\w ]+), (.+)\)%>/mtl_cond($1, $2, \%rep)/ge;

    ## <%perl(expr)%>
    ##   Evaluate an arbitrary perl expression, which can reference the normal
    ##   variable replacements (see <%var%>, below) as $rep{'name'}.
    $t =~ s/<%perl\((.+)\)%>/join "\n", eval $1/ge;

    ## <%apply(listvar, text)%>
    ##   Treat the value of variable 'listvar' as a list (splitting on spaces)
    ##   and repeat the text for each element of the list, substituting $_ in
    ##   the text with the current list element.
    $t =~ s/<%apply\((\w+), (.+)\)%>/mtl_apply($1, $2, \%rep)/ge;

    ## <%var(default)%> or <%var%>
    ##   Output the value of variable 'var', either with a default value or an
    ##   error if 'var' is unknown.  If 'default' is enclosed in double-quotes,
    ##   they are ignored (for compatibility with TemplateParser).
    $t =~ s/<%(\w+)(?:\("?([^)"]*)"?\))?%>/mtl_var($1, $2, \%rep)/ge;

    print $fh $t;

    # comment will go in the %changelog section of the .spec
    $self->print_workspace_comment($fh, map {$_ . "\n"} (
      "* $now This file was generated by MPC.",
      '  $Id$',
      '  Any changes made directly to this file will',
      '  be lost the next time it is generated.',
      '  MPC Command:', '  ' . $self->create_command_line_string($0, @ARGV)));
    close $fh;
  }

  # write the script to build .rpm files from .spec files
  my $fh = new FileHandle;
  my $name = $outdir . '/' . $self->{'workspace_name'} . '_rpm.sh';
  open($fh, ">$name") or die "can't open $name";
  print $fh "#!/bin/sh\n";
  $self->print_workspace_comment($fh, map {$_ . "\n"} (
    '# RPM creation script for MPC-generated .spec files.',
    "# $now",
    '# This file was generated by MPC.  Any changes made directly to',
    '# this file will be lost the next time it is generated.',
    '# $Id$',
    '# MPC Command:', '# ' . $self->create_command_line_string($0, @ARGV)));

  my $script = get_script();
  my $temporary = $assign{'rpm_mpc_temp'};
  $script =~ s!/tmp/mpcrpm!$temporary!g if defined $temporary;
  print $fh $script;

  my %seen;
  foreach my $project ($self->sort_dependencies($self->get_projects(), 0)) {
    my $rpm = $proj2rpm{$self->mpc_basename($project)};
    next if !defined $rpm;
    if (!$seen{$rpm}) {
      $seen{$rpm} = 1;
      my $dir = $self->mpc_dirname($rpm2mwc{$rpm});
      $dir = ($dir eq '.' ? '' : "$dir/");
      print $fh "build $dir$rpm\n";
    }
  }

  close $fh;
  chmod 0755, $name;
}


sub get_template {
  return <<'EOT';
License: <%rpm_license("Freeware")%>
Version: <%rpm_version%>
Release: <%rpm_releasenumber%>
Source: <%rpm_source_base("")%><%rpm_name%>.tar.gz
Name: <%rpm_name%>
Group: <%rpm_group%>
Summary: <%rpm_summary%>
<%cond(rpm_url, URL: )%>
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Prefix: <%rpm_prefix("/usr")%>
AutoReqProv: <%rpm_autorequiresprovides("no")%>
<%cond(rpm_buildrequires, BuildRequires: )%>
<%cond(rpm_mpc_requires rpm_requires, Requires: )%>
<%cond(rpm_provides, Provides: )%>

%description
<%rpm_description%>

%files -f %{_tmppath}/<%rpm_name%>.flist
%defattr(-,root,root)
%doc
%config

%pre
<%rpm_pre_cmd()%>

%post
<%rpm_post_cmd()%>

%preun
<%rpm_preun_cmd()%>

%postun
<%rpm_postun_cmd()%>

%prep
%setup -n <%rpm_name%>-<%rpm_version%>

%build
<%apply(env_check, [ -z $$_ ] && echo Environment variable $_ is required. && exit 1)%>
rm -rf $RPM_BUILD_ROOT
<%prebuild()%>
<%makefile_generator(mwc.pl -type gnuace)%> -base install -value_project libpaths+=<%rpm_mpc_temp(/tmp/mpcrpm)%>/inst/lib -value_project includes+=<%rpm_mpc_temp(/tmp/mpcrpm)%>/inst/include <%mkgen_args()%> <%rpm_mpc_workspace%>
make <%makeflags()%>

%install
if [ "$RPM_BUILD_ROOT" = "/" ]; then
  echo "Build root of / is a bad idea.  Bailing."
  exit 1
fi
rm -rf $RPM_BUILD_ROOT
export staging_dir=$RPM_BUILD_ROOT/install<%rpm_prefix("/usr")%>
mkdir -p $staging_dir
export pkg_dir=$RPM_BUILD_ROOT/<%rpm_name%>_dir
mkdir -p $RPM_BUILD_ROOT/<%rpm_name%>_dir
make INSTALL_PREFIX=${staging_dir} install
if [ -d ${staging_dir}/share/man ]; then
  files=$(find ${staging_dir}/share/man -name '*.bz2')
  if [[ "${files}" ]]; then echo "${files}" | xargs bunzip2 -q; fi
  files=$(find ${staging_dir}/share/man -name '*.[0-9]')
  if [[ "${files}" ]]; then echo "${files}" | xargs gzip -9; fi
fi
cp -a $RPM_BUILD_ROOT/install/* ${pkg_dir}
find ${pkg_dir} ! -type d | sed s^${pkg_dir}^^ | sed /^\s*$/d > %{_tmppath}/<%rpm_name%>.flist
find ${pkg_dir} -type d | sed s^${pkg_dir}^^ | sed '\&^/usr$&d;\&^/usr/share/man&d;\&^/usr/games$&d;\&^/lib$&d;\&^/etc$&d;\&^/boot$&d;\&^/usr/bin$&d;\&^/usr/lib$&d;\&^/usr/share$&d;\&^/var$&d;\&^/var/lib$&d;\&^/var/spool$&d;\&^/var/cache$&d;\&^/var/lock$&d;\&^/tmp/apkg&d' | sed /^\s*$/d | sed 's&^&%dir &' >> %{_tmppath}/<%rpm_name%>.flist
cp -a $RPM_BUILD_ROOT/*_dir/* $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_ROOT/*_dir
rm -rf $RPM_BUILD_ROOT/install

%clean
make realclean
find . -name '<%makefile_name_pattern(GNUmakefile*)%>' -o -name '.depend.*' | xargs rm -f

%changelog
EOT
}


sub get_script {
  return <<'EOT';
RPM_TOP=`rpmbuild --showrc | grep ': _topdir\b' | sed 's/^.*: _topdir\s*//' | perl -pe's/%{getenv:(\w+)}/$ENV{$1}/g'`
START_DIR=`pwd`
TMP_DIR=/tmp/mpcrpm
DB_DIR=`rpmbuild --showrc | grep ': _dbpath\b' | sed 's/^.*: _dbpath\s*//' | perl -pe's/%\{(\w+)\}/$x = qx(rpmbuild --showrc | grep ": $1\\\b" | sed "s\/^.*: $1\\\s*\/\/"); chomp $x; $x/e'`
RPM_ARCH=${1-`uname -m`}
echo MPC RPM build script: output files will be placed in $RPM_TOP/RPMS
[ -z $MPC_ROOT ] && echo ERROR: MPC_ROOT must be set && exit 1
rm -rf $TMP_DIR && mkdir $TMP_DIR && cp -a $DB_DIR $TMP_DIR/db || exit $?

build () {
  [ ! -r $1 ] && echo ERROR: File not found $1 && exit 1
  PKG_DIR=`dirname $1`
  PKG=`basename ${1%.spec}`
  cd $PKG_DIR
  VER=`grep ^Version: $PKG.spec | sed 's/^Version: //'`
  REL=`grep ^Release: $PKG.spec | sed 's/^Release: //'`
  echo Building source .tar.gz for $PKG version $VER release $REL
  rm -rf $TMP_DIR/$PKG-$VER
  $MPC_ROOT/clone_build_tree.pl -b $TMP_DIR $PKG-$VER > /dev/null
  cd $TMP_DIR
  tar chzf $RPM_TOP/SOURCES/$PKG.tar.gz $PKG-$VER && rm -rf $PKG-$VER
  cp $START_DIR/$PKG_DIR/$PKG.spec $RPM_TOP/SPECS
  echo Running rpmbuild on $PKG.spec for arch $RPM_ARCH, see rpm-$PKG.log for details
  rpmbuild -ba --target $RPM_ARCH $RPM_TOP/SPECS/$PKG.spec > $START_DIR/rpm-$PKG.log 2>&1
  if [ $? != 0 ]; then
    echo rpmbuild of $PKG.spec failed. STOPPING.
    exit $?
  fi
  echo Installing $PKG to the temporary area
  rpm --ignorearch --dbpath $TMP_DIR/db --prefix $TMP_DIR/inst -iv $RPM_TOP/RPMS/$RPM_ARCH/$PKG-$VER-$REL.$RPM_ARCH.rpm || exit $?
  cd $START_DIR
}

EOT
}

1;
