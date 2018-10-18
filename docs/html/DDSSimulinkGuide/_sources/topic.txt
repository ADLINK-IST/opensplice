.. _`Topic Block`:



###########
Topic Block
###########

The topic block represents a DDS topic type.  The DDS topic corresponds to a single data type.  In DDS, data is distributed by publishing and subscribing topic data samples.

For a DDS Topic type definition, a corresponding BUS should be defined in the MATLAB workspace.  The name of the BUS and the fields and field types should correspond to the DDS topic IDL definition.

In Simulink, a BUS definition can be used as an input or output signal of the Simulink building blocks.


.. figure:: images/topic_block.png 
        :alt: DDS Topic Block 



=========== ========= ======= =========================== ====================
  Port Type  Optional Name    Description                 Output consumed by
=========== ========= ======= =========================== ====================
Input       yes       pp      DDS Domain Participant      
                              entity instance             
Output      no        topic   DDS Topic entity instance   Writer, Reader
=========== ========= ======= =========================== ====================


Topic Block Parameters
**********************

.. figure:: images/topic_block_parameters.png 
        :alt: Topic Block Parameters 

Topic Tab
=========
The output port named **topic**, needs to be configured by the user.  No defaults are provided.  To configure the **topic** output port, edit the required parameters in the **Block Parameters / Topic** tab.  The following topic parameters must be specified:   **Bus Type** and **Topic Name**. 

Ports Tab
=========
The **Ports** tab allows the user to toggle on or off optional ports.

QoS Tab
=======
The **QoS** tab is used to set the QoS profile.   By default, the OSPL default profile is used.

In DDS - The Data-Distribution Service (DDS) relies on the usage of QoS.  A QoS (Quality of Service) is a set of characteristics that controls some aspect of the behavior of the DDS Service.

Each DDS block has an associated QoS profile.   By default, the OSPL default profile is used.  An XML file that specifies QoS profiles can be used to set the QoS of a DDS block.

