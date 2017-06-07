import utils, os, sys, csv, subprocess, time, optparse, pdb

def pingpong(o):

    texec = []

    ping = 'ping'
    pong = 'pong'
    if sys.platform == 'win32':
        ping = 'ping.exe'
        pong = 'pong.exe'

    apiselect=0
    if o.capi:
        #C
        texec.append([])
        texec[apiselect].append(os.environ['OSPL_HOME'] + '/examples/dcps/RoundTrip/c/' + ping)
        texec[apiselect].append(os.environ['OSPL_HOME'] + '/examples/dcps/RoundTrip/c/' + pong)
        texec[apiselect].append('C')
        apiselect+=1

    if o.cppapi:
        #SACPP
        texec.append([])
        texec[apiselect].append(os.environ['OSPL_HOME'] + '/examples/dcps/RoundTrip/cpp/' + ping)
        texec[apiselect].append(os.environ['OSPL_HOME'] + '/examples/dcps/RoundTrip/cpp/' + pong)
        texec[apiselect].append('SACPP')
        apiselect+=1

    if o.isoapi:
        #ISOCPP
        texec.append([])
        texec[apiselect].append(os.environ['OSPL_HOME'] + '/examples/dcps/RoundTrip/isocpp/' + ping)
        texec[apiselect].append(os.environ['OSPL_HOME'] + '/examples/dcps/RoundTrip/isocpp/' + pong)
        texec[apiselect].append('ISOCPP')
        apiselect+=1


    ''' Create or append to total averages file '''
    tafcsv = utils.getCSV(o.averagesfile)

    #Create nested dictionary
    results = utils.tree()

    for i in texec:
        resultsApi = results[i[2]]
        #1KB
        Bsize = 1000

        try:
            if o.pongonly:
                pong = subprocess.Popen([i[1]],stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                utils.setPriority(pong.pid, o.pongnice, o.pongaffinity)

            if o.pongonly and not o.pingonly:
                #Run for 10 minutes and exit program
                time.sleep(600)
                sys.exit(0)
            time.sleep(1)
            ''' Set the CSV output file (af) '''
            csvfile = i[0] + ".csv"
            cw = utils.getCSV(csvfile)

            cw.writerow([str(time.strftime("%x %H:%M:%S"))])
            try:
                while(Bsize <= (o.maxpayload * 1000)):
                    resultsBsize = resultsApi[int(Bsize)]
                    print "launching " + i[0] + "with args:" + str(Bsize) + " " + str(o.samples) + " " + str(o.seconds)
                    cw.writerow([str(Bsize/1000)+"KB"])
                    cw.writerow(['Seconds'] + ['RT Count'] + ['RT median'] + ['RT min'] +
                                ['W Count'] + ['W median'] + ['W min'] +
                                ['R Count'] + ['R mean'] + ['R min']);
                    try:
                        if o.pingonly:
                            ping = subprocess.Popen( [i[0], str(Bsize), str(o.samples), str(o.seconds) ],
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE)
                            utils.setPriority(ping.pid, o.pingnice, o.pingaffinity)
                    except OSError:
                        print "Cannot find ping executable: " + str([i[0]])

                    #Wait for ping to terminate
                    ping.wait()
                    for line in ping.stderr:
                        print 'err: ' + line

                    for line in ping.stdout:
                        utils.parseRT(line,resultsBsize)

                    for key in sorted(resultsBsize):
                        k = resultsBsize[key]
                        cw.writerow([key] +
                        [k['RoundTrip']['Count']] + [k['RoundTrip']['Median']] + [k['RoundTrip']['Min']] +
                        [k['Read']['Count']] + [k['Read']['Median']] + [k['Read']['Min']] +
                        [k['Write']['Count']] + [k['Write']['Median']] + [k['Write']['Min']])

                    Bsize = Bsize*2
            except OSError:
                print "Cannot find ping executable: " + [i[0]]

            finally:
                if o.pongonly:
                    #Quit pong
                    pingq = subprocess.Popen( [i[0], 'quit' ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                    pingq.wait()
                    for line in zip(pingq.stdout, pingq.stderr):
                        print line
                    pong.terminate()

        except OSError:
            print "Cannot find pong executable: " + str([i[1]])



    tafcsv.writerow([str(time.strftime("%x %H:%M:%S"))])
    tafcsv.writerow(['Payload KB'] + ['RoundTrip C'] + ['RoundTrip SACPP'] + ['RoundTip ISOCPP']
                    + ['Read C'] + ['Read SACPP'] + ['Read ISOCPP']
                    + ['Write C'] + ['Write SACPP'] + ['Write ISOCPP'])
    Bsize = 1000
    while Bsize <= (o.maxpayload * 1000):
        KB = Bsize/1000
        #pdb.set_trace()
        tafcsv.writerow([KB] + utils.is_empty(results['C'][Bsize]['Overall']['RoundTrip']['Median'])
                            + utils.is_empty(results['SACPP'][Bsize]['Overall']['RoundTrip']['Median'])
                            + utils.is_empty(results['ISOCPP'][Bsize]['Overall']['RoundTrip']['Median'])
                            + utils.is_empty(results['C'][Bsize]['Overall']['Read']['Median'])
                            + utils.is_empty(results['SACPP'][Bsize]['Overall']['Read']['Median'])
                            + utils.is_empty(results['ISOCPP'][Bsize]['Overall']['Read']['Median'])
                            + utils.is_empty(results['C'][Bsize]['Overall']['Write']['Median'])
                            + utils.is_empty(results['SACPP'][Bsize]['Overall']['Write']['Median'])
                            + utils.is_empty(results['ISOCPP'][Bsize]['Overall']['Write']['Median']))

        Bsize = Bsize*2

def main():
    parser = optparse.OptionParser()

    parser.add_option("-C", "--capi", dest="capi",
                        help="Run C API Roundtrip",
                        action="store_true",
                        default=False)

    parser.add_option("-S", "--sacppapi", dest="cppapi",
                        help="Run SACPP API Roundtrip",
                        action="store_true",
                        default=False)

    parser.add_option("-I", "--isocppapi", dest="isoapi",
                        help="Run ISOCPP API Roundtrip",
                        action="store_true",
                        default=False)

    parser.add_option("-o", "--output", dest="averagesfile",
                        help=("Optional path and filename for a overall average payload and API size by"
                        "default this is stored in the current working directory"),
                        default="averages.csv")

    parser.add_option("", "--pingonly", dest="pingonly",
                        help="Only create the ping daemon",
                        action="store_true",
                        default=False)

    parser.add_option("", "--pongonly", dest="pongonly",
                        help="Only create the pong daemon",
                        action="store_true",
                        default=False)

    pingopt = optparse.OptionGroup(parser, "Ping options",
                            "Change arguments for ping, run time in seconds, number of samples and maxpayload")

    pingopt.add_option("", "--seconds", type="int", dest="seconds",
                        help="The number of seconds ping should execute for, the default is 10",
                        default=10)

    pingopt.add_option("", "--samples", type="int", dest="samples",
                        help="The number of samples ping should send, the default is infinite",
                        default=0)

    pingopt.add_option("", "--maxpayload", type="int", dest="maxpayload",
                        help="The max payload in kB, the default is 64",
                        default=64)
    parser.add_option_group(pingopt)

    cpuopt = optparse.OptionGroup(parser, "CPU and priority options",
                            "Allow the setting of NICE and CPU affinity")

    cpuopt.add_option("", "--pingaffinity", type="int", dest="pingaffinity",
                        help="Set the CPU affinity for the ping process, the default is cpu 1",
                        default=1)

    cpuopt.add_option("", "--pongaffinity", type="int", dest="pongaffinity",
                        help="Set the CPU affinity for the pong process, the default is cpu 0",
                        default=0)

    cpuopt.add_option("", "--pingnice", type="int", dest="pingnice",
                        help="Set the nice value for the ping process, the default is -20. NOTE: This option is available on Linux only, Windows will run under REALTIME_PRIORITY_CLASS",
                        default=-20)

    cpuopt.add_option("", "--pongnice", type="int", dest="pongnice",
                        help="Set the nice value for the pong process, the default is -20. NOTE: This option is available on Linux only, Windows will run under REALTIME_PRIORITY_CLASS",
                        default=-20)

    parser.add_option_group(cpuopt)

    (options, args) = parser.parse_args()

    if(not options.capi and not options.cppapi and not options.isoapi):
        #Nothing was set, run them all
        options.capi = True
        options.cppapi = True
        options.isoapi = True

    if not options.pingonly and not options.pongonly:
        #Ping and pong (default)
        options.pingonly = True
        options.pongonly = True


    pingpong(options)

if __name__ == "__main__":
    main()
