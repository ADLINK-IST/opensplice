import os

class TestError (Exception):
    """
    A simple exception for test problems
    """
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class Process:
    """Represents a process"""

    def __init__(self, node=None, test_case=None):
        """Constructs a Process associating it with a particualar TestScenario and Node"""
        self.node_ = node
        self.test_case_ = test_case
        self.handle_ = -1
        self.process_port_ = test_case.get_unique_port()
        self.args_ = ""
        self.process_number_ = 0
        self.log_file_ = ""
        self.working_dir_ = ""
        self.command_ = ""
        self.uses_test_sync_lib_ = 1

    def set_command(self, command):
        """Set the process executable name"""
        self.command_ = command

    def get_command(self):
        """Get the process executable name"""
        return self.command_

    def set_log_file(self, file_name):
        """Set a log file name that is to be used for standard out and error collection
        for this process. Will be created in the working directory.
        Take care to make this unique within this scenario if you don't want
        it to be overwritten."""
        self.log_file_ = file_name

    def get_log_file(self):
        """Return the standard out / error log file name for this process that has been set
        explicitly or a suitable default otherwise."""
        if self.log_file_ == "":
            log_file_name = '{STAF/Config/Sep/File}%s.log' % (self.get_command())
        else:
            log_file_name = '{STAF/Config/Sep/File}%s' % (self.log_file_)
        return log_file_name

    def set_working_dir(self, working_dir):
        """Explicitly set the process working directory path.
        Required if you don't want to take the default (which will be
        testsuite/tests/<scenario name>
        or whatever else has been set explicitly on the scenario
        """
        self.working_dir_ = working_dir

    def get_working_dir(self):
        """Get the process working directory path. Returns default for this
        TestScenario on this Process's Node"""
        if self.working_dir_ == "":
            if self.test_case_ == None:
                raise TestError ("Can't get a default working directory for Process " +
                                 self.command_ + " not associated with a TestScenario")
            if self.node_ == None:
                raise TestError ("Can't get a default working directory for Process " +
                                 self.command_ + " not associated with a Node")
            return self.test_case_.get_working_dir(self.node_)

        return self.working_dir_

    def set_args(self, args):
        """Set the process 'user' arguments"""
        self.args_ = args

    def get_args(self):
        """Get the process 'user' arguments"""
        return self.args_

    def set_uses_test_sync_lib(self, does_use_it = 1):
        """Set whether this process uses the test sync lib or not.
        Default is true. Set to 0 to prevent STAX providing extra config args
        when this process is started."""
        self.uses_test_sync_lib_ = does_use_it

    def get_test_lib_args(self):
        """
        Get the additional process args required to initialise the
        OSPL test lib interprocess synchronisation stuff (if required)
        You can of course do this 'manually' but this does it for you
        if you've stuck to the usual case.
        """
        if self.uses_test_sync_lib_ == 0:
            return ""

        extra_args = " -ORBListenEndpoints iiop://:" + str(self.process_port_)
        for other_process in self.test_case_.processes_:
            if other_process == self:
                pass
            else:
                extra_args += " -ORBInitRef " + other_process.get_command () + "=corbaloc:iiop:" + \
                              other_process.node_.host_name_ + ":" + str(other_process.process_port_) + \
                              "/" + other_process.get_command ()
        return extra_args

    def get_process_env(self):
        """
        Return the env that this process should be spawned or run with. Adds the
        """
        cloned_env = (self.node_.get_ospl_env())[:]
        i = 0
        while i < len(cloned_env):
            upper_case_env_line = cloned_env[i].upper()
            if upper_case_env_line.startswith('PATH=') or upper_case_env_line.startswith('LD_LIBRARY_PATH='):
                cloned_env[i] += '{STAF/Config/Sep/Path}%s{STAF/Config/Sep/Path}%s{STAF/Config/Sep/File}testsuite{STAF/Config/Sep/File}tests{STAF/Config/Sep/File}testlibs' % \
                                                                                      (self.get_working_dir(),
                                                                                      self.node_.get_test_root())
            i += 1
        return cloned_env

    def get_process_id_number(self):
        """
        Return a unique number identifying this process instance within the test run.
        Handy as a key if starting asynchronously.
        """
        return self.process_number_

class TestNode:
    """
    A machine environment that a part of a TestScenario runs on.

    Each requires a STAF daemon.
    """
    def __init__(self, host_name, test_root, ospl_home = None, staf_port = 6500, staf_url = ""):
        """Constructor for a TestNode. Required are the/a DNS name of the host, the root at
        which a source checkout of OSPL exists. Optionally the value of OSPL_HOME that is to be tested
        or None if you are taking responsibility for configuring the STAF daemon environment
        yourself & the location of the STAF daemon if not at the default port"""
        self.host_name_ = host_name
        self.test_root_ = test_root
        self.staf_port_ = staf_port
        self.staf_url_ = staf_url
        self.requires_static_load_ = 0
        if self.staf_url_ == "":
            self.staf_url_ = "tcp://" + self.host_name_ + "@" + str(self.staf_port_)
        self.initialised_ = 0
        self.ospl_home_ = ospl_home
        self.ospl_env_ = []

    def get_test_root(self):
        """
        Get the location that the built test checkout resides at on this machine
        for this run
        """
        return self.test_root_

    def get_ospl_home(self):
        """
        Get the value of OSPL_HOME on this TestNode within this particular TestRun
        """
        return self.ospl_home_

    def get_ospl_env(self):
        """
        Return the list of environment variable values applicable to the
        configured OSPL_HOME (if any) within this particular TestRun on this TestNode
        """
        return self.ospl_env_

    def get_ospl_uri(self):
        """
        Convenience accessor for OSPL_URI env prop on this box. We need this to make sure we can
        ospl list only the default domain to find out if it is running or not
        """
        for env_line in self.ospl_env_:
            if env_line.startswith("OSPL_URI="):
                return env_line[9:] # return the value after the =
        return ""

    def get_staf_url(self):
        """
        Returns the url that the STAF daemon is running at on this Node
        """
        return self.staf_url_

    def get_host_name(self):
        """
        Returns the Node hostname or IP or whatever
        """
        return self.host_name_

    def is_windows(self):
        """
        Returns true if this TestNode is a windows box
        """
        return self.config_map_["OSName"].startswith('Win')

    def file_sep(self):
        """
        Returns the file path separator for this TestNode. i.e. / or \
        """
        return self.config_map_["FileSep"]

    def path_sep(self):
        """
        Returns the path sepator on this TestNode. i.e. : or ;
        """
        return self.config_map_["PathSep"]

    def os_name(self):
        """
        Returns a string describing the OS on this TestNode
        """
        return (self.config_map_["OSName"] + " " + self.config_map_["OSMajorVersion"]
                + "." + self.config_map_["OSMinorVersion"] + "." + self.config_map_["OSRevision"])

