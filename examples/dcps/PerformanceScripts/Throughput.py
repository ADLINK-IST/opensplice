import utils, sys, csv, subprocess, time, optparse, os, pdb

def kill(pid):
    try:
        if not pid.poll():
            pid.terminate()
    except OSError:
        pass

def pubsub(o):

    texec = []
    apiselect=0
    publisher = 'publisher'
    subscriber = 'subscriber'
    if sys.platform == 'win32':
        publisher = 'publisher.exe'
        subscriber = 'subscriber.exe'
    #if o.capi:
    #    #C
    #    texec.append([])
    #    texec[apiselect].append(os.environ['OSPL_HOME'] + '/examples/dcps/Throughput/c/' + publisher)
    #    texec[apiselect].append(os.environ['OSPL_HOME'] + '/examples/dcps/Throughput/c/' + subscriber)
    #    texec[apiselect].append('C')
    #    apiselect+=1

    if o.cppapi:
        #SACPP
        texec.append([])
        texec[apiselect].append(os.environ['OSPL_HOME'] + '/examples/dcps/Throughput/cpp/' + publisher)
        texec[apiselect].append(os.environ['OSPL_HOME'] + '/examples/dcps/Throughput/cpp/' + subscriber)
        texec[apiselect].append('SACPP')
        apiselect+=1

    if o.isoapi:
        #ISOCPP
        texec.append([])
        texec[apiselect].append(os.environ['OSPL_HOME'] + '/examples/dcps/Throughput/isocpp/' + publisher)
        texec[apiselect].append(os.environ['OSPL_HOME'] + '/examples/dcps/Throughput/isocpp/' + subscriber)
        texec[apiselect].append('ISOCPP')
        apiselect+=1

        tafcsv = utils.getCSV(o.averagesfile)
        results = utils.tree()
        for i in texec:
            #Open per exec csv
            csvfile = i[0] + ".csv"
            cw = utils.getCSV(csvfile)
            cw.writerow([str(time.strftime("%x %H:%M:%S"))])

            resultsApi = results[i[2]]
            Bsize = 0
            while Bsize <= o.maxpayload:
                resultsBsize = resultsApi[Bsize]
                try:
                    if o.subonly:
                        print ("launching Subscriber " +
                                i[1] + " " +
                                str(o.maxcycles) + " " +
                                str(o.pollingdelay)+ " " +
                                str(o.subpartition))
                        subscriber = subprocess.Popen([i[1],
                            str(o.maxcycles),
                            str(o.pollingdelay),
                            str(o.subpartition)],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
                        utils.setPriority(subscriber.pid, o.subnice, o.subaffinity)
                    try:
                        if o.pubonly:
                            print ("launching publisher " +
                                i[0] + " " +
                                str((Bsize*1024)) + " " +
                                str(o.burstI) + " " +
                                str(o.burstS) + " " +
                                str(o.timeout) + " " +
                                str(o.pubpartition))

                            publisher = subprocess.Popen([i[0],
                                str(Bsize*1024),
                                str(o.burstI),
                                str(o.burstS),
                                str(o.timeout),
                                str(o.pubpartition)],
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE)
                            utils.setPriority(publisher.pid, o.pubnice, o.pubaffinity)
                        if not o.pubonly:
                            subscriber.wait()
                        elif not o.subonly:
                            publisher.wait()
                        elif o.subonly and o.pubonly:
                            #Waiting for a return on windows is broken sometimes for some reason
                            if sys.platform == 'win32':
                                time.sleep(12)
                            else:
                                subscriber.wait()

                            for line in subscriber.stdout:
                                sys.stdout.write(line)
                                utils.parseTP(line, resultsBsize)


                            if resultsBsize['Average Transfer']['Mbit']:
                                if o.pubonly:
                                    kill(publisher)
                                if o.subonly:
                                    kill(subscriber)

                        for line in zip(publisher.stderr if o.pubonly else [],subscriber.stderr if o.subonly else []):
                            print 'Publisher err: ' + line[0]
                            print 'Subscriber err: ' + line[1]

                        if o.subonly:
                            cw.writerow([str(Bsize)+'KiB'])
                            cw.writerow(['Count'] + ['Payload Size'] +
                                        ['Received Samples'] + ['Received Bytes'] +
                                        ['Out of order'] + ['Sample Rate'] +
                                        ['Transfer Rate'])

                            for key in sorted(resultsBsize):
                                if str(key).isdigit():
                                    k = resultsBsize[key]
                                    cw.writerow([str(key)] + [k['Payload Size']] + [k['Received Samples']] +
                                    [k['Received Bytes']] + [k['Out of order']] +
                                    [k['Sample Rate']] + [k['Transfer Rate']])

                        #utils.jsonPrint(resultsBsize)

                        if o.pubonly:
                            kill(publisher)
                        if o.subonly:
                            kill(subscriber)

                    except OSError as detail:
                        print "Cannot find publisher executable: " + i[0]
                        print detail
                        sys.exit(1)

                except OSError as detail:
                    print "Cannot find subscriber executable: " + i[0]
                    print detail
                    sys.exit(1)

                Bsize += 1

    if o.subonly:
        ''' Create or append to total averages file '''
        tafcsv = utils.getCSV(o.averagesfile)
        tafcsv.writerow([str(time.strftime("%x %H:%M:%S"))])
        tafcsv.writerow(['Payload KiB']
        #+ ['C Total Samples'] + ['C Total Bytes'] + ['C Total out of order'] + ['C Average Samples'] + ['C Average Mbit']
        + ['SACPP Total Samples'] + ['SACPP Total Bytes'] + ['SACPP Total out of order'] + ['SACPP Average Samples'] + ['SACPP Average Mbit']
        + ['ISOCPP Total Samples'] + ['ISOCPP Total Bytes'] + ['ISOCPP Total out of order'] + ['ISOCPP Average Samples'] + ['ISOCPP Average Mbit']
        )

        #Grab existing API
        apis = results.keys()
        api = apis[0]
        for size in sorted(results[api]):
            tafcsv.writerow([size]
                #+ utils.is_empty(results['C'][size]['Total']['Samples'])
                #+ utils.is_empty(results['C'][size]['Total']['Bytes'])
                #+ utils.is_empty(results['C'][size]['Total out of order'])
                #+ utils.is_empty(results['C'][size]['Average Transfer']['Samples'])
                #+ utils.is_empty(results['C'][size]['Average Transfer']['Mbit'])

                + utils.is_empty(results['SACPP'][size]['Total']['Samples'])
                + utils.is_empty(results['SACPP'][size]['Total']['Bytes'])
                + utils.is_empty(results['SACPP'][size]['Total out of order'])
                + utils.is_empty(results['SACPP'][size]['Average Transfer']['Samples'])
                + utils.is_empty(results['SACPP'][size]['Average Transfer']['Mbit'])

                + utils.is_empty(results['ISOCPP'][size]['Total']['Samples'])
                + utils.is_empty(results['ISOCPP'][size]['Total']['Bytes'])
                + utils.is_empty(results['ISOCPP'][size]['Total out of order'])
                + utils.is_empty(results['ISOCPP'][size]['Average Transfer']['Samples'])
                + utils.is_empty(results['ISOCPP'][size]['Average Transfer']['Mbit'])
                )






def main():
    parser = optparse.OptionParser()

    #parser.add_option("-C", "--capi", dest="capi",
    #                    help="Run C API Throughput",
    #                    action="store_true",
    #                    default=False)

    parser.add_option("-S", "--sacppapi", dest="cppapi",
                        help="Run SACPP API Throughput",
                        action="store_true",
                        default=False)

    parser.add_option("-I", "--isocppapi", dest="isoapi",
                        help="Run ISOCPP API Throughput",
                        action="store_true",
                        default=False)

    parser.add_option("-o", "--output", dest="averagesfile",
                        help=("Optional path and filename for a overall average payload and API size by"
                        "default this is stored in the current working directory"),
                        default="TPAverages.csv")

    parser.add_option("", "--pubonly", dest="pubonly",
                        help="Only create the publisher",
                        action="store_true",
                        default=False)

    parser.add_option("", "--subonly", dest="subonly",
                        help="Only create the subscriber",
                        action="store_true",
                        default=False)

    pubopt = optparse.OptionGroup(parser, "Publisher options",
                            "Change arguments for publisher, payload size, burst interval, burst size, timeout and partition name")

    pubopt.add_option("", "--burstinterval", type="int", dest="burstI",
                        help="The interval between each burst in ms",
                        default=10)

    pubopt.add_option("", "--burstsize", type="int", dest="burstS",
                        help="The number of samples to send each burst",
                        default=1)

    pubopt.add_option("", "--maxpayload", type="int", dest="maxpayload",
                        help="The max payload in KiB, the default is 8",
                        default=8)

    pubopt.add_option("", "--timeout", type="int", dest="timeout",
                        help="The number of seconds the publisher should run for",
                        default=0)

    pubopt.add_option("", "--pubpartition", type="str", dest="pubpartition",
                        help="Partition name",
                        default="Throughputexample")

    parser.add_option_group(pubopt)

    subopt = optparse.OptionGroup(parser, "Subscriber options",
                            "Change arguments for subscriber, max cycles, polling delay and partition name")

    subopt.add_option("", "--maxcycles", type="int", dest="maxcycles",
                        help="The number of times to output statistics before terminating",
                        default=10)

    subopt.add_option("", "--pollingdelay", type="int", dest="pollingdelay",
                        help="The number of ms to wait between reads, 0 = event based",
                        default=0)

    subopt.add_option("", "--subpartition", type="str", dest="subpartition",
                        help="Partition name",
                        default="Throughputexample")
    parser.add_option_group(subopt)

    cpuopt = optparse.OptionGroup(parser, "CPU and priority options",
                            "Allow the setting of NICE and CPU affinity")

    cpuopt.add_option("", "--pubaffinity", type="int", dest="pubaffinity",
                        help="Set the CPU affinity for the ping process, the default is cpu 1",
                        default=1)

    cpuopt.add_option("", "--subaffinity", type="int", dest="subaffinity",
                        help="Set the CPU affinity for the pong process, the default is cpu 0",
                        default=0)

    cpuopt.add_option("", "--pubnice", type="int", dest="pubnice",
                        help="Set the nice value for the ping process, the default is -20. NOTE: This option is available on Linux only, Windows will run under REALTIME_PRIORITY_CLASS",
                        default=-20)

    cpuopt.add_option("", "--subnice", type="int", dest="subnice",
                        help="Set the nice value for the pong process, the default is -20. NOTE: This option is available on Linux only, Windows will run under REALTIME_PRIORITY_CLASS",
                        default=-20)

    parser.add_option_group(cpuopt)

    (options, args) = parser.parse_args()

    #Nothing was set, run them all
    if(not options.cppapi and not options.isoapi):
        #options.capi = True
        options.cppapi = True
        options.isoapi = True

    #Nothing was set, run them both
    if not options.pubonly and not options.subonly:
        options.pubonly = True
        options.subonly = True


    pubsub(options)

if __name__ == "__main__":
    main()
