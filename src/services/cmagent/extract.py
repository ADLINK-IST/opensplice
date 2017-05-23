#!/usr/bin/env python
# This script extracts service-configuration from a source file and merges it with the OpenSplice
# meta-config XML file. The source-file needs to adhere to the DDSI2-style configuration backend.

# Any data that already exists is overwritten unless the element or attribute XPath is in blacklist
#
# Requires: clang, lxml, asciitree==0.2

import sys
import os

import clang.cindex
from clang.cindex import CursorKind

from lxml import etree
from pprint import pprint

# Cfgelem data imported from C can be ignored if name appears on blacklist
# Note: 'name' is treated as a mandatory field of struct cfgelem
blacklist = [
    "Domain" # Don't generate meta-config for Domain tag, it only exists for retrieving parameters (lease etc)
]

typehints = {
    "uf_nopstring": {
        "xmltype": "String"
    },
    "uf_boolean": {
        "xmltype": "Boolean"
    },
    "uf_tracingOutputFileName": {
        "xmltype": "String",
        "dimension": "file path"
    },
    "uf_verbosity": {
        "xmltype": "Enum",
        "values": [ "finest", "finer", "fine", "config", "info", "warning", "severe", "none" ]
    },
    "uf_logcat": { "xmltype": "String" },
    "uf_float": { "xmltype": "Float" },
    "uf_timeReal": { "xmltype": "Float" },
    "uf_int32": { "xmltype": "Int" },
    "uf_sched_prio_class": {
        "xmltype": "Enum",
        "values": [ "relative", "absolute" ]
    },
    "uf_sched_class": {
        "xmltype": "Enum",
        "values": [ "realtime", "timeshare", "default"]
    }
}

# todo: Should this be part of struct cfgelem?
# In case of cmagent, everything is community so hard-code it for now
VERSION="COMMUNITY"

class CfgElem(object):
    # This class represents the cfgelem struct in python
    def __init__(self, name):
        self.name = name

    def __eq__(self, other):
        # This should not be used to compare things that have the same name but a different parent!
        return self.name == other.name

    def __str__(self):
        str = "Name: {}\n".format(self.name)
        if self.children:
            str += " Children: {}\n".format(", ".join(x.name for x in self.children))
        else:
            str += " XML Type: leaf{}\n".format(self.typehint["xmltype"])
        if self.attributes:
            str += " Attributes: {}\n".format(", ".join(x.name for x in self.attributes))
        str += " Multiplicity: {}\n".format(self.multiplicity)
        str += " Default: {}\n".format(self.default)
        str += " Description: {}".format(self.description)
        return str

    @property
    def name(self):
        return self.__name

    @name.setter
    def name(self, name):
        self.__name = sanitizeString(name)

    @property
    def children(self):
        return self.__children

    @children.setter
    def children(self, children):
        self.__children = children

    @property
    def attributes(self):
        return self.__attrs

    @attributes.setter
    def attributes(self, attrs):
        if attrs:
            for attr in attrs:
                assert not attr.children and not attr.attributes, \
                    "Attribute cannot have children or attributes"
        self.__attrs = attrs

    @property
    def multiplicity(self):
        return self.__multiplicity

    @multiplicity.setter
    def multiplicity(self, multiplicity):
        self.__multiplicity = int(multiplicity)

    @property
    def default(self):
        return self.__default

    @default.setter
    def default(self, default):
        self.__default = sanitizeString(default)

    @property
    def description(self):
        return self.__description

    @description.setter
    def description(self, description):
        self.__description = sanitizeString(description)

    @property
    def typehint(self):
        return self.__typehint

    @typehint.setter
    def typehint(self, hint):
        if hint:
            assert not self.children, \
                "group element {} should not have typehint ({})".format(self.name, hint)
            assert hint in typehints, \
                "typehint '{}' for leaf element {} is unknown".format(hint, self.name)
            self.__typehint = typehints[hint]
        else:
            self.__typehint = None

def sanitizeString(strval):
    # Used for optional string-literals (parsed from C code)
    if strval:
        assert strval.startswith('"') and strval.endswith('"'), \
            "Expected double-quoted string-literal, got '{}'".format(strval)
        return strval[1:-1].replace('\\n', '').replace('\\"', '"')
    else:
        return None

