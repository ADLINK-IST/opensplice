This test is for Coflight test requirement eFDPfi_MW_DDS_31 Distinguish and filter (by means of hardware routers) each topic or group of topics (each domain) 
########################################################################################################
Test objectives
NetworkPartitions and mapping them on logical partition/topic combinations supports the ‘assignment’ of HW-filtered multicast-groups to individual topics. To prove this feature is working, a multi-node test is done in which we look at the data on the wire to prove that data is not received on a specific node. For example, 2 nodes which are separated by a router that filters the multicast messages used by the domain. Each node only receives messages from its subscribed multicast address. The rest will be filtered out by the router. 

########################################################################################################
Prerequisite, assumptions, constraints


The HW platform consist of 3 Linux nodes, named "A", "B" and "C" in this description. Each node has a different configuration file.

A has NodeA.xml Configure a mulit-cast address M1 for the GlobalPartition and additionally create two NetworkPartitions, one for PartitionP.B and one for PartitionP.C and assign different multi-cast address M2 and M3 to them respectively.

    * M1 = 230.230.230.1
    * M2 = 230.230.230.2
    * M3 = 230.230.230.3 

B has NodeB.xml Configure a mulit-cast address M1 for the GlobalPartition and additionally create ONE NetworkPartition for PartitionP.B and assign multi-cast address M2 to it.

    * M1 = 230.230.230.1
    * M2 = 230.230.230.2 

C has NodeC.xml Configure a mulit-cast address M1 for the GlobalPartition and additionally create ONE NetworkPartition for PartitionP.C and assign multi-cast address M3 to it.

    * M1 = 230.230.230.1
    * M3 = 230.230.230.3 

There are 2 programs used during this test. The "dds2368_sub" program as subscriber to all partition/topic combinations. And the "dds2368_pub" program as publisher for the different partition/topic combinations. The dds2368_pub program can be used with 3 parameters:

    * 'a' only write to topic A
    * 'b' only write to topic B
    * 'c' only write to topic C
    * if no parameter is given a write will be done to all topics (A,B and C) 

For validating the output on the wire iptables will be used for logging. Iptables configuration for logging network traffic:

    * Iptables rule for Node A: iptables -A OUTPUT -j LOG -d 230.230.230.0/24 ,the output from this log can be found in /var/log/messages 
########################################################################################################
Building
on each of the three nodes
 - download and build OSPLI branch
 - download and build OSPLO branch
 - download and install OSPL HDE (only in case if you want to use HDE key for OSPL startup)
 - download, install, configure, start Staf and Stax
 - building test in \osplo\testsuite\rbt\services\networking\dds2368\
 - install Iptables on master host (host1) and add startup iptables directory to PATH 
 - run run_dds2368_test.xml script on Stax monitor on master host (host1) starting with root access (cause IpTables works only with root access)
 - see analyze.txt on test_root1 on master host (host1)
########################################################################################################
Startup and configuration
'host_name1','ospl_start', 'test_root1', 'osplo_home1', 'ospli_home1', 'ospl_release_name1', 'ospl_inst_home1', 'host_name2', 'test_root2', 'osplo_home2', 'ospli_home2', 'ospl_release_name2', 'ospl_inst_home2', 'host_name3', 'test_root3', 'osplo_home3', 'ospli_home3', 'ospl_release_name3', 'ospl_inst_home3', 'staf_port_1', 'staf_port_2', 'staf_port_3'

host_name1 - address of first host (master host, on this machine will be copied all logs in test_root1 folder and on this machine should be installed Stax)
host_name2 - address of second host
host_name3 - address of third host

ospl_start - key for ospl startup  = ('SRC': for start from OSPLI directory) | ('HDE': for start from HDE install directory, by default)

test_root1 - test log directory on first host (master) (host1)
test_root2 - test log directory on second host
test_root3 - test log directory on third host

osplo_home1 - Location of OSPLO branch on first host
osplo_home2 - Location of OSPLO branch on second host
osplo_home2 - Location of OSPLO branch on third host

ospli_home1 - Location of OSPLI branch on first host
ospli_home2 - Location of OSPLI branch on second host
ospli_home3 - Location of OSPLI branch on third host

ospl_release_name1 - Release name of OSPLO/OSPLI on first host
ospl_release_name2 - Release name of OSPLO/OSPLI on second host
ospl_release_name3 - Release name of OSPLO/OSPLI on third host

ospl_inst_home1 - Location of OSPL HDE installation folder on first host
ospl_inst_home2 - Location of OSPL HDE installation folder on second host
ospl_inst_home3 - Location of OSPL HDE installation folder on third host

staf_port_1 - Staf port number on first host
staf_port_2 - Staf port number on second host
staf_port_3 - Staf port number on third host
########################################################################################################


