#!/bin/bash
TEST_ROOT=~/ospli/testsuite/tests
export TEST_ROOT

# ------------  Common variables ------------------------------
# the name for the host:
#'glasgow'
#'vm-dds-rhel53x64'
#'vm-dds-rhel53x86'
#-----------------------------------------------------------------------------------------------------------------------------------
#The host name of the machine 1
host_name1=glasgow
#Location of test case build on machine 1 
test_root1=~/log
#Location of OSPL root on machine 1 , root for ospli and osplo
ospl_root1=
#name of ospl release for host1
ospl_release_name1=x86.linux-release
export host_name1 test_root1 ospl_root1 ospl_release_name1
#-----------------------------------------------------------------------------------------------------------------------------------
#The host name of the machine 2
host_name2=vm-dds-rhel53x64
#Location of test case build on machine 2 
test_root2=~/log
#Location of OSPL root on machine 2 , root for ospli and osplo
ospl_root2=
#name of ospl release for host2
ospl_release_name2=x86_64.linux-release
export host_name2 test_root2 ospl_root2 ospl_release_name2
#-----------------------------------------------------------------------------------------------------------------------------------
#The host name of the machine 3
host_name3=vm-dds-rhel53x86
#Location of test case build on machine 3 
test_root3=~/log
#Location of OSPL root on machine 13, root for ospli and osplo
ospl_root3=
#name of ospl release for host3
ospl_release_name3=x86.linux-release
export host_name3 test_root3 ospl_root3 ospl_release_name3
#-----------------------------------------------------------------------------------------------------------------------------------
#The host name of the machine 4
host_name4=empty
#Location of test case build on machine  4
test_root4=
#Location of OSPL root on machine 4 , root for ospli and osplo
ospl_root4=
#name of ospl release for host 4
ospl_release_name4=
export host_name4 test_root4 ospl_root4 ospl_release_name4
#-----------------------------------------------------------------------------------------------------------------------------------
#The host name of the machine 5 
host_name5=empty
#Location of test case build on machine  5
test_root5=
#Location of OSPL root on machine , root for ospli and osplo
ospl_root5=
#name of ospl release for host 5
ospl_release_name5=
#-----------------------------------------------------------------------------------------------------------------------------------
#The host name of the machine 6 
host_name6=empty
#Location of test case build on machine  6
test_root6=
#Location of OSPL root on machine , root for ospli and osplo
ospl_root6=
#name of ospl release for host 6
ospl_release_name6=
#-----------------------------------------------------------------------------------------------------------------------------------
#The host name of the machine 7
host_name7=empty
#Location of test case build on machine  7
test_root7=
#Location of OSPL root on machine , root for ospli and osplo
ospl_root7=
#name of ospl release for host 7
ospl_release_name7=
#-----------------------------------------------------------------------------------------------------------------------------------
#The host name of the machine 8
host_name8=empty
#Location of test case build on machine  8
test_root8=
#Location of OSPL root on machine , root for ospli and osplo 8
ospl_root8=
#name of ospl release for host
ospl_release_name8=
#-----------------------------------------------------------------------------------------------------------------------------------
      
staf_port_1=6505
staf_port_2=6503
staf_port_3=6504
staf_port_4=6500
staf_port_5=6501
staf_port_6=6502
staf_port_7=6506
staf_port_8=6507



. ~/staf342/STAFEnv.sh
