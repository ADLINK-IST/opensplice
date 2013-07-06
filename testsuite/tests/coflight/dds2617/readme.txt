    This test is for Coflight test requirement eFDPfi_MW_GEN_3.

    CoFlight requirement eFDPfi_MW_GEN_3 requires that on a given platform, 
    the CoFlight eFDP middleware shall support multiple, 
    and running simultaneously, instances of a system. 
########################################################################################################
Test description:
    To test this requirement install 3 instances of OpenSplice DDS. 
    Change the port numbers on the Network Service to ensure that each installation is using different port numbers.
    Build a PingPong example in each of the installations.
    Run the PingPong example in each of the installations using the RUN script, 
    making sure that all installations are running the PingPong example simultaneously.

The expected test results:
        * thePingPong example will run successfully for each of the installations
########################################################################################################
Configuration

host_name - address of host 

ospl_home1, ospl_home2 and ospl_home3 - location on the host where ospls have been installed

log_dir - location where the logs will be stored on the host

operation_key1 and operation_key2 - launch test on selected language
key format:
operation_key1 and operation_key2 = Java | C | C++ | C++OnC | OF_C++ | JO_Java | RTO_Java 
operation_key1(optional operation_key2 in this case is not required) = CORBA  | STANDALONE  | ALL | ALL_EXT
if operation_key1 is (operation_key2 in this case is not required)
STANDALONE  - launch all combinations of standalone tests
CORBA       - launch all combinations of corba tests
ALL         - launch all combinations of corba and standalone tests
ALL_EXT     - launch all of corba and standalone tests without combinations

staf_port - Staf port number on host
########################################################################################################
