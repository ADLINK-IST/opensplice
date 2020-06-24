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
"""
DDS descriptor utility module

Main purpose of this module is to provide methods to convert
DDS descriptor to Python classes dynamically.

Users can use IDL file as an input to generate classes

"""
import xml.etree.ElementTree as ET
import subprocess
import dds
import struct
import enum
from collections import OrderedDict, namedtuple
import functools
from dds import DDSException

# constants
_MODULE_TAG      = 'Module'
_TYPEDEF_TAG     = 'TypeDef'
_STRUCT_TAG      = 'Struct'
_MEMBER_TAG      = 'Member'
_ARRAY_TAG       = 'Array'
_SEQUENCE_TAG    = 'Sequence'
_TYPE_TAG        = 'Type'
_STRING_TAG      = 'String'
_CHAR_TAG        = 'Char'
_ENUM_TAG        = 'Enum'
_ELEMENT_TAG     = 'Element'

_NAME_ATTRIBUTE  = 'name'
_SIZE_ATTRIBUTE  = 'size'
_VALUE_ATTRIBUTE = 'value'

_MODULE_SEPARATOR = '::'

#############################################################################
### integer range constants
class _IntSizes(enum.Enum):
    OCTET_MAX = (1<<8)-1
    CHAR_MIN = -(1<<7)
    CHAR_MAX = (1<<7)-1
    USHORT_MAX = (1<<16)-1
    SHORT_MIN = -(1<<15)
    SHORT_MAX = (1<<15)-1
    ULONG_MAX = (1<<32)-1
    LONG_MIN = -(1<<31)
    LONG_MAX = (1<<31)-1
    ULONGLONG_MAX = (1<<64)-1
    LONGLONG_MIN = -(1<<63)
    LONGLONG_MAX = (1<<63)-1


# keep references for created data classes
_class_dict = {}


class GeneratedClassInfo:
    '''
    Place holder for generated classes

    :ivar topic_data_class: Topic data class
    :ivar type_support_class: Type support class

    '''
    def __init__(self, data_cls, type_support_cls, nested_types):
        self.topic_data_class = data_cls
        self.type_support_class = type_support_cls
        self._nested_types = nested_types

    def get_class(self, type_name):
        ''' Return classes created dynamically

        :type type_name: string
        :param type_name: struct identifier (.e.g, "test::basic::inner_struct")
        '''
        if type_name not in self._nested_types:
            raise TypeError("{} class not found".format(type_name))

        return self._nested_types[type_name]

    def register_topic(self, dp, name, qos = None, listener = None):
        ''' Register topic for generated topic data class

        :type dp: DomainParticipant
        :param dp: Domain participant

        :type name: string
        :param name: topic name

        :type qos: Qos
        :param qos: topic QoS

        :type listener: dds.Listener
        :param listener: optional topic listener

        :rtype: Topic
        :returns: topic
        '''

        return dds.Topic(dp, name, self.type_support_class(), qos, listener)


def get_dds_classes_from_idl(idl_path, type_name):
    """Create DDS topic data class and DDS type support class
    from the given IDL file source.

    :type idl_path: string
    :param idl_path: path to IDL file

    :type type_name: string
    :param type_name: struct module (e.g., test::basic::my_struct)

    :rtype: GeneratedClassInfo
    :return: GeneratedClassInfo

    Examples:
        gen_info = dds_class("sample.idl", "basic::test::Type1", "long_1")

    """
    topictype = _get_dds_descriptor_from_idl(idl_path, type_name)
    descriptor = topictype.findtext('descriptor')
    keys = topictype.findtext('keys')
    #To Do : check output for error

    return _get_dds_classes_from_descriptor(descriptor, type_name, keys)

def get_dds_classes_for_found_topic(found_topic):
    """Create DDS topic data class and DDS type support class
    from a topic found via DomainParticipant.find_topic().

    :type found_topic: dds.FoundTopic
    :param found_topic: the topic entity for the found topic

    :rtype: GeneratedClassInfo
    :return: GeneratedClassInfo

    Examples:
        dp = DomainParticipant()
        found_topic = dp.find_topic('OsplTestTopic')
        gen_info = get_dds_classes_for_found_topic(found_topic)

    """
    descriptor = found_topic.metadescriptor
    keys = found_topic.keylist
    type_name = found_topic.type_name

    return _get_dds_classes_from_descriptor(descriptor, type_name, keys)