class TestRun:
    """
    Represents a collection of TestScenario objects involving a number (1 or more)
    of TestNode's
    """
    def __init__(self, nodes = [], run_id = ""):
        self.nodes_ = nodes
        self.test_run_id_ = run_id
        if len (self.nodes_) == 0 :
            self.nodes_ = [ TestScenario.LOCALHOST ]
        self.test_cases_ = []
        self.verbose_ = 1
        self.process_count_ = 0

    the_test_run_ = None

    #@staticmethod
    #def get_test_run():
    #def get_test_run():
    #    """
    #    Static accessor for the TestRun
    #    """
    #    return TestRun.the_test_run_

    # get_test_run = staticmethod(get_test_run)

    def create_test_scenario(self, test_name):
        """
        Factory method to create a TestScenario within this test run.
        """
        self.test_cases_.append(TestScenario(test_name, self))
        return self.test_cases_[len(self.test_cases_) - 1]

class TestScenario:
    """
    Represents a single test scenario within a test. A scenario has
    1 or more Nodes, and an arbitrary number of processes
    """

    def __init__(self, name="", test_run=TestRun.the_test_run_, description=""):
        self.name_ = name
        self.test_run_ = test_run
        self.required_nodes_ = 1
        self.description_ = description
        self.nodes_ = test_run.nodes_
        self.node_index_ = 0
        self.locked_and_loaded_ = 0
        self.processes_ = []
        self.next_port_ = 23231
        self.work_dir_override_ = ""

    def get_name(self):
        """
        Accessor for the TestScenario name
        """
        return self.name_

    def requires_multiple_nodes(self, min_number = 2):
        """
        Specify that this test scenario requires a *minimum* number of nodes to
        do its thing. The default is 1 i.e. single node. If your test cannot operate
        at all on one node specify an appropriate higher value. Do *not* specify a
        higher value just because the test is less meaningful on less.
        Keep this value as low as possible.
        """
        self.required_nodes_ = min_number

    def get_next_node(self):
        """
        Gets a new node definition from the scenario. Will be a new node iff the scenario has enough for it to be so.
        """
        next_node = self.nodes_[self.node_index_]
        if (self.node_index_ + 1) >= len (self.nodes_) :
            pass
        else:
            self.node_index_ += 1
        return next_node

    def get_working_dir(self, node):
        """
        Get the default working directory for this scenario on the given Node.
        Assumes this scenario directory is under the usual test root and is named
        as per convention.
        """
        if self.name_ == "":
            raise TestError("Attempt to get the default working directory on "  +
                            node.get_host_name() + " from unnamed scenario")

        return (node.get_test_root()
               + "{STAF/Config/Sep/File}testsuite{STAF/Config/Sep/File}tests{STAF/Config/Sep/File}"
               + self.name_)

    def get_log_dest_dir(self):
        """
        Where the logs need to be copied to
        """
        if self.name_ == "":
            raise TestError("TestScenario must have a name")

        return ("{STAF/Config/Sep/File}..{STAF/Config/Sep/File}"
               + self.name_)

    def lock_and_load(self):
        """
        Checks the requirements of the whole TestScenario are met by the TestRun it
        is within. Will actually have build and load the test processes in some embedded
        circumstances if there's no other alternative. Scenario should exit if this does not return True.
        No changes should be made to the scenario after this.
        """
        if self.locked_and_loaded_ :
            raise TestError("Duplicate call to lock_and_load scenario: " + self.name_)

        if self.check_test_reqs_met () :
            # Hook for static build & load actions
            self.locked_and_loaded_ = 1

        return self.locked_and_loaded_

    def check_test_reqs_met(self):
        """
        Check the TestScenario requirements can be satisfied by the TestRun it is
        within.

        If it can't it will throw an error and the scenario should be recorded as skipped.
        """
        if self.required_nodes_ > len (self.nodes_):
            raise TestError("TestRun does not comprise enough Nodes for scenario: " + self.name_)

        return 1

    def define_process(self, node):
        """
        Declare a process to run on the specified Node within this TestScenario
        """
        process_result = Process(node, self)
        self.processes_.append(process_result)
        self.test_run_.process_count_ += 1
        process_result.process_number_ = self.test_run_.process_count_
        return process_result

    def get_unique_port(self):
        """
        Returns a unique port. Used to make soure Process instances'
        IPC can be initialised in a non conflicting manner.
        """
        self.next_port_ += 1
        return self.next_port_

    LOCALHOST = TestNode("local", os.path.join(os.getcwd(), ".."))
