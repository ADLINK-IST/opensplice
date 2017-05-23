import csv, sys, os, re, pdb, json, psutil
from collections import defaultdict
def tree():
    return defaultdict(tree)

def jsonPrint(dic):
    print json.dumps(dic)

def is_empty(dic):
    if dic:
        return [dic]
    else:
        return ['0']

def getCSV(filename):
    filehandle=0
    csvhandle = csv.writer
    if not os.path.exists(filename):
        filehandle = open(filename, 'wb')
        csvhandle = csv.writer(filehandle,quoting=csv.QUOTE_ALL)
    else:
        filehandle = open(filename, 'ab')
        csvhandle = csv.writer(filehandle,quoting=csv.QUOTE_ALL)
        csvhandle.writerow(['']);
    return csvhandle

def setPriority(pid, nice, affinity):
    try:
        pup = psutil.Process(pid)
        if sys.platform == 'win32':
            if nice != 0:
                pup.set_nice(psutil.REALTIME_PRIORITY_CLASS)
        else:
             pup.set_nice(nice)
        pup.set_cpu_affinity([affinity])
    except psutil._error.AccessDenied:
        print "You dont have the required privileges to change the priority of the proccess, Try running as root"
        sys.exit(1)

def parseTP(line, nested):
    if not re.match('^Waiting', line):
        split = re.split('\s+', line)
        if re.match('^Payload', line):
            if nested:
                node = nested[int(max(nested)+1)]
            else:
                node = nested[0]
            node['Payload Size'] = int(split[2])
            node['Received Samples'] = int(split[6])
            node['Received Bytes'] = int(split[8])
            node['Out of order'] = int(split[14])
            node['Sample Rate'] = float(split[19])
            node['Transfer Rate'] = float(split[21])
        elif re.match('^Total', line):
            nested['Total']['Samples'] = float(split[2])
            nested['Total']['Bytes'] = float(split[4])
        elif re.match('^Out', line):
            nested['Total Out of order'] = int(split[3])
        elif re.match('^Average', line):
            nested['Average Transfer']['Samples'] = float(split[3])
            nested['Average Transfer']['Mbit'] = float(split[5])

        return nested

def parseRT(line, nested):
    #pdb.set_trace()
    if not re.match('^# [WRSp\s]', line) and not re.match('^\s*$', line):
        split = re.split('\s+', line)
        typekeys=['RoundTrip', 'Read', 'Write']
        splitoffset=2

        try:
            for typekey in typekeys:
                if re.match('^# Overall', line):
                    node = nested['Overall'][typekey]
                else:
                    node = nested[int(split[1])][typekey]

                node['Count'] = int(split[splitoffset])
                node['Median'] = int(split[splitoffset+1])
                node['Min'] =  int(split[splitoffset+2])
                splitoffset += 3
        except ValueError as detail:
            print "Parse error with line:"
            print line
            print detail

        return nested
