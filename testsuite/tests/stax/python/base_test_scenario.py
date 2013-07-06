import os

from ospl        import OSPL
from host        import Host
from test_errors import TestError
#===============================================================================
class BaseTestScenario:
    """Represents a single test scenario within a test."""
    NOT_TESTED = 0
    PASSED     = 1
    FAILED     = 2

    RESULT =\
        {NOT_TESTED : "NOT TESTED",
         PASSED     : "PASSED",
         FAILED     : "FAILED"}
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self, name = "", description = "", log_root = ".", result_file = ""):
        """Constructs a test scenario."""
        # The name of the scenario:
        self.name        = name
        # The description of the scenario:
        self.description = description
        # The root for the logs:
        self.log_root    = log_root

        # Result file with summary:
        self.result_file = result_file

        # The list of nodes.
        self.host_list = []

        # Result of the test scenario:
        self.result = BaseTestScenario.NOT_TESTED

        # Warning messages for the test scenraio:
        self.warnings = []
        # Error messages for the test scenraio:
        self.errors = []
        # Is test scenario initialized flag (false by default):
        self.initialized = 0
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def is_test_scenario_initialized(self):
        """Get test scenario initialized flag"""
        return self.initialized
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_test_scenario_initialized(self, flag):
        """Set test scenario initialized flag"""
        self.initialized = flag
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_name(self):
        """Get the scenario name"""
        return self.name
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_description(self):
        """Get the scenario description"""
        return self.description
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_log_root(self, log_root):
        """Set the scenario log root"""
        self.log_root = log_root
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_log_root(self):
        """Get the scenario log root"""
        return self.log_root
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def add_host(self, host):
        """Add new host to the scenraio"""
        self.host_list.append(host)
        self.check_host(host)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def define_host(self,
                    ospl_home,
                    test_root = ".",
                    ospl_uri  = "",
                    host_name = Host.LOCAL_HOST_NAME,
                    staf_port = Host.STAF_DEFAULT_PORT):
        """Define new host for the scenraio"""
        # Create new host:
        new_host = Host(ospl_home, test_root, ospl_uri, host_name, staf_port)
        # Add to the list:
        self.host_list.append(new_host)
        # Return the instance:
        return self.host_list[-1:][0]
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_host(self, host):
        """Check if the host object is not 'None'"""
        if host == None:
            raise TestError("Host::check_host - invalid value \"%s\" for the host object!"%\
                            host)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_all_hosts(self):
        """Return the scenario hosts list"""
        return self.host_list
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_result_file(self, file_name):
        """Set the file name for the summary file"""
        self.result_file = file_name
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_result_file(self):
        """Get the file name of the summary file"""
        return self.result_file
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def is_passed(self):
        """Check if test scenario is passed"""
        return (self.result == BaseTestScenario.PASSED)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def is_failed(self):
        """Check if test scenario is failed"""
        return (self.result == BaseTestScenario.FAILED)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def is_not_tested(self):
        """Check if test scenario is not tested"""
        return (self.result == BaseTestScenario.NOT_TESTED)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def ok(self):
        """Set test scenrio result ot PASSED"""
        self.result = BaseTestScenario.PASSED
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def fail(self):
        """Set test scenrio result ot FAILED"""
        self.result = BaseTestScenario.FAILED
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def not_tested(self):
        """Set test scenrio result to NOT_TESTED"""
        self.result = BaseTestScenario.NOT_TESTED
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_results(self):
        """Make all checks for the scenario"""
        print "Make all checks for the scenario"
        pass
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def analyze(self):
        print "ANALYZE"
        """Analize the test scenario results"""
        # If result file is not set:
        if self.result_file == "":
            # Exit:
            return

        # Result content:
        result_content = "[ TEST SCENARIO ]\n"
        result_content += "Name        = \"%s\"\n"% self.name
        result_content += "Description = \"%s\"\n"% self.description

        # Add all test scenario hosts: 
        if len(self.host_list) != 0:
            result_content += "[ HOSTS ] ---\n"
        for host in self.host_list:
            result_content += "%s\n"% host.get_host_name()

        # Add all test scenario warnings:
        if len(self.warnings) != 0:
            result_content += "[ WARNINGS ] ---\n"
        for warn in self.warnings:
            result_content += "[WARNING] %s.\n"% warn

        # Add all test scenario errors:
        if len(self.errors) != 0:
            result_content += "[ ERRORS ] ---\n"
        for err in self.errors:
            result_content += "[ERROR] %s!\n"% err

        # If no errors occured test is PASSED:
        if not self.initialized:
            self.not_tested()
        elif self.is_failed():
            self.fail()
        elif len(self.errors) == 0:
            self.ok()
        result_content += "\n--------------------\n"
        result_content += "RESULT: %s"% BaseTestScenario.RESULT[self.result]

        # Write result content:
        if not os.path.exists(os.path.split(self.result_file)[0]):
            os.makedirs(os.path.split(self.result_file)[0])
        print self.result_file
        file = open(self.result_file, 'w')
        file.write(result_content)
        file.close()

    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_host_by_role(self, role):
        """Get the host by role"""
        result = []
        for host in self.host_list:
            if role == host.get_role():
                result.append(host)
        return result
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_ospl_error_log(self, host):
        """Check if ospl-error.log exists and not empty"""
        ospl_error_log_file = host.get_ospl_log_dir() + host.get_file_sep() + "ospl-error.log" 
        print ospl_error_log_file
        if os.path.isfile(ospl_error_log_file):
            if os.path.getsize(ospl_error_log_file) != 0:
                return 1
        return 0
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_for_ospl_error_log(self):
        """Check for OSPL error log and set the warning if it exists"""
        # Check for OSPL error log:
        for host in self.host_list:
            ospl_error_log = self.log_root        +\
                             os.sep               +\
                             host.get_host_name() +\
                             os.sep               +\
                             OSPL.ospl_error_log_name
            if os.path.isfile(ospl_error_log):
                self.warnings.append("OSPL error log file [%s] is present for the host [%s]"%\
                                    (OSPL.ospl_error_log_name, host.get_host_name()))
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __str__(self):
        """String object representation"""
        string = "BaseTestScenario:\nname [%s]\ndescription [%s]\nlog root [%s]\n"%\
                 (self.name,
                  self.description,
                  self.log_root)
        for host in self.host_list:
            string += "=====\n"
            string += str(host)
        return string
#===============================================================================