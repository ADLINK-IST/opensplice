#!/usr/bin/perl -w

my $stackFrame = '^(\S+)\((\S*)\)\s+\[(\S+)\]';
my $gdbListItem = '^\(gdb\)\s*(.+)';
my $gdbScript = 'gdb_script.txt';

while ($string = <>) {
    chomp $string;
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

unlink($gdbScript);