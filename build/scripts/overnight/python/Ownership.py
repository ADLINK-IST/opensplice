import sys
import os
import json
import subprocess
import time
import example_logparser
from example_exceptions import LogCheckFail
from example_exceptions import MissingExecutable
from example_exceptions import ExampleFail
from Example import Example
from Example import ExeThread

"""
   Class specific to the Ownership example as it runs more than a simple publisher/subscriber
"""
class ownership (Example):

    def __init__(self, host, logger):

        super(ownership, self).__init__(host, logger, "Ownership", "dcps")

        with open ('examples.json') as data_file:
            data = json.load(data_file)

        self.pub1_params = data["dcps"]["Ownership"]["params"]["pub1"]

        self.pub2_params = data["dcps"]["Ownership"]["params"]["pub2"]

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
                    currPath = os.getcwd()

                    try:
                        super(ownership, self).setExampleResultDir(lang, extra)

                        with open ('examples.json') as data_file:
                            data = json.load(data_file)

                        pubName = data[self.expath][self.name]["executables"][lang]["pubName"]
                        subName = data[self.expath][self.name]["executables"][lang]["subName"]

                        sub_conds_file = data[self.expath][self.name]["log_conditions_file"][lang]["sub_conds"]
                        pub_conds_file = data[self.expath][self.name]["log_conditions_file"][lang]["pub_conds"]

                        sub_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', sub_conds_file)
                        pub_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', pub_conds_file)

                        msg = "NONE"
                        result = "PASS"
                   
                        try:
                            super(ownership, self).setLogPathAndLogs(lang, extra)   
                            pub1Log = os.path.join(self.pPath, 'publisher1.log')
                            pub2Log = os.path.join(self.pPath, 'publisher2.log')
                            subLog = os.path.join(self.pPath, 'subscriber.log')

                            exSfx = ""
  
                            if self.host.isWindows() and not "java" in lang:
                                exSfx = ".exe"

                            pub1exe = os.path.join(self.pPath, pubName) + exSfx

                            if "cs" in lang:
                                self.pub1_params[3] = "true"
                                self.pub2_params[3] = "false"
                            else:
                                self.pub1_params[3] = "1"
                                self.pub2_params[3] = "0"

                            if os.path.isfile (pub1exe):
                                pub1Thread = ExeThread(self.classpath, pub1Log, lang, pub1exe, self.pub1_params, self.example_timeout + 10)
                            else:
                                msg = "MissingExecutable: " + pub1exe                    

                            subexe = os.path.join(self.pPath, subName) + exSfx

                            if os.path.isfile (subexe):
                                subThread = ExeThread(self.classpath, subLog, lang, subexe, "", self.example_timeout)
                            else:
                                msg = "MissingExecutable: " + subexe
                   
                            pub2exe = os.path.join(self.pPath, pubName) + exSfx

                            if os.path.isfile (pub2exe):
                                pub2Thread = ExeThread(self.classpath, pub2Log, lang, pub2exe, self.pub2_params, self.example_timeout)
                            else:
                                msg = "MissingExecutable: " + pub2exe

                            os.chdir(self.pPath)

                            if msg == "NONE":
                                self.startOSPL()

                                pub1Thread.start()
                                time.sleep(2)
                                subThread.start()
                                time.sleep(2)
                                pub2Thread.start()
                                pub1Thread.join(self.example_timeout)
                                subThread.join(self.example_timeout)
                                pub2Thread.join(self.example_timeout)

                        except Exception:
                            msg = "Exception running " + str(sys.exc_info()[0])

                        self.stopOSPL()

                        if msg == "NONE":
                            try:
                                super(ownership, self).copyLogs()

                                if os.path.isfile (self.ospl_error_log):
                                    msg = "ospl-error.log found"
                             
                                self.checkResults(subLog, sub_conds)
                                self.checkResults(pub1Log, pub_conds)
                                self.checkResults(pub2Log, pub_conds)

                                self.checkOSPLInfoLog(self.ospl_info_log)

                            except LogCheckFail as lf:
                                reason = str(lf)
                                if "OpenSpliceDDS Warnings" in reason:
                                    msg = "LogCheckFail: OpenSpliceDDS Warnings in ospl-info.log"
                                else:
                                    msg = "LogCheckFail: " + str(lf)                        
                            except Exception:
                                msg = "Exception checking logs" + str(sys.exc_info()[0])

                        if msg != "NONE":
                            result = "FAIL"

                        try:
                            self.writeResult (result, self.expath + self.name, lang, msg)
                        except Exception as ex:
                            print "Failure writing result", str(ex)
 
                        try:
                            self.cleanUp()
                        except Exception as ex:
                            print "Failure cleaning up", str(ex)
  
                    except Exception as ex:
                        print "Unexpected exception", str(ex)
                    finally:
                        os.chdir(currPath)       