def print_cfgelem_ast(translation_unit):
    # for debugging
    import asciitree # must be v0.2, newer doesn't work!
    for cursor in tu.cursor.get_children():
        if (cursor.kind == CursorKind.VAR_DECL and
            cursor.type.spelling.startswith("const struct cfgelem")):
            asciitree.draw_tree(cursor,
                                lambda n: n.get_children(),
                                lambda n: "{} ({}) at {}".format(n.spelling or n.displayname,
                                                                 str(n.kind).split(".")[1],
                                                                 n.extent))

def findOrCreateElement(expr, parent, tagname, name_attr=None):
    r = parent.xpath(expr)
    if r:
        # sanity-check the existing XML
        assert len(r) == 1, \
            "Multiple ({}) occurrences of {} are not allowed!".format(len(r))
        print("Merge {} in {}".format(expr[2:], parent.getroottree().getpath(parent)[1:]))
        return r[0]
    else:
        print("Insert {} in {}".format(expr[2:], parent.getroottree().getpath(parent)[1:]))
        if name_attr:
            return etree.SubElement(parent, tagname, name=name_attr)
        else:
            return etree.SubElement(parent, tagname)

def doSimpleElement(tagname, parent_element, source=None, as_cdata=False):
    # if source-data available, find or create a new element and set source-data as text
    # if source-data is None, find and remove element

    if not source is None:
        # Use source as text-data for child-element 'tagname'
        child = findOrCreateElement("./{}".format(tagname), parent_element, tagname)
        if as_cdata:
            data = etree.CDATA(source)
        else:
            data = source

        if child.text and child.text != source:
            print("Replace '{}' text".format(tagname))
        child.text = data
    else:
        # Remove child-element 'tagname' if it exists
        r = parent_element.xpath("./{}".format(tagname))
        if r:
            assert len(r) == 1, \
                "Multiple ({}) occurrences of '{}' are not allowed!".format(len(r))
            print("Remove '{}'".format(tagname))
            parent_element.remove(r[0])

def doLeafElement(element, ce):
    # process content common for attribute-leafs and element-leafs

    # default
    doSimpleElement("default", element, ce.default, False)

    # dimension
    dimension = ce.typehint.get("dimension", None)
    doSimpleElement("dimension", element, dimension, False)

    # Leaf-kind specific child(s)
    if ce.typehint["xmltype"] == "Enum":
        # enum values
        r = element.xpath("./value")
        values = list(ce.typehint["values"]) # copy list for removals
        if r:
            for value_element in r:
                if not value_element.text in values:
                    print("Remove enum value {}".format(value_element.text))
                    element.remove(value_element)
                else:
                    values.remove(value_element.text)

        for value in values:
            print("Insert enum value {}".format(value))
            value_element = etree.SubElement(element, "value")
            value_element.text = value

    elif (ce.typehint["xmltype"] == "Int" or
          ce.typehint["xmltype"] == "Float"):
        # minimum
        minimum = ce.typehint.get("minimum")
        doSimpleElement("minimum", element, minimum, False)

        # maximum
        maximum = ce.typehint.get("maximum")
        doSimpleElement("maximum", element, maximum, False)
    elif ce.typehint["xmltype"] == "String":
        # maxLength
        maxlength = ce.typehint.get("maxlength") or "0"
        doSimpleElement("maxLength", element, maxlength, False)

