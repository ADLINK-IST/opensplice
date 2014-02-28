#!/bin/bash
echo ""
echo "-------------------------------"
echo "  <<< Start setting DDS 2368 test vars >>> "
PREFIX=$(pwd)
TEST_ROOT=~/ospli/testsuite/tests
export TEST_ROOT

# ------------  Common variables ------------------------------
# the name for the host:
#'vm-dds-rhel53x64'
#'glasgow'
#'vm-dds-rhel53x86'
#-----------------------------------------------------------------------------------------------------------------------------------
#The host name of the machine 1
host_name1=vm-dds-rhel53x64
#Location of test case build on machine 1 
test_root1=~/log
#Location of OSPL root on machine 1 , root for ospli and osplo
osplo_home1=~/osplo
ospli_home1=~/ospli
ospl_inst_home1=~/PrismTech/OpenSpliceDDS/V5.4.1/HDE/x86_64.linux
#name of ospl release for host1
ospl_release_name1=x86_64.linux-dev
ospl_start=HDE
export ospl_start host_name1 test_root1 osplo_home1 ospli_home1 ospl_inst_home1 ospl_release_name1 
#-----------------------------------------------------------------------------------------------------------------------------------
#The host name of the machine 2
host_name2=glasgow
#Location of test case build on machine 2 
test_root2=~/log
#Location of OSPL root on machine 2 , root for ospli and osplo
osplo_home2=~/osplo
ospli_home2=~/ospli
ospl_inst_home2=~/PrismTech/OpenSpliceDDS/V5.4.1/HDE/x86.linux
#name of ospl release for host2
ospl_release_name2=x86.linux-dev
export host_name2 test_root2 osplo_home2 ospli_home2 ospl_inst_home2 ospl_release_name2
#-----------------------------------------------------------------------------------------------------------------------------------
#The host name of the machine 3
host_name3=vm-dds-rhel53x86
#Location of test case build on machine 3
test_root3=~/log
#Location of OSPL root on machine 3, root for ospli and osplo
osplo_home3=~/osplo
ospli_home3=~/ospli
ospl_inst_home3=~/PrismTech/OpenSpliceDDS/V5.4.0/HDE/x86.linux
#name of ospl release for host3
ospl_release_name3=x86.linux-dev
export host_name3 test_root3 osplo_home3 ospli_home3 ospl_inst_home3 ospl_release_name3
#-----------------------------------------------------------------------------------------------------------------------------------

      
staf_port_1=6503
staf_port_2=6505
staf_port_3=6504
export staf_port_1 staf_port_2 staf_port_3

. ~/staf/STAFEnv.sh

echo "  <<< End setting DDS 2368 test vars >>> "
echo "-------------------------------"
