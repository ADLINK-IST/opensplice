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
   Class specific to the Lifecycle example as it runs more than a simple publisher/subscriber
"""
class lifecycle (Example):

    def __init__(self, host, logger):

        super(lifecycle, self).__init__(host, logger, "Lifecycle", "dcps")

        with open ('examples.json') as data_file:
            data = json.load(data_file)

        self.sub_params = data["dcps"]["Lifecycle"]["params"]["sub_params"]

        self.pub1_params = data["dcps"]["Lifecycle"]["params"]["pub1_params"]

        self.pub2_params = data["dcps"]["Lifecycle"]["params"]["pub2_params"]

        self.pub3_params = data["dcps"]["Lifecycle"]["params"]["pub3_params"]

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
                    print "In runExample for " + self.expath + ": " + self.name + ": " + lang

                    try:
                        super(lifecycle, self).setExampleResultDir(lang, extra)

                        with open ('examples.json') as data_file:
                            data = json.load(data_file)

                        pubName = data[self.expath][self.name]["executables"][lang]["pubName"]
                        subName = data[self.expath][self.name]["executables"][lang]["subName"]

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
                            super(lifecycle, self).setLogPathAndLogs(lang, extra)
                            pub1Log = os.path.join(self.pPath, 'publisher1.log')
                            pub2Log = os.path.join(self.pPath, 'publisher2.log')
                            pub3Log = os.path.join(self.pPath, 'publisher3.log')
                            sub1Log = os.path.join(self.pPath, 'subscriber1.log')
                            sub2Log = os.path.join(self.pPath, 'subscriber2.log')
                            sub3Log = os.path.join(self.pPath, 'subscriber3.log')

                            os.chdir(self.pPath)

                            pubexe = os.path.join(self.pPath, pubName) + exSfx

                            if os.path.isfile (pubexe):
                                pub1Thread = ExeThread(self.classpath, pub1Log, lang, pubexe, self.pub1_params, self.example_timeout * 2)
                            else:
                                msg = "MissingExecutable: " + pubexe

                            subexe = os.path.join(self.pPath, subName) + exSfx

                            if os.path.isfile (subexe):
                                sub1Thread = ExeThread(self.classpath, sub1Log, lang, subexe, self.sub_params, self.example_timeout)
                            else:
                                msg = "MissingExecutable: " + subexe

                            if msg == "NONE":
                                self.startOSPL()
                                print "Starting sub1Thread for ", subexe
                                sub1Thread.start()
                                print "Starting pub1Thread for ", pubexe
                                pub1Thread.start()

                                pub1Thread.join(self.example_timeout)
                                sub1Thread.join(self.example_timeout)

                        except Exception as ex:
                            msg = "Exception running " + str(ex)

                        self.stopOSPL()

                        if msg == "NONE":
                            pub2Thread = ExeThread(self.classpath, pub2Log, lang, pubexe, self.pub2_params, self.example_timeout * 2)

                            sub2Thread = ExeThread(self.classpath, sub2Log, lang, subexe, self.sub_params, self.example_timeout)

                            self.startOSPL()
                            print "Starting sub2Thread for ", subexe
                            sub2Thread.start()
                            print "Starting pub2Thread for ", pubexe
                            pub2Thread.start()

                            pub2Thread.join(self.example_timeout)
                            sub2Thread.join(self.example_timeout)

                            self.stopOSPL()

                            pub3Thread = ExeThread(self.classpath, pub3Log, lang, pubexe, self.pub3_params, self.example_timeout * 2)

                            sub3Thread = ExeThread(self.classpath, sub3Log, lang, subexe, self.sub_params, self.example_timeout)

                            self.startOSPL()
                            print "Starting sub3Thread for ", subexe
                            sub3Thread.start()
                            print "Starting pub3Thread for ", pubexe
                            pub3Thread.start()

                            pub3Thread.join(self.example_timeout)
                            sub3Thread.join(self.example_timeout)

                            self.stopOSPL()

                        if msg == "NONE":
                            try:
                                super(lifecycle, self).copyLogs()

                                if os.path.isfile (self.ospl_error_log):
                                    msg = "ospl-error.log found"

                                self.checkResults(sub1Log, sub_conds)
                                self.checkResults(sub2Log, sub_conds)
                                self.checkResults(sub3Log, sub_conds)
                                self.checkResults(pub1Log, pub_conds)
                                self.checkResults(pub2Log, pub_conds)
                                self.checkResults(pub3Log, pub_conds)

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

                        try:
                            self.writeResult (result,  self.expath +  self.name, lang, msg)
                        except Exception as ex:
                            print "Exception checking logs ", str(ex)

                        if self.host.isWindows():
                            time.sleep(5)

                        try:
                            self.cleanUp()
                        except Exception as ex:
                            print "Exception cleaning up ", str(ex)

                    except Exception as ex:
                        print "Unexpected exception ", str(ex)

                    finally:
                        os.chdir(currPath)
