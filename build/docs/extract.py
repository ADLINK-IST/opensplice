#!/usr/bin/python

import sys
import re

all_features = ['OSPL']

ignore_regex = ['<\?xml .*\?>',
                '<splice_meta_config version=.*>',
                '<!--xmlns:xsi=.*-->',
                '<serviceMapping>',
                '<element name=.* command=.*>',
                '</serviceMapping>',
                '</splice_meta_config>' ]

class ReMatcher(object):
    def __init__(self, patterns):
        self._patterns = []
        for p in patterns:
            self._patterns.append(re.compile(p, re.IGNORECASE))

    def match(self, string):
        for p in self._patterns:
            if bool(p.match(string)):
                return True
        return False

def extract (in_file, out_file, version, service, features):
  matcher = ReMatcher(ignore_regex)
  state = []
  in_scope = True
  cmd_line = False
  fr = open (in_file, 'r')
  fw = open (out_file, 'w')
  fw.write ("<dummyElement>\n")
  for line in fr:
    ignore_line = matcher.match(line.strip());

    if not ignore_line:
      line = line.replace ('VERSION', version)
      line = line.replace ('LC_SNAME', service.lower ())
      line = line.replace ('SNAME', service)
      fw.write (line),

  fw.write ("</dummyElement>")          
  fr.close ()
  fw.close ()

def usage ():
  sys.stdout.write ('usage: extract.py meta_config_file (osplconf)')
  for f in all_features: sys.stdout.write (' [' + f + ']')
  sys.stdout.write ('\n')
  sys.exit (1)

def main (argv):
  features = ['OSPL']
  version = 'COMMERCIAL'
  service = 'DDSI2E'
  try:
    meta = argv[0]
    config = argv[1]
  except: usage ()
  extract (meta, config + '.xml', version, service, features)

if __name__ == "__main__":
   main (sys.argv[1:])
