import sys
import time

try:
    import psutil
    hasPsutil = True
except ImportError:
    hasPsutil = False
    

def splicedCheck (mode):

    splicedFound = True
    
    if hasPsutil == True:
        if mode == "start":
            splicedFound = False
            count = 0
    
            while splicedFound == False and count < 10:

                time.sleep(1)
                for proc in psutil.process_iter():
                    if  proc.name() == u"spliced.exe":
                        splicedFound = True

                if splicedFound == False:
                    count += 1
                     
        elif mode == "stop":

            count = 0
            splicedFound = True
            while splicedFound == True and count < 10:

                time.sleep(1)
                splicedFound = False            
                for proc in psutil.process_iter():
                    if  proc.name() == u"spliced.exe":
                        splicedFound = True
                        count += 1
                        break
              
        else:
            raise Exception("Invalid mode for splicedCheck..")
        
    return splicedFound