def register_found_topic_as_local(found_topic):
    '''
    Register a topic found by dds.DomainParticipant.find_topic as a locally available topic.
    The locally registered topic may be used to create data readers and writers.

    :type found_topic: dds.FoundTopic
    :param found_topic: a found topic
    :rtype: dds.Topic
    :return: a locally registered topic in the same DDS domain as the found topic
    '''
    dp = found_topic.parent
    gen_info = get_dds_classes_for_found_topic(found_topic)
    return gen_info.register_topic(dp, found_topic.name, found_topic.qos)

def find_and_register_topic(dp, topic_name):
    '''
    Find a topic in the specified domain, register it for local use

    :type dp: dds.DomainParticipant
    :param dp: the domain in which to search
    :type topic_name: str
    :param topic_name:  the name of the topic for which to search

    :rtype: (dds.Topic,ddsutil.GeneratedClassInfo)
    :return: a namedtuple(topic,gen_info) of the local topic handle and generated class information
    :raise DDSException: if the topic is not found or cannot be registered locally
    '''
    Info = namedtuple('LocalTopicInfo', 'topic,gen_info')
    found_topic = dp.find_topic(topic_name)
    if found_topic is not None:
        local_topic = register_found_topic_as_local(found_topic)
        gen_info = get_dds_classes_for_found_topic(found_topic)
        return Info(local_topic,gen_info)
    else:
        raise DDSException('Topic not found: {}'.format(topic_name))


def _get_dds_descriptor_from_idl(idl_path, type_name):
    """ Return DDS Type descriptor from IDL file

    :type idl_path: string
    :param idl_path: path to the IDL file

    :type type_name: string
    :param type_name: module hierarchy to the struct type

    """
    out = subprocess.Popen(["idlpp", "-l", "pythondesc", idl_path], stdout=subprocess.PIPE).communicate()[0]
    #print ("Descriptor : ", out)
    if not out.startswith(b'<topics'):
        raise RuntimeError("Problem found with given IDL file:\n" + out.decode())

    doc = ET.fromstring(out)
    topictype = doc.find("topictype[id='%s']" % type_name)
    if topictype is None:
        raise RuntimeError("Specified IDL file does not define topic type: " + type_name)

    return topictype

def _get_dds_classes_from_descriptor(descriptor, type_name, keys):
    """Dynamically create Python classes from DDS type descriptor.
    It creates DDS topic data class, DDS type support class,
    and other required classes for nested structs and enums.

    :type descriptor: string
    :param descriptor: DDS type descriptor

    :type type_name: string
    :param type_name: struct module (e.g., test::basic::my_struct)

    :type keys: string
    :param keys: key string

    """
    root = ET.fromstring(descriptor)
    # Compose xpath to find a topic struct
    gen_classes = {}
    _process_descriptor_element(root, "", gen_classes)
    #print(_class_dict)

    if type_name not in _class_dict:
        raise RuntimeError('Could not find topic data.')

    data_class = _class_dict[type_name]
    typesupport_class = _dds_type_support(descriptor, type_name, keys, data_class)
    return GeneratedClassInfo(data_class, typesupport_class, gen_classes)

