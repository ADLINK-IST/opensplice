#!/bin/sh

# Set the name of the hosts for the test:
export HOST_NAME1=""
export HOST_NAME2=""

# Set the OSPL HOME - distribution location on the hosts:
export OSPL_HOME1=""
export OSPL_HOME2=""

# Set the OSPL source code location on the hosts (where ospli/osplo stored):
export OSPL_SRC1=""
export OSPL_SRC2=""

# Set the TAO_ROOT for the hosts:
export TAO_ROOT1=""    
export TAO_ROOT2=""

# Set the STAF port for the hosts:
export STAF_PORT1=6500
export STAF_PORT2=6500

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

echo "Starting dds2466 test..."
STAF local STAX EXECUTE FILE $RUN_TEST_XML JOBNAME "dds2466" FUNCTION dds2466_main ARGS "['$HOST_NAME1','$HOST_NAME2', '$OSPL_HOME1', '$OSPL_HOME2', '$OSPL_SRC1', '$OSPL_SRC2', '$TAO_ROOT1', '$TAO_ROOT2', '$STAF_PORT1', '$STAF_PORT2']" WAIT
echo "Done."
