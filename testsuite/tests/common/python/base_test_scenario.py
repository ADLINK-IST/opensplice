import os

from TestHost        import TestHost
from test_errors     import TestError
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
        self.hosts_by_role = {}

        # Result of the test scenario:
        self.result = BaseTestScenario.NOT_TESTED

        # Warning messages for the test scenraio:
        self.warnings = []
        # Error messages for the test scenraio:
        self.errors = []
        # Is test scenario initialized flag (false by default):
        self.initialized = 0
        
        self.host_app_logs = {}
        self.ospl_error_log_name = "ospl-error.log"
        self.ospl_info_log_name = "ospl-info.log"
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def is_test_scenario_initialized(self):
        """Get test scenario initialized flag"""
        return self.initialized
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_test_scenario_initialized(self, flag = 1):
        """Set test scenario initialized flag"""
        self.initialized = flag
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_name(self):
        """Get the scenario name"""
        return self.name
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_name(self, name):
        """Get the scenario name"""
        self.name = name
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
        
    def set_role_for_host(self, host, role):
        """Add host:role pair to the hosts_by_role"""
        self.hosts_by_role[host] = role
    
    def get_role_by_host(self, host):
        """Get role of host"""
        return self.hosts_by_role.get(host, '')
    
    def get_host_by_role(self, role):
        """Get host by role"""
        for host in self.hosts_by_role.keys():
            if self.hosts_by_role[host] == role:
                return host
                
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def define_host(self,
                    ospl_home,
                    test_root = ".",
                    ospl_uri  = "",
                    host_name = TestHost.LOCAL_HOST_NAME,
                    staf_port = 6500):
        """Define new host for the scenraio"""
        # Create new host:
        new_host = TestHost(ospl_home = ospl_home, test_root = test_root, ospl_uri = ospl_uri, hostname = host_name, port = staf_port, targets = 'x86.win32')
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
        self.check_ospl_error_log()
        self.check_ospl_info_log()
        self.check_app_log_files()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_host_app_log_file(self, host, file_name):
        try:
            self.host_app_logs[host].append(file_name)
        except KeyError:
            self.host_app_logs[host] = []
            self.host_app_logs[host].append(file_name)
            
            
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
            result_content += "%s\n"% host.get_hostname()

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
    def check_app_log_files(self):
        """Check files with file_name for error messages"""
        for host in  self.host_app_logs.keys():
            for file in self.host_app_logs[host]:
                app_log_file = os.path.join(self.log_root, host.get_hostname(), file)
                if not os.path.isfile(app_log_file):
                        self.errors.append("Application log file %s is not present for the host [%s]"%\
                                            (file_name, host.get_hostname()))
                        return
                        
                f = open(app_log_file, 'r')
                for x in f.readlines():
                    if x.lower().find('error') != -1 or x.lower().find('fail') != -1:
                        self.errors.append("Application log file %s for the host [%s] contains error or fail messages"%\
                                    (file_name, host.get_hostname()))
                        break
                f.close()

    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_ospl_error_log(self):
        """Check for OSPL error log and set the warning if it exists"""
        for host in self.host_list:
            ospl_error_log = os.path.join(self.log_root, host.get_hostname(), self.ospl_error_log_name)
            if os.path.isfile(ospl_error_log):
                self.warnings.append("OSPL error log file [%s] is present for the host [%s]"%\
                                    (self.ospl_error_log_name, host.get_hostname()))
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    def check_ospl_info_log(self):
        """Check OSPL info log for error messages and set the warning if it doesn't exist"""
        # Check for OSPL error log:
        for host in self.host_list:
            ospl_info_log = os.path.join(self.log_root, host.get_hostname(), self.ospl_info_log_name)
                             
            if not os.path.isfile(ospl_info_log):
                self.warnings.append("OSPL info log file [%s] isn't present for the host [%s]"%\
                                    (self.ospl_info_log_name, host.get_hostname()))
            else:
                f = open(ospl_info_log)
                for x in f.readlines():
                    if x.lower().find('error') != -1:
                        self.warnings.append("OSPL info log file [%s] for the host [%s] contains error messages"%\
                                    (self.ospl_info_log_name, host.get_hostname()))
                        break
                f.close()
                
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __str__(self):
        '''String object representation'''
        string = "BaseTestScenario:\nname [%s]\ndescription [%s]\nlog root [%s]\n"%\
                 (self.name,
                  self.description,
                  self.log_root)
        for host in self.host_list:
            string += "=====\n"
            string += str(host)
        return string
#===============================================================================