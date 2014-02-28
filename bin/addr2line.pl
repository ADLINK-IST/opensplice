#!/usr/bin/perl -w

my $stackFrame = '^(\S+)\((\S*)\)\s+\[(\S+)\]';
my $gdbListItem = '^\(gdb\)\s*(.+)';
my $gdbScript = 'gdb_script.txt';

my $stackStart = '^={12}.*\((0x[[:xdigit:]]+)\).*={12}$';
my @pointers = ();
my $processLine = 1;

while (my $arg = shift @ARGV) {
    if ($arg =~ /^0x[[:xdigit:]]+$/) {
        push(@pointers, $arg);
    } else {
        unshift(@ARGV, $arg);
        last;
    }
}

while ($string = <>) {
    chomp $string;
    if (@pointers && ((my $pointer) = $string =~ /$stackStart/)) {
        if (/$pointer/i ~~ @pointers) {
            $processLine = 1;
            print $string, "\n";
        } else {
            $processLine = 0;
        }
    } elsif ($processLine) {
        (my $deliverable, my $functionAddr, my $absAddr) = $string =~ /$stackFrame/;
        if ($deliverable && $absAddr) {
            if ($functionAddr) {
                $gdbCmd = "l *($functionAddr)";
            } else {
                $gdbCmd = "l *($absAddr)";
            }
            open (FILEHANDLE, ">$gdbScript");
            print FILEHANDLE $gdbCmd;
            close (FILEHANDLE);
            my $gdbExec = "gdb --quiet $deliverable 2>&1 < $gdbScript";
            my @srcLine = qx($gdbExec);
            if ($#srcLine > 5) {
                my @strippedSrcLine = $srcLine[1] =~ /$gdbListItem/;
                print $strippedSrcLine[0], "\n";
            } else {
                print $string, "\n";
            }
        } else {
            print $string, "\n";
        }
    }
}

unlink($gdbScript);
