#!/usr/bin/python

import xml.etree.ElementTree as ET
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("input", help="Input file")
parser.add_argument("output", help="Output file")
args = parser.parse_args()

tree = ET.parse(args.input)
parent_map = {c:p for p in tree.iter() for c in p}
root = tree.getroot()
rootName = ""

file = open(args.output, "wb")
file.write(".. sectnum::\n\t:start: 2\n\n")

underline = ['#', '*', '=', '-', '^', '~', '+', '%']

def process_comment(string):

    #Fix for backslashes in paths
    string = string.replace("\\", "\\\\")

    #*'s in text need to be escaped before the html
    #notation is changed
    string = string.replace("*", "\*")

    #paragraphs
    string = string.replace("<p>", "")
    string = string.replace("</p>", "\n")

    #bold
    string = string.replace("<b>", "**")
    string = string.replace("</b>", "**")

    #italics
    string = string.replace("<i>", "*")
    string = string.replace("</i>", "*")

    #superscript
    string = string.replace("<sup>", " :sup:`")
    string = string.replace("</sup>", "`")

    #lists
    string = string.replace("<ul>", "")
    string = string.replace("</ul>", "\n")
    string = string.replace("<li>", "- ")
    string = string.replace("</li>", "")

    #line breaks
    string = string.replace("<br>", "\n")
    string = string.replace("<br/>", "\n")

    string = string.rstrip()

    return string

def get_full_path(element, string=""):
    if element.tag.startswith("rootElement"):
       global rootName
       rootName = "//" + element.get("name")
    elif element in parent_map:
        string = get_full_path(parent_map[element], string)
    else:
        string = rootName
    if element.get("name") is not None:
        if element.tag.startswith("attribute"):
           string = string + "[@"
           string = string + element.get("name")
           string = string + "]"
        else:
           if string == "":
              string = "//"
           else:
              string = string + "/"
           string = string + element.get("name")
    return string

def get_format(element):
    if element.find("format") is not None:
        return element.find("format").text
    elif element.tag == "attributeInt" or element.tag == "leafInt":
        return "integer"
    elif element.tag == "attributeBoolean" or element.tag == "leafBoolean":
        return "boolean"
    elif element.tag == "attributeString" or element.tag == "leafString":
        return "string"
    elif element.tag == "attributeEnum" or element.tag == "leafEnum":
        return "enumeration"
    return None

def get_dimension(element):
    if element.find("dimension") is not None:
        return element.find("dimension").text
    else:
        return None

def get_default_value(element):
    if element.find("default") is not None:
        if element.find("default").text == None:
            if element.get("required") is not None:
                if element.get("required") == "true":
                    return "n/a"
                else:
                    return '""'
        else:
            return element.find("default").text
    else:
        return None

def get_valid_values(element):
    if element.find("value") is not None:
        string = ""
        for value in element.findall("value"):
            string = string + value.text + ", "
        if len(string) is not 0:
            return string[:len(string) - 2]
        else:
            return None
    if element.find("minimum") is not None or element.find("maximum") is not None:
        string = ""
        if element.find("minimum") is not None:
           string = element.find("minimum").text + " / "
        else:
           string = " - / "
        if element.find("maximum") is not None:
           string = string + element.find("maximum").text
        else:
           string = string +" -"
        return string
    else:
        return None

def get_occurrences(element):
    min_ocurrences = element.get("minOccurrences")
    max_ocurrences = element.get("maxOccurrences")
    if min_ocurrences is not None and max_ocurrences is not None:
        if max_ocurrences == "0":
            return min_ocurrences + "-*"
        else:
            return min_ocurrences + "-" + max_ocurrences
    else:
        return None

def get_child_elements(element):
    string = ""
    for child in element:
        if child.get("name") is not None:
            if child.tag.startswith("leaf"):
                string = string + child.get("name") + ", "
    if len(string) is not 0:
        return string[:len(string) - 2]
    else:
        return None

def get_required_attributes(element):
    string = ""
    for child in element:
        if child.get("name") is not None:
            if child.tag.startswith("attribute"):
                if child.get("required") is not None and child.get("required") == "true":
                    string = string + child.get("name") + ", "
    if len(string) is not 0:
        return string[:len(string) - 2]
    else:
        return None

def get_optional_attributes(element):
    string = ""
    for child in element:
        if child.get("name") is not None:
            if child.tag.startswith("attribute"):
                if child.get("required") is None or child.get("required") == "false":
                    string = string + child.get("name") + ", "
    if len(string) is not 0:
        return string[:len(string) - 2]
    else:
        return None

def get_required(element):
    if element.tag.startswith("attribute"):
        if element.get("required") is not None:
            if element.get("required") == "true":
                return "true"
        return "false"
    return None

def output_elements(element, depth):
    if element.get("hidden", default=False):
        print("Skipping hidden element '{}'".format(element.tag))
    else:
        name = element.get("name")
        if name is not None:
            if name == "Address":
                print "Address"
            file.write(name + "\n")
            file.write(underline[depth] * len(name) + "\n")
            if element.find("comment") is not None:
                file.write(process_comment(element.find("comment").text) + "\n\n")
            file.write("- Full path: " + get_full_path(element) + "\n")
            if get_format(element) is not None:
                file.write("- Format: " + get_format(element) + "\n")
            if get_dimension(element) is not None:
                file.write("- Dimension: " + get_dimension(element) + "\n")
            if get_default_value(element):
                defstring = get_default_value(element)
                if defstring == "__BUILT-IN PARTITION__":
                    defstring = "__BUILT-IN PARTITION\__"
                file.write("- Default value: " + defstring + "\n")
            if get_valid_values(element) is not None:
                file.write("- Valid values: " + get_valid_values(element) + "\n")
            if get_occurrences(element) is not None:
                file.write("- Occurrences min-max: " + get_occurrences(element) + "\n")
            if get_child_elements(element) is not None:
                file.write("- Child elements: " + get_child_elements(element) + "\n")
            if get_required_attributes(element) is not None:
                file.write("- Required attributes: " + get_required_attributes(element) + "\n")
            if get_optional_attributes(element) is not None:
                file.write("- Optional attributes: " + get_optional_attributes(element) + "\n")
            if get_required(element) is not None:
                file.write("- Required: " + get_required(element) + "\n")
            file.write("\n")

        for child in element:
            output_elements(child, depth + 1)


output_elements(root, 0)
