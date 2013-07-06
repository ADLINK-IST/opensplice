#!/bin/bash

. ~/ospldev/setup_test_variables.sh

echo ""
echo "-------------------------------"
echo "  <<< Start DDS 1581 test >>> "
STAF local STAX EXECUTE FILE $TEST_ROOT/dds1581/run_dds1581_test.xml JOBNAME "DDS 1581" FUNCTION dds1581_main ARGS "['$host_name1', '$test_root1', '$ospl_root1', '$ospl_release_name1','$host_name2', '$test_root2', '$ospl_root2', '$ospl_release_name2','$host_name3', '$test_root3', '$ospl_root3', '$ospl_release_name3', '$host_name4', '$test_root4', '$ospl_root4', '$ospl_release_name4', '$host_name5', '$test_root5', '$ospl_root5', '$ospl_release_name5', '$host_name6', '$test_root6', '$ospl_root6', '$ospl_release_name6', '$host_name7', '$test_root7', '$ospl_root7', '$ospl_release_name7', '$host_name8', '$test_root8', '$ospl_root8', '$ospl_release_name8', '$staf_port_1', '$staf_port_2', '$staf_port_3', '$staf_port_4', '$staf_port_5', '$staf_port_6', '$staf_port_7', '$staf_port_8'  ]" WAIT RETURNRESULT DETAILS
