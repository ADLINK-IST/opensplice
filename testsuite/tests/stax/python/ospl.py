from process import Process
#===============================================================================
class OSPL:
    """Represents the OSPL command process"""

    # OSPL error log name:
    ospl_error_log_name = "ospl-error.log"
    # OSPL info log name:
    ospl_info_log_name = "ospl-info.log"

    # The values for OSPL command modes:
    # Default mode:
    none     = 0
    # Start OSPL:
    start    = 1
    # Stop OSPL:
    stop     = 2
    # List OSPL:
    list     = 3
    # List OSPL:
    status   = 4
    
    # List of all OSPL modes:
    modes = [none, start, stop, list, status]

    # Command line parameters for the OSPL modes:
    modes_options = {none     : "",
                     start    : "start",
                     stop     : "stop",
                     list     : "list",
                     status   : "status"}

    # 'ospl' command:
    command = "ospl"
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __init__(self, ospl_home_bin = "", uri = ""):    
        """Constructs the 'ospl' command"""

        # OSPL uri:
        self.uri = uri
        # OSPL HOME binary folder:
        self.ospl_home_bin = ospl_home_bin

        # OSPL command process:
        self.process = Process(OSPL.command)
        # Point OSPL command to the OSPL HOME:
        self.reset_ospl_command()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def reset_ospl_command(self):
        """Reset 'ospl' command to point to the current OSPL HOME binary folder"""
        if self.ospl_home_bin != "":
            self.process.set_command(self.ospl_home_bin + OSPL.command)
        else:
            self.process.set_command(OSPL.command)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_working_dir(self, working_dir):
        """Set the OSPL working directory"""
        self.process.set_working_dir(working_dir)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_working_dir(self):
        """Get the process working directory"""
        return self.process.get_working_dir()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_ospl_home_bin(self, ospl_home_bin):
        """Set the OSPL HOME binary folder"""
        self.ospl_home_bin = ospl_home_bin
        self.reset_ospl_command()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_ospl_home_bin(self):
        """Get the OSPL HOME binary folder"""
        return self.ospl_home_bin
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def set_uri(self, uri):
        """Set the OSPL URI"""
        self.uri = uri
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_uri(self):
        """Get the OSPL URI"""
        return self.uri
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_command(self):
        """Return OSPL command"""
        return self.process.get_command()
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_start_cmd_args(self):
        """Get OSPL arguments to start the/a domain"""
        return self.get_args(OSPL.start)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_stop_cmd_args(self):
        """Get OSPL arguments to stop the/a domain"""
        return self.get_args(OSPL.stop)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_stop_all_cmd_args(self):
        """Get OSPL arguments to stop all OSPL domains"""
        return self.get_args(OSPL.stop_all)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_list_cmd_args(self):
        """Get OSPL arguments to list the/a domain"""
        return self.get_args(OSPL.list)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def get_status_cmd_args(self):
        """Get OSPL arguments to get the status of the/a domain"""
        return self.get_args(OSPL.status)
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
    def get_args(self, mode = none):
        """Get OSPL command arguments"""
        args = OSPL.modes_options[mode]
        if self.uri != "":
            if args != "":
                args += " "
            args += self.uri
        return args
    #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def __str__(self):
        """String object representation"""
        return "OSPL:\ncommand [%s]\nargs [%s]\nworking/log dir [%s]\n"%\
               (self.get_command(),
                self.get_args(),
                self.get_working_dir())
#=============================================================================== 
