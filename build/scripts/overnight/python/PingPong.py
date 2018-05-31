import sys
import os
import json
import subprocess
import time
import example_logparser
from example_exceptions import LogCheckFail
from Example import Example
from Example import ExeThread

"""
   Class specific to the PingPong example as it runs more than a simple publisher/subscriber
"""
class pingpong (Example):

    def __init__(self, host, logger):

        super(pingpong, self).__init__(host, logger, "PingPong", "dcps")

        with open ('examples.json') as data_file:
            data = json.load(data_file)

        self.types = data["dcps"]["PingPong"]["params"]["ping"]

        self.BLOCKSIZE = data["dcps"]["PingPong"]["params"]["BLOCKSIZE"]

        self.BLOCKCOUNT = data["dcps"]["PingPong"]["params"]["BLOCKCOUNT"]

        self.quit_params = data["dcps"]["PingPong"]["params"]["ping-quit"]

        self.partitions = data["dcps"]["PingPong"]["params"]["partitions"]

        self.pingTimeout = 25

        self.pongTimeout = 125

        self.pingQuitArgs = self.quit_params
        self.pingQuitArgs.extend(self.partitions)

    def runExample(self, lang, extra, types):
        print("runExample ", lang, extra, types)
        if lang == "cs" and not self.host.isWindows():
            print("C# not supported on " + self.host.name)
        else:
            if lang == "all":
                self.runExampleAll(extra)
            else:
                if extra == "all":
                    self.runExampleAllExtra(lang, extra, types)
                else:
                    currPath = os.getcwd()     
        
                    if lang == "cs":
                        ping_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)),  'yaml', 'ping_cs_conditions.yaml')
                    else:
                        ping_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)),  'yaml', 'ping_conditions.yaml')

                    pong_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)),  'yaml', 'pong_conds.yaml')

                    try:
                        super(pingpong, self).setExampleResultDir(lang, extra)

                        msg = "NONE"
                        result = "PASS"

                        exSfx = ""

                        isJava = False

                        if lang == "java" or lang == "java5":
                            isJava = True

                        if self.host.isWindows() and not isJava:
                            exSfx = ".exe"

                        try:
                            self.setLogPathAndLogs(lang, extra)

                            pingLog = os.path.join(self.pPath, 'ping.log')
                            pongLog = os.path.join(self.pPath, 'pong.log')

                            with open ('examples.json') as data_file:
                                data = json.load(data_file)

                            os.chdir(self.pPath)

                            if extra == "corba" and isJava:
                                if lang == "java":
                                    runLang = "cj"
                                    corbaJar = "dcpscj.jar"
                                else:
                                    runLang = "cj5"
                                    corbaJar = "dcpscj5.jar"

                                ospljar = os.path.join(os.environ['OSPL_HOME'], "jar", corbaJar)
                                classes = os.path.join(os.environ['OSPL_HOME'], "examples", self.expath, self.name, lang, extra, "classes")
                                self.classpath = ospljar + os.pathsep + classes
                            else:
                                runLang = lang

                            print("runLang is ", runLang)

                            if extra == "corba":
                                exes = "corba_executables"
                            else:
                                exes = "executables"

                            pongName = data[self.expath][self.name][exes][lang]["pubName"]
                            pingName  = data[self.expath][self.name][exes][lang]["subName"]

                            if pongName != "":
                                if self.classpath == "" and not isJava:
                                    pongExe = os.path.join(self.pPath, pongName) + exSfx
                                    if not os.path.isfile (pongExe):
                                        msg = "MissingExecutable: " + pongExe

                                else:
                                    pongExe = pongName

                            if pingName != "":
                                if self.classpath == "" and not isJava:
                                    pingExe = os.path.join(self.pPath, pingName) + exSfx
                                    if not os.path.isfile (pingExe):
                                        msg = "MissingExecutable: " + pingExe
                                else:
                                    pingExe = pingName

                            if msg == "NONE":
                                self.startOSPL()

                                print("Going to start pong thread")
                                pongThread = ExeThread(self.classpath, pongLog, runLang, pongExe, self.partitions, self.pongTimeout)  
                                pongThread.start()

                        except Exception as ex:
                            msg = "Failure running PingPong:  pong " + str(ex)

                        if msg == "NONE":
                            try:
                                if types == "all":
                                    runTypes = self.types
                                else:
                                    runTypes = types

                                for t in runTypes:
                                    print("Running ping with", t)

                                    pingArgs = [self.BLOCKSIZE, self.BLOCKCOUNT, t]
                                    pingArgs.extend(self.partitions)
                                    pingThread = ExeThread(self.classpath, pingLog, runLang, pingExe, pingArgs, self.pingTimeout)  
                                    pingThread.start()
                                    pingThread.join(self.pingTimeout)

                                    # Allow time for output to be written to log
                                    time.sleep(2)                                                               
                            except Exception as ex:
                                msg = "Failure running PingPong:  ping " + str(ex)

                        if msg == "NONE":
                            try:
                                pingQuitThread = ExeThread(self.classpath, pingLog, runLang, pingExe, self.pingQuitArgs, self.pingTimeout)
                                pingQuitThread.start()
                                pingQuitThread.join(self.pingTimeout)
                                pongThread.join(self.pongTimeout)
                            except:
                                msg = "Failure running PingPong: ping quit " + str(sys.exc_info()[0])
  
                        try:
                            self.stopOSPL()
                        except Exception as ex:
                            print("Exception stopping OpenSplice ", str(ex))

                        try:
                            self.copyLogs()

                            if os.path.isfile (self.ospl_error_log):
                                msg = "ospl-error.log found"
                             
                            self.checkResults(pingLog, ping_conds)

                            self.checkResults(pongLog, pong_conds)
  
                            self.checkOSPLInfoLog(self.ospl_info_log)
                        except LogCheckFail as lf:
                            reason = str(lf)
                            if "Ignore excess messages" in reason:
                                msg = "LogCheckFail: \"Ignore excess messages\" in run.log"
                            elif "PING_min triggered, but no data available" in reason:
                                msg = "LogCheckFail: \"PING_min triggered\" in run.log"
                            elif "OpenSpliceDDS Warnings" in reason:
                                msg = "LogCheckFail: OpenSpliceDDS Warnings in ospl-info.log"
                            else:
                                msg = "LogCheckFail: " + str (lf)
                        except Exception as ex:
                            msg = "Exception: " + str (ex)

                        if msg != "NONE":
                            result = "FAIL"

                        resultLang = lang

                        if extra == "corba":
                            if lang == "cpp":
                                resultLang = "ccpp"
                            elif lang == "java":
                                resultLang = "cj"
                            elif lang == "java5":
                                resultLang = "cj5"

                        try:
                            print("WRiting result for ", self.name + " " + resultLang)
                            self.writeResult (result, self.expath +  self.name, resultLang, msg)
                        except Exception as ex:
                            print("Exception writing result", str(ex))

                        try:
                            self.cleanUp()
                        except Exception as ex:
                            print("Exception cleaning up", str(ex))

                    except Exception as ex:
                        print("Unexpected exception", str(ex))
                    finally:
                        os.chdir(currPath)
