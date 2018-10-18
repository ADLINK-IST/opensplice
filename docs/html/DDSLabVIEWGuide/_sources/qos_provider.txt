.. _`QoS Provider`:


############
QoS Provider
############

Each Vortex DDS virtual instrument (VI) has a QoS file uri terminal and a QoS profile terminal.
These terminals are used to set the QoS profile. By default, the OSPL default profile is used.
In DDS - The Data-Distribution Service (DDS) relies on the usage of QoS. A QoS (Quality of Service) is a set of
characteristics that controls some aspect of the behavior of the DDS Service.

Each DDS VI has an associated QoS profile. By default, the OSPL default profile is used. An XML file that
specifies QoS profiles can be used to set the QoS of a DDS block.

The following section explains how the QoS is set for a DDS entity using the QoS Provider.


QoS Provider File
*****************

Quality of Service for DDS entities is set using XML files based on the XML schema file DDS_QoSProfile.xsd. 
These XML files contain one or more QoS profiles for DDS entities.

**Note:** Sample QoS Profile XML files can be found in the LabVIEW DDS examples directories.

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
    

Setting QoS Profile in LabVIEW
******************************

The QoS Profiles from the XML file can be obtained using the **List_qos_profiles** Tools menu in LabVIEW. 

*Tools/VortexDDS/List_qos_profiles*

.. figure:: images/qos_profile_tools.png 
        :alt: QoS Profile Tools

.. raw:: latex

    \newpage

**Steps to set the QoS Profile**

1.  A QoS Provider file can be selected by browsing to the XML file from the **List_qos_profiles** dialog box. Once a valid QoS file is chosen the **Available qos profiles** table is populated with the list of qos profiles that are available in the QoS XML file. If there are QoS profiles found in the file, then **Copy to Clipboard** button will be enabled. 

2.  Select the QoS Profile that you want to use and click on **Copy to Clipboard**.

.. figure:: images/list_qos_profiles.png
        :alt: List qos profiles

3.  In your VI, create a String constant and press Ctrl + V. Connect this String constant to the DDS VI **qos_profile** terminal. Set the **qos_uri** as a LabVIEW control or constant and navigate to the path of the QoS Provider file.

.. figure:: images/set_qos.png
        :alt: Set QoS

The **qos_profile** and **qos_uri** are optional terminals. If they are not set then the default QoS settings will be used.
 
**Note:** Seeing the QoS Profile in the list only guarantees the QoS Profile exists in the file.
It does not mean the qos tag exists for the entity. The user is responsible for verifying the entity qos
tag exists in the file.


