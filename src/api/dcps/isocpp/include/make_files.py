#!/usr/bin/python

import os, fnmatch, errno

matches = []

for path, dirs, files in os.walk("./spec"):
    for filename in fnmatch.filter(files, "*.hpp"):
        matches.append(os.path.join(path, filename))

for match in matches:
    filename = match[2:]
    print "Wugga " + filename
    new_filename = filename[5:]
    implementation = ""
    try:
        myfile = open(new_filename, "r")
        existing_contents = myfile.read()
        search_string = "// Implementation\n"
        if search_string in existing_contents:
            implementation = existing_contents[existing_contents.find(search_string) + len(search_string) : existing_contents.rfind("// End of implementation\n")]
        myfile.close()
    except IOError as e:
        pass
    try:
        os.makedirs(os.path.dirname(new_filename))
    except OSError as exc:
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise
    myfile = open(new_filename, "w")
    myfile.write("/*\n\
 *                         Vortex OpenSplice\n\
 *\n\
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech\n\
 *   Limited, its affiliated companies and licensors. All rights reserved.\n\
 *\n\
 *   Licensed under the Apache License, Version 2.0 (the "License");\n\
 *   you may not use this file except in compliance with the License.\n\
 *   You may obtain a copy of the License at\n\
 *\n\
 *       http://www.apache.org/licenses/LICENSE-2.0\n\
 *\n\
 *   Unless required by applicable law or agreed to in writing, software\n\
 *   distributed under the License is distributed on an "AS IS" BASIS,\n\
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n\
 *   See the License for the specific language governing permissions and\n\
 *   limitations under the License.\n\
 *\n\
 */\n")
    mymacro = new_filename.upper().replace("/", "_").replace(".", "_")
    myfile.write("#ifndef OSPL_" + mymacro + "_\n")
    myfile.write("#define OSPL_" + mymacro + "_\n")
    myfile.write("\n")
    myfile.write("/**\n")
    myfile.write(" * @file
    myfile.write(" */\n")
    myfile.write("\n")
    if not "detail" in new_filename:
        myfile.write("/*\n")
        myfile.write(" * OMG PSM class declaration\n")
        myfile.write(" */\n")
        myfile.write("#include <" + filename + ">\n")
        myfile.write("\n")
    myfile.write("// Implementation\n")
    if "detail" in new_filename:
        try:
            detailfile = open(filename, "r")
            existing_contents = detailfile.read()
            search_string = "License.\n */\n"
            if len(implementation) == 0 and search_string in existing_contents:
                implementation = existing_contents[existing_contents.find(search_string) + len(search_string) : existing_contents.rfind("#endif")]
                implementation = implementation.replace("foo::bar::", "org::opensplice::").replace("foo/bar", "org/opensplice")
            detailfile.close()
        except IOError as e:
            print "Aiee! No detail file " + filename
    myfile.write(implementation)
    myfile.write("// End of implementation\n")
    myfile.write("\n")
    myfile.write("#endif /* OSPL_" + mymacro + "_ */\n")
    myfile.close()
