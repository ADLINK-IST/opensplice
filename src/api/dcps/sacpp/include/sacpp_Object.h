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
#ifndef SACPP_OBJECT_H
#define SACPP_OBJECT_H

#include "sacpp_DDS_DCPS.h"
#include "sacpp_Counter.h"
#include "sacpp_if.h"

class SACPP_API DDS_DCPS::Object
{
   friend void DDS_DCPS::release(DDS_DCPS::Object * p);
   friend void DDS_DCPS::release(DDS_DCPS::LocalObject * p);

public:

   static DDS_DCPS::Object_ptr _duplicate(DDS_DCPS::Object_ptr obj);
   static DDS_DCPS::Object_ptr _narrow(DDS_DCPS::Object_ptr obj);
   static DDS_DCPS::Object_ptr _nil();
   
   DDS_DCPS::Boolean _is_a(const char * logical_is_type); 
   virtual DDS_DCPS::Boolean _local_is_a(const char * id);
   virtual DDS_DCPS::Boolean is_local_object();

   Object();

protected:
   DDS_DCPS::Counter m_count;
};


// --------------------------------------------------
//  inline implementations
// --------------------------------------------------

inline DDS_DCPS::Object::Object() : m_count (1)
{
}

inline DDS_DCPS::Object_ptr DDS_DCPS::Object::_nil()
{
   return NULL;
}

inline DDS_DCPS::Object_ptr
DDS_DCPS::Object::_narrow( DDS_DCPS::Object *obj)
{
   return (obj?_duplicate(obj):NULL);
}

inline DDS_DCPS::Object_ptr
DDS_DCPS::Object::_duplicate(DDS_DCPS::Object * p)
{
   if (p && (p->m_count != ~0U)) {
      p->m_count++;
   }

   return p;
}

#undef SACPP_API

#endif /* SACPP_OBJECT_H */