def _process_descriptor_element(element, module, gen_classes):
    ''' Process descriptor xml tree recursively

        Args:
            element: element to process
            modules: module hierarchy
            _class_dict: dictionary for classes created
    '''
    if element.tag == 'MetaData':
        for el in element:
            _process_descriptor_element(el, module, gen_classes)

    elif element.tag == _MODULE_TAG:
        new_module = element.attrib[_NAME_ATTRIBUTE]
        if module:
            new_module = module + _MODULE_SEPARATOR + new_module
        for el in element:
            _process_descriptor_element(el, new_module, gen_classes)

    elif element.tag == _ENUM_TAG:
        enum_name = element.attrib[_NAME_ATTRIBUTE]
        key = enum_name
        if module:
            key = module + _MODULE_SEPARATOR + key
        if key not in _class_dict:
            enum_literals = {}
            for child in element:
                if child.tag == _ELEMENT_TAG:
                    enum_literals[child.attrib[_NAME_ATTRIBUTE]] = int(child.attrib[_VALUE_ATTRIBUTE])
            enum_cls = enum.Enum(enum_name, enum_literals)
            _class_dict[key] = enum_cls
        gen_classes[key] = _class_dict[key]

    elif element.tag == _TYPEDEF_TAG:
        key = element.attrib[_NAME_ATTRIBUTE]
        if module:
            key = module + _MODULE_SEPARATOR + key
        if key not in _class_dict:
            child = element[0]
            if child.tag == _TYPE_TAG:
                name = child.attrib[_NAME_ATTRIBUTE]
                if name.find(_MODULE_SEPARATOR) == -1 and module:
                    name = module + _MODULE_SEPARATOR + name
                    child.set(_NAME_ATTRIBUTE, name)
                    #print("name set to :", child.attrib['name'])
            elif child.tag == _ARRAY_TAG or child.tag == _SEQUENCE_TAG:
                #see if this is array of struct
                array_element = child
                array_type_element = array_element[0]
                if array_type_element.tag == _TYPE_TAG:
                    type_name = array_type_element.attrib[_NAME_ATTRIBUTE]
                    if type_name.find(_MODULE_SEPARATOR) == -1 and module:
                        type_name = module + _MODULE_SEPARATOR + type_name
                        array_type_element.set(_NAME_ATTRIBUTE, type_name)
            _class_dict[key] = child

    elif element.tag == _STRUCT_TAG:
        key = element.attrib[_NAME_ATTRIBUTE]
        if module:
            key = module + _MODULE_SEPARATOR + key
        if key not in _class_dict:
            members = OrderedDict()
            for child in element:
                if child.tag == _MEMBER_TAG:
                    if child[0].tag == _TYPE_TAG:
                        type_name = child[0].attrib[_NAME_ATTRIBUTE]
                        if type_name.find(_MODULE_SEPARATOR) == -1 and module:
                            type_name = module + _MODULE_SEPARATOR + type_name
                            child[0].set(_NAME_ATTRIBUTE, type_name)
                            #print("type name set to :", child[0].attrib['name'])
                    if child[0].tag == _ARRAY_TAG or child[0].tag == _SEQUENCE_TAG:
                        #see if this is array of struct
                        array_element = child[0]
                        array_type_element = array_element[0]
                        if array_type_element.tag == _TYPE_TAG:
                            type_name = array_type_element.attrib[_NAME_ATTRIBUTE]
                            if type_name.find(_MODULE_SEPARATOR) == -1 and module:
                                type_name = module + _MODULE_SEPARATOR + type_name
                                array_type_element.set(_NAME_ATTRIBUTE, type_name)
                                #print("type name set to :", array_type_element.attrib['name'])
                    members[child.attrib[_NAME_ATTRIBUTE]] = child[0]

            cls = _create_class(key.replace(_MODULE_SEPARATOR, "_"), members)
            _class_dict[key] = cls
        gen_classes[key] = _class_dict[key]

class TopicDataClass(object):
    '''
    Abstract topic data class.
    Generated classes inherits this base class.
    '''

    def __init__(self, member_names = []):
        _member_attributes = member_names
        _typesupport = None
        _nested_types = {}
        pass

    def get_vars(self):
        '''
        Return the dictionary of attribute and value pair for the topic data members.
        '''
        result = OrderedDict()
        for member in self._member_attributes:
            result[member] = getattr(self, member)

        return result

    def _get_print_vars(self):
        result = []
        for key, val in self.get_vars().items():
            if isinstance(val, TopicDataClass):
                result.append("{}: {{{}}}".format(key, val._get_print_vars()))
            elif isinstance(val, list):
                result.append("{}: [{}]".format(key, self._format_list(val)))
            else:
                result.append("{}: {}".format(key, val))
        return ', '.join(result)

    def _format_list(self, list_val):

        result = []
        for val_item in list_val:
            if isinstance(val_item, TopicDataClass):
                result.append("{{{}}}".format(val_item._get_print_vars()))
            elif isinstance(val_item, list):
                result.append("[{}]".format(self._format_list(val_item)))
            else:
                result.append("{}".format(val_item))
        return ', '.join(result)

    def __str__(self):
        return self._get_print_vars()

    def print_vars(self):
        '''
        Print values of all member variables.
        '''
        print(self._get_print_vars())

def _get_field_default(ele):
    if ele.tag == _STRING_TAG:
        return ''
    elif ele.tag == _SEQUENCE_TAG:
        return []
    elif ele.tag == _ARRAY_TAG:
        array_size = int(ele.attrib[_SIZE_ATTRIBUTE])
        array_type = ele[0]
        return [_get_field_default(array_type) for _ in range(array_size)]
    elif ele.tag == _TYPE_TAG:
        typename = ele.attrib[_NAME_ATTRIBUTE]
        actual_type = _get_actual_type(typename)
        if isinstance(actual_type, enum.EnumMeta):
            return actual_type(0)
        elif isinstance(actual_type, type):
            return actual_type()
        else:
            # it's a typedef, recurse...
            return _get_field_default(actual_type)
    elif ele.tag == 'Boolean':
        return False
    elif ele.tag == 'Char':
        return '\x00'
    elif ele.tag in ('Octet', 'Short', 'Long', 'LongLong', 'UShort', 'ULong', 'ULongLong'):
        return 0
    elif ele.tag in ('Float', 'Double'):
        return 0.0
    return None

