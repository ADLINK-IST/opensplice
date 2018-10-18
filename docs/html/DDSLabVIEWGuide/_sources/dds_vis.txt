.. _`Vortex DDS Virtual Instruments`:


####################################
Vortex DDS Virtual Instruments (VIs)
####################################


The DDS LabVIEW Integration provides a function palette with custom virtual instruments (VIs) to model reading and writing data with DDS.

.. figure:: images/dds_palette.png 
        :alt: VortexDDS VIs

The Vortex DDS LabVIEW VIs are included in **VortexDDS** functions palette.

The following DDS VIs are provided:

     - create_participant.vi
     - create_publisher.vi
     - create_subscriber.vi
     - create_writer.vi
     - create_reader.vi
     - wait_historical_data.vi
     - delete_entity.vi

DDS VIs usage
*************

The typical way to model a DDS application in LabVIEW is as follows:

* model your DDS topics using IDL
* using the LabVIEW IDLPP process generate DDS Topic, Read and Write VIs from the IDL file
* add the generated VIs to your LabVIEW project 
* create a DDS LabVIEW application using the VortexDDS functions palette and the generated VIs from the previous step

QoS Profiles
************

In DDS - “The Data-Distribution Service (DDS) relies on the usage of QoS.  A QoS (Quality of Service) is a set of characteristics that controls some aspect of the behavior of the DDS Service."

Each DDS entity VI has an associated QoS profile.   By default, the OSPL default profile is used.  An XML file that specifies QoS profiles can be used to set the QoS of a DDS entity.

The QoS profile of an entity is set using the **qos_uri** and **qos_profile** terminals.


Please see section :ref:`QoS Provider` for more information.


create_participant.vi
*********************

The create_participant VI represents a DDS domain participant entity.

In DDS - "A domain participant represents the local membership of the application in a domain. A domain is a distributed concept that links all the applications able to communicate with each other. It represents a communication plane: only the publishers and subscribers attached to the same domain may interact."

The domain id is the OSPL default domain id specified in the OSPL configuration file (file pointed by "OSPL_URI" environment variable).


.. figure:: images/dds_participant.png 
        :alt: create_participant VI


============= ========= ======================= =========================== ============================
Terminal Type Optional  Name                    Description                 Output consumed by
============= ========= ======================= =========================== ============================
Output        no        pp                      DDS Domain Participant      create_publisher.vi
                                                entity instance             create_subscriber.vi 
                                                                            RegisterTopic.vi

Input         yes       qos_uri                 QoS file uri

Input         yes       qos_profile             Name of QoS profile

Input         yes       error in (no error)     Input Error cluster

Output        yes       error out               Error out cluster           
============= ========= ======================= =========================== ============================

.. raw:: latex

    \newpage

create_publisher.vi
*******************

The create_publisher VI represents a DDS publisher entity.

In DDS, a publisher is "an object responsible for data distribution.  It may publish data of different data types."


.. figure:: images/create_publisher.png 
        :alt: create_publisher VI


============= ========= ======================= =========================== ============================
Terminal Type Optional  Name                    Description                 Output consumed by
============= ========= ======================= =========================== ============================
Input         no        pp                      DDS Domain Participant      
                                                entity instance 

Input         yes       qos_uri                 QoS file uri

Input         yes       qos_profile             Name of QoS profile

Input         yes       error in (no error)     Input Error cluster

Output        no        pub                     DDS publisher               create_writer.vi
                                                entity instance  

Output        yes       error out               Error out cluster           
============= ========= ======================= =========================== ============================

.. raw:: latex

    \newpage

create_subscriber.vi
********************

The create_subscriber VI represents a DDS subscriber entity.

In DDS, a subscriber is "an object responsible for receiving published data and making it available to the receiving application.  It may receive and dispatch data of different specified types."


.. figure:: images/create_subscriber.png 
        :alt: create_subscriber VI


============= ========= ======================= =========================== ============================
Terminal Type Optional  Name                    Description                 Output consumed by
============= ========= ======================= =========================== ============================
Input         no        pp                      DDS Domain Participant      
                                                entity instance 

Input         yes       qos_uri                 QoS file uri

Input         yes       qos_profile             Name of QoS profile

Input         yes       error in (no error)     Input Error cluster

Output        no        sub                     DDS subscriber              create_reader.vi
                                                entity instance  

Output        yes       error out               Error out cluster           
============= ========= ======================= =========================== ============================


create_writer.vi
****************

The create_writer VI represents a DDS data writer entity.

In DDS - "The DataWriter is the object the application must use to communicate to a publisher the existence and value of data-objects of a given type."

.. figure:: images/create_writer.png 
        :alt: create_writer VI


============= ========= ======================= =========================== ============================
Terminal Type Optional  Name                    Description                 Output consumed by
============= ========= ======================= =========================== ============================
Input         no        pub                     DDS publisher      
                                                entity instance 

Input         no        topic                   DDS Topic      
                                                entity instance 

Input         yes       qos_uri                 QoS file uri

Input         yes       qos_profile             Name of QoS profile

Input         yes       error in (no error)     Input Error cluster

Output        no        writer                  DDS writer                  write.vi
                                                entity instance  

Output        yes       error out               Error out cluster           
============= ========= ======================= =========================== ============================


create_reader.vi
****************

The create_reader VI represents a DDS data reader entity.

In DDS - "To access the received data, the application must use a typed DataReader attached to the subscriber."


.. figure:: images/create_reader.png 
        :alt: create_reader VI


============= ========= ======================= =========================== ============================
Terminal Type Optional  Name                    Description                 Output consumed by
============= ========= ======================= =========================== ============================
Input         no        sub                     DDS subscriber      
                                                entity instance 

Input         no        topic                   DDS Topic      
                                                entity instance 

Input         yes       qos_uri                 QoS file uri

Input         yes       qos_profile             Name of QoS profile

Input         yes       error in (no error)     Input Error cluster

Output        no        reader                  DDS reader                  read.vi
                                                entity instance  

Output        yes       error out               Error out cluster           
============= ========= ======================= =========================== ============================


wait_historical_data.vi
***********************

The wait_historical_data VI specifies that the Reader will wait for historical data to arrive. The timeout terminal is for setting time period (in seconds) determining how long the Reader should wait for the historical data. If the timeout is reached, then any remaining historical data may be interleaved with new data.


.. figure:: images/wait_historical.png 
        :alt: wait_historical VI


============= ========= ======================= =========================== ============================
Terminal Type Optional  Name                    Description                 Output consumed by
============= ========= ======================= =========================== ============================
Input         no        reader                  DDS Reader      
                                                entity instance 

Input         yes       historical_timeout      wait for historical data
                                                timeout (seconds)

Input         yes       error in (no error)     Input Error cluster

Output        yes       error out               Error out cluster           
============= ========= ======================= =========================== ============================


delete_entity.vi
****************

The delete_entity VI is used to delete a DDS entity. Connect the DDS participant to the entity terminal to delete the participant (pp) in a LabVIEW DDS application. 

 *NOTE:   If the user application VI stops due to an error and does not run to completion, the participant entity is not deleted and leaks occur. The participants are deleted once the user closes LabVIEW.*

.. figure:: images/delete_entity.png 
        :alt: delete_entity VI


============= ========= ======================= =========================== ============================
Terminal Type Optional  Name                    Description                 Output consumed by
============= ========= ======================= =========================== ============================
Input         no        entity                  DDS entity instance 
============= ========= ======================= =========================== ============================


