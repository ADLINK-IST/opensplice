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
Created on Jan 8, 2018

@author: prismtech
'''
import os
import unittest
import ddsutil
import collections

Info = collections.namedtuple('Info', ['field','def_val'])

class TestDynamicDefaults(unittest.TestCase):

    def get_idl(self, name, *args):
        dds_name = "basic::module_" + name + "::" + name + "_struct"
        gci = ddsutil.get_dds_classes_from_idl(os.path.join('idl',name + '.idl'), dds_name)
        names = [ name ]
        names.extend(args)
        dict = {}
        for n in names:
            dds_name = "basic::module_" + name + "::" + n
            if n == name:
                dds_name += '_struct'
            dict[n] = gci.get_class(dds_name)
        return dict

    def check_defaults(self, obj, field_info):
        for info in field_info:
            f = getattr(obj, info.field)
            self.assertIsInstance(f, type(info.def_val), 'Field {} is not expected type'.format(info.field))
            self.assertEqual(f, info.def_val, 'Field {} not expected value'.format(info.field))

    def testSimpleTypes(self):
        types = self.get_idl('SimpleTypes')
        data = types['SimpleTypes']()
        self.check_defaults(data, [
            Info('long1', 0),
            Info('ulong1', 0),
            Info('longlong1', 0),
            Info('ulonglong1', 0),
            Info('float1', 0.0),
            Info('short1', 0),
            Info('ushort1', 0),
            Info('char1', '\x00'),
            Info('octet1', 0),
            Info('double1', 0.0),
            Info('bool1', 0),
            Info('string1', ''),
            ])

    def testArray(self):
        types = self.get_idl('Array')
        data = types['Array']()
        
        self.check_defaults(data, [
            Info('long1', 0),
            Info('array1', [0.0, 0.0, 0.0]),
            ])

    def testArrayOfStruct(self):
        types = self.get_idl('ArrayOfStruct', 'Inner')
        data = types['ArrayOfStruct']()
        Inner = types['Inner']
        
        self.check_defaults(data, [
            Info('long1', 0),
            Info('mylong1', 0),    
            ])
        for i in range(2):
            self.assertIsInstance(data.array1[i], Inner, 'array1[{}] is not an Inner')
            self.check_defaults(data.array1[i], [
                Info('long1', 0),
                Info('double1', 0.0),
                ])

    def testArrayOfStructSequence(self):
        types = self.get_idl('ArrayOfStructSequence', 'Inner')
        data = types['ArrayOfStructSequence']()
        
        self.check_defaults(data, [
            Info('long1', 0),
            Info('array1', [[], []]),    
            ])
        
    def testArrayOfSimpleSequence(self):
        types = self.get_idl('ArrayOfSimpleSequence')
        data = types['ArrayOfSimpleSequence']()
        
        self.check_defaults(data, [
            Info('long1', 0),
            Info('array1', [[], []]),    
            ])
        
    def testSequence(self):
        types = self.get_idl('Sequence')
        data = types['Sequence']()
        
        self.check_defaults(data, [
            Info('long1', 0),
            Info('seq1', []),    
            ])

    def testSequenceOfSimpleArray(self):
        types = self.get_idl('SequenceOfSimpleArray')
        data = types['SequenceOfSimpleArray']()
        
        self.check_defaults(data, [
            Info('long1', 0),
            Info('sequence1', []),    
            ])
        
    def testSequenceOfStruct(self):
        types = self.get_idl('SequenceOfStruct')
        data = types['SequenceOfStruct']()
        
        self.check_defaults(data, [
            Info('long1', 0),
            Info('seq1', []),    
            ])

    def testSequenceOfStructArray(self):
        types = self.get_idl('SequenceOfStructArray')
        data = types['SequenceOfStructArray']()
        
        self.check_defaults(data, [
            Info('long1', 0),
            Info('seq1', []),    
            ])

if __name__ == "__main__":
    #import sys;sys.argv = ['', 'TestDynamicDefaults.testSimpleTypes']
    unittest.main()