def _create_class(name, members):
    ''' Create python class given data members

        Args:
            name(string): name of the class
            members(dict): name and type of properties
        Returns:
            dynamically created python class
    '''
    def _serialize(self):
        ''' Serialize data for writing
        '''
        fmt = self._get_packing_fmt()
        args = self._get_packing_args()
        #print("Packing format: ", fmt)
        #print("Packing args: ", args)
        result = struct.pack(fmt, *args)
        #print("packing ({}): {}".format(struct.calcsize(fmt), str(result)))
        return result

    def _deserialize(self, data):
        ''' Initialize values from data which is unpacked already
        '''
        #print("Deserializing data: ", data)
        for mem, mem_type in self._members.items():
            result = _deserialize_data(mem_type, data)
            setattr(self, mem, result)

    def _get_packing_fmt(self):
        ''' Returns packing format of this class data
        '''
        fmt_args = []
        _compute_cls_packing_fmt(self, fmt_args)

        result = ''.join(fmt_args)
        return result

    def _get_packing_args(self):
        args = []
        for m, mem_type in self._members.items():
            val = getattr(self, m)
            _compute_packing_args(mem_type, args, val)
        return args

    def __init__(self, **kwargs):

        setattr(self, '_members', members)
        setattr(self, '_member_attributes', members.keys())

        # define variables with default value
        for member in members.keys():
            setattr(self, member, _get_field_default(members[member]))

        # set values for variables passed in
        for key, value in kwargs.items():
            if key not in members:
                raise TypeError("Invalid argument name : %s" %(key))
            setattr(self, key, value)

    cls_name = name
    slots =list(members.keys())
    slots.append("_members")
    slots.append("__dict__")
    cls_attrs = {"__init__": __init__,
                 #"__slots__": slots,
                 #"get_vars": get_vars,
                 "_serialize": _serialize,
                 "_deserialize": _deserialize,
                 "_get_packing_fmt": _get_packing_fmt,
                 "_get_packing_args":_get_packing_args}

    # create topic data class
    data_class = type(cls_name, (TopicDataClass,), cls_attrs)
    return data_class

def _deserialize_data(mem_type, data):
    ''' Helper function to deserialize data
    '''
    if mem_type.tag == _TYPE_TAG:
        typename = mem_type.attrib[_NAME_ATTRIBUTE]
        actual_type = _get_actual_type(typename)
        if isinstance(actual_type, enum.EnumMeta):
            val = actual_type(data.pop(0))
            return val
        elif isinstance(actual_type, type):
            _cls = actual_type()
            _cls._deserialize(data)
            return _cls
        else:
            mem_type = actual_type

    if mem_type.tag == _STRING_TAG:
        s = _ptr_to_bytes(data.pop(0)).decode('ISO-8859-1')
        return s
    elif mem_type.tag == _CHAR_TAG:
        return data.pop(0).decode('ISO-8859-1')
    elif mem_type.tag == _SEQUENCE_TAG:
        result = _deserialize_sequence(mem_type, data)
        return result

    elif mem_type.tag == _ARRAY_TAG:
        result = _deserialize_array(mem_type, data)
        return result

    else:
        return data.pop(0)

def _deserialize_sequence(mem_type, data):
    ''' Helper function to deserialize sequence data
    '''
    seq_type = mem_type[0].tag
    seq_bound = data.pop(0)
    seq_size = data.pop(0)
    seq_ptr = data.pop(0)
    seq_release = data.pop(0)
    actual_type = mem_type[0]
    result = []
    if seq_size != 0:
        if seq_type == _TYPE_TAG:
            typename = mem_type[0].attrib[_NAME_ATTRIBUTE]
            actual_type = _get_actual_type(typename)
            if isinstance(actual_type, enum.EnumMeta):
                fmt = seq_size * _to_packing_fmt([_ENUM_TAG])
                fmt_size = struct.calcsize(fmt)
                buff = _ptr_to_bytes(seq_ptr, fmt_size)
                unpacked = struct.unpack(fmt, buff)
                [result.append(actual_type(unpacked.pop(0))) for _ in range(seq_size)]
                return result
            elif isinstance(actual_type, type):
                cls = actual_type()
                cls_fmt = seq_size * cls._get_packing_fmt()
                fmt_size = struct.calcsize(cls_fmt)
                buff = _ptr_to_bytes(seq_ptr, fmt_size)
                unpacked = list(struct.unpack(cls_fmt, buff))
                for _ in range(seq_size):
                    cls = actual_type()
                    cls._deserialize(unpacked)
                    result.append(cls)
                return result
        seq_type = actual_type.tag
        seq_fmt_args = []
        _compute_packing_fmt(actual_type, seq_fmt_args, 1)
        fmt = seq_size * ''.join(seq_fmt_args)
        fmt_size = struct.calcsize(fmt)
        buff = _ptr_to_bytes(seq_ptr, fmt_size)
        unpacked = list(struct.unpack(fmt, buff))
        [result.append(_deserialize_data(actual_type, unpacked)) for _ in range(seq_size)]
    return result

