# Add the path to the STAX/Python framework:
import os
import re
import sys
sys.path.append("../stax/python")

from test_errors        import TestError
from base_test_scenario import BaseTestScenario
#===============================================================================
class DDS2734TransLocParser:
    """Helper class to extract all information from the test logs"""
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self):
        """Constructs the object"""
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_process_log_content(self, log_root, host, process_name):
        
        print log_root
        print host
        print process_name
        print host.get_process_by_name(process_name)
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
class DDS2734TransLocTestScenario(BaseTestScenario):
    """Test scenario for the DDS2734 test"""
    # DDS2734 test types:
    TRANSLOC_NORMAL   = 0
    TRANSLOC_LATE_READER = 1
    TRANSLOC_LATE_NODE = 2
    TRANSLOC_TOO_LATE_READER = 3
    TRANSLOC_TOO_LATE_NODE = 4

    TEST_TYPES = ["transloc_normal",
                  "transloc_late_reader",
                  "transloc_late_node",
                  "transloc_too_late_reader",
                  "transloc_too_late_node"]
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self, type):
        """Constructs the object"""

        # Call base init with the predifined params:
        BaseTestScenario.__init__(
            self,
            "dds2734_" + DDS2734TransLocTestScenario.TEST_TYPES[type],
            "The transient local test")

        self.type   = type
        self.parser = DDS2734TransLocParser()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def all_messages_sent(self, log_content):
        for id in range(0, 10):
            if log_content.find("Writing test topic: %i." %(id)) == -1:
                return 0
            return 1    
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def all_messages_red(self, log_content):
        for id in range(0, 10):
            if log_content.find("Reading testtopic: %i." %(id)) == -1:
                return 0
            return 1    
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def at_least_one_message_red(self, log_content):
        for id in range(0, 10):
            if log_content.find("Reading testtopic: %i." %(id)) != -1:
                return 1
            return 0
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def all_sent_nothing_red(self, log_contents):
        if not self.all_messages_sent(log_contents["Pub"]):
            raise TestError("DDS2734TransLocTestScenario. Publisher didn't send all messages")
        if self.at_least_one_message_red(log_contents["Pub"]):
            raise TestError("DDS2734TransLocTestScenario. Subscriber recieved some messages")
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def all_sent_and_red(self, log_contents):
        if not self.all_messages_sent(log_contents["Pub"]):
            raise TestError("DDS2734TransLocTestScenario. Publisher didn't send all messages")
        if not self.all_messages_red(log_contents["Sub"]):
            raise TestError("DDS2734TransLocTestScenario. Subscriber didn't recieve all messages")
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_transloc_normal(self, log_contents):
        self.all_sent_and_red(log_contents)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_transloc_late_reader(self, log_contents):
        self.all_sent_and_red(log_contents)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_transloc_too_late_reader(self, log_contents):
        self.all_sent_nothing_red(log_contents)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_transloc_late_node(self, log_contents):
        self.all_sent_and_red(log_contents)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_transloc_too_late_node(self, log_contents):
        self.all_sent_nothing_red(log_contents)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def analyze(self):
        """Analize the test scenario results"""
        if not self.is_failed():
            try:
                # Check for the OSPL error log file:
                self.check_for_ospl_error_log()

                # Get test hosts:
                hosts = {}
                hosts["Pub"] = self.get_host_by_role("Pub")[0]
                hosts["Sub"] = self.get_host_by_role("Sub")[0]

                # Read node mopnitor logs:
                app_log_contents = {}
                app_log_contents["Pub"] = self.parser.get_process_log_content(
                    self.log_root,
                    hosts["Pub"],
                    "dds2734_publisher")
                app_log_contents["Sub"] = self.parser.get_process_log_content(
                    self.log_root,
                    hosts["Sub"],
                    "dds2734_subscriber")

                for index in app_log_contents.keys():
                    if len(app_log_contents[index]) == 0:
                        raise TestError("DDS2734TestScenario::analyze - empty application log for node [%s]"% hosts[index].get_host_name())

                # Check test case expected result:
                if self.type == DDS2734TransLocTestScenario.TRANSLOC_NORMAL:
                    self.check_transloc_normal(app_log_contents)
                elif  self.type == DDS2734TransLocTestScenario.TRANSLOC_LATE_READER:
                    self.check_transloc_late_reader(app_log_contents)
                elif self.type == DDS2734TransLocTestScenario.TRANSLOC_LATE_NODE:
                    self.check_transloc_late_node(app_log_contents)
                elif self.type == DDS2734TransLocTestScenario.TRANSLOC_TOO_LATE_READER:
                    self.check_transloc_too_late_reader(app_log_contents)
                elif self.type == DDS2734TransLocTestScenario.TRANSLOC_TOO_LATE_NODE:
                    self.check_transloc_too_late_node(app_log_contents)
                
            except:
                self.fail()
                self.errors.append("Cannot analyze results: %s %s"% (sys.exc_info()[0], sys.exc_info()[1]))

        # Call parent analyze to create log file:
        BaseTestScenario.analyze(self)
#===============================================================================
