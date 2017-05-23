.. _`OpenSplice Architectural Modes`:

##############################
OpenSplice Architectural Modes
##############################

OpenSplice Enterprise provides two main architectural modes. These 
are the **Single Process** deployment mode, which provides a **Standalone** 
architecture, and, unique to OpenSplice, the **Shared Memory** deployment 
mode which provides a **Federated** architecture. 

*******************************************
The Single Process or Standalone deployment
*******************************************

Features of this mode are: 

+ Simplest to run and get started with.

+ Each DDS application process contains the entire DDS infrastructure.

+ Uses in-process heap memory for the DDS database.

+ OpenSplice Enterprise services run as threads within the 
  application process.

+ When there are multiple DDS application processes on a single 
  machine, the communication between them must be done *via* a 
  networking service; this introduces additional overhead so 
  performance in this scenario is not optimal.


.. _`Single Process or Standalone deployment`:

.. figure:: /images/SingleProcess.png
   :height: 60mm
   :alt: Single Process or Standalone deployment

   **Single Process or Standalone deployment**




*****************************************
The Shared Memory or Federated deployment
*****************************************

Features of this mode are: 

+ The DDS infrastructure is started once per machine.

+ Uses shared memory for the DDS database.

+ Each DDS application process interfaces with the shared 
  memory rather than creating the DDS infrastructure itself.

+ Allows the data to be physically present only once on any machine.

+ Reading and writing directly to locally-mapped memory is far 
  more efficient than having to actually move the data *via* a 
  networking service, allowing for improved performance and scalability.

+ OpenSplice Enterprise services are able to arbitrate over all of the 
  DDS data on the node, and so can make smart decisions with respect 
  to data delivery so that priority QoS values (for example) are 
  respected; this is not possible when there are multiple standalone 
  deployments on a machine.


.. _`Shared Memory or Federated deployment`:

.. figure:: /images/SharedMemory.png
   :height: 60mm
   :alt: Shared Memory or Federated deployment

   **Shared Memory or Federated deployment**



**When there are multiple DDS applications running on a single 
computing node, the use of OpenSplice's unique Shared Memory 
architecture can provide greater performance, smaller footprint and 
better scalability than other DDS deployment options.**

.. _`How to select the Architectural Mode`:

************************************
How to select the Architectural Mode
************************************

+ For a Single Process deployment, set the OSPL_URI variable to 
  refer to a single process (sp) xml file such as ``ospl_sp_ddsi.xml`` 
  or ``ospl_sp_nativeRT.xml``. Note that a networking service (such 
  as ddsi or nativeRT) is required for two DDS application processes 
  to communicate even if they are running on the same physical machine.
  See the next section for more details on networking options. 

**A single process deployment is enabled when the Domain section of the 
XML configuration contains a '<SingleProcess> TRUE' attribute.** 

NOTE for VxWorks kernel mode builds of OpenSplice the single process feature of the OpenSplice domain must not be enabled. i.e. "<SingleProcess>true</SingleProcess>" must not be included in the OpenSplice Configuration xml. The model used on VxWorks kernel builds is always that an area of kernel memory is allocated to store the domain database ( the size of which is controlled by the size option in the Database configuration for opensplice as is used on other platforms for the shared memory model. ) This can then be accessed by any task on the same VxWorks node.

+ For a Shared Memory deployment, set the ``OSPL_URI`` variable to 
  refer to a shared memory (*shmem*) xml file such as 
  ``ospl_shmem_no_network.xml``, ``ospl_shmem_ddsi.xml``, or 
  ``ospl_shmem_nativeRT.xml``. Note that two or more DDS applications 
  running on the same physical machine are able to communicate 
  *via* the shared memory so a networking service (such as ddsi 
  or nativeRT) is not necessarily required. 


**A shared memory deployment is enabled when the Domain section of 
the XML configuration does not contain a '<SingleProcess> TRUE' 
attribute but does contain a '<Database>' attribute.** 

|caution|
Note that by default the ``OSPL_URI`` environment variable refers to a 
*Single Process* configuration, so to see the extra performance and 
scalability benefits of OpenSplice DDS's Shared Memory 
architecture it is necessary to switch from the default. 



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


