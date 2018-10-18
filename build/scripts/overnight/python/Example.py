import commands
import sys
import os
import glob
import json
import subprocess
import threading
from threading import Timer
import time
from shutil import copy
import example_logparser
from example_exceptions import LogCheckFail
from example_exceptions import MissingExecutable
from example_exceptions import ExampleFail
from ExampleLogger import examplelogger
import pdb
import splicedCheck

"""
    Main Example class which is the basis for most of the straightforward examples.  If an
    example inolves just a single client/server, publisher/subscriber then this class can
    be used to run it.  An entry should be made in the examples.json file for any new
    example and the example should also be included in the runExample.py script.
"""
class Example(object):

    def __init__(self, host, logger, example, expath):

        #pdb.set_trace()

        with open ('examples.json') as data_file:
            data = json.load(data_file)

        # host on which the examples are running
        self.host = host

        # extra path name e.g. dcps, face
        self.expath = expath

        # Need a local setting of protobuf to read the .json file but don't need
        # it globally as the protobuf example is not in a directory protobuf/protobuf
        if example == "protobuf":
            expath = "protobuf"

        # path to the example
        self.path =  os.path.join(os.environ['OSPL_HOME'], 'examples', self.expath)

        # languages this example runs in e.g. c / c++ / cs / java / java5 / isocpp2
        self.langs = data[expath][example]["langs"]

        # corba languages if the example has a corba version
        self.corba_langs = data[expath][example]["corba_langs"]

        # extra directories in the example directory structure e.g. standalone, corba
        self.extra = data[expath][example]["extra"]

        # the example name
        self.name = example

        # the logger - which prints out the final result
        self.logger = logger

        # location of yaml file containing details of expected / allowed output
        self.example_conditions_path = ""

        # location of yaml file containing details of errors that are not accepted
        self.error_conditions_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml',
                                                 'error_conditions.yaml')

        # location of yaml file containing conditions related to the ospl-info.log
        self.ospl_info_conditions_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml',
                                                 'ospl_info_conds.yaml')

        # name of ospl-info.log - defaults to ospl-info.log
        self.ospl_info_log = ""

        # name of ospl-error.log - defaults to ospl-error.log
        self.ospl_error_log = ""

        # timeout example is allowed to run before it is killed
        self.example_timeout = int(data[expath][example]["timeout"])

        # OSPL_URI for sp mode, if not the default
        self.sp_uri = data[expath][example]["sp-uri"]

        # OSPL_URI for shm mode, if not the default
        self.shm_uri = data[expath][example]["shm-uri"]

        # If the publisher should run before the subscriber
        self.pubFirst = data[expath][example]["pub-first"]

        # OSPL_URI for this test if not the default
        self.uri = ""

        # suffix for c language - used in results directory
        self.csfx = data["languages"]["c"]["prefix"]

        # suffix for c++ language - used in results directory
        self.cppsfx = data["languages"]["cpp"]["prefix"]

        # suffix for corba c++ language - used in results directory
        self.ccppsfx = data["languages"]["cpp"]["corba_prefix"]

        # suffix for cs language - used in results directory
        self.cssfx = data["languages"]["cs"]["prefix"]

        # suffix for java language - used in results directory
        self.javasfx = data["languages"]["java"]["prefix"]

        # suffix for java5 language - used in results directory
        self.java5sfx = data["languages"]["java5"]["prefix"]

        # suffix for corba java language - used in results directory
        self.cjsfx = data["languages"]["java"]["corba_prefix"]

        # suffix for corba java language - used in results directory
        self.cj5sfx = data["languages"]["java5"]["corba_prefix"]

        # suffix for c99 language - used in results directory
        self.c99sfx = data["languages"]["c"]["prefix"]


        # classpath - used for java examples - set at run time
        self.classpath = ""

        # location of executables
        self.pPath = ""

        # example result directory
        self.exdir = ""

        # summary log - holds results of all examples in this run
        self.summary_log = os.path.join(os.environ['LOGDIR'], "examples", "run_" + os.environ['EXRUNTYPE'], "summary.log")

        # set the  OSPL_URI for this test
        self.setURI()


    # Called by sub classes to set the path where it is non-standard e.g. DBMSConnect example
    def setPath(self, path):
        self.path = path

    # Sets the OSPL_URI for this instance of the example
    def setURI(self):
        if self.shm_uri != "" and os.environ['EXRUNTYPE'] == "shm":
            self.uri = "file://" + os.path.join(self.path, self.name, self.shm_uri)
        elif self.sp_uri != "" and os.environ['EXRUNTYPE'] == "sp":
            self.uri = "file://" + os.path.join(self.path, self.name, self.sp_uri)
        else:
            self.uri = os.environ['OSPL_URI']

    # Run all versions of this example
    def runExampleAllTypes(self):
        """
        self.extra can be set in .json file as standalone / corba.  This does
        not apply to all examples and so will be an empty string if not found
        """
        if self.extra != "":
            for ex in self.extra:
                self.runExampleAll(ex)
        else:
            self.runExampleAll("")

    """
    Run all languages for type specified
    extra can be
        "all" - values obtained from the .json file
        "standalone" or "corba" - can be specified at the command line
        "" - where there is no extra type e.g. for RoundTrip / Throughput
        the runExample.py script will determine appropriate entry if none supplied
        on the command line
    """
    def runExampleAll(self, extra):
        if extra == "all":
            if self.extra != "":
                for ex in self.extra:
                    if "java5" in lang:
                        self.runExampleAll ("")
                        break
                    else:
                        self.runExampleAll(ex)
            else:
                self.runExampleAll ("")
        else:
            if extra == "corba":
                if self.corba_langs == "":
                    print "No corba version for " + self.name + " example"
                else:
                    # run the corba example for each language found
                    for lang in self.corba_langs:
                        if self.host.runExample(self.expath, self.name, "c" + lang):
                            self.runExample(lang, extra, "all")
            else:
                # run the example for each language found (non-corba)
                for lang in self.langs:
                    if self.host.runExample(self.expath, self.name, lang):
                        self.runExample(lang, extra, "all")

    """
    Run language for all types - this method will only be called if a language is
    specified but the extra is *all* - the possible values will be obtained from
    the examples.json file
    """
    def runExampleAllExtra(self, lang, extra, types):
        if self.extra != "":
           for ex in self.extra:
                self.runExample(lang, ex, types)
        else:
            self.runExampleAll ("")

    """
    Run the example
        lang - can be c / cpp / cs / java / java5 / isocpp2
               could be "" for instance if DBMSConnect example
               could be "all" if running all versions of an example
        extra - can be standalone / corba  / "" / all
        types - only really valid for PingPong where running the different
                topic types e.g. s / f / q etc - not really handled at present
    """
    def runExample(self, lang, extra, types):
        if lang == "cs" and not self.host.isWindows():
            print "C# not supported on " + self.host.name
        else:
            if lang == "all":
                self.runExampleAll(extra)
            else:
                if extra == "all":
                    self.runExampleAllExtra(lang, extra, types)
                else:
                    # get the current location
                    currPath = os.getcwd()

                    try:
                        if self.name == "protobuf":
                            exKey = "protobuf"
                        else:
                            exKey = self.expath

                        print "In runExample for " + exKey + ": " + self.name + ": " + lang

                        # set the example result directory name e.g. dcpsPingPongsac
                        self.setExampleResultDir(lang, extra)

                        exSfx = ""

                        if self.host.isWindows() and not "java" in lang:
                            exSfx = ".exe"

                        """
                           Get the names of the executables for this example
                           Obtained from the example.json file
                        """
                        with open ('examples.json') as data_file:
                            data = json.load(data_file)

                        if extra == "corba":
                            exes = "corba_executables"
                        else:
                            exes = "executables"

                        pubName = data[exKey][self.name][exes][lang]["pubName"]
                        subName = data[exKey][self.name][exes][lang]["subName"]

                        """
                           Get the runtime parameters for this example
                           Obtained from the example.json file
                        """
                        pubParams = data[exKey][self.name]["params"]["pub_params"]
                        subParams = data[exKey][self.name]["params"]["sub_params"]

                        """
                           Get the yaml conditions file for this example
                           Obtained from the example.json file
                        """
                        sub_conds_file = data[exKey][self.name]["log_conditions_file"][lang]["sub_conds"]
                        pub_conds_file = data[exKey][self.name]["log_conditions_file"][lang]["pub_conds"]

                        sub_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', sub_conds_file)
                        pub_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', pub_conds_file)

                        msg = "NONE"
                        result = "PASS"

                        try:
                            self.setLogPathAndLogs(lang, extra)

                            pubLog = ""

                            if pubName != "":
                                pubLog = os.path.join(self.pPath, 'publisher.log')

                            subLog = os.path.join(self.pPath, 'subscriber.log')

                            os.chdir(self.pPath)

                            # Set the classpath and the runLang which is needed to identify corba versions of java as opposed to non-corba
                            if extra == "corba" and lang == "java":
                                ospljar = os.path.join(os.environ['OSPL_HOME'], "jar", "dcpscj.jar")
                                classes = os.path.join(os.environ['OSPL_HOME'], "examples", self.expath, self.name, lang, extra, "classes")
                                self.classpath = ospljar + os.pathsep + classes
                                runLang = "cj"
                            else:
                                runLang = lang

                            if pubName != "":
                                """
                                 Check that the executable actually exists if it's not a java class.  The classpath is not set if we are
                                 running a jar file, not all java examples currently run with a jar file.
                                """
                                if self.classpath == "" and runLang is not "cj":
                                    pubexe = os.path.join(self.pPath, pubName) + exSfx
                                    if not os.path.isfile (pubexe):
                                        msg = "MissingExecutable: " + pubexe
                                else:
                                    pubexe = pubName

                                # If we found the executable create the thread in which to run it
                                if msg == "NONE":
                                    pubThread = ExeThread(self.classpath, pubLog, runLang, pubexe, pubParams, self.example_timeout * 2)

                            if subName != "":

                                """
                                 Check that the executable actually exists if it's not a java class.  The classpath is not set if we are
                                 running a jar file, not all java examples currently run with a jar file.
                                """
                                if self.classpath == "" and runLang is not "cj":
                                    subexe = os.path.join(self.pPath, subName) + exSfx
                                    if not os.path.isfile (subexe):
                                        msg = "MissingExecutable: " + subexe
                                else:
                                    subexe = subName

                                # If we found the executable create the thread in which to run it
                                if msg == "NONE":
                                    subThread = ExeThread(self.classpath, subLog, runLang, subexe, subParams, self.example_timeout)

                            if msg == "NONE":
                                # start the ospl daemon, this will only happen if it's SHM
                                self.startOSPL()

                                # start the publisher if it exists and is to start first
                                if self.pubFirst == "True" and pubName != "":
                                    pubThread.start()

                                    # Wait for publisher to get fully set up
                                    time.sleep(3)

                                # start the subscriber
                                if subName != "":
                                    subThread.start()

                                # Start the publisher if it wasn't to start first

                                if self.pubFirst == "False" and pubName != "":
                                    # Wait for subscriber to get fully set up
                                    time.sleep(3)
                                    pubThread.start()

                                # Wait for the subscriber and publisher threads to complete
                                if subName != "":
                                    subThread.join(self.example_timeout)

                                if pubName != "":
                                    pubThread.join(self.example_timeout)

                        except Exception as ex:
                            msg = "Exception running " + str(ex)

                        try:
                            if self.logger.debug:
                                print "Going to stopOSPL"
                                sys.stdout.flush()

                            # start the ospl daemon, this will only happen if it's SHM
                            self.stopOSPL()

                            if self.logger.debug:
                                print "Back from stopping OSPL"
                                sys.stdout.flush()

                        except Exception as ex:
                            print "Exception stopping OpenSplice ", str(ex)

                        if msg == "NONE":
                            try:
                                #Allow some time for all output to be written to the logs
                                time.sleep(10)
                                # copy the logs to the results directory
                                self.copyLogs()

                                # Check if an ospl-error.log exists - if so this is a failure
                                if os.path.isfile (self.ospl_error_log):
                                    msg = "ospl-error.log found"
                                else:
                                    # check the results in the subscriber logfile using the yaml conditions file
                                    self.checkResults(subLog, sub_conds)

                                    # if a publisher log exists check it using the publisher yaml conditions file
                                    if pubLog != "":
                                        self.checkResults(pubLog, pub_conds)

                                    # check the contents of the ospl-info.log
                                    self.checkOSPLInfoLog(self.ospl_info_log)

                            except LogCheckFail as lf:
                                reason = str(lf)
                                if "OpenSpliceDDS Warnings" in reason:
                                    msg = "LogCheckFail: OpenSpliceDDS Warnings in ospl-info.log"
                                else:
                                    msg = "LogCheckFail: " + str(lf)
                            except Exception as ex:
                                msg = "Exception checking logs " + str(ex)

                        if msg != "NONE":
                            result = "FAIL"

                        if extra == "corba":
                            if lang == "cpp":
                                resultLang = "ccpp"
                            elif lang == "java":
                                resultLang = "cj"
                            elif lang == "java5":
                                resultLang = "cj5"
                        else:
                            resultLang = lang

                        try:
                            # Write the result for this instance of the example to the summary log
                            self.writeResult (result, self.expath + self.name, resultLang, msg)
                        except Exception as ex:
                            print "Exception writing result ", str(ex)

                        if self.host.isWindows():
                            time.sleep(5)

                        try:
                            # Tidy up - deletes logs and things like pstore so clean for next run of the example
                            self.cleanUp()
                        except Exception as ex:
                            print "Exception cleaning up ", str(ex)

                    except Exception as ex:
                        print "Unexpected exception ", str(ex)

                    finally:
                        os.chdir(currPath)

                        print "Completed " + self.name + ": " + lang + ":" + extra


    """
      Start the ospl daemon if appropriate
    """
    def startOSPL(self):

        if self.logger.debug:
            print "STARTING OSPL *****"
            sys.stdout.flush()

        if os.environ["EXRUNTYPE"] == "shm":
            command = ['ospl', 'start']

            print command
            ospl = subprocess.Popen(command)
            sys.stdout.flush()

            """
               Check that the ospl daemon has started within expected time
            """
            count = 0
            cmax = 20
            splicedFound = False
            if self.host.isWindows():
                if self.host.use_psutil == "True":
                    splicedFound = splicedCheck.splicedCheck ("start")

                    if splicedFound == False:
                        count = cmax
                else:
                    while splicedFound == False and count < cmax:
                        try:
                            time.sleep(1)
                            s = subprocess.check_output('tasklist', shell=True)
                            if "spliced.exe" in s:
                                splicedFound = True
                            else:
                                count += 1

                        except Exception as e:
                            print "Exception checking if OSPL has started ...", str(e)
                            sys.stdout.flush()
                            count += 1
            else:
                output = commands.getoutput('ps -A')
                while not 'spliced' in output and count < cmax:
                    time.sleep(1)
                    output = commands.getoutput('ps -A')
                    count += 1

            if count == cmax:
                print "spliced failed to start"
                sys.stdout.flush()
                raise Exception("spliced not started ....")
            else:
                print "OpenSplice started. ...."

    """
       stop the ospl daemon if appropriate
    """
    def stopOSPL(self):

        if self.logger.debug:
            print "STOPPING OSPL *****"
            sys.stdout.flush()

        if os.environ["EXRUNTYPE"] == "shm":
            if self.uri != "":
                command = ['ospl', 'stop', self.uri]
            else:
                command = ['ospl', 'stop']

            print command
            ospl = subprocess.Popen(command)

            """
               Check that the ospl daemon has stopped within expected time
            """
            count = 0
            cmax = 20
            splicedFound = True
            if self.host.isWindows():
                if self.host.use_psutil == "True":
                    splicedFound = splicedCheck.splicedCheck ("stop")
                    if splicedFound == True:
                        count = cmax
                else:
                    while splicedFound == True and count < cmax:
                        try:
                            time.sleep(1)
                            s = subprocess.check_output('tasklist', shell=True)
                            if "spliced.exe" in s:
                                count += 1
                            else:
                                splicedFound = False

                        except Exception as e:
                            print "Exception checking if OSPL has stopped ...", str(e)
                            sys.stdout.flush()
                            count += 1

            else:
                output = commands.getoutput('ps -A')
                while 'spliced' in output and count < cmax:
                    time.sleep(1)
                    output = commands.getoutput('ps -A')
                    count += 1

            if count == cmax:
                raise Exception("spliced not stopped within expected time ....")
            else:
                print "OpenSplice stopped. ...."


    """
       Set the path to this instance of the example and the directory for the logs
       lang - can be c / cpp / cs / java / java5 / isocpp2
       extra  - can be standalone / corba / ""
    """
    def setLogPathAndLogs(self, lang, extra):

        if lang == "":
            self.pPath = self.path
        else:
            self.pPath = os.path.join(self.path, self.name, lang)

        if extra != "":
            pPathTemp = os.path.join(self.pPath, extra)
            if os.path.isdir (pPathTemp):
                self.pPath = pPathTemp

        self.ospl_info_log = os.path.join(self.pPath, 'ospl-info.log')
        self.ospl_error_log = os.path.join(self.pPath, 'ospl-error.log')

    """
       Set the name of the example results directory e.g.
       dcpsHelloWorldsac
    """
    def setExampleResultDir(self, lang, extra):
        if self.expath == "dcps":
            if lang == "isocpp2":
                sfx = lang
            elif lang == "c99":
                sfx = lang
            else:
                sfx = self.getSuffix(lang, extra)
        else:
            sfx = lang

        if self.name == "protobuf":
            self.exdir = self.name + sfx
        else:
            self.exdir = self.expath + self.name + sfx

    """
       Delete the logs from the example directory so clean for next instance
       of this example
    """
    def cleanUp(self):
        logs = os.path.join(self.pPath, '*.log')
        for log in glob.glob(logs):
            os.remove(log)

    """
      Get the suffix for this instance of the example.  Used to set the
      example result directory
    """
    def getSuffix (self, lang, extra):

        sfx = ""

        if lang == "java" or lang == "java5":
            if lang == "java":
                if extra == "corba":
                    sfx = self.cjsfx
                else:
                    sfx = self.javasfx
            else:
                if extra == "corba":
                    sfx = self.cj5sfx
                else:
                    sfx = self.java5sfx
        elif lang == "c" or lang == "cpp" or lang == "c99":
            if lang == "c":
                sfx = self.csfx
            elif lang == "c99":
                sfx = self.c99sfx
            else:
                if extra == "corba":
                    sfx = self.ccppsfx
                else:
                    sfx = self.cppsfx
        elif lang == "cs":
            sfx = self.cssfx

        return sfx

    """
       Copy the logs from the example directory to the results directory
    """
    def copyLogs (self):

        if self.logger.debug:
            print "Copying logs and checking results"
            sys.stdout.flush()

        logdir =  os.path.join(os.environ['LOGDIR'], "examples", "run_" + os.environ['EXRUNTYPE'], self.exdir)

        logs = os.path.join(self.pPath, '*.log')

        if not os.path.exists(logdir):
            os.makedirs(logdir)

        if os.path.isfile (self.ospl_info_log):
            copy(self.ospl_info_log, logdir)

        if os.path.isfile (self.ospl_error_log):
            copy(self.ospl_error_log, logdir)

        for log in glob.glob(logs):
            if os.path.isfile (log):
                copy(log, logdir)

        if self.host.isWindows():
             time.sleep(5)

    """
       check the results for this instance of the example
    """
    def checkResults(self, log, conds):
        if os.path.isfile (log):
            with open(log) as f:
                example_logparser.checkLogs(self.error_conditions_path, f)

            with open(log) as f:
                example_logparser.checkLogs(conds, f)


    """
      check the ospl-info.log
    """
    def checkOSPLInfoLog(self, log):
        with open(log) as f:
            example_logparser.checkLogs(self.ospl_info_conditions_path, f)

    """
       Write the final result to the summary logs
    """
    def writeResult(self, result, name, lang, msg):
        fres = msg

        """
          The result is written to the examples.log which is retained as a quick reference to all results and to
          the summary.log which is written in html and concatenated into a summary.html in the loggers finalizeResults
          method.  A running total of pass / fail is retained by the logger.
        """
        examplesLog = os.path.join(os.environ['LOGDIR'], "examples", "run_" + os.environ['EXRUNTYPE'], "examples.log")
        summ_html=os.path.join(os.environ['LOGDIR'], "examples", "run_" + os.environ['EXRUNTYPE'], "summary.log")
        fs = open(summ_html, 'a')
        exlog = open(examplesLog, 'a')

        if result == "PASS":
            fres = "N/A"
            bcol = "ACF0BA"
            self.logger.addPass()
        else:
            bcol = "F1BAAD"
            self.logger.addFail()

        fs.write("<TR  bgcolor=" + bcol + "><TD>" + name + "</TD><TD>" + lang + "</TD><TD>" + fres + "<TD><a HREF=" + self.exdir + ">" + result + "<br></TR>\n")
        fs.close()

        if fres == "N/A":
            fres = ""

        exlog.write(name + "\t" + lang  + "\t" + result + "\t" + fres +"\n")
        exlog.close()