def groupToXML(group, parent):
    # Remove XML elements that are not in group, unless parent is splice_meta_config
    # since this script only handles service-config and not the entire config tree...

    if parent.tag != "splice_meta_config":
        for e in parent.getchildren():
            if ("name" in e.attrib and
                not e.tag.startswith("attribute")):
                ce = filter(lambda x: x.name == e.attrib["name"], group)[:1]
                if len(ce) == 0:
                    print("Remove {}[@name='{}'] {}".format(e.tag, e.attrib["name"], parent.tag))
                    parent.remove(e)

    for ce in group:
        if not ce.name in blacklist:
            # element or leaf
            tagname = "element" if ce.children else "leaf{}".format(ce.typehint["xmltype"])
            expr = "./{}[@name='{}']".format(tagname, ce.name)
            element = findOrCreateElement(expr, parent, tagname, ce.name)
            # version attr
            if "version" in element.attrib and element.attrib["version"] != VERSION:
                print("Replace version attribute {} -> {}".format(element.attrib["version"], VERSION))
            element.attrib["version"] = VERSION
            # minOccurrences attr
            if ce.multiplicity == 1:
                if ce.children or ce.attributes:
                    min = 0
                elif not ce.default:
                    min = 1
                else:
                    min = 0
            else:
                min = ce.multiplicity
            if "minOccurrences" in element.attrib and element.attrib["minOccurrences"] != str(min):
                print("Replace minOccurrences attribute {} -> {}".format(element.attrib["minOccurrences"],
                                                                         min))
            element.attrib["minOccurrences"] = str(min)
            # maxOccurrences attr
            if "maxOccurrences" in element.attrib and element.attrib["maxOccurrences"] != str(ce.multiplicity):
                print("Replace maxOccurrences attribute {} -> {}".format(element.attrib["maxOccurrences"],
                                                                         ce.multiplicity))
            element.attrib["maxOccurrences"] = str(ce.multiplicity)

            # comment child
            doSimpleElement("comment", element, ce.description, True)

            # attribute childs
            # remove unused from XML
            for e in element.getchildren():
                if (e.tag.startswith("attribute") and
                    "name" in e.attrib):
                    attr_ce = filter(lambda x: (e.tag == "attribute{}".format(x.typehint["xmltype"]) and
                                            e.attrib["name"] == x.name), ce.attributes)[:1]
                    if len(attr_ce) == 0:
                        print("Remove {}[@name={}]".format(e.tag, e.attrib["name"]))
                        element.remove(e)
            if ce.attributes:
                for attr_ce in ce.attributes:
                    # attribute element
                    tagname = "attribute{}".format(attr_ce.typehint["xmltype"])
                    expr = "./{}[@name='{}']".format(tagname, attr_ce.name)
                    attr_element = findOrCreateElement(expr, element, tagname, attr_ce.name)
                    # version attr
                    if "version" in attr_element.attrib and attr_element.attrib["version"] != VERSION:
                        print("Replace version attribute {} -> {}".format(attr_element.attrib["version"],
                                                                          VERSION))
                    attr_element.attrib["version"] = VERSION
                    # required attr
                    required = "true" if attr_ce.multiplicity > 0 else "false"
                    if "required" in attr_element.attrib and attr_element.attrib["required"] != required:
                        print("Replace required attribute {} -> {}".format(attr_element.attrib["required"],
                                                                           required))
                    attr_element.attrib["required"] = required

                    # comment
                    doSimpleElement("comment", attr_element, attr_ce.description, True)

                    # remaining (leaf-specific) elements
                    doLeafElement(attr_element, attr_ce)

            if not ce.children:
                # Remove existing element childs
                r = element.xpath("./element")
                if r:
                    for e in r:
                        print("CHILD Remove {}[@name={}]".format(child.tag, child.attrib["name"]))
                        element.remove(e)

                # remaining (leaf-specific) elements
                doLeafElement(element, ce)

            else:
                groupToXML(ce.children, element)
        else:
            print("Skipping blacklisted cfgelem '{}'".format(ce.name))

