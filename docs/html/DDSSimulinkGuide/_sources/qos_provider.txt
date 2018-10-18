.. _`QoS Provider`:


############
QoS Provider
############

Each Vortex DDS block has a QoS that can be set using the **Block Parameters**.

The following section explains how the QoS is set for a DDS entity using the QoS Provider.


QoS Provider File
*****************

Quality of Service for DDS entities is set using XML files based on the XML schema file DDS_QoSProfile.xsd. 
These XML files contain one or more QoS profiles for DDS entities. An example with a default QoS profile 
for all entity types can be found at DDS_DefaultQoS.xml_.

**Note:** Sample QoS Profile XML files can be found in the examples directories.

QoS Profile
***********

A QoS profile consists of a name. The file contains QoS 
elements for one or more DDS entities. A skeleton file without any QoS values is displayed below to show 
the structure of the file.


.. code-block:: xml
    
    <dds xmlns="http://www.omg.org/dds/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
                xsi:schemaLocation="file:DDS_QoSProfile.xsd">
        <qos_profile name="DDS QoS Profile Name">
            <datareader_qos></datareader_qos>
            <datawriter_qos></datawriter_qos>
            <domainparticipant_qos></domainparticipant_qos>
            <subscriber_qos></subscriber_qos>
            <publisher_qos></publisher_qos>
            <topic_qos></topic_qos>
        </qos_profile>
    </dds>

**Example: Specify Publisher Partition**

The example below specifies the publisher's partitions as A and B.

.. code-block:: xml

    <publisher_qos>
        <partition>
            <name>
                <element>A</element>
                <element>B</element>
            </name>
        </partition>
    </publisher_qos>
    

Setting QoS Profile in Simulink
*******************************
QoS profiles are set using the Simulink block's parameters dialog under the QoS tab. If the 
QoS File parameter is set to None the default QoS settings will be used. The Reset button sets
the parameters to the default values.

.. figure:: images/default_qos_params.png 
        :alt: Default QoS Provider Parameters

A QoS Provider file can be selected by browsing to the XML file. Once the file is chosen the 
file name is displayed and the user is presented with a drop down list of all QoS Providers in 
the file. 

.. figure:: images/persistent_qos_params.png  
        :alt: DDS User Selected QoS Provider Parameters

 
**Note:** Seeing the QoS Profile in the drop down list only guarantees the QoS Profile exists in the file.
It does not mean the qos tag exists for the entity. The user is responsible for verifying the entity qos
tag exists in the file.

.. raw:: latex

    \newpage

Simulink block annotations are visible by default to display the QoS File Name and the QoS Profile settings.

.. figure:: images/sub_annotations_qos.png  
        :alt: QoS File and Profile annotations

Known Limitations
*****************

See `QoS Provider Known Limitations <../qos_provider.html#KnownLimitations>`_ for a list of limitations
on QoS Provider support.

.. external links
.. _QoSProfile.xsd: http://www.omg.org/spec/dds4ccm/20110201/DDS_QoSProfile.xsd
.. _DDS_DefaultQoS.xml: http://www.omg.org/spec/dds4ccm/20110201/DDS_DefaultQoS.xml



