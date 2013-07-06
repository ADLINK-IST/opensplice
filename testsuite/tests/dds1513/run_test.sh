#!/bin/sh

# Set the IP addresses of the hosts for the test:
export host1_name=""
export host2_name=""

# Set the OSPL HOME - distribution location on the hosts:
export host1_ospl_home=""
export host2_ospl_home=""

# Set the OSPL SRC HOME - Location on the host1 where /osplo and /ospli can be found
export host1_ospl_src_home=""
export host2_ospl_src_home=""

# TAO_ROOT on the hosts
export host1_tao_root=""
export host2_tao_root=""

# The location where the logs will be stored on the master host
export log_dir=""

# Absolute path of stax framework root on the local host in double quotes and double backslash.
# By default it this_xml_folder/../common.
export test_framework_root="'../common'"


# STAX script file name:
RUN_TEST_XML=$PWD/dds1513_run_test.xml

# CYGWIN case handler:
case `uname` in
    CYGWIN_NT*)
        RUN_TEST_XML=`cygpath -w $RUN_TEST_XML`
        ;;
    *)
        ;;
esac

echo "Starting dds1513 test..."
STAF local STAX EXECUTE FILE $RUN_TEST_XML JOBNAME "dds1513" FUNCTION dds1513_main ARGS "['$host1_name', '$host2_name', '$host1_ospl_home', '$host2_ospl_home', '$host1_ospl_src_home', '$host2_ospl_src_home', '$host1_tao_root', '$host2_tao_root', '$log_dir', '$test_framework_root']" WAIT
echo "Done."
