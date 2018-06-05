import json
import os
import subprocess
import platform

"""
  Class holding details of the host on which the example is running
"""
class host(object):
    
    def __init__(self):
        """
           Get the name of the host - not always set the same when
           running overnight
        """
        try:
            self.name = os.environ['HOSTNAME'].lower()
        except Exception:
            try:
                self.name = platform.node().lower()
            except Exception:
                self.name = subprocess.check_output("hostname").strip().lower()

        with open ('hosts.json') as data_file:
            data = json.load(data_file)

        self.excludedLangs = data[self.name]["excluded_langs"]

        self.excludedExamples = data[self.name]["excluded_examples"]

        self.partialExamples = data[self.name]["partial_examples"]

    def isWindows(self):
        return "win" in self.name

    """
       Check whether the example is to run on this host.  If so check that the example is
       to run in the specified language
    """    
    def runExample(self, xpath, example, lang):
        runExample = True
    
        """
           To cater for dcpsHelloWorld, rmiHelloWorld, faceHelloWorld etc we need to 
           include the extra bit e.g. dcps/rmi/face to avoid the json file becoming too
           complicated
        """
        if xpath == "protobuf":
            exkey = example
        elif xpath == "services":
            exkey = example
        else:
            exkey = xpath + example

        """
          If the entire example is to be included it will be in the excludedExamples list
        """
        for e in self.excludedExamples:
            if e == exkey:
                runExample = False

        """
          If a language is excluded for all examples it will be in the excludedLangs list        
        """
        if runExample and lang != "":
            for l in self.excludedLangs:
                if l == lang:
                    runExample = False       

        """
          If an example can run in some but not all languages it will be in the partialExamples list        
        """
        if runExample and lang != "":
            for p in self.partialExamples:

                if p == exkey:
                    
                    with open ('hosts.json') as data_file:
                        data = json.load(data_file)
                    
                    langs = data[self.name]["partial_examples"][exkey]["langs"]
                    for l in langs:

                        if l == lang:
                            runExample = False       

        if runExample == False:
            print("Not running " + exkey + ":" + lang + " on " +self.name)

        return runExample

if __name__ == "__main__":
    
    me = host()
    if me.isWindows() == True:
        print("It's windows")
    else:
        print("It's not windows")
