#This class is implementation of entity of results of test
class LogResult:
    """
    This class is implementation of entity of results of test
    """
    def __init__(self, name, log_name, log_dir_name, op_key, param_name):
        self.name_ = name
        self.op_key_ = op_key
        self.log_name_ = log_name
        self.log_dir_name_ = log_dir_name
        self.param_name_ = param_name
    #getting  name of test
    def get_name(self):
        """
        getting  name of test 
        """
        return self.name_
    #getting  log file name of test    
    def get_log_name(self):
        """
        getting  log file name of test    
        """
        return self.log_name_
    #getting  of directory of log file name of test
    def get_log_dir_name(self):
        """
        getting  of directory of log file name of test
        """
        return self.log_dir_name_
    #getting  of operation key
    def get_op_key(self):
        """
        getting  of operation key
        """
        return self.op_key_
    #getting  of parametr name
    def get_param_name(self):
        """
        getting  of parametr name
        """
        return self.param_name_