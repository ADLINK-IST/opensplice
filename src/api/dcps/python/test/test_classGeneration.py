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
import sys
sys.path.append('../dds')
from ddsutil import *
import unittest
import xmlrunner
        
class DynamicClassGenerationTests(unittest.TestCase):
    
    def test_basic_types(self):
        self.dynamic_create_test("SimpleTypes")
    
    def test_nested_struct(self):
        self.dynamic_create_test("NestedStruct")

    def test_array(self):
        self.dynamic_create_test("Array")
    
    def test_sequence(self):
        self.dynamic_create_test("Sequence")

    def test_nested_struct_array(self):
        self.dynamic_create_test("ArrayOfStruct")
    
    def test_nested_struct_sequence(self):
        self.dynamic_create_test("SequenceOfStruct")
                                
    def dynamic_create_test(self, idl_name):
        
        idl_path = 'idl/' + idl_name + '.idl'
        type_info = "basic::module_{}::{}".format(idl_name, idl_name + '_struct')
        gen_info = get_dds_classes_from_idl(idl_path, type_info)
        Sample = gen_info.topic_data_class
        SampleType = gen_info.type_support_class
        s = Sample()
        self.assertIsNotNone(Sample, "Topic data class not created")
        self.assertIsNotNone(SampleType, "Topic support class not created")         
if __name__ == '__main__':
    unittest.main(testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        # these make sure that some options that are not applicable
        # remain hidden from the help menu.
        failfast=False, buffer=False, catchbreak=False)