import sys
import os
import json
import shutil
import subprocess
import fileinput
import platform
import time
from shutil import copy
import example_logparser
from example_exceptions import LogCheckFail
from Example import Example
from Example import ExeThread
import pdb

"""
   Class specific to the DBMSConnect example as it is very different
   to all other examples having a different directory structure and
   also runs more than a simple publisher/subscriber
"""
class dbmsconnect (Example):

    def __init__(self, host, logger):

        super(dbmsconnect, self).__init__(host, logger, "dbmsconnect", "services")

        with open ('examples.json') as data_file:
            data = json.load(data_file)

        self.odbcMsgBoard_params = data["services"]["dbmsconnect"]["params"]["odbcMsgBoard_params"]

        self.odbcChatter1_params = data["services"]["dbmsconnect"]["params"]["odbcChatter1_params"]

        self.odbcChatter2_params = data["services"]["dbmsconnect"]["params"]["odbcChatter2_params"]

        self.cppChatter1_params = data["services"]["dbmsconnect"]["params"]["cppChatter1_params"]

        self.cppChatter2_params = data["services"]["dbmsconnect"]["params"]["cppChatter2_params"]

        self.odbcChatterQuit_params = data["services"]["dbmsconnect"]["params"]["odbcChatterQuit_params"]

        self.cppChatterQuit_params = data["services"]["dbmsconnect"]["params"]["cppChatterQuit_params"]

        super(dbmsconnect, self).setPath(os.path.join(os.environ['OSPL_HOME'], 'examples', 'services', 'dbmsconnect', 'SQL', 'C++', 'ODBC'))       

        if os.environ['EXRUNTYPE'] == "shm":
            self.uri = "file://" + os.path.join(os.environ['OSPL_HOME'], 'examples', 'services', 'dbmsconnect', self.shm_uri) 
        else:
            self.uri = "file://" + os.path.join(os.environ['OSPL_HOME'], 'examples', 'services', 'dbmsconnect', self.sp_uri)                    

        self.runDBMSConnect = self.host.runExample(self.expath, self.name, "")

    def runExample(self):

        print "In runExample for " + self.expath + ": " + self.name

        currPath = os.getcwd()

        try:
            self.exdir = "servicesdbmsconnectSQLCPPODBC"

            exSfx = ""
  
            if self.host.isWindows():
                exSfx = ".exe"
                os.putenv("ODBC_LIB_NAME", "odbc32")
            else:
                os.putenv("ODBC_LIB_NAME", "odbc")

            msg = "NONE"
            result = "PASS"

            dsn = self.odbcMsgBoard_params[0]
            os.putenv("MY_DSN", dsn);
            os.environ["MY_DSN"]= dsn;

            os.putenv("OSPL_URI", self.uri)
            os.environ["OSPL_URI"] = self.uri
            
            try:
                self.convertConfig()

                self.setLogPathAndLogs("", "")
                odbcMsgBoardLog = os.path.join(self.pPath, 'odbcMsgBoard.log')
                odbcChatter1Log = os.path.join(self.pPath, 'odbcChatter1.log')
                odbcChatter2Log = os.path.join(self.pPath, 'odbcChatter2.log')
                odbcChatterQuitLog = os.path.join(self.pPath, 'odbcChatterQuit.log')
                cppMsgBoardLog = os.path.join(self.pPath, 'cppMsgBoard.log')
                cppChatter1Log = os.path.join(self.pPath, 'cppChatter1.log')
                cppChatter2Log = os.path.join(self.pPath, 'cppChatter2.log')
                cppChatterQuitLog = os.path.join(self.pPath, 'cppChatterQuit.log')

                with open ('examples.json') as data_file:
                    data = json.load(data_file)

                odbcMsgBoardName = data[self.expath][self.name]["executables"]["odbc"]["msgBoardName"]
                odbcChatterName = data[self.expath][self.name]["executables"]["odbc"]["chatterName"]
                cppMsgBoardName = data[self.expath][self.name]["executables"]["cpp"]["msgBoardName"]
                cppChatterName = data[self.expath][self.name]["executables"]["cpp"]["chatterName"]

                odbcmsgboard_conds_file = data[self.expath][self.name]["log_conditions_file"]["odbcmsgboard_conds"]
                cppmsgboard_conds_file = data[self.expath][self.name]["log_conditions_file"]["msgboard_conds"]
                odbcchatter_conds_file = data[self.expath][self.name]["log_conditions_file"]["odbcchatter_conds"]
                chatter_conds_file = data[self.expath][self.name]["log_conditions_file"]["chatter_conds"]

                odbcmsgboard_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', odbcmsgboard_conds_file)
                odbcchatter_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', odbcchatter_conds_file)
                cppmsgboard_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', cppmsgboard_conds_file)
                cppchatter_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', chatter_conds_file)

                if odbcMsgBoardName != "":
                    if self.classpath == "":
                        odbcMsgBoardExe = os.path.join(self.pPath, odbcMsgBoardName) + exSfx
                        if not os.path.isfile (odbcMsgBoardExe):
                            msg = "MissingExecutable: " + odbcMsgBoardExe
                    else:
                        odbcMsgBoardExe = odbcMsgBoardName

                if odbcChatterName != "":
                    if self.classpath == "":
                        odbcChatterNameExe = os.path.join(self.pPath, odbcChatterName) + exSfx
                        if not os.path.isfile (odbcChatterNameExe):
                            msg = "MissingExecutable: " + odbcChatterNameExe
                    else:
                        odbcChatterNameExe = odbcChatterName

                cppPath = os.path.join(os.environ['OSPL_HOME'], 'examples', "dcps", "Tutorial", "cpp", "standalone")

                if cppMsgBoardName != "":
                    if self.classpath == "":
                        cppMsgBoardExe = os.path.join(cppPath, cppMsgBoardName) + exSfx
                        if not os.path.isfile (cppMsgBoardExe):
                            msg = "MissingExecutable: " + cppMsgBoardExe
                    else:
                        cppMsgBoardExe = cppMsgBoardName

                if cppChatterName != "":
                    if self.classpath == "":
                        cppChatterNameExe = os.path.join(cppPath, cppChatterName) + exSfx
                        if not os.path.isfile (cppChatterNameExe):
                            msg = "MissingExecutable: " + cppChatterNameExe
                    else:
                        cppChatterNameExe = cppChatterName

                if msg == "NONE":  
                    odbcMsgBoard_Thread = ExeThread(self.classpath, odbcMsgBoardLog, "", odbcMsgBoardExe, self.odbcMsgBoard_params, self.example_timeout * 2)
                    odbcChatter1_Thread = ExeThread(self.classpath, odbcChatter1Log, "", odbcChatterNameExe, self.odbcChatter1_params, self.example_timeout)
                    odbcChatter2_Thread = ExeThread(self.classpath, odbcChatter2Log, "", odbcChatterNameExe, self.odbcChatter2_params, self.example_timeout)
                    cppMsgBoard_Thread = ExeThread(self.classpath, cppMsgBoardLog, "", cppMsgBoardExe, "", self.example_timeout * 2)

                    cppChatter1_Thread = ExeThread(self.classpath, cppChatter1Log, "", cppChatterNameExe, self.cppChatter1_params, self.example_timeout)
                    cppChatter2_Thread = ExeThread(self.classpath, cppChatter2Log, "", cppChatterNameExe, self.cppChatter2_params, self.example_timeout)

                    odbcChatterQuit_Thread = ExeThread(self.classpath, odbcChatterQuitLog, "", odbcChatterNameExe, self.odbcChatterQuit_params, self.example_timeout)
                    cppChatterQuit_Thread = ExeThread(self.classpath, cppChatterQuitLog, "", cppChatterNameExe, self.cppChatterQuit_params, self.example_timeout)
 
                    os.chdir(self.pPath)   
   
                    self.startOSPL()

                    cppMsgBoard_Thread.start()
                    odbcMsgBoard_Thread.start()                                            
                    time.sleep(5)
                    odbcChatter1_Thread.start()
                    odbcChatter2_Thread.start()
                    cppChatter1_Thread.start()
                    cppChatter2_Thread.start()

                    odbcChatter1_Thread.join(self.example_timeout)
                    odbcChatter2_Thread.join(self.example_timeout)
                    cppChatter1_Thread.join(self.example_timeout)
                    cppChatter2_Thread.join(self.example_timeout)

                    time.sleep(10)
                    odbcChatterQuit_Thread.start()
                    cppChatterQuit_Thread.start()

                    odbcChatterQuit_Thread.join(self.example_timeout)
                    cppChatterQuit_Thread.join(self.example_timeout)
                    cppMsgBoard_Thread.join(self.example_timeout)
                    odbcMsgBoard_Thread.join(self.example_timeout)  

            except Exception as ex:
                msg = "Exception running ", str(ex)

            try:
                self.stopOSPL()
            except Exception as ex:
                print "Exception stopping OpenSplice ", str(ex)

            if msg == "NONE":
                try:
                    #Allow time for all messages to be written to log
                    time.sleep (15)
                    super(dbmsconnect, self).copyLogs()

                    if os.path.isfile (self.ospl_error_log):
                        msg = "ospl-error.log found"
                             
                    print "checking odbcMsgBoardLog with odbcmsgboard_conds", odbcMsgBoardLog, odbcmsgboard_conds
                    self.checkResults(odbcMsgBoardLog, odbcmsgboard_conds)

                    print "checking odbcChatter1Log with odbcchatter_conds", odbcChatter1Log, odbcchatter_conds
                    self.checkResults(odbcChatter1Log, odbcchatter_conds)

                    print "checking odbcChatter2Log with odbcchatter_conds", odbcChatter2Log, odbcchatter_conds
                    self.checkResults(odbcChatter2Log, odbcchatter_conds)

                    self.checkResults(cppMsgBoardLog, cppmsgboard_conds)
                    self.checkResults(cppChatter1Log, cppchatter_conds)
                    self.checkResults(cppChatter2Log, cppchatter_conds)

                    self.checkOSPLInfoLog(self.ospl_info_log)
                except LogCheckFail as lf:
                    reason = str(lf)
                    if "OpenSpliceDDS Warnings" in reason:
                        msg = "LogCheckFail: OpenSpliceDDS Warnings in ospl-info.log"
                    else:
                        msg = "LogCheckFail: " + str(lf)
                except Exception:
                    msg = "Exception checking logs " + str(sys.exc_info()[0])
         
                logdir =  os.path.join(os.environ['LOGDIR'], "examples", "run_" + os.environ['EXRUNTYPE'], self.exdir)
                dbmsconnLog = os.path.join(self.pPath, 'dbmsconnect.log')
                print "dbmsconnect.log is ", dbmsconnLog
                copy(dbmsconnLog, logdir)

                
            if msg != "NONE":
                result = "FAIL"

            try:
                self.writeResult (result,  self.exdir, "", msg)
            except Exception as ex:
                print "Exception writing result", str(ex)

            try:
                self.cleanUp()
            except Exception as ex:
                print "Exception cleaning  up", str(ex)

        except Exception as ex:
            print "Unexpected exception ", str(ex)   
        finally:
            os.chdir(currPath)

    def convertConfig(self):
        if os.environ['EXRUNTYPE'] == "shm":
            uri = self.shm_uri
        else:
            uri = self.sp_uri
        fcfg = os.path.join(os.environ['OSPL_HOME'], 'examples', 'services', 'dbmsconnect', uri)
        forig = os.path.join(os.environ['OSPL_HOME'], 'examples', 'services', 'dbmsconnect', uri+'.orig')
        os.rename(fcfg, forig)
        if self.host.name != "default":
            hn = self.host.name
        else:
            hn = platform.uname()[1]
        prefix = hn[:16].replace('-', '_') + '_'
        fout = open(fcfg, "w")
        for line in fileinput.input(forig):
            fout.write(line.replace("Sql", prefix))
        fout.close()