def _deserialize_array(mem_type, data):
    ''' Map deserialized array data into list variable
    Args:
        mem(string): member name
        mem_type(element): member type
        data: deserialized data from buffer
    '''
    array_size = int(mem_type.attrib[_SIZE_ATTRIBUTE])
    array_type = mem_type[0]
    result = []
    [result.append(_deserialize_data(array_type, data)) for _ in range(array_size)]
    return result

# Global variables
_global_packed = []

def _bytes_to_ptr(packed):
    ''' Call to bytes to pointer
    '''

    encoded = packed
    if isinstance(packed, str):
        encoded = str.encode(packed, 'ISO-8859-1')
    _global_packed.append(encoded)
    return dds._SerializationHelper.bytes_to_ptr(encoded)

def _ptr_to_bytes(addr, size = 0):
    ''' Call to pointer to bytes
    '''
    return dds._SerializationHelper.ptr_to_bytes(addr, size)

def _compute_packing_args(mem_type, args, val):
    ''' Helper function to compute packing arguments for
        struct.pack

        Args: mem_type (element): member type
              args(list) : args to be stored
              val: values to be packed

    '''

    if mem_type.tag == _TYPE_TAG:
        typename = mem_type.attrib[_NAME_ATTRIBUTE]
        actual_type = _get_actual_type(typename)
        if isinstance(actual_type, enum.EnumMeta):
            args.append(val.value)
            return
        elif isinstance(actual_type, type):
            sub_args = val._get_packing_args()
            args.extend(sub_args)
            return
        else:
            mem_type = actual_type

    if mem_type.tag == _STRING_TAG:
        addr = _bytes_to_ptr(val)
        args.append(addr)
    elif mem_type.tag == _CHAR_TAG:
        args.append(val[0:1].encode('ISO-8859-1'))
    elif mem_type.tag == _SEQUENCE_TAG:
        _compute_sequence_packing_args(mem_type, args, val)

    elif mem_type.tag == _ARRAY_TAG:
        _compute_array_packing_args(mem_type, args, val)

    else:
        args.append(val)

def _compute_sequence_packing_args(mem_type, args, val):
    ''' Line up arguments for sequence packing
    Args:
        mem_type(element): array type
        args(list): array to append arguments
        val(list): array values in list
    '''
    seq_type = mem_type[0].tag
    seq_size = len(val)
    seq_bound = len(val)
    if _SIZE_ATTRIBUTE in mem_type.attrib:
        seq_bound = int(mem_type.attrib[_SIZE_ATTRIBUTE])
    actual_type = mem_type[0]

    if seq_type == _TYPE_TAG:
        typename = mem_type[0].attrib[_NAME_ATTRIBUTE]
        actual_type = _get_actual_type(typename)
        if isinstance(actual_type, type):
            cls_packed = []
            [cls_packed.append(cls._serialize()) for cls in val]
            packed = b''.join(cls_packed)
            #print("packed", packed)
            addr = _bytes_to_ptr(packed)
            args.extend([seq_bound, seq_size, addr, True])
            return

    seq_type = actual_type.tag
    seq_fmt_args = []
    _compute_packing_fmt(actual_type, seq_fmt_args, 1)
    seq_fmt = seq_size * ''.join(seq_fmt_args)
    seq_args = []
    for i in range(seq_size):
        _compute_packing_args(actual_type, seq_args, val[i])

    packed = struct.pack(seq_fmt, *seq_args)
    addr = _bytes_to_ptr(packed)
    args.extend([seq_bound, seq_size, addr, True])

def _compute_array_packing_args(mem_type, args, val):
    ''' Line up arguments for array packing
    Args:
        mem_type(element): array type
        args(list): array to append arguments
        val(list): array values in list
    '''
    array_size = int(mem_type.attrib[_SIZE_ATTRIBUTE])
    array_type = mem_type[0]
    for i in range(array_size):
        _compute_packing_args(array_type, args, val[i])

