#
# $Id: parse_compiler_output.pm 8676 2012-01-04 15:37:56Z johnnyw $
#
package common::parse_compiler_output;
use strict;
use warnings;

sub new {
  my $proto = shift;
  my $class = ref($proto) || $proto;
  my $self = {};

  bless($self, $class);
  return $self;
}

sub Output_Subsection($) {
}

sub Output_Normal($) {
}

sub Output_Warning($) {
}

sub Output_Error($) {
}

sub Detected_Build_Error($) {
}

sub handle_compiler_output_line($) {
  my $self = shift;
  my $s = shift;


  # Check for the subsection indicator
  # VC71 || GNU make || nmake || borland make
  if ($s =~ m/^(?:\d+>)?------ Build started: Project: (.*), Config.*/ || $s =~ /GNUmakefile: (.*) MAKEFLAGS.*/ || $s =~ /nmake.exe\" \/f Makefile.(.*) CFG.*/ || $s =~ /make .* -f (.*).bor .*/ || $s =~ /^make.*Entering directory \`(.*)\/bld\/.*/ || $s =~ /Entering directory .*\`\/(ospl.\/.*)/ || $s =~ /^make.*Entering directory \`(.*)\'$/ ) {
    my $subsec = $1;
    $subsec .= " ($1)" if $s =~ /^(\d+)>---/;
    $self->Output_Subsection ($subsec);
    return;
  }

  # VC10 / MSBuild
  if ($s =~ /^\s*(?:\d+>)?Project "[^"]+" \(\d+\) is building "([^"]+)" \((\d+)\)/) {
    my ($prj, $seq) = ($1, $2);
    $prj =~ /\\([^.\\]+)\.vcxproj$/;
    $self->Output_Subsection ("$1 ($seq)");
    return;
  }

  # NMake
  if ($s =~ /^Project: Makefile\.(\w+)\.mak\s*$/) {
    $self->Output_Subsection ($1);
    return;
  }

  if ($s =~ /^\s*0 (Error|Warning)\(s\)\s*$/) {
    $self->Output_Normal ($s);
    return;
  }

  # Prelim to "MS-DOS style path detected" seems to happen in every configure
  if ($s =~ /^cygwin warning:/) {
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/^Warning:.* will be ignored/) {
    # Warning from RTI ddsgen, ignore because this is not important for us
    $self->Output_Normal ($s);
    return;
  }

  # Check out the line to figure out whether it is an error or not

  if ($s =~ m/^ICECC[\[]\d+[\]]/) {
    # debug messages of the Icecream distributed compiler; can be ignored
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/^error\W\(future\)\W908\:\W\".*\.idl\".*\'export\'/i) {
    # HP1 aCC complains about "future errors" but these particular ones are
    # for IDL compilations not c++.
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/^warning\:\W[0-9]+\Wfuture errors were detected and ignored/i) {
    # HP1 aCC complains about "future errors" but the summary line is just redundant.
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/--fail-on-analysis-error/i || $s =~ m/--no-unsupported-warning/i) {
    # Coverity logs have these strings in as options
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/^Warning #\d+-D:/) {
    # This is a warning from HP aCC even though it may
    # contain the word "error" later on the line.
    $self->Output_Warning ($s);
    return;
  }

  # The NMAKE output on Windows includes the complete command line; if there
  # are options with the word "warning" or "error" these get flagged as
  # errors. So if it's the command line, just output as normal.
  if ($s =~ m/^[ \t]*cl\.exe/ || $s =~ m/depgen\.pl/) {
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/clang/)
  {
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/^[ \t]*icl.exe/) {
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/^[ \t]*rc.exe/) {
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/^Info: .* will be ignored./) {
    # Warning from RTI ddsgen, ignore because this is not important for us
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/^Error scanning file .* for dependencies/) {
    # EVC 4 complains about non-existent files during dependency
    # generation.  This is not an actual error.
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/^ld: \d+\-\d+ WARNING: Duplicate symbol:/) {
    # AIX reports a bazillion multiple defines when doing templates; some
    # have the word 'error' in the symbol name - ignore those.
    $self->Output_Normal ($s);
    return;
  }

  # Dependency check file not present. No problem.
  if ($s =~ m/\.depend\..*:\s+no\s+such\s+file/i || # gmake / linux
        $s =~ m/\.depend\..*:\s+file\s+or\s+directory\s+in\s+the\s+path/i ) #aix
  {
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/Clock skew detected/) {
    # Can be given when building on NFS volumes, just ignore
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/has modification time/) {
    # Can be given when building on NFS volumes, just ignore
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/is dangerous, better use/) {
    # Linux has this annoying mktemp, mkstemp stuff. Ignore that
    # for the timebeing
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/possibly used unsafely, use/) {
    # Similar warnings on NetBSD
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/possibly used unsafely[;,] consider using/) {
    # Similar warnings on OpenBSD
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/almost always misused/) {
    # Ignore some annoying warnings on OpenBSD
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/is often misused/) {
    # Ignore some annoying warnings on OpenBSD
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/\-W:c:,\-Xmismatch\-warning\=2/) {
    # catch VxWorks DIAB warning option before it is caught as Warning
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/qt.\/include\/private\/qucom/) {
      # We don't particularly care about warnings in QT code.
      $self->Output_Normal ($s);
      return;
  }

  if ($s =~ m/^inflating:.*DDS\/ERRORCODE/) {
      # We unzip a load of files with ERROR in the filename
      $self->Output_Normal ($s);
      return;
  }

  if ($s =~ m/ is deprecated. use /) {
    # Among these are glibc warnings to stop using the deprecated
    # pthread_setstackaddr in favor of pthread_setstack.
    $self->Output_Warning ($s);
    return;
  }

  if ($s =~ m/ is deprecated and will be removed /) {
    # Given by Intel C++
    $self->Output_Warning ($s);
    return;
  }

  if ($s =~ m/Too many levels of symbolic links/) {
    # Indicates a broken file system. Must be fixed to prevent
    # other errors in the future
    $self->Output_Warning ($s);
    return;
  }

  if ($s =~ m/identifier spellings differ only in case/) {
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/not be built due to the following missing/) {
    # Indicates that something is not properly configured - ie,
    # incorrect depenancies in the mpc files, default.featres setting,
    # etc.
      $self->Output_Warning ($s);
      return;
  }

  if ($s =~ m/Rule line too long/) {
    # Can be given by Borland make
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/Unknown Error/) {
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/illegal redefinition/) {
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/is not recognized as an internal or external command/) {
    # Something can't be found that needs to get executed
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/Warning:.+command not found/) {
    # This is a warning from Doxygen and not an error
    $self->Output_Warning ($s);
    return;
  }

  if ($s =~ m/cannot execute binary file/) {
    # Means we can't execute the binary, probably using target executable on host
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/command not found/) {
    # Means we can't find something to execute
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/(BUILD ERROR detected in [\w\/]+)/ ) {
    # If we see "BUILD ERROR detected in" then increment the build counter
    # but don't print it in the report.
    $self->Detected_Build_Error($1);
  }

  if ($s =~ m/^.*:[0-9]+: /) {
    # filename:linenumber is the typical format for an error
    if(# ... unless it is a warning
       $s =~ m/^.*:[0-9]+:\s+warning:/i
       # ... or a template location where it could be either a
       # warning or an error, but the lines around it would show the
       # real reason ...
       || $s =~ m/^.*:[0-9]+:\s+let op:/i
       || $s =~ m/^.*:[0-9]+:\s+note:/i
       || $s =~ m/^.*:[0-9]+:\s+instantiated\sfrom\s/) {
      if (defined $main::verbose and $main::verbose == 1) {
        print STDERR "Possible ERROR $s\n";
      }
      $self->Output_Warning ($s);
      return;
    }
    # It could also be part of a split line warning relating to
    # mktemp/mkstemp
    if (/mkstemp/) {
      $self->Output_Normal ($s);
      return;
    }
    # Definately an error
    $self->Output_Error ($s);
    return;
  }


  if ($s =~ m/ld: Can't find dependent library/) {
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/Assertion failed/) {
    # Definitely an error, can be given by the BCB6 compiler
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/unknown user/) {
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/^ld: \d+\-\d+/) {
    # AIX linking errors from ld
    if ($s =~ m/^ld: 0711\-345/) {
      # But don't report the extra "check the map" message
      $self->Output_Normal ($s);
      return;
    }
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/feupdateenv is not implemented and will always fail/) {
    # Library mismatch on Linux Intel C++ - can safely be ignored.
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/\s*warning:\s/i) {
    $self->Output_Warning ($s);
    return;
  }

  if ($s =~ m/exists but should be cleaned up/i) {
    $self->Output_Warning ($s);
    return;
  }

  if ($s =~ m/out of memory/) {
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/undefined reference to/
      || $s =~ m/: cannot open/
      || $s =~ m/: cannot find/
      || $s =~ m/: multiple definition of/
      || $s =~ m/path name does not exist/) {
    # Look for linking errors too
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/^make.*:.*Error [2-9]$/ ) {
    # We only want to flag as errors
    # make messages which are of the form:
    # make: ........Error 1
    # If we have Error 2 or higher, it is a recursive error, and has already
    # been flagged earlier at the occurrence of "Error 1".
    $self->Output_Normal ($s);
    return;
  }

  #(AHM) special case for HP aCC:
  # Warning #20013-D: Optimization aborted in function unknown. Attempting compilation unit again but dropping the optimization level to +O1. Please report this warning and error Signal 11 to HP. (20013)

  if (($s =~ m/\berror\b/i
       && $s !~ m/::error/i
       && $s !~ m;[/.]error;i
       && $s !~ m;error[/.];i
       && $s !~ m/ error\(s\), /
       && $s !~ m/error \(future\)/i)
      || $s =~ m/^Fatal\:/
      || $s =~ m/: fatal:/)
    {
      # Look for possible errors
      $self->Output_Error ($s);
      return;
    }

  # "Diagnostic messages have the following format when the -qsrcmsg option is specified:
  # x - 15dd-nnn (severity) text."
  # IBM compiler severity classification states:
  # (E) Error : Compilation continues and object code is generated. Error conditions exist that the compiler can correct, but the program might not produce the expected results.
  # (S) Severe Error : Compilation continues, but object code is not generated. Error conditions exist that the compiler cannot correct.
  # (U) Unrecoverable : The compiler halts. An internal compiler error has occurred
  # See further below for warnings.
  if ($s =~ m/.*\d+\-\d+:? \([ESU]\)/) {
    # Again, IBM's compilers speak in code langauge
    if ($s =~ m/.*Compilation will proceed shortly./) {
      # Ignore licensing messages
      $self->Output_Normal ($s);
      return;
    }
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/^LINK : warning LNK4089:/) {
    # Ignore this warning from MSVC
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/^ld: \d+\-\d+ WARNING: Duplicate symbol:/) {
    # AIX reports a bazillion multiple defines when doing templates.
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/vnocompatwarnings/) {
    # HP-UX uses 'nocompatwarnings' as an option to the compiler.
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/in call to __pthread_cleanup_push\(extern/) {
    # Solaris 8 defines __pthread_cleanup_push as a macro which
    # causes warnings. See /usr/include/pthread.h and
    # $ACE_ROOT/examples/Timer_Queue/Thread_Timer_Queue_Test.cpp
    # for more information.
    $self->Output_Normal ($s);
    return;
  }

  if ($s =~ m/in call to ASN1_dup\(extern/) {
    # Sun CC 5.x flags a warning regarding a problem in the
    # OpenSSL headers where the ASN1_dup() function expects an
    # extern "C" int(*)() function for its first parameter but a
    # non-extern "C" int(*)() function is passed in instead.  This
    # is a problem with the OpenSSL headers, not ACE/TAO/CIAO.
    $self->Output_Normal ($s);
    return;
  }

  if ( $s =~ m/^make.*\*\*\*/ ) {
    $self->Output_Error ($s);
    return;
  }

  if ( $s =~ m/No rule to make target/ ) {
    $self->Output_Error ($s);
    return;
  }

  if ( $s =~ m/redefinition after use/ ) {
    $self->Output_Error ($s);
    return;
  }

  if ( $s =~ m/The project cannot be loaded./ ) {
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/CXX\-I\-/) {
      $self->Output_Warning ($s);
      return;
  }

  if ($s =~ m/CXX\-W\-/) {
      $self->Output_Warning ($s);
      return;
  }

  if ($s =~ m/CXX\-E\-/) {
      $self->Output_Error ($s);
      return;
  }

  if ($s =~ m/LINK\-E\-/) {
      $self->Output_Error ($s);
      return;
  }

  if (($s =~ m/LINK\-W\-REXPORT/
       || $s =~ m/LINK\-W\-COMPWARN/
       || $s =~ m/LINK\-W\-SHRWRNERS/)) {
      $self->Output_Normal ($s);
      return;
  }

  if ($s =~ m/LINK\-W\-/) {
      $self->Output_Warning ($s);
      return;
  }

  if (($s =~ m/warning/i
       && ($s !~ m/ warning\(s\)/ && $s !~ m/\-undefined warning/ && $s !~ m/-i[^\s]*warning/i && $s !~ m/-l[^\s]*warning/i && $s !~ m/[^\s]+warning+[^\s]/i) && $s !~ m/0 warnings/)
      || $s =~ m/info: /i
      || $s =~ m/^error \(future\)/i
      || $s =~ m/^.*\.(h|i|inl|hpp|ipp|cpp|java): /)
    {
      # Catch any other warnings
      $self->Output_Warning ($s);
      return;
    }

  # "Diagnostic messages have the following format when the -qsrcmsg option is specified:
  # x - 15dd-nnn (severity) text."
  # IBM compiler severity classification scheme:
  # (I) Informational : Compilation continues. The message reports conditions found during compilation.
  # (W) Warning : Compilation continues. The message reports valid but possibly unintended conditions.
  if ($s =~ m/^.*\d+\-\d+:? \([WI]\)/) {
    # IBM's compilers don't say the word "warning" - check for their code
    if ($s =~ m/.*Compilation will proceed shortly./) {
      # Ignore licensing messages
      $self->Output_Normal ($s);
      return;
    }
    $self->Output_Warning ($s);
    return;
  }

  if ($s =~ m/ defined in discarded section /) {
    # Debian GCC compiler bug with templates/inline bits
    $self->Output_Warning ($s);
    return;
  }

  if ($s =~ m/mangled name collision for template function/) {
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/aborted/i ||
        $s =~ m/cannot access/i ||
        $s =~ m/nothing known about/ ||
        $s =~ m/processing is incomplete/ ||
        $s =~ m/syntax error near unexpected token/ ||
        $s =~ m/Error processing command/ ||
        $s =~ m/t connect to host/ ||
        $s =~ m/Checksum mismatch for/ ||
        $s =~ m/unknown user/ ||
        $s =~ m/Failed to add file/ ||
        $s =~ m/Failed to add directory/ ||
        $s =~ m/error: Autoconf version/ ||
        $s =~ m/m4 failed with/ ||
        $s =~ m/^ERROR/ ||
        $s =~ m/[^\w]+ERROR/ ||
        $s =~ m/no such file/i ||
        $s =~ m/BUILD FAILED/ ||
        $s =~ m/Error: Another instance of MPC is using/ ||
        $s =~ m/The following error occurred while executing this line/ ||
        $s =~ m/No commands are being checked/ ||
        $s =~ m/When Checking \"/ ||
        $s =~ m/No commands are being executed/ )
  {
        $self->Output_Error ($s);
  }
  elsif ($s =~ m/WARNING/ ||
            $s =~ m/IGNORING/ ||
            $s =~ m/has not been defined/ )
  {
        $self->Output_Warning ($s);
  }

  # OSPL additions from dcps_functions
  if ($s =~ m/No such file or directory/) {
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/ ## /) {
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/Could not load project file/) {
    $self->Output_Error ($s);
    return;
  }

  # 'command not found' already covered

  if ($s =~ m/syntax error/) {
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/Permission denied/) {
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/cannot create/) {
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/: is a directory/) {
    $self->Output_Error ($s);
    return;
  }

  if ($s =~ m/   Error building HDE./) {
    $self->Output_Error ($s);
    return;
  }

  # Must be normal
  $self->Output_Normal ($s);
}

1
