.. _`Impact on DDS Domain`:


####################
Impact on DDS Domain
####################

*This section describes additional aspects of the Record and 
Replay Service and its interaction with other systems.*


Intrusiveness
*************

Relevant characteristics of the Record and Replay Service 
with respect to ‘intrusiveness’ for an existing system are:

• The service can be optionally configured on any DDS node in the system.

  - When run as part of an existing federation of applications, 
    it utilizes the federation’s shared-memory segment to obtain 
    the data (so locally-published data is not required to travel 
    over the network to be recorded by the service, and *vice-versa*
    for replaying towards co-located subscribers).
  - When run on a dedicated RnR node, data to be recorded is 
    transparently forwarded to that RnR node, typically using 
    multicast network features (and so not inducing extra network 
    traffic).
  
• Services are controlled in ‘the DDS way’, *i.e.* a data-centric 
  way where command and status topics allow DDS-based ‘remote control’ 
  over the service from anywhere in the system.
  
  - A dedicated ``RecordAndReplay`` partition is utilized by RnR 
    to bound (contain) the control/status flows.
  - In the case of a dedicated RnR node, this partition can be 
    configured to be a so-called ‘local Partition’ thus bounding 
    (containing) all control/status traffic to the RnR node.
    
• Replaying (subsets) of recorded data *‘by definition’* has impact 
  on an existing system:
  
  - It can induce unanticipated traffic-flows towards subscribing 
    applications
  - It typically triggers application-processing of such replayed 
    data...
  - which can be considered ‘intentional’ and inherent to the 
    purpose of replaying recorded data
    
Summarizing, it can be stated that when dedicating a specific computing 
node for Record and Replay and confining the control and status traffic 
to control the service to stay ‘inside’ that node, recording of data 
in a multicast-enabled network is non-intrusive.

|info|
Note that the few shared topic-definitions (definitions *only*, not actual samples of
these topics when these are ‘confined’ to the RnR node) that would be visible
system-wide when inspecting the built-in topics of the system (for instance with a
tool like the Vortex OpenSplice Tuner) are considered non-intrusive as they only imply a
small and static amount of data occupied by the related built-in topic samples.












.. |caution| image:: ./images/icon-caution.*
            :height: 6mm
.. |info|   image:: ./images/icon-info.*
            :height: 6mm
.. |windows| image:: ./images/icon-windows.*
            :height: 6mm
.. |unix| image:: ./images/icon-unix.*
            :height: 6mm
.. |linux| image:: ./images/icon-linux.*
            :height: 6mm
.. |c| image:: ./images/icon-c.*
            :height: 6mm
.. |cpp| image:: ./images/icon-cpp.*
            :height: 6mm
.. |csharp| image:: ./images/icon-csharp.*
            :height: 6mm
.. |java| image:: ./images/icon-java.*
            :height: 6mm