def extract(translation_unit):
    groups = {} # key: group-name, value: list of CfgElem

    for cursor in tu.cursor.get_children():
        # Filter variable declarations of type 'const struct cfgelem'
        if (cursor.kind == CursorKind.VAR_DECL and
            cursor.type.spelling.startswith("const struct cfgelem")):

            cfgelem_decl = list(cursor.get_children())
            # expected: cfgelem typeref + initializer-list expr
            assert len(cfgelem_decl) == 2, \
                "expected two children, got {}".format(len(cfgelem_decl))
            assert cfgelem_decl[0].kind == CursorKind.TYPE_REF, \
                "expected first child to be type-ref, got {}".format(cfgelem_decl[0].kind)
            assert cfgelem_decl[0].spelling == "struct cfgelem", \
                "expected struct cfgelem type-ref, got '{}'".format(cfgelem_decl[0].spelling or
                                                                    cfgelem_decl[0].displayname)

            for elem in cfgelem_decl[1].get_children():
                if elem.kind == CursorKind.INIT_LIST_EXPR:
                    #print("Processing member of cfgelem group '{}'".format(cursor.spelling))

                    # Find existing group or create a new empty group
                    if cursor.spelling not in groups:
                        groups[cursor.spelling] = []
                    group = groups[cursor.spelling]

                    members = list(elem.get_children())
                    assert len(members) == 12, \
                        "expected struct with 12 members, got {} members".format(len(members))

                    # const char *name
                    member = members[0].get_children().next().get_children().next()
                    if member.kind == CursorKind.STRING_LITERAL:
                        name = member.spelling
                        if name == "\"*\"":
                            #print("Skipping wildcard")
                            continue
                    elif member.kind == CursorKind.CSTYLE_CAST_EXPR:
                        # assume END_MARKER, anything else should have a name
                        #print("Skipping end-marker")
                        continue
                    else:
                        assert False, \
                            "Unsupported member kind {}".format(member.kind)
                    ce = CfgElem(name)

                    # const struct cfgelem *children
                    member = members[1].get_children().next()
                    if member.spelling:
                        assert member.spelling != cursor.spelling, \
                            "cfgelem '{}' cannot be in group '{}' (must be unique)".format(member.spelling,
                                                                                           cursor.spelling)
                        assert member.spelling in groups, \
                            "element group {} is unknown".format(member.spelling)
                        ce.children = groups.pop(member.spelling)
                    else:
                        ce.children = None # This cfgelem has no children

                    # const struct cfgelem *attributes
                    member = members[2].get_children().next()
                    if member.spelling:
                        assert member.spelling != cursor.spelling, \
                            "cfgelem '{}' cannot be in group '{}' (must be unique)".format(member.spelling,
                                                                                           cursor.spelling)
                        assert member.spelling in groups, \
                            "attribute group {} is unknown".format(member.spelling)
                        ce.attributes = groups.pop(member.spelling)
                    else:
                        ce.attributes = None # This cfgelem has no attributes

                    # int multiplicity
                    member = members[3]
                    assert member.kind == CursorKind.INTEGER_LITERAL, \
                        "expected multiplicity integer-literal, member has kind {}".format(member.kind)
                    ce.multiplicity = list(member.get_tokens())[0].spelling

                    # const char *default
                    member = members[4].get_children().next()
                    if member.kind == CursorKind.UNEXPOSED_EXPR:
                        member = member.get_children().next()
                        assert member.kind == CursorKind.STRING_LITERAL, \
                            "expected default string-literal, member has kind {}".format(member.kind)
                        ce.default = member.spelling
                    elif member.kind == CursorKind.PAREN_EXPR:
                        # assume NULL, (0): no default
                        ce.default = None
                    else:
                        assert False, \
                            "expected default, got unsupported member kind {}".format(member.kind)

                    # int relative_offset
                    # int elem_offset
                    # init_fun_t init;

                    # update_fun_t update;
                    member = members[8].get_children().next()
                    if member.kind == CursorKind.DECL_REF_EXPR:
                        ce.typehint = member.spelling
                    elif ce.children:
                        # groups don't need a typehint
                        ce.typehint = None
                    else:
                        assert False, \
                            "expected update-func, got unsupported member kind {}".format(member.kind)

                    # free_fun_t free;
                    # print_fun_t print;

                    # const char *description;
                    member = members[11].get_children().next().get_children().next()
                    if member.kind == CursorKind.STRING_LITERAL:
                        ce.description = member.spelling
                    elif member.kind == CursorKind.CSTYLE_CAST_EXPR:
                        print("Warning: element '{}' has no description!".format(name))
                        ce.description = None

                    assert ce not in group, \
                        "element '{}' already exists in group '{}'".format(ce.name, cursor.spelling)
                    group.append(ce)

                    #print(ce)
                else:
                    #print("Unsupported kind {} at {}".format(elem.kind, elem.extent))
                    pass

    # Everything should have been merged down to one "root" cfgelem group
    assert len(groups) == 1, \
        "Single root group expected, but multiple groups retrieved: {}".format(",".join(groups.keys()))

    # Obtain the the meta-config
    assert "OSPL_HOME" in os.environ, \
        "OSPL_HOME env.var not set: unable to locate and parse OpenSplice meta-config file"

    meta_path = os.path.join(os.environ["OSPL_HOME"], "src", "tools", "cm", "config", "code")
    meta_file = os.path.join(meta_path, "splice_metaconfig_6.1.xml")

    parser = etree.XMLParser(strip_cdata=False, remove_blank_text=True)
    tree = etree.parse(meta_file, parser=parser)
    root = tree.getroot()

    group = groups[groups.keys()[0]]
    groupToXML(group, root)

    backup_file = "{}.bak.xml".format(meta_file[:-4])
    print("Rename original XML-file to {}".format(backup_file))
    os.rename(meta_file, backup_file)
    tree.write(meta_file, pretty_print=True, xml_declaration=True, encoding="UTF-8")

if __name__ == "__main__":
    includes = sys.argv[1:-1]
    cfile = sys.argv[-1]

    index = clang.cindex.Index.create()
    tu = index.parse(cfile, args=includes)

    # Fail on parse errors
    diag = list(tu.diagnostics)
    if len(diag) > 0:
        pprint(diag)
        sys.exit(1)

    #print_cfgelem_ast(tu)
    extract(tu)
