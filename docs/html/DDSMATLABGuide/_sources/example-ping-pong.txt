.. _`Ping Pong Example`:

#################
Ping Pong Example
#################

A simple ping pong example is provided to demonstrate the basic capabilities of the MATLAB DDS integration.

The ping pong example can be found in the following directory:  
    
   `OSPL_HOME`/tools/matlab/examples/matlab/pingpong

The ping pong example creates two participants. 

1. A pinger that writes to the PING partition and reads from the PONG partition.
2. A ponger that writes to the PONG partition and reads from the PING partition. 

A matlab script is provided that writes and reads sample data in the pinger and ponger participants.

Example Files
*************

Files with the .m extension are MATLAB script files.

An explanation of what each example file does is provided below.

**pingpong.idl**

- Defines the PingPongType in idl
- Used to generate the MATLAB file PingPongType.m via::
idlpp -l matlab pingpong.idl

**PingPongType.m**

- Defines a PingPongType; generated from idlpp   
- The PingPongType represents a DDS topic type.
- PingPongType specifies two properties:  id, count.

**DDS_PingerVolatileQoS.xml**

- XML file that specifies the DDS QoS (quality of service) settings for pinger.

**DDS_PongerVolatileQoS.xml**

- XML file that specifies the DDS QoS (quality of service) settings for ponger.

**setup_pinger.m**
  
- Creates the pinger participant on the default DDS domain, with specified QoS profile. 
- Creates the topic PingPongType.
- Creates the publisher using the domain participant on the PING partition specified in the specified QoS profile. 
- Creates the writer using the publisher and the specified QoS profile.
- Creates the subscriber using the domain participant on the PONG partition specified in the QoS profile.
- Creates the reader using the subscriber and the specified QoS profile.

**setup_ponger.m**

- Creates the ponger participant on default domain with specified QoS profile.
- Creates the topic PingPongType. 
- Creates the publisher using the domain participant on the PONG partition specified in the QoS profile.
- Creates the writer using the publisher and the specified QoS profile.
- Creates the subscriber using the domain participant on the PING partition specified in the QoS profile.
- Creates the reader using the subscriber and the specified QoS profile.

**pinger_ponger.m**

- MATLAB script that writes and reads sample data in the pinger and ponger participants. 
- This script calls the setup_pinger.m and setup_ponger.m scripts.
- This is the main script to run the ping pong example. 
- This script is run from the MATLAB Command Window.


Steps to run example
********************

1. In the MATLAB command window, run pinger_ponger.m.   
    - Type "pinger_ponger".
    - Hit enter.



