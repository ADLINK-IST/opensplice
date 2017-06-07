Entity Factory QoS              {#DCPS_QoS_EntityFactory}
==================

This QosPolicy controls the behaviour of the Entity as a factory for other entities.

Attributes
----------
<table>
    <tr>
        <th>Value</th>
        <th>Meaning</th>
        <th>Concerns</th>
        <th>RxO</th>
        <th>Changeable</th>
    </tr>
    <tr>
        <td>
            A boolean:<br/>
            autoenable_created_entities
        </td>
        <td>
            Specifies whether the entity acting
            as a factory automatically enables
            the instances it creates.
            If autoenable_created_
            entities==TRUE, the factory will
            automatically enable each created
            Entity otherwise it will not.
            By default, TRUE.
        </td>
        <td>\ref DCPS_Modules_DomainParticipant "DomainParticipant", \ref DCPS_Modules_Publication "Publisher", \ref DCPS_Modules_Subscription "Subscriber"</td>
        <td>No</td>
        <td>Yes</td>
    </tr>
</table>

This policy controls the behaviour of the Entity as a factory for other entities. This policy concerns only \ref DCPS_Modules_DomainParticipant "DomainParticipant" (as factory for \ref DCPS_Modules_Publication "Publisher", \ref DCPS_Modules_Subscription "Subscriber", and \ref DCPS_Modules_TopicDefinition "Topic"), \ref DCPS_Modules_Publication "Publisher" (as factory for \ref DCPS_Modules_Publication_DataWriter "DataWriter"), and \ref DCPS_Modules_Subscription "Subscriber" (as factory for \ref DCPS_Modules_Subscription_DataReader "DataReader").

This policy is mutable. A change in the policy affects only the entities created after the change; not the previously created
entities.
The setting of autoenable_created_entities to TRUE indicates that the factory create_<entity> operation will automatically invoke the enable operation each time a new Entity is created. Therefore, the Entity returned by create_<entity> will already be enabled.

A setting of FALSE indicates that the Entity will not be automatically enabled. The application will need to enable it explicitly by means of the enable operation. The default setting of autoenable_created_entities = TRUE means that, by default, it is not necessary to explicitly call enable on newly created entities.
