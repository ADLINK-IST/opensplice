import sys
import os
import json
import shutil
import subprocess
import time
import example_logparser
from example_exceptions import LogCheckFail
from Example import Example
from Example import ExeThread
#import pdb

"""
   Class specific to the Durability example as it runs more than a simple publisher/subscriber
"""
class durability (Example):

    def __init__(self, host, logger):

        super(durability, self).__init__(host, logger, "Durability", "dcps")

        with open ('examples.json') as data_file:
            data = json.load(data_file)

        self.transSub_params = data["dcps"]["Durability"]["params"]["trans_sub"]

        self.persSub_params = data["dcps"]["Durability"]["params"]["pers_sub"]

        self.pub1_params = data["dcps"]["Durability"]["params"]["pub_1"]

        self.pub2_params = data["dcps"]["Durability"]["params"]["pub_2"]

        self.pub3_params = data["dcps"]["Durability"]["params"]["pub_3"]


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

                    print "In runExample for " + self.expath + ": " + self.name + ": " + lang + ":" + extra

                    currPath = os.getcwd()
                    try:
                        super(durability, self).setExampleResultDir(lang, extra)

                        exSfx = ""

                        if self.host.isWindows() and not "java" in lang:
                            exSfx = ".exe"

                        msg = "NONE"
                        result = "PASS"

                        os.putenv("OSPL_URI", self.uri)
                        os.environ["OSPL_URI"] = self.uri

                        try:
                            self.setLogPathAndLogs(lang, extra)

                            pub1Log = os.path.join(self.pPath, 'publisher1.log')
                            pub2Log = os.path.join(self.pPath, 'publisher2.log')
                            pub3Log = os.path.join(self.pPath, 'publisher3.log')

                            transSub1Log = os.path.join(self.pPath, 'trans_subscriber1.log')
                            transSub2Log = os.path.join(self.pPath, 'trans_subscriber2.log')
                            transSub3Log = os.path.join(self.pPath, 'trans_subscriber3.log')
                            persSub1Log = os.path.join(self.pPath, 'pers_subscriber1.log')
                            persSub2Log = os.path.join(self.pPath, 'pers_subscriber2.log')

                            try:
                                tmpDir = os.path.join(self.pPath, "tmp")
                                if os.path.isdir(tmpDir):
                                    shutil.rmtree (tmpDir)
                            except:
                                print "Failed to remove", tmpDir

                            with open ('examples.json') as data_file:
                                data = json.load(data_file)

                            pubName = data[self.expath][self.name]["executables"][lang]["pubName"]
                            subName = data[self.expath][self.name]["executables"][lang]["subName"]

                            sub_conds_file = data[self.expath][self.name]["log_conditions_file"][lang]["sub_conds"]
                            pub_conds_file = data[self.expath][self.name]["log_conditions_file"][lang]["pub_conds"]

                            sub_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', sub_conds_file)
                            pub_conds = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'yaml', pub_conds_file)

                            if pubName != "":
                                if self.classpath == "":
                                    pubExe = os.path.join(self.pPath, pubName) + exSfx
                                    if not os.path.isfile (pubExe):
                                        msg = "MissingExecutable: " + pubExe
                                else:
                                    pubExe = pubName

                            if subName != "":
                                if self.classpath == "":
                                    subExe = os.path.join(self.pPath, subName) + exSfx
                                    if not os.path.isfile (subExe):
                                        msg = "MissingExecutable: " + subExe
                                else:
                                    pubExe = subName

                            if msg == "NONE":
                                transSub1_Thread = ExeThread(self.classpath,  transSub1Log, lang, subExe, self.transSub_params, self.example_timeout * 2)
                                transSub2_Thread = ExeThread(self.classpath,  transSub2Log, lang, subExe, self.transSub_params, self.example_timeout * 2)
                                transSub3_Thread = ExeThread(self.classpath,  transSub3Log, lang, subExe, self.transSub_params, self.example_timeout * 2)

                                persSub1_Thread = ExeThread(self.classpath, persSub1Log, lang, subExe, self.persSub_params, self.example_timeout * 2)
                                persSub2_Thread = ExeThread(self.classpath, persSub2Log, lang, subExe, self.persSub_params, self.example_timeout * 2)

                                pub1Thread = ExeThread(self.classpath, pub1Log, lang, pubExe, self.pub1_params, self.example_timeout * 2)
                                pub2Thread = ExeThread(self.classpath, pub2Log, lang, pubExe, self.pub2_params, self.example_timeout * 2)
                                pub3Thread = ExeThread(self.classpath, pub3Log, lang, pubExe, self.pub3_params, self.example_timeout * 2)
                                os.chdir(self.pPath)

                                self.startOSPL()

                                transSub1_Thread.start()
                                pub1Thread.start()

                                pub1Thread.join(self.example_timeout)
                                transSub1_Thread.join(self.example_timeout)

                                self.stopOSPL()

                                self.startOSPL()
                                transSub2_Thread.start()
                                pub2Thread.start()

                                time.sleep(2)
                                transSub3_Thread.start()

                                pub2Thread.join(self.example_timeout)
                                transSub2_Thread.join(self.example_timeout)
                                transSub3_Thread.join(self.example_timeout)

                                self.stopOSPL()

                                self.startOSPL()

                                persSub1_Thread.start()
                                pub3Thread.start()
                                pub3Thread.join(self.example_timeout)
                                persSub1_Thread.join(self.example_timeout)

                                self.stopOSPL()

                                self.startOSPL()

                                persSub2_Thread.start()
                                persSub2_Thread.join(self.example_timeout)

                        except Exception:
                            msg = "Exception running " + str(sys.exc_info()[0])

                        try:
                            self.stopOSPL()
                        except Exception as ex:
                            print "Exception stopping OpenSplice ", str(ex)

                        if msg == "NONE":
                            try:
                                time.sleep (5)
                                super(durability, self).copyLogs()

                                if os.path.isfile (self.ospl_error_log):
                                    msg = "ospl-error.log found"

                                # the java versions of the Durability example do not write anything
                                # to the publisher output log - so only check the publisher log if
                                # the language is not a java language
                                if not "java" in lang:
                                    self.checkResults(pub1Log, pub_conds)
                                    self.checkResults(pub2Log, pub_conds)
                                    self.checkResults(pub3Log, pub_conds)

                                self.checkResults(transSub1Log, sub_conds)
                                self.checkResults(transSub2Log, sub_conds)
                                self.checkResults(transSub3Log, sub_conds)
                                self.checkResults(persSub1Log, sub_conds)
                                self.checkResults(persSub2Log, sub_conds)

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
                            self.writeResult (result,  self.expath + self.name, lang, msg)
                        except Exception as ex:
                            print "Exception writing result ", str(ex)

                        try:
                            self.cleanUp()
                        except Exception as ex:
                            print "Exception cleaning up ", str(ex)

                    except Exception as ex:
                        print "Unexpected exception", str(ex)
                    finally:
                        os.chdir(currPath)
