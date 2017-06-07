.. _`QoS policies XML schema`:

#######################
QoS policies XML schema
#######################

.. code-block:: XML

   <?xml version="1.0" encoding="UTF-8"?>
   <xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
               targetNamespace="http://www.omg.org/dds/"
               xmlns="http://www.omg.org/dds/"
               elementFormDefault="qualified">
       <xsd:element name="dcps">
           <xsd:complexType>
               <xsd:all>
                   <xsd:element ref="domain" minOccurs="1" maxOccurs="1"/>
               </xsd:all>
           </xsd:complexType>
       </xsd:element>
       <xsd:element name="domain">
           <xsd:complexType>
               <xsd:sequence>
                   <xsd:element ref="topic" minOccurs="1" maxOccurs="unbounded"/>
               </xsd:sequence>
               <xsd:attribute name="id" type="xsd:string" use="required"/>
           </xsd:complexType>
       </xsd:element>
       <xsd:element name="topic">
           <xsd:complexType>
               <xsd:all>
                   <xsd:element ref="keylist" minOccurs="1" maxOccurs="1"/>
                   <xsd:element ref="topic_qos" minOccurs="0" maxOccurs="1"/>
               </xsd:all>
               <xsd:attribute name="name" type="xsd:string" use="required"/>
               <xsd:attribute name="idltype" type="xsd:string" use="required"/>
               <xsd:attribute name="idlfile" type="xsd:string" use="required"/>
           </xsd:complexType>
       </xsd:element>

       <xsd:element name="keylist">
           <xsd:complexType>
             <xsd:sequence>
               <xsd:element ref="keyMember" minOccurs="0" maxOccurs="unbounded"/>
             </xsd:sequence>
           </xsd:complexType>
       </xsd:element>
       <xsd:element name="keyMember" type="xsd:string"/>

        <xsd:element name="topic_qos">
          <xsd:complexType>
           <xsd:all>
               <xsd:element ref="topicDataQosPolicy" minOccurs="0" maxOccurs="1"/>
               <xsd:element ref="deadlineQosPolicy" minOccurs="0" maxOccurs="1"/>
               <xsd:element ref="durabilityQosPolicy" minOccurs="0" maxOccurs="1"/>
               <xsd:element ref="durabilityServiceQosPolicy" minOccurs="0"
                                maxOccurs="1"/>
               <xsd:element ref="latencyBudgetQosPolicy" minOccurs="0" maxOccurs="1"/>
               <xsd:element ref="livelinessQosPolicy" minOccurs="0" maxOccurs="1"/>
               <xsd:element ref="reliabilityQosPolicy" minOccurs="0" maxOccurs="1"/>
               <xsd:element ref="destinationOrderQosPolicy" minOccurs="0" 
                                maxOccurs="1"/>
               <xsd:element ref="historyQosPolicy" minOccurs="0" maxOccurs="1"/>
               <xsd:element ref="resourceLimitsQosPolicy" minOccurs="0" 
                                maxOccurs="1"/>
               <xsd:element ref="transportPriorityQosPolicy" minOccurs="0" 
                                maxOccurs="1"/>
               <xsd:element ref="lifespanQosPolicy" minOccurs="0" maxOccurs="1"/>
               <xsd:element ref="ownershipQosPolicy" minOccurs="0" maxOccurs="1"/>
               <xsd:element ref="timeBasedFilterQosPolicy" minOccurs="0" 
                                maxOccurs="1"/>
           </xsd:all>
         </xsd:complexType>
       </xsd:element>
   
       <xsd:element name="deadlineQosPolicy">
          <xsd:complexType>
             <xsd:all>
               <xsd:element ref="duration" minOccurs="1" maxOccurs="1"/>
             </xsd:all>
          </xsd:complexType>
       </xsd:element>

       <xsd:element name="timeBasedFilterQosPolicy">
          <xsd:complexType>
             <xsd:all>
               <xsd:element ref="duration" minOccurs="1" maxOccurs="1"/>
             </xsd:all>
          </xsd:complexType>
       </xsd:element>

        <xsd:element name="topicDataQosPolicy">
           <xsd:complexType>
             <xsd:all>
               <xsd:element name="value" type="xsd:base64Binary" minOccurs="1"
                                 maxOccurs="1"/>
             </xsd:all>
           </xsd:complexType>
        </xsd:element>

       <xsd:element name="duration">
         <xsd:complexType>
           <xsd:all>
               <xsd:element name="sec" type="xsd:string" minOccurs="1" maxOccurs="1"/>
               <xsd:element name="nanosec" type="xsd:string" minOccurs="1"
                                 maxOccurs="1"/>
           </xsd:all>
         </xsd:complexType>
       </xsd:element>

       <xsd:element name="durabilityQosPolicy">
         <xsd:complexType>
           <xsd:all>
               <xsd:element ref="durabilityKind" minOccurs="1" maxOccurs="1"/>
           </xsd:all>
         </xsd:complexType>
       </xsd:element>

       <xsd:element name="durabilityKind">
        <xsd:simpleType>
           <xsd:restriction base="xsd:string">
               <xsd:enumeration value="VOLATILE_DURABILITY_QOS"/>
               <xsd:enumeration value="TRANSIENT_LOCAL_DURABILITY_QOS"/>
               <xsd:enumeration value="TRANSIENT_DURABILITY_QOS"/>
               <xsd:enumeration value="PERSISTENT_DURABILITY_QOS"/>
           </xsd:restriction>
        </xsd:simpleType>
       </xsd:element>

       <xsd:element name="durabilityServiceQosPolicy">
         <xsd:complexType>
           <xsd:all>
               <xsd:element ref="duration" minOccurs="1" maxOccurs="1"/>
               <xsd:element ref="historyKind" minOccurs="1" maxOccurs="1"/>
               <xsd:element name="history_depth" type="xsd:positiveInteger"
                                 minOccurs="1" maxOccurs="1"/>
               <xsd:element name="max_samples" type="xsd:positiveInteger"
                                 minOccurs="1" maxOccurs="1"/>
               <xsd:element name="max_instances" type="xsd:positiveInteger"
                                 minOccurs="1" maxOccurs="1"/>
               <xsd:element name="max_samples_per_instance" 
                            type="xsd:positiveInteger" minOccurs="1" maxOccurs="1"/>
           </xsd:all>
         </xsd:complexType>
       </xsd:element>

       <xsd:element name="historyKind">
         <xsd:simpleType>
           <xsd:restriction base="xsd:string">
               <xsd:enumeration value="KEEP_LAST_HISTORY_QOS"/>
               <xsd:enumeration value="KEEP_ALL_HISTORY_QOS"/>
           </xsd:restriction>
         </xsd:simpleType>
       </xsd:element>

       <xsd:element name="latencyBudgetQosPolicy">
           <xsd:complexType>
             <xsd:all>
               <xsd:element ref="duration" minOccurs="1" maxOccurs="1"/>
             </xsd:all>
           </xsd:complexType>
       </xsd:element>
       <xsd:element name="livelinessQosPolicy">
           <xsd:complexType>
             <xsd:all>
               <xsd:element ref="duration" minOccurs="1" maxOccurs="1"/>
               <xsd:element ref="livelinessKind" minOccurs="1" maxOccurs="1"/>
             </xsd:all>
           </xsd:complexType>
       </xsd:element>

       <xsd:element name="reliabilityQosPolicy">
         <xsd:complexType>
           <xsd:all>
               <xsd:element ref="reliabilityKind" minOccurs="1" maxOccurs="1"/>
               <xsd:element ref="duration" minOccurs="1" maxOccurs="1"/>
           </xsd:all>
         </xsd:complexType>
       </xsd:element>

       <xsd:element name="reliabilityKind">
         <xsd:simpleType>
           <xsd:restriction base="xsd:string">
               <xsd:enumeration value="BEST_EFFORT_RELIABILITY_QOS"/>
               <xsd:enumeration value="RELIABLE_RELIABILITY_QOS"/>
           </xsd:restriction>
         </xsd:simpleType>
       </xsd:element>

       <xsd:element name="destinationOrderQosPolicy">
         <xsd:complexType>
           <xsd:all>
               <xsd:element ref="destinationOrderKind" minOccurs="1" maxOccurs="1"/>
           </xsd:all>
         </xsd:complexType>
       </xsd:element>

       <xsd:element name="destinationOrderKind">
         <xsd:simpleType>
           <xsd:restriction base="xsd:string">
               <xsd:enumeration value="BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS"/>
               <xsd:enumeration value="BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS"/>
           </xsd:restriction>
         </xsd:simpleType>
       </xsd:element>

       <xsd:element name="livelinessKind">
         <xsd:simpleType>
           <xsd:restriction base="xsd:string">
               <xsd:enumeration value="AUTOMATIC_LIVELINESS_QOS"/>
               <xsd:enumeration value="MANUAL_BY_PARTICIPANT_LIVELINESS_QOS"/>
               <xsd:enumeration value="MANUAL_BY_TOPIC_LIVELINESS_QOS"/>
           </xsd:restriction>
         </xsd:simpleType>
       </xsd:element>

       <xsd:element name="historyQosPolicy">
         <xsd:complexType>
           <xsd:all>
               <xsd:element ref="historyKind" minOccurs="1" maxOccurs="1"/>
               <xsd:element name="depth" type="xsd:positiveInteger" default="1" 
                                 minOccurs="1" maxOccurs="1"/>
           </xsd:all>
         </xsd:complexType>
       </xsd:element>

       <xsd:element name="resourceLimitsQosPolicy">
         <xsd:complexType>
           <xsd:all>
               <xsd:element name="max_samples" type="xsd:positiveInteger"
                                 minOccurs="1" maxOccurs="1"/>
               <xsd:element name="max_instances" type="xsd:positiveInteger"
                                 minOccurs="1" maxOccurs="1"/>
               <xsd:element name="max_samples_per_instance" 
                                 type="xsd:positiveInteger" minOccurs="1" 
                                 maxOccurs="1"/>
               <xsd:element name="initial_samples" type="xsd:positiveInteger"
               minOccurs="1" maxOccurs="1"/>
               <xsd:element name="initial_instances" type="xsd:positiveInteger" 
                                 minOccurs="1" maxOccurs="1"/>
           </xsd:all>
         </xsd:complexType>
       </xsd:element>

       <xsd:element name="transportPriorityQosPolicy">
         <xsd:complexType>
           <xsd:all>
               <xsd:element name="value" type="xsd:nonNegativeInteger" 
                                 minOccurs="1" maxOccurs="1"/>
           </xsd:all>
         </xsd:complexType>
       </xsd:element>

       <xsd:element name="lifespanQosPolicy">
         <xsd:complexType>
           <xsd:all>
               <xsd:element ref="duration" minOccurs="1" maxOccurs="1"/>
           </xsd:all>
         </xsd:complexType>
       </xsd:element>

       <xsd:element name="ownershipQosPolicy">
         <xsd:complexType>
           <xsd:all>
               <xsd:element ref="ownershipKind" minOccurs="1" maxOccurs="1"/>
           </xsd:all>
         </xsd:complexType>
       </xsd:element>

       <xsd:element name="ownershipKind">
         <xsd:simpleType>
           <xsd:restriction base="xsd:string">
               <xsd:enumeration value="SHARED_OWNERSHIP_QOS"/>
               <xsd:enumeration value="EXCLUSIVE_OWNERSHIP_QOS"/>
           </xsd:restriction>
         </xsd:simpleType>
       </xsd:element>
   </xsd:schema>


..




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

         