"""
   This thread class is used to run an executable.
"""
class ExeThread (threading.Thread):

    def __init__(self, classpath, runLog, lang, prog, params, timeout):
        threading.Thread.__init__(self)
        self.lang = lang
        self.prog = prog
        self.params = params
        self.log = runLog
        self.timeout = timeout
        self.classpath = classpath


    def terminate(self, proc):
        pid = str(proc.pid)
        print "Terminating proc: {}".format(pid)

        cmd_exists = lambda x: any(os.access(os.path.join(path, x), os.X_OK) for path in os.environ["PATH"].split(os.pathsep))
        if cmd_exists('gdb'):
            cmd = ['gdb', '-p','{}'.format(pid), '--batch','-ex', 'thread apply all bt full', '-ex','quit']
            gdb  = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            timer = Timer(self.timeout, gdb.kill)
            try:
                timer.start()
                gdboutput, _ = gdb.communicate()
                res = gdb.returncode
                print "GDB return code is ", res
                if gdboutput:
                    with open(self.log, 'a') as f:
                        f.write(gdboutput)
            except Exception as e:
                print str(e)
            finally:
                timer.cancel()

        proc.kill()

    def run(self):

        isJava = False
        proc = None
        print "OSPL_URI is ", os.environ["OSPL_URI"]

        if self.lang == "java" or self.lang == "java5" or self.lang == "cj" or self.lang == "cj5":
            isJava = True

        if isJava:
            envJava = "java"
            spliceExtraCP = ""

            try:
                envJava = os.environ['SPLICE_JAVA']
                spliceExtraCP = os.environ['SPLICE_EXTRA_CP']
            except KeyError:
                print "Ignoring KeyError getting SPLICE_EXTRA_CP"

            if envJava != "java":
                spliceJava = envJava
            else:
                spliceJava = os.path.join(os.environ['JAVA_HOME'], "bin", envJava)

            if self.lang == "cj" or self.lang == "cj5":
                jacend = "-Djava.endorsed.dirs=" + os.path.join(os.environ['JACORB_HOME'], "lib", "endorsed")
                args = [self.prog]
                command = [spliceJava, jacend, '-classpath', self.classpath + os.pathsep + spliceExtraCP]
                command.extend(list(args))
            else:
                if self.classpath == "":
                    args = [self.prog]
                    command = [spliceJava, '-jar']
                    command.extend (list(args))
                else:
                    args = [self.classpath, self.prog]
                    command = [spliceJava, '-classpath']
                    command.extend (list(args))
        else:
            command = [self.prog]

        if self.params != "":
            args = [self.params]
            command.extend (list(self.params))

        print command

        res = 0
        proc  = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

        """
           If the processes runs then set a timer to kill it if it exceeds the timeout
           specified in the examples.json file for this particular example
        """
        if proc:
            timer = Timer(self.timeout, self.terminate, (proc,))
            try:
                timer.start()
                exeOutput, _ = proc.communicate()
                res = proc.returncode
                print "Process return code is ", res
            finally:
                timer.cancel()

            # Write the output to the log for this executable
            with open(self.log, 'a') as f:
                    f.write(exeOutput)

            """
               Check if the example returned a non-zero return code.
            """
            if res != 0:
                raise Exception("Non zero return code from ...", self.prog)
        else:
            print "Process not created ..."
