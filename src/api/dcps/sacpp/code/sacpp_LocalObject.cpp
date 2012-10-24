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
#include "os_abstract.h"
#include "sacpp_LocalObject.h"

DDS::LocalObject::LocalObject()
{
}

DDS::LocalObject::~LocalObject()
{
}

void DDS::LocalObject::_add_ref()
{
}

void DDS::LocalObject::_remove_ref()
{
}

DDS::Boolean
DDS::LocalObject::_is_a(  
   const char * logical_type_id)    
{    
   return (this->_local_is_a(logical_type_id));  
}

DDS::Boolean
DDS::LocalObject::_is_equivalent(
   const DDS::Object * obj)
{
   return (obj == this);
}

DDS::Boolean
DDS::LocalObject::is_local_object()
{
   return TRUE;
}

DDS::ULong
DDS::LocalObject::_hash(DDS::ULong maximum)
{
   DDS::ULong hash;

   hash =
    static_cast<DDS::ULong> (reinterpret_cast<DDS::ULongLong> (this));

   if (maximum) {
      hash = hash % maximum;
   }
   return hash;
}

void
DDS::release(DDS::LocalObject * p)
{
   if (p && (p->m_count != ~0U) && (--(p->m_count) == 0)) {
      delete p;
   }
}
