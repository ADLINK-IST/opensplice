import os
import socket

from process     import Process
from ospl        import OSPL 
from test_errors import TestError
#===============================================================================
class Host:
    """A machine and its environment that a part of a test case runs on.
       Each requires a STAF daemon.
    """
    # Local host default name:
    LOCAL_HOST_NAME = socket.gethostname()
    # STAF default TCP port:
    STAF_DEFAULT_PORT = 6500

    default_config_map = {
        "OSMajorVersion" : "unknown",
        "OSMinorVersion" : "unknown",
        "OSName"         : os.name,
        "OSRevision"     : "unknown",
        "FileSep"        : os.sep,
        "LineSep"        : os.linesep,
        "PathSep"        : os.pathsep}
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self,
                 ospl_home = "",
                 test_root = ".",
                 ospl_uri  = "",
                 host_name = LOCAL_HOST_NAME,
                 staf_port = STAF_DEFAULT_PORT):
        """Constructs a host.
        Required are:
            1) The root at which the test framework can be found.
            2) The value of OSPL_HOME that is to be tested.
            3) The OSPL URI value.
            4) The/A DNS name of the host.
            5) The location of the STAF daemon if not at the default port.
        """

        # The location where the test framework can be found:
        self.test_root    = test_root
        # The OSPL_HOME of the host to be tested:
        self.ospl_home    = ospl_home
        # The host name:
        self.host_name    = host_name
        # The port of the host STAF:
        self.staf_port    = staf_port
        # Host system environment:
        self.host_env     = []
        # List of the processes of the host:
        self.process_list = []
        # Config map for the host - contain system dependent issues:
        self.config_map   = Host.default_config_map

        # 'ospl' command instance:
        self.ospl = OSPL("%s%sbin%s"% (self.ospl_home,
                                       self.get_file_sep(),
                                       self.get_file_sep()), ospl_uri)

        # Check if host name is valid:
        self.check_host_name()
        # Check if STAF port is valid:
        self.check_staf_port()

        # Host log dir:
        self.log_dir = "" 
        # Host release script name:
        self.release_script_name = "release"
        # Host release script ext:
        self.release_script_ext = ""

        # The host role:
        self.role = ""
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_release_script_name(self):
        """Get the host release script name"""
        return self.release_script_name
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_release_script_name(self, name):
        """Set the host release script name"""
        self.release_script_name = name
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_release_script_ext(self):
        """Get the host release script ext"""
        return self.release_script_ext
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_release_script_ext(self, ext):
        """Set the host release script ext"""
        self.release_script_ext = ext
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_test_root(self, test_root):
        """Set the host test root"""
        self.test_root = test_root
        self.check_test_root()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_test_root(self):
        """Get the host test root"""
        return self.test_root
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_test_root(self):
        """Check if test root location is valid"""
        # A test root must be a non-empty string: 
        if (self.test_root == "" or self.test_root == None):
            raise TestError("Host::check_test_root - invalid value \"%s\" for the test root!"%\
                            self.test_root)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_ospl_home(self, ospl_home):
        """Set the host OSPL_HOME"""
        # Set:
        self.ospl_home = ospl_home
        # Check:
        self.check_ospl_home()
        # Reset 'ospl' command to new OSPL HOME:
        self.ospl.set_ospl_home_bin(self.ospl_home + "/bin/")
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
    def set_ospl_bin(self, ospl_home_bin):
        """Set the host OSPL_BIN"""
        self.ospl.set_ospl_home_bin(ospl_home_bin)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_ospl_home(self):
        """Get the host OSPL_HOME"""
        return self.ospl_home
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_ospl_home(self):
        """Check if OSPL_HOME is valid"""
        # OSPL_HOME must be a non-empty string: 
        if (self.ospl_home == "" or self.ospl_home == None):
            raise TestError("Host::check_ospl_home - invalid value \"%s\" for the OSPL_HOME!"%\
                            self.ospl_home)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_host_name(self, host_name):
        """Set the host name"""
        self.host_name = host_name
        self.check_host_name()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_host_name(self):
        """Get the host name"""
        return self.host_name
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_host_name(self):
        """Check if host name is valid"""
        # A host name must be a non-empty string: 
        if (self.host_name == "" or self.host_name == None):
            raise TestError("Host::check_host_name - invalid value \"%s\" for the host name!"%\
                            self.host_name)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_staf_port(self, staf_port):
        """Set the host STAF port"""
        self.staf_port = staf_port
        self.check_staf_port()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_staf_port(self):
        """Get the host STAF port"""
        return self.staf_port
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_staf_port(self):
        """Check if STAF port is valid"""
        # STAF port must be greater than 0: 
        if (self.staf_port <= 0):
            raise TestError("Host::check_staf_port - invalid value \"%d\" for the STAF port!"%\
                            self.staf_port)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_staf_url(self):
        """Get the host STAF URL"""
        return "tcp://%s@%s"% (self.host_name, self.staf_port)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def add_process(self, process):
        """Add new process to the process list"""
        # Check if the process is Ok:
        self.check_process(process)
        # Set process ID on the host:
        process.set_id(len(self.process_list) + 1)
        # Add process to the list:
        self.process_list.append(process)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def define_process(self,
                       command,
                       args        = "",
                       working_dir = ".",
                       log_file    = ""):
        """Define new process for the host"""
        # Create process: 
        new_process = Process(command, args, working_dir, log_file)
        # Set process ID on the host:
        new_process.set_id(len(self.process_list) + 1) 
        # Add the process to the list:
        self.process_list.append(new_process)
        # Return the instance:
        return self.process_list[-1:][0]
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_process(self, process):
        """Check if the process object is not 'None'"""
        if process == None:
            raise TestError("Host::check_process - invalid value \"%s\" for the process object!"%\
                            process)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_all_processes(self):
        """Return the host process list"""
        return self.process_list
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_ospl_log_dir(self, ospl_log_dir):
        """Set log directory for OSPL"""
        self.ospl.set_working_dir(ospl_log_dir)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_ospl_log_dir(self):
        """Get log directory for OSPL"""
        return self.ospl.get_working_dir()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_ospl_args_to_start(self):
        """Get arguments for OSPL to start the/a domain"""
        return self.ospl.get_start_cmd_args()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_ospl_args_to_stop(self):
        """Get arguments for OSPL to stop the/a domain"""
        return self.ospl.get_stop_cmd_args()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_ospl_args_to_list(self):
        """Get arguments for OSPL to list the/a domain"""
        return self.ospl.get_list_cmd_args()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_ospl_args_to_status(self):
        """Get arguments for OSPL to get the status of the/a domain"""
        return self.ospl.get_status_cmd_args()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
    def get_ospl_command(self):
        """Get command for OSPL"""
        return self.ospl.get_command()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_config_map(self, config_map):
        """Set config for the host"""
        self.config_map = config_map
        self.check_config_map()

        # 'ospl' command instance:
        self.ospl = OSPL("%s%sbin%s"% (self.ospl_home,
                                       self.get_file_sep(),
                                       self.get_file_sep()), self.ospl.get_uri())
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def check_config_map(self):
        """Check if the config_map object is not 'None'"""
        if self.config_map == None:
            raise TestError("Host::config_map - invalid value \"%s\" for the process object!"%\
                            self.config_map)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_config_map(self):
        """Get config of the host"""
        return self.config_map
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def is_windows(self):
        """Check if host OS is Windows family"""
        return (self.config_map["OSName"].find("Win") != -1) 
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_file_sep(self):
        """Returns the file path separator for this host"""
        return self.config_map["FileSep"]
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_path_sep(self):
        """Returns the path separator for this host"""
        return self.config_map["PathSep"]
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_line_sep(self):
        """Returns the line separator for this host"""
        return self.config_map["LineSep"]
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_os_name(self):
        """Returns a string describing the OS on this host"""
        return (self.config_map["OSName"]         + " " +\
                self.config_map["OSMajorVersion"] + "." +\
                self.config_map["OSMinorVersion"] + "." +\
                self.config_map["OSRevision"])
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_ospl_uri(self, uri):
        """Set OSPL URI"""
        self.ospl.set_uri(uri)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_ospl_uri(self):
        """Get OSPL URI"""
        return self.ospl.get_uri()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_host_env(self, host_env):
        """Set the host system environment"""
        self.host_env = host_env
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_host_env(self):
        """Get the host system environment"""
        return self.host_env
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_env_value(self, name):
        """Get the host system environment by the name"""
        value = ""
        for env in self.host_env:
            if env.find(name) != -1:
                value = env[env.find("=") + 1:]
                break
        return value
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_env_value(self, name, value):
        """Set the host system environment by the name"""
        for env in self.host_env:
            if env.find(name) != -1:
                self.host_env.remove(env)
                break

        self.host_env.append("%s=%s"% (name, value))
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_log_dir(self, log_dir):
        """Set the host log dir"""
        self.log_dir = log_dir
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_log_dir(self):
        """Get the host log dir"""
        return self.log_dir
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def translate_path(self, host_path, host_file_sep):
        """Translate FS path"""
        return host_path.replace(host_file_sep, self.get_file_sep())
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_process_by_name(self, name):
        """Get the host process by name"""
        for process in self.process_list:
            if name == process.get_name():
                return process
        return None
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_role(self, role):
        """Set the host role"""
        self.role = role
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_role(self):
        """Get the host role"""
        return self.role
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __str__(self):
        """Define new host for the scenraio"""
        string = "Host:\nhostname [%s]\nrole [%s]\ntest root [%s]\nOSPL_HOME [%s]\nlog dir [%s]\nSTAF URL [%s]\nENV %s\n"%\
                 (self.host_name,
                  self.role,
                  self.test_root,
                  self.ospl_home,
                  self.log_dir,
                  self.get_staf_url(),
                  self.host_env)
        string += "-----\n"
        string += str(self.ospl)
        for process in self.process_list:
            string += "-----\n"
            string += str(process)
        return string
#===============================================================================