def _compute_cls_packing_fmt(o, fmt_args):
    ''' Compute packing format for class
    '''
    align = 1
    sub_fmt_args = []
    for mem_type in o._members.values():
        align = _compute_packing_fmt(mem_type, sub_fmt_args, align)
    fmt_args.extend(list(_align(''.join(fmt_args),''.join(sub_fmt_args))))
    offset = struct.calcsize(''.join(fmt_args))
    paddings = _get_padding(offset, align)
    if paddings :
        fmt_args.append(paddings)
        fmt_args.append(' ')
    return align

def _compute_packing_fmt(mem_type, fmt_args, align):
    ''' Compute packing format for the given data class

        Args:
            mem_type(element) : member data type
            fmt_args(list): list to store argument format
            align: max alignment

    '''
    if mem_type.tag == _TYPE_TAG:
        actual_key = mem_type.attrib[_NAME_ATTRIBUTE]
        actual_type = _get_actual_type(actual_key)
        if isinstance(actual_type, enum.EnumMeta):
            align = _calcalign('i')
            fmt_args.append('i')
            return align
        elif isinstance(actual_type, type):
            sub_cls = actual_type()
            sub_align = _compute_cls_packing_fmt(sub_cls, fmt_args)
            align = max(align, sub_align)
            return align
        else:
            mem_type = actual_type

    if mem_type.tag == _SEQUENCE_TAG:
        seq_align = struct.calcsize('P')
        align = max(align, seq_align)
        offset = struct.calcsize(''.join(fmt_args))
        paddings = _get_padding(offset, align)
        if paddings:
            if fmt_args[-1] == ' ':
                fmt_args.pop()
            fmt_args.append(paddings)
            fmt_args.append(' ')
        seq_fmt = _to_packing_fmt([mem_type.tag])
        seq_offset = struct.calcsize(seq_fmt)
        seq_paddings = _get_padding(seq_offset, seq_align)
        fmt_args.append(seq_fmt)
        fmt_args.append(seq_paddings)

    elif mem_type.tag == _ARRAY_TAG:
        _compute_array_packing_fmt(mem_type, fmt_args, align)

    else:
        mem_type = mem_type.tag
        fmt = _to_packing_fmt([mem_type])
        align = _calcalign(fmt)
        fmt_args.append(fmt)

    fmt_args.append(' ')
    return align

def _compute_array_packing_fmt(mem_type, fmt_args, align):
    ''' Get struct format string for array type
    Args:
        mem_type(element): Element tree element for array type
        fmt_args(array): format string to append
        align: max alignment of struct including this element
    '''
    #This is array type
    array_size = int(mem_type.attrib[_SIZE_ATTRIBUTE])
    array_type = mem_type[0]
    array_fmt = []
    align =_compute_packing_fmt(array_type, array_fmt, align)

    offset = struct.calcsize(''.join(fmt_args))
    paddings = _get_padding(offset, align)
    if paddings:
        if fmt_args[-1] == ' ':
            fmt_args.pop()
        fmt_args.append(paddings)
        fmt_args.append(' ')

    fmt = ''.join(array_fmt)
    fmt_args.append("{}".format(array_size * fmt))

def _get_padding(offset, align):
    ''' Calculate paddings required
    '''
    remainder = offset % align
    if remainder != 0:
        return str(align - remainder) + 'x';
    else:
        ''

def _get_actual_type(type_name):
    ''' Return actual type
    '''

    # typedef in other root module starts with :: so remove it
    if type_name.startswith(_MODULE_SEPARATOR):
        type_name = type_name[2:]

    if type_name not in _class_dict:
        raise TypeError("Fail to resolve type for " + type_name)

    result = _class_dict[type_name]
    if isinstance(result, type):
        return result

    if result.tag == _TYPE_TAG:
        type_key = result.attrib[_NAME_ATTRIBUTE]
        return _get_actual_type(type_key)

    return result

def _dds_type_support(descriptor, type_name, keys, data_class):
    """Create type support class.
    Intended to be called from get_dds_classes_from_descriptor method.

    Args:
        descriptor: DDS descriptor
        type_name: module hierarchy to the struct type
        keys: DDS keys
        data_class(class): topic data class
        members (dictionary): member name and type

    Returns:
        dds.TypeSupport class for given data_class

    """
    def __init__(self):
        fmt = data_class()._get_packing_fmt()
        setattr(self, '_packing_format', fmt)
        setattr(self, '_cls', data_class)
        dds.TypeSupport.__init__(self, type_name, keys, descriptor, struct.calcsize(self._packing_format))

    def _serialize(self, o):
        global _global_packed
        # initialize stack
        _global_packed.clear()

        if not isinstance(o, self._cls):
            raise TypeError('Incorrect data type')

        result = o._serialize()
        #print("Result: ", str(result))
        return result

    def _deserialize(self, buf):
        #print("incoming buff: {}".format(buf))
        data = struct.unpack(self._packing_format, buf)
        result = self._cls()
        result._deserialize(list(data))
        return result
    # create methods
    methods = {'__init__': __init__, '_serialize':_serialize, '_deserialize':_deserialize}
    # create class for type support
    new_class = type(data_class.__name__ + 'TypeSupport', (dds.TypeSupport,), methods)
    return new_class

