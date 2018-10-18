.. _`Vortex DDS Blocks`:


#################
Vortex DDS Blocks
#################


The DDS Simulink Integration provides a block library with custom blocks to model reading and writing data with DDS.

The Vortex DDS Simulink block library provides blocks which correspond to DDS entities.   (Each DDS block is covered in its own section in this user guide.)

The following DDS block types are provided:

     - Topic
     - Domain
     - Publisher
     - Subscriber
     - Writer
     - Reader

Optional DDS Blocks and Ports
*****************************

Some of the DDS blocks are optional. (Domain, Publisher and Subscriber)

When the optional blocks are not added to model diagrams, defaults are used.     This allows for simpler model diagrams.
If the model requires block parameter customization, the optional blocks can be added to a model to use non-default settings.

Many of the ports for the DDS blocks are also optional.   They can be toggled on or off in the **Block Parameters** dialog, in the **Ports** tab.

QoS Profiles
************

In DDS - “The Data-Distribution Service (DDS) relies on the usage of QoS.  A QoS (Quality of Service) is a set of characteristics that controls some aspect of the behavior of the DDS Service."

Each DDS block has an associated QoS profile.   By default, the OSPL default profile is used.  An XML file that specifies QoS profiles can be used to set the QoS of a DDS block.

The QoS profile of a block is set in the **QoS** tab of the **Block Parameters** dialog.  (This dialog is opened by double clicking on a selected block.)


Please see section :ref:`QoS Provider` for more information.

.. raw:: latex

    \newpage

Simulink Block Sample Time
**************************

**"What is sample time?"**

**"The sample time of a block is a parameter that indicates when, during simulation, the block produces outputs and if appropriate, updates its internal state."
-Simulink documentation**



The DDS blocks have different sample times set.  The only DDS block that allows for the user specification of a sample time is the **Reader** block.
A reader's sample time can be set in the **Block Parameters** **Data** tab.

A sample time of 0 means that the block step will only execute once.

A sample time of -1 means that the block will inherit its sample time from its inputs or from the parent model.


+---------------------+--------------------------------------+       
|DDS Block Type       |Sample Time                           |
+=====================+======================================+                        
|Topic                | -1 (inherits) uneditable             |
+---------------------+--------------------------------------+        
|Domain               | -1 (inherits) uneditable             |
+---------------------+--------------------------------------+                  
|Publisher            | -1 (inherits) uneditable             |
+---------------------+--------------------------------------+           
|Subscriber           | -1 (inherits) uneditable             |
+---------------------+--------------------------------------+                      
|Writer               | -1 (inherits) uneditable             |
+---------------------+--------------------------------------+        
|Reader               | - default -1 (inherits) editable     | 
|                     | - Inherits from inputs or model      | 
|                     | - Valid values: -1 and Numeric > 0   |
+---------------------+--------------------------------------+   
   






