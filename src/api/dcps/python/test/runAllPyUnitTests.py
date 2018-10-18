#
#                         Vortex OpenSplice
#
#   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
#   Technology Limited, its affiliated companies and licensors. All rights
#   reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
import unittest
import os
import sys
import re
import subprocess

def package_contents():
    files = os.listdir()
    # Use a set because some may be both source and compiled.
    print(files)
    return set([os.path.splitext(module)[0]
        for module in files
        if re.match(r'[tT]est.*\.py', module)])

def compile_idl():
    idl_dir = 'idl'
    idlfiles = [os.path.join(idl_dir,f) for f in os.listdir(path=idl_dir) if os.path.isfile(os.path.join(idl_dir,f)) and f.endswith('.idl')]
    idlfiles.append(os.getenv('OSPL_HOME_NORMALIZED') + '/etc/idl/dds_IoTData.idl')
    for f in idlfiles:
        print('Compiling %s' % f)
        retcode = subprocess.call(['idlpp','-l','python', f])
        print('  returned ', retcode)

def all_tests():
    suite = unittest.TestSuite()
    testmodules = package_contents()
    print(testmodules)
    for t in testmodules:
        try:
            # If the module defines a suite() function, call it to get the suite.
            mod = __import__(t, globals(), locals(), ['suite'])
            suitefn = getattr(mod, 'suite')
            suite.addTest(suitefn())
        except (ImportError, AttributeError):
            # else, just load all the test cases from the module.
            suite.addTest(unittest.defaultTestLoader.loadTestsFromName(t))
    if dbt_testrun:
        xmlrunner.XMLTestRunner(output='test-reports').run(suite)
    else:
        unittest.TextTestRunner().run(suite)

if __name__ == '__main__':
    dbt_testrun = False
    if '--dbt' in sys.argv:
        import xmlrunner
        dbt_testrun = True
    compile_idl()
    all_tests()
