.. _`Domain Block`:



############
Domain Block
############

The Domain block represents a DDS domain participant entity.

In DDS - "A domain participant represents the local membership of the application in a domain. A domain is a distributed concept that links all the applications able to communicate with each other. It represents a communication plane: only the publishers and subscribers attached to the same domain may interact."

This block is optional on a DDS Simulink model diagram. If it is not present on a model diagram, a default participant will be created by either a topic, writer or reader block.

.. figure:: images/domain_block.png 
        :alt: DDS Domain Block 


=========== ========= ======= =========================== ====================
  Port Type  Optional Name    Description                 Output consumed by
=========== ========= ======= =========================== ====================
Output      no        pp      DDS Domain Participant      Publisher,
                              entity instance             Subscriber, Topic
=========== ========= ======= =========================== ====================


Domain Block Parameters
***********************

.. figure:: images/domain_block_parameters.png 
        :alt: Domain Block Parameters

Domain Tab
==========
The domain id is read only.  The domain id is the OSPL default domain id specified in the OSPL configuration file.    

QoS Tab
=======
The **QoS** tab is used to set the QoS profile.   By default, the OSPL default profile is used.

In DDS - The Data-Distribution Service (DDS) relies on the usage of QoS.  A QoS (Quality of Service) is a set of characteristics that controls some aspect of the behavior of the DDS Service.

Each DDS block has an associated QoS profile.   By default, the OSPL default profile is used.  An XML file that specifies QoS profiles can be used to set the QoS of a DDS block.

