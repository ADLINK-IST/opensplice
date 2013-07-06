    This test is for Coflight test requirement eFDPfi_MW_DDS_5 of OpenSplice 
    network messages (enhancement dds2117).
    It consists of a publisher which sends a series of strings to a subscriber.

    CoFlight requirement eFDPfi_MW_DDS_5 requires that the loss of a data update diffused 
    with a nonsecured protocol (udp, ipm, MAC, LLC1?) is not considered as a fault and 
    shall be gracefully and transparently handled and overcome by the DDS within 1 second on a LAN and 5 seconds on a WAN.
########################################################################################################
Test objectives
The purpose of the tests is to show that loss of data is handled and overcome by the DDS within 1 second on a LAN and 5 seconds on a WAN.
########################################################################################################
Prerequisites, assumptions, constraints
This is a manual test run on a single node.
The platform on which the test is run is linux 64 bit. The machine used only has 5GB of RAM so swap is used to handle 6GB.
There are 2 tests performed. The first attempts to use a 5GB database and a request to populate 6GB of data - this tests a failure situation. The second test uses a 6GB database and a request to populate 6GB of data - this tests the requirement.
########################################################################################################
Building
on node
 - download and build OSPLI branch
 - download and build OSPLO branch
 - download, install, configure, start Staf and Stax
 - building  \osplo\testsuite\rbt\
 - building  \osplo\testsuite\rbt\services\networking\dds2117\
########################################################################################################
Configuration

master_host_name and slave_host_name - address of hosts 

master_ospl_root and slave_ospl_root - location on the hosts where /osplo and /ospli can be found

master_log_dir and slave_log_dir - location where the logs will be stored on the hosts

master_ospl_release_name and slave_ospl_release_name - release name of OSPLO/OSPLI on hosts

staf_port - Staf port number on hosts

network_type - Network type, L for a LAN and W for a WAN.
########################################################################################################


