#!/bin/sh

# Set the IP addresses of the hosts for the test:
export HOST_IP1=""
export HOST_IP2=""
export HOST_IP3=""

# Set the OSPL HOME - distribution location on the hosts:
export OSPL_HOME1=""
export OSPL_HOME2=""
export OSPL_HOME3=""

# Set the STAF port for the hosts:
export STAF_PORT1=6500
export STAF_PORT2=6500
export STAF_PORT3=6500

# STAX script file name:
RUN_TEST_XML=$PWD/run_test.xml

# CYGWIN case handler:
case `uname` in
    CYGWIN_NT*)
        RUN_TEST_XML=`cygpath -w $RUN_TEST_XML`
        ;;
    *)
        ;;
esac

echo "Starting dds2734 test..."
STAF local STAX EXECUTE FILE $RUN_TEST_XML JOBNAME "dds2734" FUNCTION dds2734_main ARGS "['$HOST_NAME1', '$HOST_NAME2', '$HOST_NAME3', '$OSPL_HOME1', '$OSPL_HOME2', '$OSPL_HOME3', '$STAF_PORT1', '$STAF_PORT2', '$STAF_PORT3']" WAIT
echo "Done."
