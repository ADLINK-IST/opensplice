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
'''
Created on Dec 17, 2017

@author: prismtech
'''
import unittest
import struct
import countTest

if countTest.count_test == False:
    from StringTypes.basic.module_StringTypes import StringTypes_struct

class TestStringTypes(unittest.TestCase):


    def testCopyInCopyOut(self):
        data = StringTypes_struct(
            long1 = 32,
            string1='Hello World!',
            string2='Five5',
            string_array=['Alpha','Bravo','Delta'],
            string_seq1=['Fee','Fi','Foe','Fum'],
            string_seq2=['Echo','Delta', 'Espsi', 'Foxtr'])
        
        print('data: ' + str(data))
        
        print('data._get_packing_fmt(): ', data._get_packing_fmt())
        buffer = data._serialize()
        print('buffer: ', buffer)
        
        values = struct.unpack(data._get_packing_fmt(), buffer)
        data1 = StringTypes_struct()
        data1._deserialize(list(values))
        
        for attr in ('string1','string2','string_array','string_seq1','string_seq2'):
            self.assertEqual(getattr(data,attr),getattr(data1,attr),attr)


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'TestStringTypes.testCopyInCopyOut']
    unittest.main()