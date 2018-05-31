import sys
import os
import json
import subprocess
import time
import example_logparser
from example_exceptions import LogCheckFail
from Example import Example
from Example import ExeThread
#import pdb

"""
   Class specific to the Tutorial example as it runs more than a simple publisher/subscriber
"""
class tutorial (Example):

    def __init__(self, host, logger):

        super(tutorial, self).__init__(host, logger, "Tutorial", "dcps")

        with open ('examples.json') as data_file:
            data = json.load(data_file)

        self.msgboard_params = data["dcps"]["Tutorial"]["params"]["messageboard"]

        self.chatter_params = data["dcps"]["Tutorial"]["params"]["chatter"]

        self.quit_params = data["dcps"]["Tutorial"]["params"]["quit"]


    def runExample(self, lang, extra, types):
        if lang == "cs" and not self.host.isWindows():
            print("C# not supported on " + self.host.name)
        else:
            if lang == "all":
                self.runExampleAll(extra)
            else: 
                if extra == "all":
                    self.runExampleAllExtra(lang, extra, types)
                else:
                    print("In runExample for " + self.expath + ": " + self.name + ": " + lang + ":" + extra)

                    currPath = os.getcwd()

                    try:
                        super(tutorial, self).setExampleResultDir(lang, extra)

                        with open ('examples.json') as data_file:
                            data = json.load(data_file)

                        msgBoardName = data[self.expath][self.name]["executables"][lang]["msgBoardName"]
                        chatterName = data[self.expath][self.name]["executables"][lang]["chatterName"]
                        userLoadName = data[self.expath][self.name]["executables"][lang]["userLoadName"]

                        msgboard_conds_file = data[self.expath][self.name]["log_conditions_file"][lang]["msgboard_conds"]
                        chatter_conds_file = data[self.expath][self.name]["log_conditions_file"][lang]["chatter_conds"]

                        msgboard_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', msgboard_conds_file)
                        chatter_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', chatter_conds_file)

                        exSfx = ""
      
                        if self.host.isWindows() and not "java" in lang:
                            exSfx = ".exe"
  
                        msg = "NONE"
                        result = "PASS"

                        try:
                            self.setLogPathAndLogs(lang, extra)
                            msgBoardLog = os.path.join(self.pPath, 'msgBoard.log')
                            chatterLog = os.path.join(self.pPath, 'chatter.log')
                            userLoadLog = os.path.join(self.pPath, 'userLoad.log')

                            if extra == "corba" and lang == "java":
                                ospljar = os.path.join(os.environ['OSPL_HOME'], "jar", "dcpscj.jar")
                                classes = os.path.join(os.environ['OSPL_HOME'], "examples", self.expath, self.name, lang, extra, "classes")
                                self.classpath = ospljar + os.pathsep + classes
                                runLang = "cj"
                            else:
                                runLang = lang

                            with open ('examples.json') as data_file:
                                data = json.load(data_file)

                            if extra == "corba":
                                exes = "corba_executables"
                            else:
                                exes = "executables"

                            msgBoardName = data[self.expath][self.name][exes][lang]["msgBoardName"]
                            chatterName  = data[self.expath][self.name][exes][lang]["chatterName"]
                            userLoadName = data[self.expath][self.name][exes][lang]["userLoadName"]

                            if msgBoardName != "":
                                if self.classpath == "" and runLang is not "cj":
                                    msgBoardExe = os.path.join(self.pPath, msgBoardName) + exSfx
                                    if not os.path.isfile (msgBoardExe):
                                        msg = "MissingExecutable: " + msgBoardExe
                                else:
                                    msgBoardExe = msgBoardName

                            if userLoadName != "":
                                if self.classpath == "" and runLang is not "cj":
                                    userLoadExe = os.path.join(self.pPath, userLoadName) + exSfx
                                    if not os.path.isfile (userLoadExe):
                                        msg = "MissingExecutable: " + userLoadExe
                                else:
                                    userLoadExe = userLoadName

                            if chatterName != "":
                                if self.classpath == "" and runLang is not "cj":
                                    chatterExe = os.path.join(self.pPath, chatterName) + exSfx
                                    chatterquitExe = os.path.join(self.pPath, chatterName) + exSfx
                                    if not os.path.isfile (chatterExe):
                                        msg = "MissingExecutable: " + chatterExe
                                else:
                                    chatterExe = chatterName
                                    chatterquitExe = chatterName

                            if msg == "NONE":  
                                msgBoardThread = ExeThread(self.classpath, msgBoardLog, runLang, msgBoardExe, self.msgboard_params, self.example_timeout * 2)
                                userLoadThread = ExeThread(self.classpath, userLoadLog, runLang, userLoadExe, "", self.example_timeout * 2)
                                chatterThread = ExeThread(self.classpath, chatterLog, runLang, chatterExe, self.chatter_params, self.example_timeout) 
                                quitThread = ExeThread(self.classpath, chatterLog, runLang, chatterquitExe, self.quit_params, self.example_timeout)
   
                                os.chdir(self.pPath)   

                                self.startOSPL()
                                msgBoardThread.start()
                                #Allow time for the message board to get started...
                                time.sleep(5)
                                userLoadThread.start()
                                chatterThread.start()
                                            
                                chatterThread.join(self.example_timeout)

                                quitThread.start()
                                msgBoardThread.join(self.example_timeout)
                                userLoadThread.join(self.example_timeout)
                                quitThread.join(self.example_timeout)                        

                        except Exception:
                            msg = "Exception running " + str(sys.exc_info()[0])

                        try:
                            self.stopOSPL()
                        except Exception as ex:
                            print("Exception stopping OpenSplice ", str(ex))

                        if msg == "NONE":
                            try:
                                super(tutorial, self).copyLogs()

                                if os.path.isfile (self.ospl_error_log):
                                    msg = "ospl-error.log found"
                             
                                self.checkResults(msgBoardLog, msgboard_conds)
                                self.checkResults(chatterLog, chatter_conds)

                                self.checkOSPLInfoLog(self.ospl_info_log)

                            except LogCheckFail as lf:
                                reason = str(lf)
                                if "OpenSpliceDDS Warnings" in reason:
                                    msg = "LogCheckFail: OpenSpliceDDS Warnings in ospl-info.log"
                                else:
                                    msg = "LogCheckFail: " + str(lf)
                            except Exception:
                                msg = "Exception checking logs " + str(sys.exc_info()[0])

                        if msg != "NONE":
                            result = "FAIL"

                        resultLang = lang

                        if extra == "corba":
                            if lang == "cpp":
                                resultLang = "ccpp"
                            elif lang == "java":
                                resultLang = "cj"

                        try:
                            self.writeResult (result,  self.expath + self.name, resultLang, msg)
                        except Exception as ex:
                            print("Exception writing result", str(ex))

                        try:
                            self.cleanUp()
                        except Exception as ex:
                            print("Exception cleaning up", str(ex))

                    except Exception as ex:
                        print("Unexpected exception ", str(ex))
                    finally:
                        os.chdir(currPath)   
