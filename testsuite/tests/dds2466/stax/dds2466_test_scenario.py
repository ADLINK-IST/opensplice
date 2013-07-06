# Add the path to the STAX/Python framework:
import os
import re
import sys
sys.path.append("../../stax/python")

from ospl               import OSPL
from test_errors        import TestError
from base_test_scenario import BaseTestScenario
#===============================================================================
class DDS2466Topic:
    """Class contatins all sample fields from DDS2466 topic
       and information about sample: pub/sub ids"""
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self):
        """Constructs the object"""
        self.pub_id            = 0
        self.sub_id            = 0
        self.id                = 0
        self.data              = ""
        self.timestamp_sec     = 0
        self.timestamp_nanosec = 0
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __str__(self):
        """String object representation"""
        return "pub id: %s\nsub id: %s\nmsg id: %s\nmsg data: %s\ntimestamp sec: %s\ntimestamp nanosec: %s\n"%\
               (self.pub_id, self.sub_id, self.id, self.data, self.timestamp_sec, self.timestamp_nanosec)
#===============================================================================
class DDS2466PublisherOutput:
    """Helper class to extract fields from the DDS2466 pulisher log"""
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self):
        """Constructs the object"""
        # Regular expressions to:
        # Get caption:
        self.caption_re = r"(?<=^DDS2466\s)[a-zA-Z]+"
        # Get publisher id:
        self.pub_id_re = r"(?<=^Publisher\[)[0-9]+"
        # Get instance msg id:
        self.pub_msg_id_re = r"(?<=sent\sid\s\[)[0-9]+"
        # Get instance msg data:
        self.pub_msg_data_re = r"(?<=data\s\[)(.*)\."
        # Get instance timestpamp [sec]:
        self.pub_timestamp_sec_re = r"(?<=time\sstamp\s\[)[0-9]+"
        # Get instance timestpamp [nanosec]:
        self.pub_timestamp_nanosec_re = r"(?<=:)[0-9]+"
#===============================================================================
class DDS2466SubscriberOutput:
    """Helper class to extract fields from the DDS2466 subscriber log"""
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self):
        """Constructs the object"""
        # Regular expressions to:
        # Get caption:
        self.caption_re = r"(?<=^DDS2466\s)[a-zA-Z]+"
        # Get subscriber id:
        self.sub_id_re = r"(?<=^Subscriber\[)[0-9]+"
        # Get publisher id:
        self.pub_id_re = r"(?<=from\sPublisher\[)[0-9]+"
        # Get instance msg id:
        self.sub_msg_id_re = r"(?<=with\sid\s\[)[0-9]+"
        # Get instance msg data:
        self.sub_msg_data_re = r"(?<=data\s\[)(.*)\."
        # Get instance timestpamp [sec]:
        self.sub_timestamp_sec_re = r"(?<=time\sstamp\s\[)[0-9]+"
        # Get instance timestpamp [nanosec]:
        self.sub_timestamp_nanosec_re = r"(?<=:)[0-9]+"
