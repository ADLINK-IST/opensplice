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

# The 'count_test' variable determines if generated modules from the idl file 
# will be imported or not. 
# The import will happen only if the 'count_test' variable is set to false.
# The 'count_test' variable is set to true only in the generateTestcaseList.py
# file because for generating testcase list there is no need of importing
# the idl generated modules and it happens before the generation of the modules
# from the idl file. 
# For all the other cases the count_test variable is set to false, resulting in
# import of the generated modules from the idl files.

count_test = False
