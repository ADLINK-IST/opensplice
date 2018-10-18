.. _`Writer Block`:



############
Writer Block
############

The Writer block represents a DDS data writer entity.

In DDS - "The DataWriter is the object the application must use to communicate to a publisher the existence and value of data-objects of a given type."

.. figure:: images/writer_block.png 
        :alt: DDS Writer Block 


=========== ========= =============== =========================== ====================
  Port Type  Optional Name            Description                 Output consumed by
=========== ========= =============== =========================== ====================
Input       yes       ppub            DDS Publisher      
                                      entity instance    
Input       no        topic           DDS Topic entity instance   
Input       no        data            BUS
Input       yes       action          0 write, 
                                      1 dispose, 
                                      2 write dispose, 
                                      3 no operation
Output      yes       status          0 for successful writer
                                      creation
Output      yes       samples written Number of samples written   User
=========== ========= =============== =========================== ====================

.. raw:: latex

    \newpage

Writer Block Parameters
***********************


.. figure:: images/writer_block_parameters.png 
        :alt: Writer Block Parameters

Data Tab
========
The **Data** tab is used to set the input data type (BUS) for the **data** input port and the **bus width**.

The bus width is the maximum number of samples that can be written per block step.  
The user must configure the source blocks that feed the Writer's data port so that it produces an array of the right size. 

Valid values for the bus width are:  integers >= 1.

The Reader Available field in the Wait for section is used for specifying if the Writer should wait for the Reader to become available.
The associated Timeout field is to specify how long (in seconds) the Writer should wait for the Reader to become available.

The Write after timeout field can only be enabled when the Reader Available field is checked. It specifies if the Writer should write after the Wait for Reader Available timeout.

Ports Tab
=========
The **Ports** tab allows the user to toggle on or off optional ports.

QoS Tab
=======
The **QoS** tab is used to set the QoS profile.   By default, the OSPL default profile is used.

In DDS - The Data-Distribution Service (DDS) relies on the usage of QoS.  A QoS (Quality of Service) is a set of characteristics that controls some aspect of the behavior of the DDS Service.

Each DDS block has an associated QoS profile.   By default, the OSPL default profile is used.  An XML file that specifies QoS profiles can be used to set the QoS of a DDS block.

