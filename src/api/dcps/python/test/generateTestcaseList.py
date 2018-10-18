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
import re
import countTest

def package_contents():
    files = os.listdir()
    # Use a set because some may be both source and compiled.
    return set([os.path.splitext(module)[0]
        for module in files
        if re.match(r'[tT]est.*\.py', module)])

def print_all_testcases(testcaseList):
    try:
        root = os.getenv('OSPL_OUTER_HOME_NORMALIZED')
        file = os.path.join(root,"testsuite","dbt","api","dcps","python","bin","testcases")
        etcf = open(file, 'w')
        for testcase in testcaseList:
            etcf.write ("python_{}\n".format(testcase.replace(".","_")))
    finally:
        etcf.close()


def all_pyUnit_tests():
    testcaseList = []
    testmodules = package_contents()
    for t in testmodules:
        # load all the test cases from the module.
        for test in unittest.defaultTestLoader.loadTestsFromName(t):
            for testcase in test:
                testcaseList.append(testcase.id())

    print_all_testcases(testcaseList)

if __name__ == '__main__':
    countTest.count_test = True
    all_pyUnit_tests()
    countTest.count_test = False
