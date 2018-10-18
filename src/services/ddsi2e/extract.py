#!/usr/bin/python

import sys

all_features = ['NETWORK_CHANNELS','BANDWIDTH_LIMITING','NETWORK_PARTITIONS','ENCRYPTION','SSL']

def extract (in_file, out_file, version, service, features):
  state = []
  in_scope = True
  cmd_line = False
  fr = open (in_file, 'r')
  fw = open (out_file, 'w')
  for line in fr:
    if line[0:5] == 'START':
      state.append (in_scope)
      in_scope = in_scope and line[6:len(line)-1] in features
      cmd_line = True
    elif line[0:3] == 'END':
      in_scope = state.pop ()
      cmd_line = True
    else:
      cmd_line = False
    if in_scope and not cmd_line:
      line = line.replace ('VERSION', version)
      line = line.replace ('LC_SNAME', service.lower ())
      line = line.replace ('SNAME', service)
      fw.write (line),
  fr.close ()
  fw.close ()

def usage ():
  sys.stdout.write ('usage: extract.py meta_config_file (lite | ddsi2 | ddsi2e)')
  for f in all_features: sys.stdout.write (' [' + f + ']')
  sys.stdout.write ('\n')
  sys.exit (1)

def main (argv):
  features = []
  version = 'COMMERCIAL'
  service = 'DDSI2E'
  try:
    meta = argv[0]
    config = argv[1]
  except: usage ()
  if config == 'ddsi2': 
    version = 'COMMUNITY'
    service = 'DDSI2'
    features.append ('OSPL')
  elif config == 'ddsi2e':
    features.append ('OSPL')
  elif config == 'lite':
    version = 'LITE'
    features.append ('LITE')
  else: usage ()
  for arg in argv[2:]:
    if all_features.count (arg) == 0:
      print 'Unsupported feature: ' + arg
      usage ()
    else: features.append (arg)
  extract (meta, config + '.xml', version, service, features)

if __name__ == "__main__":
   main (sys.argv[1:])
