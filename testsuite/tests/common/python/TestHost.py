import re
from string import rstrip
import socket
import os
from process import Process

class TestHost:
    LOCAL_HOST_NAME = socket.gethostname()
    default_config_map = {
        "OSMajorVersion" : "unknown",
        "OSMinorVersion" : "unknown",
        "OSName"         : os.name,
        "OSRevision"     : "unknown",
        "FileSep"        : os.sep,
        "LineSep"        : os.linesep,
        "PathSep"        : os.pathsep}

    def __init__(self,
                 hostname,
                 targets,
                 mode=['RTS'],
                 port=6500):
        """Create a TestHost object"""
        self.hostname = hostname
        
        # The default STAF port
        self.port = port
        
        # A list of OpenSplice platform strings (ie: x86.linux-dev, x86.win32-release)
        self.targets = targets
        self.active_target = None
        
        # A list of deployment modes (ie: RTS, HDE or SRC for a git checkout)
        self.modes = mode
        self.active_mode = None
        
        self.instance_map = {}
        self.config_map = TestHost.default_config_map
        
        # After successful OpenSplice deployment (while running tests) this is set to 1
        self.deployed = 0
        self.active_version = None
        
        self.uninstaller = None
        self.active_envfile = None
        
        self.ospl_ports = None
        self.ospl_home = None
        
        self.test_process_list = []
        
    def get_hostname(self):
        """Return the hostname"""
        return self.hostname
        
    def set_hostname(self, hostname):
        """Sets the hostname"""
        self.hostname = hostname;
        
    def get_staf_url(self):
        """Get the host STAF URL"""
        return "tcp://%s@%d"% (self.hostname, self.port)
    
    def get_filesep(self):
        """Returns the file path separator for this host"""
        return self.config_map["FileSep"]
        
    def get_pathsep(self):
        """Returns the path separator for this host"""
        return self.config_map["PathSep"]

    def get_linesep(self):
        """Returns the line separator for this host"""
        return self.config_map["LineSep"]
    
    def get_version(self):
        return self.active_version
    
    def get_ospl_home(self):
        """Returns current ospl_home value for this host"""
        return self.ospl_home
    
    def set_ospl_home(self, value):
        """Set ospl_home value for this host"""
        self.ospl_home = value
        
    def get_target(self):
        """ Returns current OpenSplice target for this host"""
        return self.active_target
    
    def get_mode(self):
        """ Returns the OpenSplice deployment mode for this host"""
        return self.active_mode

    def get_exec_ext(self):
        """ Returns file-extension of executable files for this host"""
        if self.isWindows():
            return '.exe'
        else:
            return ''
    
    def get_envfile(self):
        return self.active_envfile
    
    def get_basedir(self):
        return self.basedir
    
    def supportsMode(self, mode):
        """If deployment mode is supported by this host set it as active_mode and return true"""
        if mode == None:
            if self.active_mode == None:
                self.active_mode = self.modes[0]
            return 1
        else:
            if mode in self.modes:
                self.active_mode = mode
                return 1
        return 0

    def supportsTarget(self, target):
        """If target regex matches with any of this host's OpenSplice targets, set that target as active_target and return true"""
        if target == None:
            self.active_target = self.targets[0]
            return 1
        else:
            for t in self.targets:
                if re.search(target, t, re.I):
                    self.active_target = t
                    return 1
        return 0

    def get_env(self, key):
        if self.instance_map.has_key(key):
            return self.instance_map[key]
        else:
            return None
    
    def set_env(self, key, value):
        self.instance_map[key] = value
        
    def parse_env(self, env):
        for line in env.split('\n'):
            m = re.match('^([A-Z_\-]+)=\"?(.*?)\"?$', line)
            if m:
                self.set_env(m.group(1), m.group(2))

    def initCapabilities(self, staf_config, ospl_home, basedir):
        """Initialize TestHost features based on STAF properties, and verify compatibility with other host properties like target(s)"""
        self.config_map = staf_config
        invalid = 0
        msg = []
        for target in self.targets[:]:
            if ((target.find("x86_64") > -1) and not (self.config_map['OSRevision'].find('x86_64') > -1)):
                msg.append('Target %s requires 64-bit but machine only supports 32-bit builds' % target)
                self.targets.remove(target)
                continue
            if ((target.find("win") > -1) and (self.config_map['OSName'].find('Linux') > -1)):
                msg.append('Target %s requires Windows but machine runs Linux' % target)
                self.targets.remove(target)
                continue
            if ((target.find("linux") > -1) and (self.config_map['OSName'].find('Win') > -1)):
                msg.append('Target %s requires Linux but machine runs Windows' % target)
                self.targets.remove(target)
                continue

        if (len(self.targets) == 0):
            msg.append('No valid targets left, host not usable')
            invalid = 1
            
        if self.ospl_home == None:
            if (ospl_home != None and re.match('.*ospli?', ospl_home, re.I)):
                self.ospl_home = ospl_home
                if self.supportsMode('SRC'):
                    msg.append('OSPL_HOME is set and seems to be pointing to a source tree, switching active mode to \'SRC\'')
        else:
            if self.supportsMode('SRC'):
                msg.append('OSPL_HOME is set and seems to be pointing to a source tree, switching active mode to \'SRC\'')
                 
        self.basedir = basedir
        self.active_logdir = basedir + self.get_filesep() + 'ospl_logs'
            
        # Check target list vs. STAF/Config/OS
        return [invalid, msg]
    
    def isWindows(self):
        return (self.config_map['OSName'].find('Win') > -1)
    
    def set_deployed(self, value):
        self.deployed = value
        if not self.deployed:
            self.instance_map.clear()
            self.active_envfile = None
            self.active_process = None
        
    def set_version(self, value):
        self.active_version = value
        
    def set_envfile(self, value):
        self.active_envfile = value
        
    def set_uninstaller(self, value):
        self.uninstaller = value
        
    def get_uninstaller(self):
        return self.uninstaller
        
    def is_deployed(self):
        return self.deployed
    
    def detect_envfile(self):
        if self.ospl_home != None:
            prefix = self.ospl_home
        else:
            if self.active_mode == 'SRC':
                prefix = self.basedir + self.get_filesep() + 'ospli' # todo (not supported)
            else:
                if re.match('.*-release$', self.active_target):
                    target = self.active_target.replace('-release', '')
                else:
                    target = self.active_target
                # Set prefix to installer dir
                prefix = self.basedir + self.get_filesep() + 'OpenSpliceDDS' + self.get_filesep() + "V" + self.active_version + self.get_filesep() + self.active_mode + self.get_filesep() + target
        
        if self.active_mode == 'SRC':
            prefix += self.get_filesep() + '..' + self.get_filesep() + 'osplo'
            release_file = 'envs-' + self.active_target + '.sh'
        else:
            if self.isWindows():
                release_file = 'release.bat'
            else:
                release_file = 'release.com'
        return prefix + self.get_filesep() + release_file

    def set_process(self, value):
        self.active_process = value
        
    def set_logdir(self, value):
        if value[-1] == self.get_filesep():
            self.active_logdir = value
        else:
            self.active_logdir = value + self.get_filesep()
    
    def get_logdir(self):
        if self.active_logdir[-1] != self.get_filesep():
            return self.active_logdir + self.get_filesep()
        else:
            return self.active_logdir
    
    def set_uri(self, value):
        if value.startswith('file://'):
            uri = value
        else:
            uri = 'file://%s' % (value.replace('\\', '/'))
        self.set_env('OSPL_URI', uri)
    
    def get_process(self):
        if self.active_mode != 'SRC':
            if self.isWindows():
                prefix = self.active_envfile + ' && '
            else:
                prefix = '. ' + self.active_envfile + ' ; '
        else:
                prefix = ''
        return prefix + 'export OSPL_URI=' + self.get_env('OSPL_URI') + ' ; ' + self.active_process
    
    def get_process_envs(self):
        result = []
        for key,value in self.instance_map.items():
            result.append('%s=%s' % (key, value))
        return result
    
    def get_cmd_envs(self):
        result = ''
        for key,value in self.instance_map.items():
            result += 'ENV %s=%s ' % (key, value)
        return result

    def set_scenario(self, scenarioStr):
        self.test_scenario = scenarioStr or 'Unknown'
    
    def get_scenario(self):
        return self.test_scenario or 'Unknown'
    
    def add_port(self, portnr):
        self.ospl_ports.append(portnr)
        
    def remove_port(self, portnr):
        if portnr in self.ospl_ports:
            self.ospl_ports.remove(portnr)
        
    def matchInstaller(self, installers):
        if len(installers) == 0:
            return None
        
        if self.isWindows():
            inst_ext = '.exe'
        else:
            inst_ext = '.bin'
        
        # Strip '-release' from installer name if it is in active_target
        if re.match('.*-release$', self.active_target):
            target = self.active_target.replace('-release', '')
        else:
            target = self.active_target

        # Match regex to list of installers, return first matching
        if (self.active_version == None) or (self.active_version == 'unknown'):
            req_installer = '^OpenSpliceDDSV([0-9]+\.[0-9]+\.[0-9]+)-' + target + '-' + self.active_mode + '-installer' + inst_ext 
        else:
            req_installer = 'OpenSpliceDDSV(' + self.active_version + ')-' + target + '-' + self.active_mode + '-installer' + inst_ext
            
        for installer in installers:
            m = re.match(req_installer, installer)
            if m:
                self.active_version = m.group(1)
                # unset ospl_home, it will be in release.com after installer runs
                self.ospl_home = None
                print 'installer matched: %s <-> %s' % (req_installer, installer)
                return installer
            else:
                print 'installer not matched: %s <-> %s' % (req_installer, installer)
        return None
    
    def set_staf_port(self, staf_port):
        """Set the host STAF port"""
        self.port = staf_port

    def get_staf_port(self):
        """Get the host STAF port"""
        return self.port

    def add_test_process(self, test_process):
        """Add new process to the process list"""
        # Set process ID on the host:
        test_process.set_id(len(self.test_process_list) + 1)
        # Add process to the list:
        self.test_process_list.append(test_process)

    def define_test_process(self,
                       command,
                       args        = "",
                       working_dir = ".",
                       log_file    = ""):
        """Define new process for the host"""
        # Create process: 
        new_test_process = Process(command, args, working_dir, log_file)
        # Set process ID on the host:
        new_test_process.set_id(len(self.test_process_list) + 1) 
        # Add the process to the list:
        self.test_process_list.append(new_test_process)
        # Return the instance:
        return self.test_process_list[-1:][0]

    def get_all_test_processes(self):
        """Return the host process list"""
        return self.test_process_list
        
    def get_process_prearg(self):
        """Adds the sourcing of release script and setting OSPL_URI before command"""
        if self.isWindows():
            prefix = self.active_envfile + ' && ' + "set OSPL_URI=" + self.get_env('OSPL_URI') + ' && '
        else:
            prefix = '. ' + self.active_envfile + ' ; ' + "export OSPL_URI=" + self.get_env('OSPL_URI') + ' ; '
        return prefix
                            
    def pathjoin(self, arg1, *args):
        """Joins the args to path with hosts file separator """
        res = arg1
        for arg in args:
            if res.endswith(self.get_filesep()):
                res += arg
            else:
                res += self.get_filesep() + arg
        return res
