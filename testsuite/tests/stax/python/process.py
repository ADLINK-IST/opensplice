from test_errors import TestError
#===============================================================================
class Process:
    """Represents a process"""
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self,
                 command,
                 args        = "",
                 working_dir = ".",
                 log_file    = ""):
        """Constructs a process"""

        # The command name or executable for a process:
        self.command     = command
        # The arguments for a process:
        self.args        = args
        # The working directory for a process:
        self.working_dir = working_dir
        # The log file for a process:
        self.log_file    = log_file
        # The process STAF ID:
        self.id          = 0
        # The process STAF handle:
        self.handle      = 0
        # The process port:
        self.port        = 0
        # Use test sync lib:
        self.uses_test_syn_lib = 0

        # The name for the process:
        self.name = self.command

        # Check is command is valid:
        self.check_command()
        # Check is working directory is valid:
        self.check_working_directory()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_name(self, name):
        """Set the process name"""
        self.name = name
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_name(self):
        """Get the process name"""
        return self.name
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_command(self, command):
        """Set the process executable name"""
        self.command = command
        self.check_command()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_command(self):
        """Get the process executable name"""
        return self.command
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_command(self):
        """Check is value for the 'command' is Ok"""
        # Command must be a non-empty string: 
        if (self.command == "" or self.command == None):
            raise TestError("Process::check_command - invalid value \"%s\" for the command!"%\
                            self.command)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_args(self, args):
        """Set the process executable arguments"""
        self.args = args
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_args(self):
        """Get the process executable arguments"""
        return self.args
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_working_dir(self, working_dir):
        """Set the process working directory"""
        self.working_dir = working_dir
        self.check_working_directory()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_working_dir(self):
        """Get the process working directory"""
        return self.working_dir
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_working_directory(self):
        """Check is value for the working directory is Ok"""
        # Working directory must be a non-empty string: 
        if (self.working_dir == "" or self.working_dir == None):
            raise TestError("Process::check_working_directory - invalid value \"%s\" for the working directory!"%\
                            self.working_dir)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_log_file(self, log_file):
        """Set the process log file"""
        self.log_file = log_file
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_log_file(self):
        """Get the process log file"""
        return self.log_file
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_id(self, id):
        """Set the process ID"""
        self.id = id
        self.check_id()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_id(self):
        """Get the process ID"""
        return self.id
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_id(self):
        """Check is value for the process ID is Ok"""
        # ID must be > 0: 
        if (self.id <= 0):
            raise TestError("Process::check_id - invalid value \"%d\" for the ID!"%\
                            self.id)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_handle(self, handle):
        """Set the process handle"""
        self.handle = handle
        self.check_handle()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_handle(self):
        """Get the process handle"""
        return self.handle
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_handle(self):
        """Check is value for the process handle is Ok"""
        # handle must be > 0: 
        if (self.handle <= 0):
            raise TestError("Process::check_handle - invalhandle value \"%d\" for the handle!"%\
                            self.handle)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_port(self, port):
        """Set the process port"""
        self.port = port
        self.check_port()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_port(self):
        """Get the process port"""
        return self.port
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_port(self):
        """Check is value for the process port is Ok"""
        # Port must be > 0: 
        if (self.port <= 0):
            raise TestError("Process::check_port - inval value \"%d\" for the port!"%\
                            self.port)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def use_test_sync_lib(self):
        """Set use test sync lib property"""
        self.uses_test_syn_lib = True
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def dont_use_test_sync_lib(self):
        """Unset use test sync lib property"""
        self.uses_test_syn_lib = 1
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def is_test_sync_lib_used(self):
        """Return use test sync lib property"""
        return self.uses_test_syn_lib
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_test_lib_sync_args(self):
        """Return arguments for test lib sync"""
        return ""
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __str__(self):
        return "Process:\ncommand [%s]\nargs [%s]\nworking dir [%s]\nlog file [%s]\nid [%d]\nhandle [%d]\n"%\
               (self.command, self.args, self.working_dir, self.log_file, self.id, self.handle)
#===============================================================================