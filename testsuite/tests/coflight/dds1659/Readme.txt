This test is for Coflight test requirement DDS_22 of OpenSplice Coflight req. eFDPfi_MW_DDS_22
########################################################################################################
Test objectives
Verify the behaviour of the system when two nodes differ in the amount of data they have received over a reliable channel, when the node that publishes this data crashes.

########################################################################################################
Prerequisite, assumptions, constraints

The HW platform consist of 3 Linux nodes, named "publisher", "subscriber" and "lossy" in this description. The nodes run a "development-build" version of opensplice. There is one topic used, consisting of 1 integer value, that is configured with "reliable"-reliability and "keep_all"-history. On "publisher" a writer "pub" is started that writes the topic with a frequency of 10Hz, and an incremented value of the content for each sample. On both "subscriber" and "lossy" a reader "sub" that takes incoming samples and outputs their arrival for inspection. The configuration of the nodes is such, that all the testdata flows through the same channel in the default partition.

To be able to create a condition where the receiving nodes differ in the amount of data they have received, the following special configurations are made:

    * "lossy" is configured to drop every 6th incoming packet on every channel. (Networkservice>Debugging>Lossy>Receiving in the config xml-file)
    * "producer" is configured with a resend interval of 1 second. (Networkservice>Channels>Channel>Sending>Recoveryfactor in the config xml-file) 

In combination with the 10 hz datarate, this means that every 600ms a packet is dropped at "lossy", which is only resend by "publisher" a second after the original write. Thus "lossy" will always be waiting for a missed packet, while "subscriber" is expected to receive all packets immediatly.

The test verifies the behaviour both with and without the "reliability under publisher crash" functionality enabled. Two sets of configuration files are supplied; the "rupc_off" variants don't have the functionality enabled, while the "rupc_on" config do have it enabled. 

########################################################################################################
Building
on each of the three nodes
 - download and build OSPLI branch
 - download and build OSPLO branch
 - download and install OSPL HDE (only in case if you want to use HDE key for OSPL startup)
 - download, install, configure, start Staf and Stax
 - building test in \osplo\testsuite\rbt\services\networking\dds1659\
 - setup JAVA_HOME environment variables
 - run run_dds1659_test.xml script on Stax monitor
 - see analyze.txt on test_root1 on master host
########################################################################################################
Startup and configuration
'host_name1','ospl_start', 'test_root1', 'osplo_home1', 'ospli_home1', 'ospl_release_name1', 'ospl_inst_home1', 'host_name2', 'test_root2', 'osplo_home2', 'ospli_home2', 'ospl_release_name2', 'ospl_inst_home2', 'host_name3', 'test_root3', 'osplo_home3', 'ospli_home3', 'ospl_release_name3', 'ospl_inst_home3', 'staf_port_1', 'staf_port_2', 'staf_port_3'

host_name1 - address of first host (master host, on this machine will be copied all logs in test_root1 folder and on this machine should be installed Stax)
host_name2 - address of second host
host_name3 - address of third host

ospl_start - key for ospl startup  = ('SRC': for start from OSPLI directory) | ('HDE': for start from HDE install directory, by default)

test_root1 - test log directory on first host
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


