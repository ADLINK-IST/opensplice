#include "sacpp_orb_pa.h"
#include "os_abstract.h"
#include "sacpp_LocalObject.h"

DDS_DCPS::LocalObject::LocalObject()
{
}

DDS_DCPS::LocalObject::~LocalObject()
{
}

void DDS_DCPS::LocalObject::_add_ref()
{
}

void DDS_DCPS::LocalObject::_remove_ref()
{
}

DDS_DCPS::Boolean
DDS_DCPS::LocalObject::_is_a(  
   const char * logical_type_id)    
{    
   return (this->_local_is_a(logical_type_id));  
}

DDS_DCPS::Boolean
DDS_DCPS::LocalObject::_is_equivalent(
   const DDS_DCPS::Object * obj)
{
   return (obj == this);
}

DDS_DCPS::Boolean
DDS_DCPS::LocalObject::is_local_object()
{
   return TRUE;
}

DDS_DCPS::ULong
DDS_DCPS::LocalObject::_hash(DDS_DCPS::ULong maximum)
{
   DDS_DCPS::ULong hash;

   hash = (DDS_DCPS::ULong)this;

   if (maximum) {
      hash = hash % maximum;
   }
   return hash;
}

void
DDS_DCPS::release(DDS_DCPS::LocalObject * p)
{
   if (p && (p->m_count != ~0U) && (--(p->m_count) == 0)) {
      delete p;
   }
}