#===============================================================================
class DDS2466Parser:
    """Helper class to extract all information from the test logs"""
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self):
        """Constructs the object"""

        self.pub_output = DDS2466PublisherOutput()
        self.sub_output = DDS2466SubscriberOutput()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_samples(self, host, report_content):
        """Get samples list from the content (log file content)
            for the publisher or sucscriber"""

        # Get caption from the log:
        pub_caption_match = re.search(self.pub_output.caption_re,
                                      report_content)
        # Get caption from the log:
        sub_caption_match = re.search(self.sub_output.caption_re,
                                      report_content)

        # If the caption is from a publisher:
        if (pub_caption_match != None and pub_caption_match.group() == "publisher"):
            return self.get_pub_samples(host, report_content)

        # If the caption is from a subscriber:
        if (sub_caption_match != None and sub_caption_match.group() == "subscriber"):
            return self.get_sub_samples(host, report_content)

        return []
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_pub_samples(self, host, pub_content):
        """Get publisher sample list from the log file content"""
        samples = []
        # Generate list of file lines:
        content_lines = pub_content.split(host.get_line_sep())
        # Parse all lines:
        for line in content_lines:
            if line == "":
                continue
            # Find all fields values:
            pub_id_match            = re.search(self.pub_output.pub_id_re, line)
            msg_id_match            = re.search(self.pub_output.pub_msg_id_re, line)
            msg_data_match          = re.search(self.pub_output.pub_msg_data_re, line)
            timestamp_sec_match     = re.search(self.pub_output.pub_timestamp_sec_re, line)
            timestamp_nanosec_match = re.search(self.pub_output.pub_timestamp_nanosec_re, line)
            # If all values found:
            if (pub_id_match            != None and\
                msg_id_match            != None and\
                msg_data_match          != None and\
                timestamp_sec_match     != None and\
                timestamp_nanosec_match != None):
                # Create sample and fill with the values:
                sample = DDS2466Topic()
                sample.pub_id            = pub_id_match.group()
                sample.id                = msg_id_match.group()
                sample.data              = msg_data_match.group()
                sample.timestamp_sec     = timestamp_sec_match.group()
                sample.timestamp_nanosec = timestamp_nanosec_match.group()
                samples.append(sample)
        return samples
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_sub_samples(self, host, sub_content):
        """Get subscriber sample list from the log file content"""
        samples = []
        # Generate list of file lines:
        content_lines = sub_content.split(host.get_line_sep())
        # Pasrse all lines:
        for line in content_lines:
            if line == "":
                continue
            # Find all fields values:
            sub_id_match            = re.search(self.sub_output.sub_id_re, line)
            pub_id_match            = re.search(self.sub_output.pub_id_re, line)
            msg_id_match            = re.search(self.sub_output.sub_msg_id_re, line)
            msg_data_match          = re.search(self.sub_output.sub_msg_data_re, line)
            timestamp_sec_match     = re.search(self.sub_output.sub_timestamp_sec_re, line)
            timestamp_nanosec_match = re.search(self.sub_output.sub_timestamp_nanosec_re, line)
            # If all values found:
            if (sub_id_match            != None and\
                pub_id_match            != None and\
                msg_id_match            != None and\
                msg_data_match          != None and\
                timestamp_sec_match     != None and\
                timestamp_nanosec_match != None):
                # Create sample and fill with the values:
                sample = DDS2466Topic()
                sample.pub_id            = pub_id_match.group()
                sample.sub_id            = sub_id_match.group()
                sample.id                = msg_id_match.group()
                sample.data              = msg_data_match.group()
                sample.timestamp_sec     = timestamp_sec_match.group()
                sample.timestamp_nanosec = timestamp_nanosec_match.group()
                samples.append(sample)
        return samples
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
class DDS2466TestScenario(BaseTestScenario):
    """Test scenario for the DDS2466 test"""
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self):
        """Constructs the object"""

        # Call base init with the predifined params:
        BaseTestScenario.__init__(
            self,
            "dds2466",
            "CoFlight requirements eFDPfi_MW_DDS_15")

        self.parser = DDS2466Parser()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_for_timestamps(self, pub_data, sub_data):
        """Checks that all received sample instances have the same timestamp
           as used to write them"""

        # Publisher/subscriber samples list must be non-empty:
        if len(sub_data) == 0:
            raise TestError("DDS2466TestScenario::check_for_timestamps - subscriber sample list is empty")
        if len(pub_data) == 0:
            raise TestError("DDS2466TestScenario::check_for_timestamps - publisher sample list is empty")

        # Check all samples from subscriber:
        for sub_sample in sub_data:
            sample_found = 0
            sample_from_another_pub  = 0
            # In all publisher sample:
            for pub_sample in pub_data:
                # Find only in samples from this publisher:
                if pub_sample.pub_id == sub_sample.pub_id:
                    # For each sample check the timestamp:
                    if pub_sample.id == sub_sample.id:
                        sample_found = 1
                        if ((pub_sample.timestamp_sec != sub_sample.timestamp_sec) or\
                           (pub_sample.timestamp_nanosec != sub_sample.timestamp_nanosec)):
                                raise TestError("DDS2466TestScenario::check_for_timestamps - Timestamp mistatching for the sample id[%s] from publisher id[%s] received by subscriber[%s]: publisher [%s:%s], subscriber [%s:%s]"%\
                                                (sub_sample.id, sub_sample.pub_id, sub_sample.sub_id, pub_sample.timestamp_sec, pub_sample.timestamp_nanosec, sub_sample.timestamp_sec, sub_sample.timestamp_nanosec))
                        # There is only one sample with the id, so go next sample:
                        break
                else:
                    # This sample is from another publisher, so go to the next subscriber sample:
                    sample_from_another_pub = 1
                    break

            if not sample_found and not sample_from_another_pub:
                raise TestError("DDS2466TestScenario::check_for_timestamps - Cannot find sample with id [%s] from the publisher id[%s] for subscriber id[%s]"%\
                                (sub_sample.id, sub_sample.pub_id, sub_sample.sub_id))
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_for_publishers(self, sub_data):
        """Checks that the first and the last publisher for the instances are
            always the same"""

        # At least two instances must be present to compare:
        if (sub_data == None) or (len(sub_data) <= 1):
            raise TestError("DDS2466TestScenario::check_for_publishers - at least two samples must be present in the subscriber sample list")

        # Get all id for all samples:
        sample_ids   = []
        last_pub_ids = []
        for sample in sub_data:
            if sample.id not in sample_ids:
                sample_ids.append(sample.id)
                last_pub_ids.append(-1)

        index = 0
        # Check all subscriber samples:
        for id in sample_ids:
            # Find the latest publisher for the sample with id for subscriber:
            for sample in sub_data:
                # Only for the current id:
                if sample.id == id:
                    last_pub_ids[index] = sample.pub_id
            index += 1

        # Check for the same last publisher:
        first_pub_id = last_pub_ids[0]
        if first_pub_id == -1:
            raise TestError("DDS2466TestScenario::check_for_publishers - no publisher found for the msg_id[%s]"% sample_ids[0])

        index = 1
        for pub_id in last_pub_ids[1:]:
            # Check for the publisher:
            if first_pub_id != pub_id:
                raise TestError("DDS2466TestScenario::check_for_publishers - pub_id[%s] for the msg_id[%s] breaks the rule - the last publisher id must be [%s]"%\
                                (pub_id, sample_ids[index], first_pub_id))
            index += 1
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_for_subscribers(self, sub_data1, sub_data2):
        """Checks that both subscrbers have the same results"""

        # Sample lists must be non-empty to compare:
        if len(sub_data1) == 0 or len(sub_data2) == 0:
            raise TestError("DDS2466TestScenario::check_for_subscribers - subscribers sample list is empty")

        # Get all id for all samples:
        sample_ids = []
        for sample in (sub_data1 + sub_data2):
            if sample.id not in sample_ids:
                sample_ids.append(sample.id)

        # Check all subscriber samples:
        for id in sample_ids:
            # Last pubsliher id for both subscribers:
            last_pub1 = -1
            last_pub2 = -1

            # Find the latest publisher for the sample with id for 1st subscriber:
            for sample in sub_data1:
                # Only for the current id:
                if sample.id == id:
                    last_pub1 = sample.pub_id

            # Find the latest publisher for the sample with id for 2nd subscriber:
            for sample in sub_data2:
                # Only for the current id:
                if sample.id == id:
                    last_pub2 = sample.pub_id

            # If publisher is not found for the sample:
            if (last_pub1 == -1):
                # Then the check is failed:
                raise TestError("DDS2466TestScenario::check_for_subscribers - sub_id[%s] has not the sample msg_id[%s]"% (sub_data1[0].sub_id, id))
            # If publisher is not found for the sample:
            if (last_pub2 == -1):
                # Then the check is failed:
                raise TestError("DDS2466TestScenario::check_for_subscribers - sub_id[%s] has not the sample msg_id[%s]"% (sub_data2[0].sub_id, id))

            # If publisher ids for the same sample instance is different:
            if (last_pub1 != last_pub2):
                # Then the check is failed:
                raise TestError("DDS2466TestScenario::check_for_subscribers - pub_id is not the same for subscribers for msg_id[%s]"% id)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def analyze(self):
        """Analize the test scenario results"""
        if not self.is_failed():
            try:
                # Get test hosts:
                waiter_host = self.get_host_by_role("WAITER")[0]
                sync_host   = self.get_host_by_role("SYNCHRONIZER")[0]

                # Read publisher/suscriber logs:
                pub_content1 = self.parser.get_process_log_content(
                    self.log_root,
                    waiter_host,
                    "publisher1")
                pub_content2 = self.parser.get_process_log_content(
                    self.log_root,
                    sync_host,
                    "publisher2")
                sub_content1 = self.parser.get_process_log_content(
                    self.log_root,
                    waiter_host,
                    "subscriber1")
                sub_content2 = self.parser.get_process_log_content(
                    self.log_root,
                    sync_host,
                    "subscriber2")

                # Recover samples list from the publisher/subscriber logs:
                pub_data1 = self.parser.get_samples(waiter_host, pub_content1)
                pub_data2 = self.parser.get_samples(sync_host, pub_content2)
                sub_data1 = self.parser.get_samples(waiter_host, sub_content1)
                sub_data2 = self.parser.get_samples(sync_host, sub_content2)

                good_samples = 1

                # Check for samples len:
                if len(pub_data1) == 0:
                    good_samples = 0
                    self.fail()
                    self.errors.append("Cannot analyze results: publisher1 has empty samples list")
                if len(pub_data2) == 0:
                    good_samples = 0
                    self.fail()
                    self.errors.append("Cannot analyze results: publisher2 has empty samples list")
                if len(sub_data1) == 0:
                    good_samples = 0
                    self.fail()
                    self.errors.append("Cannot analyze results: subscriber1 has empty samples list")
                if len(sub_data2) == 0:
                    good_samples = 0
                    self.fail()
                    self.errors.append("Cannot analyze results: subscriber2 has empty samples list")
                if len(sub_data1) < len(pub_data1):
                    good_samples = 0
                    self.fail()
                    self.errors.append("Cannot analyze results: subscriber1 lost some messages")
                if len(sub_data2) < len(pub_data2):
                    good_samples = 0
                    self.fail()
                    self.errors.append("Cannot analyze results: subscriber2 lost some messages")

                if good_samples:
                    try:
                        # Check for right timestamps:
                        # pub1->sub1:
                        self.check_for_timestamps(pub_data1, sub_data1)
                        # pub2->sub1:
                        self.check_for_timestamps(pub_data2, sub_data1)
                        # pub1->sub2:
                        self.check_for_timestamps(pub_data1, sub_data2)
                        # pub2->sub2:
                        self.check_for_timestamps(pub_data2, sub_data2)

                        # Check for the same publisher:
                        self.check_for_publishers(sub_data1)
                        self.check_for_publishers(sub_data2)

                        # Check that subscriber have the same results:
                        self.check_for_subscribers(sub_data1, sub_data2)
                    except TestError, msg:
                        self.fail()
                        self.errors.append(msg)

                # Check for the OSPL error log file:
                self.check_for_ospl_error_log()
            except:
                self.fail()
                self.errors.append("Cannot analyze results: %s"% sys.exc_info()[0])

        # Call parent analyze to create log file:
        BaseTestScenario.analyze(self)
#===============================================================================
