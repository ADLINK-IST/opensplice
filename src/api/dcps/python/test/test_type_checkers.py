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
Created on Dec 22, 2017

@author: prismtech
'''
import unittest
import ddsutil
import enum

class MyEnum(enum.Enum):
    ONE = 0
    TWO = 1

class TestTypeCheckers(unittest.TestCase):


    _bool_checker = ddsutil._bool_checker
    def testBool(self):
        self.assertRaises(TypeError, lambda: self._bool_checker(None))
        self.assertRaises(TypeError, lambda: self._bool_checker(1))
        self._bool_checker(True)
        self._bool_checker(False)

    _octet_checker = ddsutil._octet_checker
    _ushort_checker = ddsutil._ushort_checker
    _ulong_checker = ddsutil._ulong_checker
    _ulonglong_checker = ddsutil._ulonglong_checker
    _short_checker = ddsutil._short_checker
    _long_checker = ddsutil._long_checker
    _longlong_checker = ddsutil._longlong_checker
    def testOctet(self):
        self.assertRaises(TypeError, lambda: self._octet_checker(None))
        self.assertRaises(TypeError, lambda: self._octet_checker(3.5))
        self.assertRaises(TypeError, lambda: self._octet_checker('a'))
        self.assertRaises(TypeError, lambda: self._octet_checker(b'a'))
        self.assertRaises(TypeError, lambda: self._octet_checker(-1))
        self.assertRaises(TypeError, lambda: self._octet_checker(256))
        self.assertRaises(TypeError, lambda: self._octet_checker(-1000))
        self.assertRaises(TypeError, lambda: self._octet_checker(2560))
        self._octet_checker(0)
        self._octet_checker(128)
        self._octet_checker(255)

    def testUShort(self):
        self.assertRaises(TypeError, lambda: self._ushort_checker(None))
        self.assertRaises(TypeError, lambda: self._ushort_checker(3.5))
        self.assertRaises(TypeError, lambda: self._ushort_checker('a'))
        self.assertRaises(TypeError, lambda: self._ushort_checker(b'a'))
        self.assertRaises(TypeError, lambda: self._ushort_checker(-1))
        self.assertRaises(TypeError, lambda: self._ushort_checker((1<<16)))
        self.assertRaises(TypeError, lambda: self._ushort_checker(-1000))
        self.assertRaises(TypeError, lambda: self._ushort_checker(350000))
        self._ushort_checker(0)
        self._ushort_checker((1<<15))
        self._ushort_checker((1<<16)-1)

    def testULong(self):
        self.assertRaises(TypeError, lambda: self._ulong_checker(None))
        self.assertRaises(TypeError, lambda: self._ulong_checker(3.5))
        self.assertRaises(TypeError, lambda: self._ulong_checker('a'))
        self.assertRaises(TypeError, lambda: self._ulong_checker(b'a'))
        self.assertRaises(TypeError, lambda: self._ulong_checker(-1))
        self.assertRaises(TypeError, lambda: self._ulong_checker((1<<32)))
        self.assertRaises(TypeError, lambda: self._ulong_checker(-1000))
        self.assertRaises(TypeError, lambda: self._ulong_checker((1<<40)))
        self._ulong_checker(0)
        self._ulong_checker((1<<31))
        self._ulong_checker((1<<32)-1)

    def testULongLong(self):
        self.assertRaises(TypeError, lambda: self._ulonglong_checker(None))
        self.assertRaises(TypeError, lambda: self._ulonglong_checker(3.5))
        self.assertRaises(TypeError, lambda: self._ulonglong_checker('a'))
        self.assertRaises(TypeError, lambda: self._ulonglong_checker(b'a'))
        self.assertRaises(TypeError, lambda: self._ulonglong_checker(-1))
        self.assertRaises(TypeError, lambda: self._ulonglong_checker((1<<64)))
        self.assertRaises(TypeError, lambda: self._ulonglong_checker(-1000))
        self.assertRaises(TypeError, lambda: self._ulonglong_checker((1<<66)))
        self._ulonglong_checker(0)
        self._ulonglong_checker((1<<60))
        self._ulonglong_checker((1<<64)-1)

    def testShort(self):
        self.assertRaises(TypeError, lambda: self._short_checker(None))
        self.assertRaises(TypeError, lambda: self._short_checker(3.5))
        self.assertRaises(TypeError, lambda: self._short_checker('a'))
        self.assertRaises(TypeError, lambda: self._short_checker(b'a'))
        self.assertRaises(TypeError, lambda: self._short_checker(-(1<<15) - 1)) # right outside the bounds
        self.assertRaises(TypeError, lambda: self._short_checker((1<<15))) # right outside the bounds
        self.assertRaises(TypeError, lambda: self._short_checker(-(1<<16) - 1)) # well outside the bounds
        self.assertRaises(TypeError, lambda: self._short_checker((1<<16))) # well outside the bounds
        self._short_checker(-(1<<15))
        self._short_checker(-5)
        self._short_checker(0)
        self._short_checker(5)
        self._short_checker((1<<15)-1)

    def testLong(self):
        self.assertRaises(TypeError, lambda: self._long_checker(None))
        self.assertRaises(TypeError, lambda: self._long_checker(3.5))
        self.assertRaises(TypeError, lambda: self._long_checker('a'))
        self.assertRaises(TypeError, lambda: self._long_checker(b'a'))
        self.assertRaises(TypeError, lambda: self._long_checker(-(1<<31) - 1)) # right outside the bounds
        self.assertRaises(TypeError, lambda: self._long_checker((1<<31))) # right outside the bounds
        self.assertRaises(TypeError, lambda: self._long_checker(-(1<<32) - 1)) # well outside the bounds
        self.assertRaises(TypeError, lambda: self._long_checker((1<<32))) # well outside the bounds
        self._long_checker(-(1<<31))
        self._long_checker(-5)
        self._long_checker(0)
        self._long_checker(5)
        self._long_checker((1<<31)-1)

    def testLongLong(self):
        self.assertRaises(TypeError, lambda: self._longlong_checker(None))
        self.assertRaises(TypeError, lambda: self._longlong_checker(3.5))
        self.assertRaises(TypeError, lambda: self._longlong_checker('a'))
        self.assertRaises(TypeError, lambda: self._longlong_checker(b'a'))
        self.assertRaises(TypeError, lambda: self._longlong_checker(-(1<<63) - 1)) # right outside the bounds
        self.assertRaises(TypeError, lambda: self._longlong_checker((1<<63))) # right outside the bounds
        self.assertRaises(TypeError, lambda: self._longlong_checker(-(1<<64) - 1)) # well outside the bounds
        self.assertRaises(TypeError, lambda: self._longlong_checker((1<<64))) # well outside the bounds
        self._longlong_checker(-(1<<63))
        self._longlong_checker(-5)
        self._longlong_checker(0)
        self._longlong_checker(5)
        self._longlong_checker((1<<63)-1)
        
    _char_checker = ddsutil._char_checker
    def testChar(self):
        self.assertRaises(TypeError, lambda: self._char_checker(None))
        self.assertRaises(TypeError, lambda: self._char_checker(3))
        self.assertRaises(TypeError, lambda: self._char_checker(b'a'))
        self.assertRaises(TypeError, lambda: self._char_checker(''))
        self.assertRaises(TypeError, lambda: self._char_checker('aa'))
        self.assertRaises(TypeError, lambda: self._char_checker(chr(256)))
        self._char_checker(chr(0))
        self._char_checker('1')
        self._char_checker(chr(255))
     
    _str_checker = ddsutil._str_checker
    _str2_checker = ddsutil._bounded_str_checker(2)
    def testStr(self):
        self.assertRaises(TypeError, lambda: self._str2_checker(None))
        self.assertRaises(TypeError, lambda: self._str2_checker(3))
        self.assertRaises(TypeError, lambda: self._str2_checker(b'a'))
        self.assertRaises(TypeError, lambda: self._str2_checker('aab'))
        self.assertRaises(TypeError, lambda: self._str2_checker(chr(256)))
        self._str2_checker('')
        self._str2_checker(chr(0))
        self._str2_checker('1')
        self._str2_checker('11')
        self._str2_checker(chr(255))
     
    _enum_checker = ddsutil._class_checker(MyEnum) 
    def testClass(self):
        self.assertRaises(TypeError, lambda: self._enum_checker(None))
        self.assertRaises(TypeError, lambda: self._enum_checker(1))
        self.assertRaises(TypeError, lambda: self._enum_checker('A'))
        self._enum_checker(MyEnum.ONE)
        self._enum_checker(MyEnum.TWO)
        
    _float_checker = ddsutil._float_checker
    def testFloat(self):
        self.assertRaises(TypeError, lambda: self._float_checker(None))
        self.assertRaises(TypeError, lambda: self._float_checker(1))
        self.assertRaises(TypeError, lambda: self._float_checker('A'))
        self._float_checker(3.15)
        self._float_checker(1.0)
        self._float_checker(-9.183)

    _long_array_checker = ddsutil._array_checker(2, ddsutil._long_checker)
    def testLongArray(self):
        self.assertRaises(TypeError, lambda: self._long_array_checker(None))
        self.assertRaises(TypeError, lambda: self._long_array_checker(1))
        self.assertRaises(TypeError, lambda: self._long_array_checker(1.0))
        self.assertRaises(TypeError, lambda: self._long_array_checker('A'))
        self.assertRaises(TypeError, lambda: self._long_array_checker([]))
        self.assertRaises(TypeError, lambda: self._long_array_checker([1]))
        self.assertRaises(TypeError, lambda: self._long_array_checker([1,2,3]))
        self.assertRaises(TypeError, lambda: self._long_array_checker([1.0,2]))
        self.assertRaises(TypeError, lambda: self._long_array_checker([1,2.0]))
        self.assertRaises(TypeError, lambda: self._long_array_checker([1<<32,2]))
        self.assertRaises(TypeError, lambda: self._long_array_checker([1,1<<32]))
        self._long_array_checker([1,2])
        self._long_array_checker([-(1<<31),(1<<31)-1])
        
    _long_seq_checker = ddsutil._seq_checker(2, ddsutil._long_checker)
    def testLongSeq(self):
        self.assertRaises(TypeError, lambda: self._long_seq_checker(None))
        self.assertRaises(TypeError, lambda: self._long_seq_checker(1))
        self.assertRaises(TypeError, lambda: self._long_seq_checker(1.0))
        self.assertRaises(TypeError, lambda: self._long_seq_checker('A'))
        self.assertRaises(TypeError, lambda: self._long_seq_checker([1,2,3]))
        self.assertRaises(TypeError, lambda: self._long_seq_checker([1.0,2]))
        self.assertRaises(TypeError, lambda: self._long_seq_checker([1,2.0]))
        self.assertRaises(TypeError, lambda: self._long_seq_checker([1<<32,2]))
        self.assertRaises(TypeError, lambda: self._long_seq_checker([1,1<<32]))
        self._long_seq_checker([])
        self._long_seq_checker([1])
        self._long_seq_checker([1,2])
        self._long_seq_checker([-(1<<31),(1<<31)-1])

    _long_ubseq_checker = ddsutil._seq_checker(0, ddsutil._long_checker)
    def testLongUBSeq(self):
        self.assertRaises(TypeError, lambda: self._long_ubseq_checker(None))
        self.assertRaises(TypeError, lambda: self._long_ubseq_checker(1))
        self.assertRaises(TypeError, lambda: self._long_ubseq_checker(1.0))
        self.assertRaises(TypeError, lambda: self._long_ubseq_checker('A'))
        self.assertRaises(TypeError, lambda: self._long_ubseq_checker([1.0,2]))
        self.assertRaises(TypeError, lambda: self._long_ubseq_checker([1,2.0]))
        self.assertRaises(TypeError, lambda: self._long_ubseq_checker([1<<32,2]))
        self.assertRaises(TypeError, lambda: self._long_ubseq_checker([1,1<<32]))
        self._long_ubseq_checker([])
        self._long_ubseq_checker([1])
        self._long_ubseq_checker([1,2])
        self._long_ubseq_checker([1,2,3,4,5])
        self._long_ubseq_checker([-(1<<31),(1<<31)-1])
        
    _long_matrix_checker = ddsutil._array_checker(1,ddsutil._array_checker(2,ddsutil._long_checker))
    def testLongMatrix(self):
        self.assertRaises(TypeError, lambda: self._long_matrix_checker([]))
        self.assertRaises(TypeError, lambda: self._long_matrix_checker([[]]))
        self.assertRaises(TypeError, lambda: self._long_matrix_checker([[1]]))
        self.assertRaises(TypeError, lambda: self._long_matrix_checker([[1,2,3]]))
        self.assertRaises(TypeError, lambda: self._long_matrix_checker([[1,2],[3,4]]))
        self._long_matrix_checker([[1,2]])

    _long_seqseq_checker = ddsutil._seq_checker(1,ddsutil._seq_checker(2,ddsutil._long_checker))
    def testLongSeqSeq(self):
        self._long_seqseq_checker([])
        self._long_seqseq_checker([[]])
        self._long_seqseq_checker([[1]])
        self._long_seqseq_checker([[1,2]])
        self.assertRaises(TypeError, lambda: self._long_seqseq_checker([[1,2,3]]))
        self.assertRaises(TypeError, lambda: self._long_seqseq_checker([[1,2],[3,4]]))

if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()