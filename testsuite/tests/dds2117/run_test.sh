#!/bin/sh

# Set the IP addresses of the hosts for the test:
export host1_name=""
export host2_name=""

# Set the OSPL HOME - distribution location on the hosts:
export host1_ospl_home=""
export host2_ospl_home=""

# Set application exec dir hosts
export application_exec_dir_on_host1=""
export application_exec_dir_on_host2=""

# Set the OSPL SRC HOME - Location on the host1 where /osplo and /ospli can be found
export host1_ospl_src_home=""
export host2_ospl_src_home=""

# The location where the logs will be stored on the master host
export log_dir=""

# Indicate the type of network we are running on.
# It is L for LAN and W for WAN.
export network_type="L"

# Absolute path of stax framework root on the local host in double quotes and double backslash.
# By default it this_xml_folder/../common.
export test_framework_root="'../common'"


# STAX script file name:
RUN_TEST_XML=$PWD/dds2117_run_test.xml

# CYGWIN case handler:
case `uname` in
    CYGWIN_NT*)
        RUN_TEST_XML=`cygpath -w $RUN_TEST_XML`
        ;;
    *)
        ;;
esac

echo "Starting dds2117 test..."
STAF local STAX EXECUTE FILE $RUN_TEST_XML JOBNAME "dds2117" FUNCTION dds2117_main ARGS "['$host1_name', '$host2_name', '$host1_ospl_home', '$host2_ospl_home', '$application_exec_dir_on_host1', '$application_exec_dir_on_host2', '$host1_ospl_src_home', '$host2_ospl_src_home', '$log_dir', '$network_type', '$test_framework_root']" WAIT
echo "Done."
