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
#ifndef SACPP_OBJECT_H
#define SACPP_OBJECT_H

#include "sacpp_DDS_DCPS.h"
#include "sacpp_Counter.h"
#include "sacpp_if.h"

class SACPP_API DDS::Object
{
   friend void DDS::release(DDS::Object * p);
   friend void DDS::release(DDS::LocalObject * p);

public:

   static DDS::Object_ptr _duplicate(DDS::Object_ptr obj);
   static DDS::Object_ptr _narrow(DDS::Object_ptr obj);
   static DDS::Object_ptr _nil();

   DDS::Boolean _is_a(const char * logical_is_type);
   virtual DDS::Boolean _local_is_a(const char * id);
   virtual DDS::Boolean is_local_object();

   Object();

protected:
   DDS_DCPS_Counter m_count;
   virtual ~Object();
};


// --------------------------------------------------
//  inline implementations
// --------------------------------------------------

inline DDS::Object::Object() : m_count (1)
{
}

inline DDS::Object_ptr DDS::Object::_nil()
{
   return NULL;
}

inline DDS::Object_ptr
DDS::Object::_narrow(DDS::Object *obj)
{
   return (obj?_duplicate(obj):NULL);
}

inline DDS::Object_ptr
DDS::Object::_duplicate(DDS::Object * p)
{
   if (p && (p->m_count != ~0U)) {
      p->m_count++;
   }

   return p;
}

#undef SACPP_API

#endif /* SACPP_OBJECT_H */
