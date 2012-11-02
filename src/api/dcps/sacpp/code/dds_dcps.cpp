/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include "dds_dcps.h"

#if DDS_USE_EXPLICIT_TEMPLATES
#error "this is not working!"
template class DDS_DCPSUFLSeq <DDS::SampleInfo, struct SampleInfoSeq_uniq_>;
#endif
#if DDS_USE_EXPLICIT_TEMPLATES
template class DDS_DCPSUFLSeq <DDS::InstanceStateKind, struct InstanceStateSeq_uniq_>;
#endif
#if DDS_USE_EXPLICIT_TEMPLATES
template class DDS_DCPSUFLSeq <DDS::ViewStateKind, struct ViewStateSeq_uniq_>;
#endif
#if DDS_USE_EXPLICIT_TEMPLATES
template class DDS_DCPSUFLSeq <DDS::SampleStateKind, struct SampleStateSeq_uniq_>;
#endif
#if DDS_USE_EXPLICIT_TEMPLATES
template class DDS_DCPSUObjSeq <DDS::Condition, struct ConditionSeq_uniq_>;
#endif
#if DDS_USE_EXPLICIT_TEMPLATES
template class DDS_DCPSUObjSeq <DDS::DataReader, struct DataReaderSeq_uniq_>;
#endif
#if DDS_USE_EXPLICIT_TEMPLATES
template class DDS_DCPSUObjSeq <DDS::Topic, struct TopicSeq_uniq_>;
#endif
#if DDS_USE_EXPLICIT_TEMPLATES
template class DDS_DCPSUFLSeq <DDS::QosPolicyCount, struct QosPolicyCountSeq_uniq_>;
#endif
#if DDS_USE_EXPLICIT_TEMPLATES
template class DDS_DCPSUFLSeq <DDS::InstanceHandle_t, struct InstanceHandleSeq_uniq_>;
#endif

const char * DDS::Listener::_local_id = "IDL:DDS/Listener:1.0";

DDS::Listener_ptr DDS::Listener::_duplicate (DDS::Listener_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::Listener::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::Listener::_local_id) == 0)
   {
      return TRUE;
   }

   return FALSE;
}