def _to_packing_fmt(dds_types):
    """ Calculate packing format for struct.pack

    Args:
        dds_types(list): list of member types

    Returns:
        packing format string
    """
    result = []
    for val in dds_types:
        if 'Short' == val:
            result.append('h')
        elif 'UShort' == val:
            result.append('H')
        elif 'Boolean' == val:
            result.append('?')
        elif 'Long' == val:
            result.append('i')
        elif 'ULong' == val:
            result.append('I')
        elif 'LongLong' == val:
            result.append('q')
        elif 'ULongLong' == val:
            result.append('Q')
        elif 'Float' == val:
            result.append('f')
        elif 'Double' == val:
            result.append('d')
        elif 'Char' == val:
            result.append('c')
        elif 'Octet' == val:
            result.append('B')
        elif 'String' == val:
            result.append('P')
        elif 'Enum' == val:
            result.append('i')
        elif 'Sequence' == val:
            result.append('iiP?')
    return ' '.join(result)

##########################################################################
# Code below supports statically generated Python code from IDLPP
# Statically generated Python my also uses the following methods above this
# line:
#   _bytes_to_ptr
#   _ptr_to_bytes
##########################################################################

_alignment = {}

for _c in 'cBhHiIqQfd?Px':
    _alignment[_c] = struct.calcsize('c' + _c) - struct.calcsize(_c)

def _calcalign(fmt):
    ''' calculate the alignment of the passed struct.pack format '''
    align = 1
    for c in fmt:
        if c not in ' 0123456789':
            align = max(align, _alignment[c])
    return align

def _padding(size, align_to_size):
    ''' calculate a struct.pack padding string given a current size, the required alignment '''
    pad_count = (align_to_size - (size % align_to_size)) % align_to_size
    return pad_count * 'x'

def _pad_fmt(fmt):
    ''' add appropriate struct.pack pad characters to the end of format for a C struct'''
    sz = struct.calcsize(fmt)
    align = _calcalign(fmt)
    return fmt + _padding(sz, align)

def _align(existing_fmt, fmt):
    ''' add appropriate struct.pack pad characters prior to the format for a C struct'''
    align = _calcalign(fmt)
    sz = struct.calcsize(existing_fmt)
    return _padding(sz, align) + fmt

def _lin_map_array(nDArray, ndims, helper):
    ''' do a function map of a multi-dimenional array by first linearizing it then calling helper on each element'''
    result = nDArray
    while ndims > 1:
        result = functools.reduce(lambda prev,v: (prev.extend(v),prev)[1], result, [])
        ndims -= 1
    return [helper(v) for v in result]

def _deserialize_seq(data, seq_content_fmt, helper):
    ''' deserialize a sequence struct for 'data', then find the referenced buffer
     and deserialize its contents, calling helper to properly instantiate the sequence
     data '''
    seq_length = data.pop(0)
    data.pop(0) # seq_max - we don't care about this value
    buffer_pointer = data.pop(0)
    data.pop(0) # seq 'free' flag - dont' care about this value
    if buffer_pointer != 0:
        buffer = _ptr_to_bytes(buffer_pointer, struct.calcsize(seq_content_fmt) * seq_length)
        seq_data = list(struct.unpack(seq_content_fmt * seq_length, buffer))
        return [helper(seq_data) for _ in range(seq_length)]
    else:
        return []

def _serialize_seq(seq, seq_content_fmt, helper):
    ''' serialize a sequence whose contents has pack format seq_content_fmt.
    Use helper to obtain pack arguments for each sequence entry '''
    seq_length = len(seq)
    pack_args = []
    for v in seq:
        helper(pack_args, v)
    if seq_length > 0:
        buffer = struct.pack(seq_content_fmt * seq_length, *pack_args)
        buffer_pointer = _bytes_to_ptr(buffer)
    else:
        buffer_pointer = 0
    return [seq_length, seq_length, buffer_pointer, False]

