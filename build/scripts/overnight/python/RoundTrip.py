import sys
import os
import json
import subprocess
import example_logparser
from example_exceptions import LogCheckFail
from Example import Example
from Example import ExeThread

import pdb

"""
   Class specific to the RoundTrip example as it runs more than a simple publisher/subscriber
"""
class roundtrip (Example):

    def __init__(self, host, logger):

        super(roundtrip, self).__init__(host, logger, "RoundTrip", "dcps")

        with open ('examples.json') as data_file:
            data = json.load(data_file)

        self.sub_params = data["dcps"]["RoundTrip"]["params"]["ping"]

        self.quit_params = data["dcps"]["RoundTrip"]["params"]["ping_quit"]


    def runExample(self, lang, extra, types):

        print "RoundTrip - runExample ..."

        if lang == "cs" and not self.host.isWindows():
            print "C# not supported on " + self.host.name
        else:
            if lang == "all":
                self.runExampleAll(extra)
            else:

                print "In runExample for " + self.expath + ": " + self.name + ": " + lang

                currPath = os.getcwd()

                try:
                    super(roundtrip, self).setExampleResultDir(lang, extra)

                    if lang == "java":
                        pubName = "Saj_RoundTrip_Pong.jar"
                        subName = "Saj_RoundTrip_Ping.jar"
                    elif lang == "java5":
                        pubName = "pong/java5_pong.jar"
                        subName = "ping/java5_ping.jar"
                    else:
                        pubName = "pong"
                        subName = "ping"

                    with open ('examples.json') as data_file:
                        data = json.load(data_file)

                    sub_conds_file = data[self.expath][self.name]["log_conditions_file"][lang]["sub_conds"]
                    pub_conds_file = data[self.expath][self.name]["log_conditions_file"][lang]["pub_conds"]

                    sub_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', sub_conds_file)
                    pub_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', pub_conds_file)

                    exSfx = ""

                    if self.host.isWindows() and not "java" in lang:
                        exSfx = ".exe"

                    msg = "NONE"
                    result = "PASS"

                    try:
                        super(roundtrip, self).setLogPathAndLogs(lang, extra)
                        pubLog = os.path.join(self.pPath, 'publisher.log')
                        subLog = os.path.join(self.pPath, 'subscriber.log')

                        pubexe = os.path.join(self.pPath, pubName) + exSfx

                        if os.path.isfile (pubexe):
                            pubThread = ExeThread(self.classpath, pubLog, lang, pubexe, "", self.example_timeout * 2)
                        else:
                            msg = "MissingExecutable: " + pubexe

                        subexe = os.path.join(self.pPath, subName) + exSfx

                        if os.path.isfile (pubexe):
                            subThread = ExeThread(self.classpath, subLog, lang, subexe, self.sub_params, self.example_timeout)
                        else:
                            msg = "MissingExecutable: " + subexe

                        quitexe = os.path.join(self.pPath, subName) + exSfx

                        if os.path.isfile (pubexe):
                            quitThread = ExeThread(self.classpath, subLog, lang, quitexe, self.quit_params, self.example_timeout)
                        else:
                            msg = "MissingExecutable: " + quitexe

                        os.chdir(self.pPath)

                        if msg == "NONE":
                            self.startOSPL()
                            print "Starting pubThread for ", pubexe
                            pubThread.start()
                            print "Starting subThread for ", subexe
                            subThread.start()

                            subThread.join(self.example_timeout)
                            print "Starting quitThread for ", quitexe
                            quitThread.start()
                            pubThread.join(self.example_timeout)
                            quitThread.join(self.example_timeout)

                    except Exception:
                        msg = "Exception running " + str(sys.exc_info()[0])

                    try:
                        self.stopOSPL()
                    except Exception as ex:
                        print "Exception stopping OpenSplice ", str(ex)

                    if msg == "NONE":
                        try:
                            super(roundtrip, self).copyLogs()

                            if os.path.isfile (self.ospl_error_log):
                                msg = "ospl-error.log found"
                            #pdb.set_trace()
                            self.checkResults(subLog, sub_conds)
                            self.checkResults(pubLog, pub_conds)

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

                    try:
                        self.writeResult (result,  self.expath +  self.name, lang, msg)
                    except Exception as ex:
                        print "Exception writing result", str(ex)

                    try:
                        self.cleanUp()
                    except Exception as ex:
                        print "Exception cleaning up", str(ex)

                except Exception as ex:
                    print "Unexpected exception ", str(ex)
                finally:
                    os.chdir(currPath)
