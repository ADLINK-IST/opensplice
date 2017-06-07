#!/usr/bin/python

import os
import sys
import json
from host import host 
from Durability import durability
from Example import Example
from Lifecycle import lifecycle
from Ownership import ownership
from PingPong import pingpong
from RoundTrip import roundtrip
from Tutorial import tutorial
from RMIHelloWorld import rmihelloworld
from ExampleLogger import examplelogger
from DBMSConnect import dbmsconnect
import pdb

"""
   Get the logger for this test run
"""
def getLogger(debug):
    return examplelogger(debug)

"""
   Get the host for this test run  
"""
def getHost():
    return host()

"""
   Run all the examples - called if -a provided at the command line - extype will be ""
"""
def runAllExamples(host, logger, extype):
    print "Running all examples"
    runExampleAll(host, logger, "BuiltInTopics", extype)
    runExampleAll(host, logger, "ContentFilteredTopic", extype)
    runExampleAll(host, logger, "HelloWorld", extype)
    runExampleAll(host, logger, "Lifecycle", extype)
    runExampleAll(host, logger, "Listener", extype)
    runExampleAll(host, logger, "Ownership", extype)
    runExampleAll(host, logger, "PingPong", extype)
    runExampleAll(host, logger, "QueryCondition", extype)
    runExampleAll(host, logger, "RoundTrip", extype)
    runExampleAll(host, logger, "Throughput", extype)
    runExampleAll(host, logger, "Tutorial", extype)
    runExampleAll(host, logger, "WaitSet", extype)
    runExampleAll(host, logger, "StreamsThroughput", extype)
    runExampleAll(host, logger, "FaceHelloWorld", extype)
    runExampleAll(host, logger, "protobuf", extype)

"""
   Run all versions of the specifed example e.g. standalone/corba, all languages
   as appropriate to the example
   called from runAllExamples when -a specified at the command line or
   called from main if -s specified at the command line with args other than example name 
   or all other args specified as "all"
"""
def runExampleAll(host, logger, example, extype):
    """
      Running the example expects that the OSPL_URI has been set - done via release.com/release.bat 
      or via scripts on overnight test runs
    """
    try:
        cur_uri = os.environ['OSPL_URI']
    except Exception:
        print "Exception obtaining OSPL_URI"
        raise Exception (str(sys.exc_info()[0]))

    expath = ""
    ex_uri = ""

    with open ('examples.json') as data_file:
        data = json.load(data_file)

    """
       Durability has a separate python script due to the complexity of
       the run i.e. it's not a single publisher/subscriber so set the 
       expath here
    """
    if example == "Durability":
        expath = "dcps"

    """
       Check for a config file other than the default
    """
    if expath != "":
        if os.environ['EXRUNTYPE'] == "shm":
            ex_uri = data[expath][example]["shm-uri"]
        else:
            ex_uri = data[expath][example]["sp-uri"]

    if ex_uri != "":
        os.environ["OSPL_URI"] = ex_uri

    """
       Get the example class - usually Example.py but may be non-standard
    """
    ex = getExample(host, logger, example)
    print "Got the example - ", example

    try:
        print "Running the " + example + " example on " + host.name

        if example == "DBMSconnect" and ex.runDBMSConnect:
            ex.runExample()
        elif ex != None:
            if extype == "":
                """
                  Run all types of this example e.g. standalone/corba, all languages
                """
                ex.runExampleAllTypes()         
            else:
                """
                  Run type of example specified e.g. standalone OR corba, all languages
                """
                ex.runExampleAll(extype)
                
    except Exception:
        print "Exception running " + str(sys.exc_info()[0])

    os.environ["OSPL_URI"] = cur_uri

"""
   Instantiate the appropriate class for the example being run
"""
def getExample(host, logger, example):

    print "Getting example for ", example

    if "RMI" in example:
        if "ClientServer" in example:
            ex = Example(host, logger, "ClientServer", "rmi")
        elif "HelloWorld" in example:
            ex = rmihelloworld(host, logger)
        else:
            ex = Example(host, logger, "Printer", "rmi")
    elif "Streams" in example:
        ex = Example(host, logger, "Throughput", "streams")
    elif "Face" in example:
        ex = Example(host, logger, "HelloWorld", "face")
    elif "DBMSconnect" in example:
        ex = dbmsconnect(host, logger)
    elif example == "PingPong":
        ex = pingpong(host, logger)
    elif example == "Lifecycle":
        ex = lifecycle(host, logger)
    elif example == "Ownership":
        ex = ownership(host, logger)
    elif example == "RoundTrip":
        ex = roundtrip(host, logger)
    elif example == "Tutorial":
        ex = tutorial(host, logger)
    elif example == "protobuf":
        ex = Example(host, logger, example, "")
    elif example == "Durability":
        ex = durability(host, logger)
    else:
        ex = Example(host, logger, example, "dcps")

    return ex

