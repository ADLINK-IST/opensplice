# Add the path to the STAX/Python framework:
import os
import re
import sys
sys.path.append("../stax/python")

from test_errors        import TestError
from base_test_scenario import BaseTestScenario
#===============================================================================
class DDS2734Parser:
    """Helper class to extract all information from the test logs"""
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self):
        """Constructs the object"""
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_process_log_content(self, log_root, host, process_name):
        """Reads file and return its content"""
        # Get process log path name on the host:
        log_name = host.get_process_by_name(process_name).get_log_file()
        # Get process log name:
        log_name = log_name[log_name.rfind(host.get_file_sep()) + 1:]
        # Get process log path name on the localhost:
        log_name = log_root + os.sep + host.get_host_name() + os.sep + log_name

        # Get process content:
        file = open(log_name, "r")
        content = file.read()
        file.close()

        return content
#===============================================================================
class DDS2734TestScenario(BaseTestScenario):
    """Test scenario for the DDS2734 test"""
    # DDS2734 test types:
    MULTI_FULL   = 0
    MULTI_PART   = 1
    MULTI_NONE   = 2
    UNI_FULL     = 3
    UNI_SINGLE   = 4
    MIXED_FULL   = 5
    MIXED_SINGLE = 6
    MIXED_EMPTY  = 7
    MIXED_EMPTY2 = 8
    TEST_TYPES = ["multi_full",
                  "multi_part",
                  "multi_none",
                  "uni_full",
                  "uni_single",
                  "mixed_full",
                  "mixed_single",
                  "mixed_empty",
                  "mixed_empty2"]
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self, type):
        """Constructs the object"""

        # Call base init with the predifined params:
        BaseTestScenario.__init__(
            self,
            "dds2734_" + DDS2734TestScenario.TEST_TYPES[type],
            "The test case tests the discovery process of the configured networking service")

        self.type   = type
        self.parser = DDS2734Parser()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_for_host_presence(self, host, monitor_content, host_to_discover):
        """Find the host_name in the node monitor log
           (it is present if the monitor has discovered it)"""

        # If host was not discovered:
        if monitor_content.find(host_to_discover.get_host_name()) == -1:
            raise TestError("DDS2734TestScenario::check_for_host_presence - [%s] was not discovered by the host [%s]"% (host_to_discover.get_host_name(), host.get_host_name()))
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_for_host_absence(self, host, monitor_content, host_to_discover):
        """Find the host_name in the node monitor log
           (it is present if the monitor has discovered it)"""

        # If host was not discovered:
        if monitor_content.find(host_to_discover.get_host_name()) != -1:
            raise TestError("DDS2734TestScenario::check_for_host_absence - [%s] discovered by the host [%s]"% (host_to_discover.get_host_name(), host.get_host_name()))
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_all_hosts_presence(self, hosts, contents):
        """Check if each host found the other"""
        # Find discovered hosts:
        for i in hosts.keys():
            for j in hosts.keys():
                if i != j:
                    self.check_for_host_presence(hosts[i], contents[i], hosts[j])
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_all_hosts_absence(self, hosts, contents):
        """Check if each host not found the other"""
        # Find discovered hosts:
        for i in hosts.keys():
            for j in hosts.keys():
                if i != j:
                    self.check_for_host_presence(hosts[i], contents[i], hosts[j])
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_multi_full_case(self, hosts, contents):
        """Check for OSPL_SYSTEM_NETWORK_CONNECTIVITY_MULTI_FULL case"""
        # All nodes should discover the others:
        self.check_all_hosts_presence(hosts, contents)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_multi_part_case(self, hosts, contents):
        """Check for OSPL_SYSTEM_NETWORK_CONNECTIVITY_MULTI_PART case"""
        # "A" should disover "B":
        self.check_for_host_presence(hosts["A"], contents["A"], hosts["B"])
        # "B" should disover "A":
        self.check_for_host_presence(hosts["B"], contents["B"], hosts["A"])
        # "C" should not disover "A":
        self.check_for_host_absence(hosts["C"], contents["C"], hosts["A"])
        # "C" should not disover "B":
        self.check_for_host_absence(hosts["C"], contents["C"], hosts["B"])
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_multi_none_case(self, hosts, contents):
        """Check for OSPL_SYSTEM_NETWORK_CONNECTIVITY_MULTI_NONE case"""
        # All nodes should not discover the others:
        self.check_all_hosts_absence(hosts, contents)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_uni_full_case(self, hosts, contents):
        """Check for OSPL_SYSTEM_NETWORK_CONNECTIVITY_UNI_FULL case"""
        # All nodes should discover the others:
        self.check_all_hosts_presence(hosts, contents)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_uni_single_case(self, hosts, contents):
        """Check for OSPL_SYSTEM_NETWORK_CONNECTIVITY_UNI_SINGLE case"""
        # "A" should disover "B":
        self.check_for_host_presence(hosts["A"], contents["A"], hosts["B"])
        # "A" should disover "C":
        self.check_for_host_presence(hosts["A"], contents["A"], hosts["C"])
        # "C" should disover "A":
        self.check_for_host_presence(hosts["C"], contents["C"], hosts["A"])
        # "B" should disover "A":
        self.check_for_host_presence(hosts["B"], contents["B"], hosts["A"])
        # "C" should not disover "B":
        self.check_for_host_absence(hosts["C"], contents["C"], hosts["B"])
        # "B" should not disover "C":
        self.check_for_host_absence(hosts["B"], contents["B"], hosts["C"])
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_mixed_full_case(self, hosts, contents):
        """Check for OSPL_SYSTEM_NETWORK_CONNECTIVITY_MIXED_FULL case"""
        # All nodes should discover the others:
        self.check_all_hosts_presence(hosts, contents)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_mixed_single_case(self, hosts, contents):
        """Check for OSPL_SYSTEM_NETWORK_CONNECTIVITY_MIXED_SINGLE case"""
        # All nodes should discover the others:
        self.check_all_hosts_presence(hosts, contents)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_mixed_empty_case(self, hosts, contents):
        """Check for OSPL_SYSTEM_NETWORK_CONNECTIVITY_MIXED_EMPTY case"""
        # All nodes should discover the others:
        self.check_all_hosts_presence(hosts, contents)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_mixed_empty2_case(self, hosts, contents):
        """Check for OSPL_SYSTEM_NETWORK_CONNECTIVITY_MIXED_EMPTY2 case"""
        # "A" should disover "B":
        self.check_for_host_presence(hosts["A"], contents["A"], hosts["B"])
        # "A" should disover "C":
        self.check_for_host_presence(hosts["A"], contents["A"], hosts["C"])
        # "C" should disover "A":
        self.check_for_host_presence(hosts["C"], contents["C"], hosts["A"])
        # "B" should disover "A":
        self.check_for_host_presence(hosts["B"], contents["B"], hosts["A"])
        # "C" should not disover "B":
        self.check_for_host_absence(hosts["C"], contents["C"], hosts["B"])
        # "B" should not disover "C":
        self.check_for_host_absence(hosts["B"], contents["B"], hosts["C"])
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def analyze(self):
        """Analize the test scenario results"""
        if not self.is_failed():
            try:
                # Check for the OSPL error log file:
                self.check_for_ospl_error_log()

                # Get test hosts:
                hosts = {}
                hosts["A"] = self.get_host_by_role("A")[0]
                hosts["B"] = self.get_host_by_role("B")[0]
                hosts["C"] = self.get_host_by_role("C")[0]

                # Read node mopnitor logs:
                monitor_contents = {}
                monitor_contents["A"] = self.parser.get_process_log_content(
                    self.log_root,
                    hosts["A"],
                    "NodeMonitor")
                monitor_contents["B"] = self.parser.get_process_log_content(
                    self.log_root,
                    hosts["B"],
                    "NodeMonitor")
                monitor_contents["C"] = self.parser.get_process_log_content(
                    self.log_root,
                    hosts["C"],
                    "NodeMonitor")

                for index in monitor_contents.keys():
                    if len(monitor_contents[index]) == 0:
                        raise TestError("DDS2734TestScenario::analyze - empty node monitor log for node [%s]"% hosts[index].get_host_name())

                # Check test case expected result:
                if self.type == DDS2734TestScenario.MULTI_FULL:
                    self.check_multi_full_case(hosts, monitor_contents)
                
                if self.type == DDS2734TestScenario.MULTI_PART:
                    self.check_multi_part_case(hosts, monitor_contents)
                
                if self.type == DDS2734TestScenario.MULTI_NONE:
                    self.check_multi_none_case(hosts, monitor_contents)

                if self.type == DDS2734TestScenario.UNI_FULL:
                    self.check_uni_full_case(hosts, monitor_contents)

                if self.type == DDS2734TestScenario.UNI_SINGLE:
                    self.check_uni_single_case(hosts, monitor_contents)

                if self.type == DDS2734TestScenario.MIXED_FULL:
                    self.check_mixed_full_case(hosts, monitor_contents)

                if self.type == DDS2734TestScenario.MIXED_SINGLE:
                    self.check_mixed_single_case(hosts, monitor_contents)

                if self.type == DDS2734TestScenario.MIXED_EMPTY:
                    self.check_mixed_empty_case(hosts, monitor_contents)

                if self.type == DDS2734TestScenario.MIXED_EMPTY2:
                    self.check_mixed_empty2_case(hosts, monitor_contents)
            except:
                self.fail()
                self.errors.append("Cannot analyze results: %s %s"% (sys.exc_info()[0], sys.exc_info()[1]))

        # Call parent analyze to create log file:
        BaseTestScenario.analyze(self)
#===============================================================================
