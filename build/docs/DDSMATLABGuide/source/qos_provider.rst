.. _`QoS Provider`:


############
QoS Provider
############

The following section explains how the QoS is set for a DDS entity using the QoS Provider.


QoS Provider File
*****************

Quality of Service for DDS entities is set using XML files based on the XML schema file QoSProfile.xsd_. 
These XML files contain one or more QoS profiles for DDS entities. An example with a default QoS profile 
for all entity types can be found at DDS_DefaultQoS.xml_.

**Note:** Sample QoS Profile XML files can be found in the examples directories.

QoS Profile
***********

A QoS profile consists of a name and optionally a base_name attribute. The base_name attribute allows a 
QoS or a profile to inherit values from another QoS or profile in the same file. The file contains QoS 
elements for one or more DDS entities. A skeleton file without any QoS values is displayed below to show 
the structure of the file.

.. code-block:: xml
    
    <dds xmlns="http://www.omg.org/dds/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="file:DDS_QoSProfile.xsd">
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
    

Setting QoS Profile in MATLAB
*****************************
To set the QoS profile for a DDS entity in MATLAB the user must specify the File URI and the QoS profile name.
If the file is not specified, the DDS entity will be created with the default QoS values.

.. code-block:: matlab

    % QOS File
    PINGER_QOS_FILE = '/home/dds/matlab_examples/pingpong/DDS_PingerVolatileQoS.xml';
    PINGER_QOS_PROFILE = 'DDS VolatileQosProfile';

    % create the pinger participant on default domain with specified qos profile
    dp = Vortex.DomainParticipant(['file://', PINGER_QOS_FILE], PINGER_QOS_PROFILE);

The file for the above would minimally contain the following <domainparticipant_qos> tag.

.. code-block:: xml

    <?xml version="1.0" encoding="UTF-8"?>
    <dds xmlns="http://www.omg.org/dds/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="file:DDS_QoSProfile.xsd">
        <qos_profile name="DDS VolatileQosProfile">
            <domainparticipant_qos>
                <user_data>
                    <value></value>
                </user_data>
                <entity_factory>
                    <autoenable_created_entities>true</autoenable_created_entities>
                </entity_factory>
             </domainparticipant_qos>
        </qos_profile>
    </dds>


.. external links
.. _QoSProfile.xsd: http://www.omg.org/spec/dds4ccm/20110201/DDS_QoSProfile.xsd
.. _DDS_DefaultQoS.xml: http://www.omg.org/spec/dds4ccm/20110201/DDS_DefaultQoS.xml
