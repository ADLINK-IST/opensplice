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
#include "sacpp_Object.h"

DDS::Boolean
DDS::Object::_local_is_a(const char * id)
{
   return (strcmp(id, "IDL:omg.org/DDS_DCPS/Object:1.0") == 0);
}

DDS::Boolean
DDS::Object::is_local_object()
{
   return FALSE;
}

void
DDS::release(DDS::Object * p)
{
   if (p && (p->m_count != ~0U) && (--(p->m_count) == 0)) {
      delete p;
   }
}

DDS::Boolean
DDS::Object::_is_a(const char * id)
{
   DDS::Boolean rt = false;

   if (this->_local_is_a (id)) {
      return true;
   }

   return rt;
}

DDS::Object::~Object()
{
}
