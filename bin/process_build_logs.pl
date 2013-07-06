eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
  & eval 'exec perl -S $0 $argv:q'
  if 0;

use FindBin;
use lib $FindBin::Bin;

use common::prettify;
use Getopt::Std;
use strict;
use FileHandle;
use File::Basename;

# NB - whilst this appears to be unused it's required for prettify.pm
# to compile.
sub GetVariable ($)
{
   my %a=();
   return $a{'UNDEFINED'};
}

our $VERSION = "0.0";

sub usage
{
    my $fp = shift;
    my $scriptname = basename($0);
    print $fp <<EOT;
$scriptname : Run prettify to produce warning / error logs.
                        Output will be written to pwd unless -d is set.

$scriptname [-h] [-f] [-d log directory]
    -d log dir   Specify an overnight build log directory to be processed.
                 The default named build files will be processed if found and
                 all output written in that dir.
    -e log dir   Specify a log directory to be processed. Anything found will be processed
                 and all output written in that dir.
    -f file name Specify a single additional or alternate build log file to be
                 processed.
    -h / --help  Show this help output
Iff neither -d or -f are set input is read from STDIN. Emits 'Totals' file name
on STDOUT & returns 0 if no errors are found in any logs.

Hint, try:
    make 2>&1 | $scriptname | xargs firefox &

EOT
}

sub HELP_MESSAGE
{
    usage(shift);
    exit(0);
}

$Getopt::Std::STANDARD_HELP_VERSION = 1;

our($opt_d, $opt_e, $opt_h, $opt_f);

my $tmpfilename = "BuildResults.txt";

if (!getopts ('hd:e:f:') || $opt_h) {
    usage(*STDERR);
    exit (1);
}

my @filenames;

if (defined $opt_d)
{
    chdir $opt_d || die "Couldn't chdir to $opt_d";
    @filenames = ( ["./build.txt", "OSPL Core Build"],
                   ["./build-dbt-tests.txt", "OSPL DBT Tests Build"],
                   ["./build-rbt-tests.txt", "OSPL RBT Tests Build"] );
    $tmpfilename = "ScoreboardBuildResults.txt"
}

if (defined $opt_e)
{
    chdir $opt_e || die "Couldn't chdir to $opt_e";
    unlink("$tmpfilename");
    my @myfiles = glob("*");
    foreach my $file (@myfiles)
    {
        push @filenames, [$file, "Results File $file"];
    }
}

if (defined $opt_f)
{
    if (-e $opt_f)
    {
        push @filenames, [$opt_f, "Results File $opt_f"];
        $tmpfilename = $opt_f . ".txt";
    }
    else
    {
        die "File $opt_f does not exist. See option -f";
    }
}

if ((!defined($opt_f)) && (!defined($opt_d)) && (!defined($opt_e)))
{
    push @filenames, [ "&STDIN", "Reading from stdin" ];
}

unlink("$tmpfilename");
my $results_file_handle = new FileHandle ("$tmpfilename", 'w');

defined($results_file_handle) || die "Cannot open temp output file $tmpfilename\n";

my $something_written = 0;

for my $filename ( @filenames )
{
    print $results_file_handle "#################### Compile ($filename->[1]) [" . (scalar gmtime(time())) . " UTC]\n";
    open READFILE, "<$filename->[0]";
    while (<READFILE>)
    {
        print $results_file_handle $_;
        $something_written = 1;
    }
    close(READFILE);
    print $results_file_handle "\n####################\n";
}

$results_file_handle->close();

if ($something_written)
{
    my $basename = $tmpfilename;
    $basename =~ s/\.txt$//;

    my $processor = new Prettify ($basename);

    my $input = new FileHandle ($tmpfilename, 'r');

    while (<$input>) {
        chomp;
        $processor->Process_Line ($_);
    }

    # emit the outputted _Totals.html filename.
    my $totals_file_name = File::Spec->catfile( ((defined($opt_d)) ? ($opt_d) : (File::Spec->curdir())),
                                                 substr($tmpfilename, 0, rindex($tmpfilename, ".txt")) );
    print STDOUT $totals_file_name . "_Totals.html\n";

    my $errors = $processor->ErrorCount();
    if ($errors > 0)
    {
        unlink("$tmpfilename");
        exit 1;
    }
}
else
{
    print STDERR "ERROR: nothing could be read from any input";
}

unlink("$tmpfilename");

exit ! $something_written;
