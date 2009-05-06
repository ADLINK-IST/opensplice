/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "sacpp_orb_pa.h"
#include "os_abstract.h"
#include "sacpp_Object.h"

DDS_DCPS::Boolean
DDS_DCPS::Object::_local_is_a(const char * id)
{
   return (strcmp(id, "IDL:omg.org/DDS_DCPS/Object:1.0") == 0);
}

DDS_DCPS::Boolean
DDS_DCPS::Object::is_local_object()
{  
   return FALSE;
} 

void
DDS_DCPS::release(DDS_DCPS::Object * p)
{
   if (p && (p->m_count != ~0U) && (--(p->m_count) == 0)) {
      delete p;
   }
}

DDS_DCPS::Boolean
DDS_DCPS::Object::_is_a(const char * id)    
{
   DDS_DCPS::Boolean rt = false;

   if (this->_local_is_a (id)) {
      return true;
   }

   return rt;
}
