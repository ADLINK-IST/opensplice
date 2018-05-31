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
#import pdb

"""
   Class specific to the RMI HelloWorld example as it runs more than a simple publisher/subscriber
"""
class rmihelloworld (Example):

    def __init__(self, host, logger):

        super(rmihelloworld, self).__init__(host, logger, "HelloWorld", "rmi")

    def runExample(self, lang, extra, types):
        if lang == "cs" and not self.host.isWindows():
            print("C# not supported on " + self.host.name)
        else:
            if lang == "all":
                self.runExampleAll(extra)
            else: 
                currPath = os.getcwd()  

                try:
                    self.setExampleResultDir(lang, extra)

                    with open ('examples.json') as data_file:
                        data = json.load(data_file)

                    pubName = data[self.expath][self.name]["executables"][lang]["pubName"]
                    pubParams = data[self.expath][self.name]["params"]["pub_params"]  
                    subName = data[self.expath][self.name]["executables"][lang]["subName"]
                    subParams = data[self.expath][self.name]["params"]["sub_params"] 
                    sub2Name = data[self.expath][self.name]["executables"][lang]["sub2Name"]
                    sub2Params = data[self.expath][self.name]["params"]["sub2_params"] 
 
                    exSfx = ""
  
                    if self.host.isWindows() and not "java" in lang:
                        exSfx = ".exe"

                    msg = "NONE"
                    result = "PASS"

                    try:
                        super(rmihelloworld, self).setLogPathAndLogs(lang, extra)

                        pub1Log = os.path.join(self.pPath, 'publisher1.log')
                        pub2Log = os.path.join(self.pPath, 'publisher2.log')
                        sub1Log = os.path.join(self.pPath, 'subscriber1.log')
                        sub2Log = os.path.join(self.pPath, 'subscriber2.log')

                        sub_conds_file = data[self.expath][self.name]["log_conditions_file"][lang]["sub_conds"]
                        pub_conds_file = data[self.expath][self.name]["log_conditions_file"][lang]["pub_conds"]

                        sub_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', sub_conds_file)
                        pub_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', pub_conds_file)

                        os.chdir(self.pPath)   

                        if self.classpath == "":
                            pubexe = os.path.join(self.pPath, pubName) + exSfx
                            if not os.path.isfile (pubexe):
                                msg = "MissingExecutable: " + pubexe
                        else:
                            pubexe = pubName

                        pubThread = ExeThread(self.classpath, pub1Log, lang, pubexe, "", self.example_timeout)
                        pub2Thread = ExeThread(self.classpath, pub2Log, lang, pubexe, "", self.example_timeout)                    

                        if self.classpath == "":
                            subexe = os.path.join(self.pPath, subName) + exSfx
                            if not os.path.isfile (subexe):
                                msg = "MissingExecutable: " + subexe

                            sub2exe =  os.path.join(self.pPath, sub2Name) + exSfx
                            if not os.path.isfile (sub2exe):
                                msg = "MissingExecutable: " + sub2exe                    
                        else:
                            subexe = subName
                            sub2exe = sub2Name

                        subThread = ExeThread(self.classpath, sub1Log, lang, subexe, "", self.example_timeout)
                        sub2Thread = ExeThread(self.classpath, sub2Log, lang, sub2exe, "", self.example_timeout)
                   
                        self.startOSPL()

                        pubThread.start()
                        time.sleep(1)
                        subThread.start()
                        pubThread.join(self.example_timeout)
                        subThread.join(self.example_timeout)

                        pub2Thread.start()
                        time.sleep(1)
                        sub2Thread.start()
                        pub2Thread.join(self.example_timeout)
                        sub2Thread.join(self.example_timeout)

                    except Exception:
                        msg = "Exception running " + str(sys.exc_info()[0])

                    try:
                        self.stopOSPL()
                    except Exception as ex:
                        print("Exception stopping OpenSplice ", str(ex))

                    if msg == "NONE":
                        try:
                            super(rmihelloworld, self).copyLogs()

                            if os.path.isfile (self.ospl_error_log):
                                msg = "ospl-error.log found"
                             
                            #self.checkResults(sub1Log, sub_conds)
                            #self.checkResults(sub2Log, sub_conds)
                            #self.checkResults(pub1Log, pub_conds)
                            #self.checkResults(pub2Log, pub_conds)

                            self.checkOSPLInfoLog(self.ospl_info_log)

                        except LogCheckFail as lf:
                            if "OpenSpliceDDS Warnings" in reason:
                                msg = "LogCheckFail: OpenSpliceDDS Warnings in ospl-info.log"
                            else:
                                msg = "LogCheckFail: " + str(lf)
                        except Exception:
                            msg = "Exception checking logs " + str(sys.exc_info()[0])

                    if msg != "NONE":
                        result = "FAIL"

                    try:
                        self.writeResult (result,  self.expath +  self.name, lang, msg)
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
