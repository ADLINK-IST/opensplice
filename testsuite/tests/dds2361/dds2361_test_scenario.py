import os
from base_test_scenario import BaseTestScenario

class DDS2361TestScenario(BaseTestScenario):
    '''Test scenario for the DDS2631 test'''
    def __init__(self, name = "", description = "", log_root = ".", result_file = ""):
        # Call base init with the predifined params:
        BaseTestScenario.__init__(self, name, description, log_root, result_file)
        self.scenarious = {'c_as_active': 'c', 'cpp_as_active': 'cpp', 'cs_as_active': 'cs', 'java_as_active': 'java'} 

    def check_app_log_files(self):
        '''Check app log files for necessary messages'''
        BaseTestScenario.check_app_log_files(self)
        active_lang = self.scenarious[self.name]
        silence_langs = self.scenarious.values()
        silence_langs.remove(active_lang)
        for host in  self.host_app_logs.keys():
            if self.get_role_by_host(host) == active_lang:
                search_strings = ["got message from " + lang for lang in silence_langs]
            else:
                search_strings = ["got message from " + active_lang]
            file_name = self.host_app_logs[host]
            app_log_file = os.path.join(self.log_root, host.get_hostname(), self.host_app_logs[host])
            f = open(app_log_file, 'r')
            file_lines = f.readlines()
            try:
                for string in search_strings: # all must be find
                    for x in file_lines: # xotya bi v odnoi
                        print x
                        if x.lower().find(string) != -1:
                            break
                    else:
                        self.errors.append("Application log file %s for the host [%s] did not recieve necessary messages"%\
                                        (file_name, host.get_hostname()))
                        break
            finally:
                f.close()
            
