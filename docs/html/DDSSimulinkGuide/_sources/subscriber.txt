.. _`Subscriber Block`:



################
Subscriber Block
################

The Subscriber block represents a DDS subscriber entity.

In DDS, a subscriber is "an object responsible for receiving published data and making it available to the receiving application.  It may receive and dispatch data of different specified types."

This block is optional on a DDS Simulink model diagram.    If it is not present on a model diagram, each reader will create its own default subscriber.


.. figure:: images/subscriber_block.png 
        :alt: DDS Subscriber Block


=========== ========= ======= =========================== ====================
  Port Type  Optional Name    Description                 Output consumed by
=========== ========= ======= =========================== ====================
Input       yes       pp      DDS Domain Participant      
                              entity instance       
Output      no        psub    DDS Subscriber entity       Reader
                              instance      
=========== ========= ======= =========================== ====================


Subscriber Block Parameters
***************************

.. figure:: images/subscriber_block_parameters.png 
        :alt: Subscriber Block Parameters

Ports Tab
=========
The **Ports** tab allows the user to toggle on or off optional ports.

QoS Tab
=======
The **QoS** tab is used to set the QoS profile.   By default, the OSPL default profile is used.

In DDS - The Data-Distribution Service (DDS) relies on the usage of QoS.  A QoS (Quality of Service) is a set of characteristics that controls some aspect of the behavior of the DDS Service.

Each DDS block has an associated QoS profile.   By default, the OSPL default profile is used.  An XML file that specifies QoS profiles can be used to set the QoS of a DDS block.

