from process import Process
#This class extens from Process class, operation key field was added
class ExtProcess(Process):
    """
    This class extens from Process class, operation key field was added
    """
    def __init__(self, process_for_init, op_key):
        Process.__init__(self, process_for_init.get_command())
        # The arguments for a process:
        self.args        = process_for_init.get_args()
        # The working directory for a process:
        self.working_dir = process_for_init.get_working_dir()
        # The log file for a process:
        self.log_file    = process_for_init.get_log_file()
        # The process STAF ID:
        self.id          = process_for_init.get_id()
        # The process STAF handle:
        self.handle      = process_for_init.get_handle()
        # The process port:
        self.port        = process_for_init.get_port()
        # Use test sync lib:
        self.use_test_syn_lib = process_for_init.is_test_sync_lib_used()
        # The process operation key 
        self.self_op_key = op_key
    #gettin operation key for process    
    def get_op_key(self):
        """
        gettin operation key for process
        """
        return self.self_op_key