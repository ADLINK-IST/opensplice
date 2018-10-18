import os
import sys

"""
   This class keeps a running total of the number of passes and failures, and
   writes the final results at the end of a test run.
   It also holds a flag which indicates if the test run is in debug mode - which
   basically means a few extra output messages are performed.
"""
class examplelogger(object):
    
    def __init__(self, debug):

        self.pass_count = 0

        self.fail_count = 0

        self.debug = debug

    """
       Increment pass count
    """
    def addPass(self):
        self.pass_count += 1

    """
       Increment failure count
    """
    def addFail(self):
        self.fail_count += 1

    """
       This method is called by the runExample.py script when the test run is over, it can therefore be called 
       after a full overnight run or by an individual engineer running a single example. 
    """
    def finalizeResults(self):

        logpath = os.path.join(os.environ['LOGDIR'], "examples", "run_" + os.environ['EXRUNTYPE'])

        if not os.path.isdir (logpath):
            os.mkdir(logpath)

        summ_html = os.path.join(logpath, "summary.html")
        summ_log = os.path.join(logpath, "summary.log")
        totals_log = os.path.join(logpath, "totals.log")
        issues_log = os.path.join(os.path.dirname(os.path.realpath(__file__)), "known_issues")

        total = self.pass_count + self.fail_count

        fs = open(summ_html, 'a')
        fs.write("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n")
        fs.write(" <HTML>\n")
        fs.write("<H2><a HREF=run_results.txt>Overview log</a></H2>\n")
        fs.write(" <TABLE>\n")
        fs.write(" <TBODY>\n")
        fs.write(" <TABLE border=1>\n")
        fs.write(" <TR><a HREF=examples.log>Results summary (examples.log)</a><br></TR>\n")
        fs.write(" <TR><a HREF=examples.log.gz>Results summary (examples.log.gz)</a><br></TR>\n")
        fs.write("<TR  bgcolor=white><TD>Examples Run = " + str(total) + "</TD><br></TR>\n")
        fs.write("<TR  bgcolor=white><TD>Examples Passed = " + str(self.pass_count) + "</TD><br></TR>\n")
        fs.write("<TR  bgcolor=white><TD>Examples Failed = " + str(self.fail_count) + "</TD><br></TR>\n")
        fs.write("<TR  bgcolor=white><TD>Example</a></TD><TD>Language</TD><TD>Failure Reason</TD><TD>Log</TD></FONT><br></TR>\n")
        
        if os.path.isfile (summ_log):
            with open (summ_log) as f:
                for line in f:
                    fs.write(line)

        fs.write("</TBODY>\n")
        fs.write("</TABLE>\n")

        with open (issues_log) as f:
            for line in f:
                fs.write(line)

        fs.write("</HTML")
        fs.close()

        try:
            if os.path.isfile (summ_log):
                os.remove(summ_log)
        except:
            print "Exception trying to delete summary.log "

        fs = open(totals_log, 'w')
        fs.write("Examples Run = " + str(total) + "\n")
        fs.write("Examples Passed = " + str(self.pass_count) + "\n")
        fs.write("Examples Failed = " + str(self.fail_count) + "\n")
        fs.close()
