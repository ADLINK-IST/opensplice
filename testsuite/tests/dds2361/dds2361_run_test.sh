#!/bin/sh
# Set the names of the hosts for the test:
export host1_name=""
export host2_name=""
export host3_name=""
export host4_name=""
# Set the langs of the hosts for the test ('c', 'cpp', 'cs', 'java'):
export host1_lang=""
export host2_lang=""
export host3_lang=""
export host4_lang=""
# Set the OSPL HOME - distribution location on the hosts:
export host1_ospl_home=""
export host2_ospl_home=""
export host3_ospl_home=""
export host4_ospl_home=""
# Set ospl_src_home - the path where osplo and ospli could be found on the host:
export host1_ospl_src_home=""
export host2_ospl_src_home=""
export host3_ospl_src_home=""
export host4_ospl_src_home=""
# Set test_framework_root
# Absolute path of stax framework root on the local host in double quotes and double backslash.
export test_framework_root=""
# Set location where log dir will be stored on the local host
export log_dir=""
# STAX script file name:
RUN_TEST_XML=$PWD/dds2361_run_test.xml
echo "Starting dds2361 test..."
STAF local STAX EXECUTE FILE $RUN_TEST_XML JOBNAME "dds2361" FUNCTION dds2361_main ARGS "['$host1_name', '$host2_name', 
'$host3_name', '$host4_name', '$host1_lang', '$host2_lang', '$host3_lang', '$host4_lang', '$host1_ospl_home', '$host2_ospl_home', 
'$host3_ospl_home', '$host4_ospl_home', '$host1_ospl_src_home', '$host2_ospl_src_home', '$host3_ospl_src_home', '$host4_ospl_src_home', 
'$test_framework_root', '$log_dir']" WAIT RETURNRESULT DETAILS 
echo "Done."