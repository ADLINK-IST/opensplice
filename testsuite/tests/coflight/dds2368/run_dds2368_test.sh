#!/bin/bash

. setup_test_variables.sh

echo ""
echo "-------------------------------"
echo "  <<< Start DDS 2368 test >>> "
STAF local STAX EXECUTE FILE $TEST_ROOT/CoFlight/dds2368/run_dds2368_test.xml JOBNAME "DDS 2368" FUNCTION dds2368_main ARGS "['$host_name1','$ospl_start', '$test_root1', '$osplo_home1', '$ospli_home1', '$ospl_release_name1', '$ospl_inst_home1', '$host_name2', '$test_root2', '$osplo_home2', '$ospli_home2', '$ospl_release_name2', '$ospl_inst_home2', '$host_name3', '$test_root3', '$osplo_home3', '$ospli_home3', '$ospl_release_name3', '$ospl_inst_home3', '$staf_port_1', '$staf_port_2', '$staf_port_3']" WAIT RETURNRESULT DETAILS