"""
   Called when a single example has been requested at the command line
   language can be c / cpp / cs / java / java5 / isocpp / isoccp2 or "all"
   extype can be standalone / corba / "all" or ""
   types - meant for things like PingPong s / q / f etc - not actually implemented
"""
def runExampleSingle(host, example, language, extype, types):

    ex = getExample(host, logger, example)
    print "Got the example"

    if example == "DBMSconnect" and ex.runDBMSConnect:
            ex.runExample()
    elif language == "all" and extype == "":
        ex.runExampleAllTypes()
    elif language == "all" and extype == "all":
        ex.runExampleAllTypes()        
    else:
        print "Running example ", example, extype, types
        if host.runExample(ex.expath, ex.name, language):
            ex.runExample(language, extype, types)

def usage():
    print "Usage :-"
    print "-a to run all examples"
    print ""
    print "-f <file name> use to provide a list of examples to run (NOT YET IMPLEMENTED)"
    print "      <file name> is name of file containing list of examples"
    print "       in format <example> <language> <type> [types]"
    print "         e.g. PingPong java standalone all" 
    print "         <example name> e.g. PingPong"
    print "         <language> e.g. c cpp java java5 isocpp isocpp2 or all to run all languages"
    print "         <type> e.g. standalone or corba where this exist"
    print "         <types> e.g. \"m\", \"s\" etc in PingPong example \"all\" (for all types) - can be blank"
    print " "
    print "-s to run a single example"
    print "       e.g. -s PingPong java standalone all"
    print "            -s PingPong all corba"
    print "            -s HelloWorld "
    print "            -s HelloWorld cpp standalone"
    print "            -s HelloWorld all"
    print "            -s HelloWorld all standalone"
    print "            -s DBMSConnect all"
    print "       e.g. -s RMIClientServer cpp "
    print "            -s StreamsThroughout cpp " 
    print "            -s FaceHelloWorld java"
    print " "
    print " If only the -s <Example Name> is specified then all versions of the example will be run"
    print "If you want to run with debug (to get extra output) then use -ad, -sd or -fd"
    exit()

if __name__ == "__main__":

    extype = ""
    run_all = 0
    run_single = 0
    debug = 0

    runArg = sys.argv[1]

    print "runArg is " + runArg

    if runArg == "-a":
        run_all = 1
    elif runArg == "-s":
        run_single = 1
    elif runArg == "-ad": 
        debug = 1
        run_all = 1
    elif runArg == "-sd":
        debug = 1
        run_single = 1
    elif runArg == "-h":
        usage()  
    else:
        usage()
              
    try:   
        host = getHost()
    except Exception:
        print "Unable to get host "
        raise Exception (str(sys.exc_info()[0]))

    logger = getLogger(debug)

    try:
        with open ('hosts.json') as data_file:
            data = json.load(data_file)

        print "Hostname is ", host.name.strip()
        types = data[host.name.strip()]["examples"] 

        if run_all:
            if types[0] == "All":
                runAllExamples(host, logger, "")
            elif types[0] == "AllStandalone":
                runAllExamples(host, logger, "standalone")            
            else:
                print "TO BE IMPLEMENTED..."

        else:
            print "The number of args is " + str(len(sys.argv))
            if str(len(sys.argv)) < 3:
                print "If not running all using -a you must provide -s <Example name> as a minimum"
                usage()

            if runArg == "-f":
                print "NOT YET IMPLEMENTED"
                #print "The file name is " + sys.argv[2]
            elif run_single:

                if types[0] == "AllStandalone":
                    extype = "standalone"
                else:
                    extype = "all"

                types = "all"
                language = "all"

                print "The example is " + sys.argv[2]
                example = sys.argv[2]

                if len(sys.argv) > 3:
                    print "The language is " + sys.argv[3]
                    language = sys.argv[3]

                    if len(sys.argv) > 4:
                        print "The type is " + sys.argv[4]
                        extype = sys.argv[4]

                        if len(sys.argv) == 6:
                            print "The types is " + sys.argv[5]
                            types = sys.argv[5]                
                print "Calling runExampleSingle with ", host.name, example, language, extype, types
                runExampleSingle(host, example, language, extype, types)

                sys.stdout.flush()

    except Exception:
        print "Exception running examples ", str(sys.exc_info()[0])
        sys.stdout.flush()
    finally:
        logger.finalizeResults()

