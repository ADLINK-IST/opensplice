from base_test_scenario import BaseTestScenario
from ospl               import OSPL
import os

#===============================================================================
class TouchStoneTestScenario(BaseTestScenario):
    watcher_log = "watcher.log"
    """
        Represents a single test TouchStone scenario within a test.
    """
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self, name = "", description = "", log_root = "."):
        """Constructs a test scenario."""
        BaseTestScenario.__init__(self, name, description, log_root)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_watcher_log(self, log_dir, host):
        """Check if watcher log created and is not empty"""
        print 'check_watcher_log'
        watcher_log_file_name = os.path.join(log_dir, self.watcher_log)
        print watcher_log_file_name
        if not os.path.isfile(watcher_log_file_name):
            self.errors.append("Watcher log file [%s] from the host [%s] doesn't exist."%(watcher_log_file_name, host.get_host_name()))
            return
        if os.path.getsize(watcher_log_file_name) == 0:
            self.errors.append("Watcher log file [%s] from the host [%s] is empty."%(watcher_log_file_name, host.get_host_name()))
            return
        f = open(watcher_log_file_name, 'r')
        try:
            content = f.read().lower()
        finally:
            f.close()    

        if content.find('error') != -1:
            self.errors.append("Watcher log file [%s] from the host [%s] contains error messages"%(watcher_log_file_name, host.get_host_name()))
            return
        if content.find(' cnt= 0, min= 0, avg= 0, max= 0') != -1:
            self.errors.append("Watcher log file [%s] from the host [%s] contains zero values"%(watcher_log_file_name, host.get_host_name()))
            return
        if content.find('trip latency:') == -1 or content.find('inter arrival time:') == -1:
            self.errors.append("The wrong format of watcher log file [%s] from the host [%s]"%(watcher_log_file_name, host.get_host_name()))
            return
        
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_results(self, log_dir, host):
        """Make all checks for the scenario"""
        print "check results"
        # Check for the "ospl-error.log":
        
        if os.path.isfile(os.path.join(log_dir, OSPL.ospl_error_log_name)):
            self.warnings.append("OSPL error log file [%s] is present for the host [%s]"%\
                                    (OSPL.ospl_error_log_name, host.get_host_name()))
        self.check_watcher_log(log_dir, host)
        
#===============================================================================