DDS::Listener_ptr DDS::Listener::_narrow (DDS::Object_ptr p)
{
   DDS::Listener_ptr result = NULL;
   if (p && p->_is_a (DDS::Listener::_local_id))
   {
      result = dynamic_cast<DDS::Listener_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::Listener_ptr DDS::Listener::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::Listener_ptr result;
   result = dynamic_cast<DDS::Listener_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::TopicListener::_local_id = "IDL:DDS/TopicListener:1.0";

DDS::TopicListener_ptr DDS::TopicListener::_duplicate (DDS::TopicListener_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::TopicListener::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::TopicListener::_local_id) == 0)
   {
      return TRUE;
   }

   typedef Listener NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::TopicListener_ptr DDS::TopicListener::_narrow (DDS::Object_ptr p)
{
   DDS::TopicListener_ptr result = NULL;
   if (p && p->_is_a (DDS::TopicListener::_local_id))
   {
      result = dynamic_cast<DDS::TopicListener_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::TopicListener_ptr DDS::TopicListener::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::TopicListener_ptr result;
   result = dynamic_cast<DDS::TopicListener_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::ExtTopicListener::_local_id = "IDL:DDS/ExtTopicListener:1.0";

DDS::ExtTopicListener_ptr DDS::ExtTopicListener::_duplicate (DDS::ExtTopicListener_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::ExtTopicListener::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::ExtTopicListener::_local_id) == 0)
   {
      return TRUE;
   }

   typedef TopicListener NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::ExtTopicListener_ptr DDS::ExtTopicListener::_narrow (DDS::Object_ptr p)
{
   DDS::ExtTopicListener_ptr result = NULL;
   if (p && p->_is_a (DDS::ExtTopicListener::_local_id))
   {
      result = dynamic_cast<DDS::ExtTopicListener_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::ExtTopicListener_ptr DDS::ExtTopicListener::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::ExtTopicListener_ptr result;
   result = dynamic_cast<DDS::ExtTopicListener_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::DataWriterListener::_local_id = "IDL:DDS/DataWriterListener:1.0";

DDS::DataWriterListener_ptr DDS::DataWriterListener::_duplicate (DDS::DataWriterListener_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::DataWriterListener::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::DataWriterListener::_local_id) == 0)
   {
      return TRUE;
   }

   typedef Listener NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::DataWriterListener_ptr DDS::DataWriterListener::_narrow (DDS::Object_ptr p)
{
   DDS::DataWriterListener_ptr result = NULL;
   if (p && p->_is_a (DDS::DataWriterListener::_local_id))
   {
      result = dynamic_cast<DDS::DataWriterListener_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::DataWriterListener_ptr DDS::DataWriterListener::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::DataWriterListener_ptr result;
   result = dynamic_cast<DDS::DataWriterListener_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::PublisherListener::_local_id = "IDL:DDS/PublisherListener:1.0";

DDS::PublisherListener_ptr DDS::PublisherListener::_duplicate (DDS::PublisherListener_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::PublisherListener::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::PublisherListener::_local_id) == 0)
   {
      return TRUE;
   }

   typedef DataWriterListener NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::PublisherListener_ptr DDS::PublisherListener::_narrow (DDS::Object_ptr p)
{
   DDS::PublisherListener_ptr result = NULL;
   if (p && p->_is_a (DDS::PublisherListener::_local_id))
   {
      result = dynamic_cast<DDS::PublisherListener_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::PublisherListener_ptr DDS::PublisherListener::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::PublisherListener_ptr result;
   result = dynamic_cast<DDS::PublisherListener_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::DataReaderListener::_local_id = "IDL:DDS/DataReaderListener:1.0";

DDS::DataReaderListener_ptr DDS::DataReaderListener::_duplicate (DDS::DataReaderListener_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::DataReaderListener::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::DataReaderListener::_local_id) == 0)
   {
      return TRUE;
   }

   typedef Listener NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::DataReaderListener_ptr DDS::DataReaderListener::_narrow (DDS::Object_ptr p)
{
   DDS::DataReaderListener_ptr result = NULL;
   if (p && p->_is_a (DDS::DataReaderListener::_local_id))
   {
      result = dynamic_cast<DDS::DataReaderListener_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::DataReaderListener_ptr DDS::DataReaderListener::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::DataReaderListener_ptr result;
   result = dynamic_cast<DDS::DataReaderListener_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::SubscriberListener::_local_id = "IDL:DDS/SubscriberListener:1.0";

DDS::SubscriberListener_ptr DDS::SubscriberListener::_duplicate (DDS::SubscriberListener_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::SubscriberListener::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::SubscriberListener::_local_id) == 0)
   {
      return TRUE;
   }

   typedef DataReaderListener NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::SubscriberListener_ptr DDS::SubscriberListener::_narrow (DDS::Object_ptr p)
{
   DDS::SubscriberListener_ptr result = NULL;
   if (p && p->_is_a (DDS::SubscriberListener::_local_id))
   {
      result = dynamic_cast<DDS::SubscriberListener_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::SubscriberListener_ptr DDS::SubscriberListener::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::SubscriberListener_ptr result;
   result = dynamic_cast<DDS::SubscriberListener_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::DomainParticipantListener::_local_id = "IDL:DDS/DomainParticipantListener:1.0";

DDS::DomainParticipantListener_ptr DDS::DomainParticipantListener::_duplicate (DDS::DomainParticipantListener_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::DomainParticipantListener::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::DomainParticipantListener::_local_id) == 0)
   {
      return TRUE;
   }

   typedef TopicListener NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   typedef PublisherListener NestedBase_2;

   if (NestedBase_2::_local_is_a (_id))
   {
      return TRUE;
   }

   typedef SubscriberListener NestedBase_3;

   if (NestedBase_3::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::DomainParticipantListener_ptr DDS::DomainParticipantListener::_narrow (DDS::Object_ptr p)
{
   DDS::DomainParticipantListener_ptr result = NULL;
   if (p && p->_is_a (DDS::DomainParticipantListener::_local_id))
   {
      result = dynamic_cast<DDS::DomainParticipantListener_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::DomainParticipantListener_ptr DDS::DomainParticipantListener::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::DomainParticipantListener_ptr result;
   result = dynamic_cast<DDS::DomainParticipantListener_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::ExtDomainParticipantListener::_local_id = "IDL:DDS/ExtDomainParticipantListener:1.0";

DDS::ExtDomainParticipantListener_ptr DDS::ExtDomainParticipantListener::_duplicate (DDS::ExtDomainParticipantListener_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::ExtDomainParticipantListener::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::ExtDomainParticipantListener::_local_id) == 0)
   {
      return TRUE;
   }

   typedef ExtTopicListener NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   typedef DomainParticipantListener NestedBase_2;

   if (NestedBase_2::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::ExtDomainParticipantListener_ptr DDS::ExtDomainParticipantListener::_narrow (DDS::Object_ptr p)
{
   DDS::ExtDomainParticipantListener_ptr result = NULL;
   if (p && p->_is_a (DDS::ExtDomainParticipantListener::_local_id))
   {
      result = dynamic_cast<DDS::ExtDomainParticipantListener_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::ExtDomainParticipantListener_ptr DDS::ExtDomainParticipantListener::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::ExtDomainParticipantListener_ptr result;
   result = dynamic_cast<DDS::ExtDomainParticipantListener_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::Condition::_local_id = "IDL:DDS/Condition:1.0";

DDS::Condition_ptr DDS::Condition::_duplicate (DDS::Condition_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::Condition::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::Condition::_local_id) == 0)
   {
      return TRUE;
   }

   return FALSE;
}

DDS::Condition_ptr DDS::Condition::_narrow (DDS::Object_ptr p)
{
   DDS::Condition_ptr result = NULL;
   if (p && p->_is_a (DDS::Condition::_local_id))
   {
      result = dynamic_cast<DDS::Condition_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::Condition_ptr DDS::Condition::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::Condition_ptr result;
   result = dynamic_cast<DDS::Condition_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::WaitSetInterface::_local_id = "IDL:DDS/WaitSetInterface:1.0";

DDS::WaitSetInterface_ptr DDS::WaitSetInterface::_duplicate (DDS::WaitSetInterface_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::WaitSetInterface::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::WaitSetInterface::_local_id) == 0)
   {
      return TRUE;
   }

   return FALSE;
}

DDS::WaitSetInterface_ptr DDS::WaitSetInterface::_narrow (DDS::Object_ptr p)
{
   DDS::WaitSetInterface_ptr result = NULL;
   if (p && p->_is_a (DDS::WaitSetInterface::_local_id))
   {
      result = dynamic_cast<DDS::WaitSetInterface_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::WaitSetInterface_ptr DDS::WaitSetInterface::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::WaitSetInterface_ptr result;
   result = dynamic_cast<DDS::WaitSetInterface_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::GuardConditionInterface::_local_id = "IDL:DDS/GuardConditionInterface:1.0";

DDS::GuardConditionInterface_ptr DDS::GuardConditionInterface::_duplicate (DDS::GuardConditionInterface_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::GuardConditionInterface::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::GuardConditionInterface::_local_id) == 0)
   {
      return TRUE;
   }

   typedef Condition NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::GuardConditionInterface_ptr DDS::GuardConditionInterface::_narrow (DDS::Object_ptr p)
{
   DDS::GuardConditionInterface_ptr result = NULL;
   if (p && p->_is_a (DDS::GuardConditionInterface::_local_id))
   {
      result = dynamic_cast<DDS::GuardConditionInterface_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::GuardConditionInterface_ptr DDS::GuardConditionInterface::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::GuardConditionInterface_ptr result;
   result = dynamic_cast<DDS::GuardConditionInterface_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::StatusCondition::_local_id = "IDL:DDS/StatusCondition:1.0";

DDS::StatusCondition_ptr DDS::StatusCondition::_duplicate (DDS::StatusCondition_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::StatusCondition::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::StatusCondition::_local_id) == 0)
   {
      return TRUE;
   }

   typedef Condition NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::StatusCondition_ptr DDS::StatusCondition::_narrow (DDS::Object_ptr p)
{
   DDS::StatusCondition_ptr result = NULL;
   if (p && p->_is_a (DDS::StatusCondition::_local_id))
   {
      result = dynamic_cast<DDS::StatusCondition_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::StatusCondition_ptr DDS::StatusCondition::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::StatusCondition_ptr result;
   result = dynamic_cast<DDS::StatusCondition_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::ReadCondition::_local_id = "IDL:DDS/ReadCondition:1.0";

DDS::ReadCondition_ptr DDS::ReadCondition::_duplicate (DDS::ReadCondition_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::ReadCondition::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::ReadCondition::_local_id) == 0)
   {
      return TRUE;
   }

   typedef Condition NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::ReadCondition_ptr DDS::ReadCondition::_narrow (DDS::Object_ptr p)
{
   DDS::ReadCondition_ptr result = NULL;
   if (p && p->_is_a (DDS::ReadCondition::_local_id))
   {
      result = dynamic_cast<DDS::ReadCondition_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::ReadCondition_ptr DDS::ReadCondition::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::ReadCondition_ptr result;
   result = dynamic_cast<DDS::ReadCondition_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::QueryCondition::_local_id = "IDL:DDS/QueryCondition:1.0";

DDS::QueryCondition_ptr DDS::QueryCondition::_duplicate (DDS::QueryCondition_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::QueryCondition::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::QueryCondition::_local_id) == 0)
   {
      return TRUE;
   }

   typedef ReadCondition NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::QueryCondition_ptr DDS::QueryCondition::_narrow (DDS::Object_ptr p)
{
   DDS::QueryCondition_ptr result = NULL;
   if (p && p->_is_a (DDS::QueryCondition::_local_id))
   {
      result = dynamic_cast<DDS::QueryCondition_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::QueryCondition_ptr DDS::QueryCondition::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::QueryCondition_ptr result;
   result = dynamic_cast<DDS::QueryCondition_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::Entity::_local_id = "IDL:DDS/Entity:1.0";

DDS::Entity_ptr DDS::Entity::_duplicate (DDS::Entity_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::Entity::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::Entity::_local_id) == 0)
   {
      return TRUE;
   }

   return FALSE;
}

DDS::Entity_ptr DDS::Entity::_narrow (DDS::Object_ptr p)
{
   DDS::Entity_ptr result = NULL;
   if (p && p->_is_a (DDS::Entity::_local_id))
   {
      result = dynamic_cast<DDS::Entity_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::Entity_ptr DDS::Entity::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::Entity_ptr result;
   result = dynamic_cast<DDS::Entity_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::Domain::_local_id = "IDL:DDS/Domain:1.0";

DDS::Domain_ptr DDS::Domain::_duplicate (DDS::Domain_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::Domain::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::Domain::_local_id) == 0)
   {
      return TRUE;
   }

   return FALSE;
}

DDS::Domain_ptr DDS::Domain::_narrow (DDS::Object_ptr p)
{
   DDS::Domain_ptr result = NULL;
   if (p && p->_is_a (DDS::Domain::_local_id))
   {
      result = dynamic_cast<DDS::Domain_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::Domain_ptr DDS::Domain::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::Domain_ptr result;
   result = dynamic_cast<DDS::Domain_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::DomainParticipant::_local_id = "IDL:DDS/DomainParticipant:1.0";

DDS::DomainParticipant_ptr DDS::DomainParticipant::_duplicate (DDS::DomainParticipant_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::DomainParticipant::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::DomainParticipant::_local_id) == 0)
   {
      return TRUE;
   }

   typedef Entity NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::DomainParticipant_ptr DDS::DomainParticipant::_narrow (DDS::Object_ptr p)
{
   DDS::DomainParticipant_ptr result = NULL;
   if (p && p->_is_a (DDS::DomainParticipant::_local_id))
   {
      result = dynamic_cast<DDS::DomainParticipant_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::DomainParticipant_ptr DDS::DomainParticipant::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::DomainParticipant_ptr result;
   result = dynamic_cast<DDS::DomainParticipant_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::DomainParticipantFactoryInterface::_local_id = "IDL:DDS/DomainParticipantFactoryInterface:1.0";

DDS::DomainParticipantFactoryInterface_ptr DDS::DomainParticipantFactoryInterface::_duplicate (DDS::DomainParticipantFactoryInterface_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::DomainParticipantFactoryInterface::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::DomainParticipantFactoryInterface::_local_id) == 0)
   {
      return TRUE;
   }

   return FALSE;
}

DDS::DomainParticipantFactoryInterface_ptr DDS::DomainParticipantFactoryInterface::_narrow (DDS::Object_ptr p)
{
   DDS::DomainParticipantFactoryInterface_ptr result = NULL;
   if (p && p->_is_a (DDS::DomainParticipantFactoryInterface::_local_id))
   {
      result = dynamic_cast<DDS::DomainParticipantFactoryInterface_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::DomainParticipantFactoryInterface_ptr DDS::DomainParticipantFactoryInterface::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::DomainParticipantFactoryInterface_ptr result;
   result = dynamic_cast<DDS::DomainParticipantFactoryInterface_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::TypeSupport::_local_id = "IDL:DDS/TypeSupport:1.0";

DDS::TypeSupport_ptr DDS::TypeSupport::_duplicate (DDS::TypeSupport_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::TypeSupport::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::TypeSupport::_local_id) == 0)
   {
      return TRUE;
   }

   return FALSE;
}

DDS::TypeSupport_ptr DDS::TypeSupport::_narrow (DDS::Object_ptr p)
{
   DDS::TypeSupport_ptr result = NULL;
   if (p && p->_is_a (DDS::TypeSupport::_local_id))
   {
      result = dynamic_cast<DDS::TypeSupport_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::TypeSupport_ptr DDS::TypeSupport::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::TypeSupport_ptr result;
   result = dynamic_cast<DDS::TypeSupport_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::TypeSupportFactory::_local_id = "IDL:DDS/TypeSupportFactory:1.0";

DDS::TypeSupportFactory_ptr DDS::TypeSupportFactory::_duplicate (DDS::TypeSupportFactory_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::TypeSupportFactory::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::TypeSupportFactory::_local_id) == 0)
   {
      return TRUE;
   }

   return FALSE;
}

DDS::TypeSupportFactory_ptr DDS::TypeSupportFactory::_narrow (DDS::Object_ptr p)
{
   DDS::TypeSupportFactory_ptr result = NULL;
   if (p && p->_is_a (DDS::TypeSupportFactory::_local_id))
   {
      result = dynamic_cast<DDS::TypeSupportFactory_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::TypeSupportFactory_ptr DDS::TypeSupportFactory::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::TypeSupportFactory_ptr result;
   result = dynamic_cast<DDS::TypeSupportFactory_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::TopicDescription::_local_id = "IDL:DDS/TopicDescription:1.0";

DDS::TopicDescription_ptr DDS::TopicDescription::_duplicate (DDS::TopicDescription_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::TopicDescription::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::TopicDescription::_local_id) == 0)
   {
      return TRUE;
   }

   return FALSE;
}

DDS::TopicDescription_ptr DDS::TopicDescription::_narrow (DDS::Object_ptr p)
{
   DDS::TopicDescription_ptr result = NULL;
   if (p && p->_is_a (DDS::TopicDescription::_local_id))
   {
      result = dynamic_cast<DDS::TopicDescription_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::TopicDescription_ptr DDS::TopicDescription::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::TopicDescription_ptr result;
   result = dynamic_cast<DDS::TopicDescription_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::Topic::_local_id = "IDL:DDS/Topic:1.0";

DDS::Topic_ptr DDS::Topic::_duplicate (DDS::Topic_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::Topic::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::Topic::_local_id) == 0)
   {
      return TRUE;
   }

   typedef Entity NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   typedef TopicDescription NestedBase_2;

   if (NestedBase_2::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::Topic_ptr DDS::Topic::_narrow (DDS::Object_ptr p)
{
   DDS::Topic_ptr result = NULL;
   if (p && p->_is_a (DDS::Topic::_local_id))
   {
      result = dynamic_cast<DDS::Topic_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::Topic_ptr DDS::Topic::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::Topic_ptr result;
   result = dynamic_cast<DDS::Topic_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::ContentFilteredTopic::_local_id = "IDL:DDS/ContentFilteredTopic:1.0";

DDS::ContentFilteredTopic_ptr DDS::ContentFilteredTopic::_duplicate (DDS::ContentFilteredTopic_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::ContentFilteredTopic::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::ContentFilteredTopic::_local_id) == 0)
   {
      return TRUE;
   }

   typedef TopicDescription NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::ContentFilteredTopic_ptr DDS::ContentFilteredTopic::_narrow (DDS::Object_ptr p)
{
   DDS::ContentFilteredTopic_ptr result = NULL;
   if (p && p->_is_a (DDS::ContentFilteredTopic::_local_id))
   {
      result = dynamic_cast<DDS::ContentFilteredTopic_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::ContentFilteredTopic_ptr DDS::ContentFilteredTopic::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::ContentFilteredTopic_ptr result;
   result = dynamic_cast<DDS::ContentFilteredTopic_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::MultiTopic::_local_id = "IDL:DDS/MultiTopic:1.0";

DDS::MultiTopic_ptr DDS::MultiTopic::_duplicate (DDS::MultiTopic_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::MultiTopic::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::MultiTopic::_local_id) == 0)
   {
      return TRUE;
   }

   typedef TopicDescription NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::MultiTopic_ptr DDS::MultiTopic::_narrow (DDS::Object_ptr p)
{
   DDS::MultiTopic_ptr result = NULL;
   if (p && p->_is_a (DDS::MultiTopic::_local_id))
   {
      result = dynamic_cast<DDS::MultiTopic_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::MultiTopic_ptr DDS::MultiTopic::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::MultiTopic_ptr result;
   result = dynamic_cast<DDS::MultiTopic_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::Publisher::_local_id = "IDL:DDS/Publisher:1.0";

DDS::Publisher_ptr DDS::Publisher::_duplicate (DDS::Publisher_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::Publisher::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::Publisher::_local_id) == 0)
   {
      return TRUE;
   }

   typedef Entity NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::Publisher_ptr DDS::Publisher::_narrow (DDS::Object_ptr p)
{
   DDS::Publisher_ptr result = NULL;
   if (p && p->_is_a (DDS::Publisher::_local_id))
   {
      result = dynamic_cast<DDS::Publisher_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::Publisher_ptr DDS::Publisher::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::Publisher_ptr result;
   result = dynamic_cast<DDS::Publisher_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::DataWriter::_local_id = "IDL:DDS/DataWriter:1.0";

DDS::DataWriter_ptr DDS::DataWriter::_duplicate (DDS::DataWriter_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::DataWriter::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::DataWriter::_local_id) == 0)
   {
      return TRUE;
   }

   typedef Entity NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::DataWriter_ptr DDS::DataWriter::_narrow (DDS::Object_ptr p)
{
   DDS::DataWriter_ptr result = NULL;
   if (p && p->_is_a (DDS::DataWriter::_local_id))
   {
      result = dynamic_cast<DDS::DataWriter_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::DataWriter_ptr DDS::DataWriter::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::DataWriter_ptr result;
   result = dynamic_cast<DDS::DataWriter_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::Subscriber::_local_id = "IDL:DDS/Subscriber:1.0";

DDS::Subscriber_ptr DDS::Subscriber::_duplicate (DDS::Subscriber_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::Subscriber::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::Subscriber::_local_id) == 0)
   {
      return TRUE;
   }

   typedef Entity NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::Subscriber_ptr DDS::Subscriber::_narrow (DDS::Object_ptr p)
{
   DDS::Subscriber_ptr result = NULL;
   if (p && p->_is_a (DDS::Subscriber::_local_id))
   {
      result = dynamic_cast<DDS::Subscriber_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::Subscriber_ptr DDS::Subscriber::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::Subscriber_ptr result;
   result = dynamic_cast<DDS::Subscriber_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::DataReader::_local_id = "IDL:DDS/DataReader:1.0";

DDS::DataReader_ptr DDS::DataReader::_duplicate (DDS::DataReader_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::DataReader::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::DataReader::_local_id) == 0)
   {
      return TRUE;
   }

   typedef Entity NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::DataReader_ptr DDS::DataReader::_narrow (DDS::Object_ptr p)
{
   DDS::DataReader_ptr result = NULL;
   if (p && p->_is_a (DDS::DataReader::_local_id))
   {
      result = dynamic_cast<DDS::DataReader_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::DataReader_ptr DDS::DataReader::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::DataReader_ptr result;
   result = dynamic_cast<DDS::DataReader_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::DataReaderView::_local_id = "IDL:DDS/DataReaderView:1.0";

DDS::DataReaderView_ptr DDS::DataReaderView::_duplicate (DDS::DataReaderView_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::DataReaderView::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::DataReaderView::_local_id) == 0)
   {
      return TRUE;
   }

   typedef Entity NestedBase_1;

   if (NestedBase_1::_local_is_a (_id))
   {
      return TRUE;
   }

   return FALSE;
}

DDS::DataReaderView_ptr DDS::DataReaderView::_narrow (DDS::Object_ptr p)
{
   DDS::DataReaderView_ptr result = NULL;
   if (p && p->_is_a (DDS::DataReaderView::_local_id))
   {
      result = dynamic_cast<DDS::DataReaderView_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::DataReaderView_ptr DDS::DataReaderView::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::DataReaderView_ptr result;
   result = dynamic_cast<DDS::DataReaderView_ptr> (p);
   result->m_count++;
   return result;
}

const char * DDS::ErrorInfoInterface::_local_id = "IDL:DDS/ErrorInfoInterface:1.0";

DDS::ErrorInfoInterface_ptr DDS::ErrorInfoInterface::_duplicate (DDS::ErrorInfoInterface_ptr p)
{
   if (p) p->m_count++;
   return p;
}

DDS::Boolean DDS::ErrorInfoInterface::_local_is_a (const char * _id)
{
   if (strcmp (_id, DDS::ErrorInfoInterface::_local_id) == 0)
   {
      return TRUE;
   }

   return FALSE;
}

DDS::ErrorInfoInterface_ptr DDS::ErrorInfoInterface::_narrow (DDS::Object_ptr p)
{
   DDS::ErrorInfoInterface_ptr result = NULL;
   if (p && p->_is_a (DDS::ErrorInfoInterface::_local_id))
   {
      result = dynamic_cast<DDS::ErrorInfoInterface_ptr> (p);
      result->m_count++;
   }
   return result;
}

DDS::ErrorInfoInterface_ptr DDS::ErrorInfoInterface::_unchecked_narrow (DDS::Object_ptr p)
{
   DDS::ErrorInfoInterface_ptr result;
   result = dynamic_cast<DDS::ErrorInfoInterface_ptr> (p);
   result->m_count++;
   return result;
}