def _calc_union_formats(discriminator_fmt, label_map):
    '''
    @param discriminator_fmt: str
    @param label_map: dict
    '''
    dalign = _calcalign(discriminator_fmt)
    dsize = struct.calcsize(discriminator_fmt)
    ualign = max([_calcalign(v['packing_fmt']()) for v in label_map.values()])
    usize = max([struct.calcsize(v['packing_fmt']()) for v in label_map.values()])
    ufmt = '{}B'.format(usize)

    fmt = discriminator_fmt + _padding(dsize, ualign) + ufmt
    fmt += _padding(struct.calcsize(fmt),max(dalign,ualign))

    label_to_padded_packing_fmt = {}
    for label, v in label_map.items():
        ffmt = v['packing_fmt']()
        sz_diff = usize - struct.calcsize(ffmt)
        if sz_diff > 0:
            ffmt += '{}x'.format(sz_diff)
        label_to_padded_packing_fmt[label] = ffmt

    return (fmt, label_to_padded_packing_fmt)

def _bool_checker(self,value):
    if not isinstance(value, bool):
        raise TypeError ('must be a of type bool')

def _int_checker(minval, maxval):
    def checker(self,value):
        if not isinstance(value, int):
            raise TypeError('must be of type int')
        if not(minval <= value and value <= maxval):
            raise TypeError('{} is not between {} and {}'.format(value,minval,maxval))
    return checker

_octet_checker = _int_checker(0, _IntSizes.OCTET_MAX.value)
_ushort_checker = _int_checker(0,_IntSizes.USHORT_MAX.value)
_ulong_checker = _int_checker(0,_IntSizes.ULONG_MAX.value)
_ulonglong_checker = _int_checker(0,_IntSizes.ULONGLONG_MAX.value)
_short_checker = _int_checker(_IntSizes.SHORT_MIN.value,_IntSizes.SHORT_MAX.value)
_long_checker = _int_checker(_IntSizes.LONG_MIN.value,_IntSizes.LONG_MAX.value)
_longlong_checker = _int_checker(_IntSizes.LONGLONG_MIN.value,_IntSizes.LONGLONG_MAX.value)

def _float_checker(self,value):
    if not isinstance(value, float):
        raise TypeError('must be of type float')

def _array_checker(bound, inner_checker):
    def checker(self,value):
        if not isinstance(value, list):
            raise TypeError('must be of type list')
        if len(value) != bound:
            raise TypeError('must have length {}'.format(bound))
        for i in range(len(value)):
            try:
                inner_checker(self,value[i])
            except TypeError as e:
                if hasattr(e, 'context'):
                    context = '[{}]{}'.format(i, e.context)
                    base_message = e.base_message
                else:
                    context = '[{}]'.format(i)
                    base_message = str(e.args[0])
                new_e = TypeError('value{}: {}'.format(context,base_message))
                new_e.context = context
                new_e.base_message = base_message

                raise new_e
    return checker

def _seq_checker(bound, inner_checker):
    def checker(self,value):
        if not isinstance(value, list):
            raise TypeError('must be of type list')
        if bound > 0 and len(value) > bound:
            raise TypeError('must have maximum length {}'.format(bound))
        for i in range(len(value)):
            try:
                inner_checker(self,value[i])
            except TypeError as e:
                if hasattr(e, 'context'):
                    context = '[{}]{}'.format(i, e.context)
                    base_message = e.base_message
                else:
                    context = '[{}]'.format(i)
                    base_message = str(e.args[0])
                new_e = TypeError('value{}: {}'.format(context,base_message))
                new_e.context = context
                new_e.base_message = base_message

                raise new_e
    return checker

def _char_checker(self,value):
    if not isinstance(value, str):
        raise TypeError('must be of type str')
    if len(value) != 1:
        raise TypeError('must have length of 1')
    try:
        value.encode('ISO-8859-1')
    except UnicodeEncodeError as e:
        raise TypeError('must encode to ISO-8859-1: ' + e.args[0])

def _bounded_str_checker(bound):
    def checker(self,value):
        if not isinstance(value, str):
            raise TypeError('Value must be of type str')
        if bound > 0 and len(value) > bound:
            raise TypeError('String length of {} exceeds maximum allowed of {}'.format(len(value),bound))
        try:
            value.encode('ISO-8859-1')
        except UnicodeEncodeError as e:
            raise TypeError('must encode to ISO-8859-1: ' + e.args[0])
    return checker

_str_checker = _bounded_str_checker(0)

def _class_checker(class_type):
    def checker(self,value):
        if not isinstance(value, class_type):
            raise TypeError('must be of type {}'.format(class_type.__name__))
    return checker

