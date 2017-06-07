/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef SACPP_LOCALOBJECT_H
#define SACPP_LOCALOBJECT_H

#include "sacpp_Object.h"
#include "cpp_dcps_if.h"

class OS_API DDS::LocalObject : virtual public DDS::Object
{
   friend void DDS::release(DDS::LocalObject * p);

public:

   static DDS::LocalObject_ptr _duplicate(DDS::LocalObject_ptr obj);
   static DDS::LocalObject_ptr _narrow(DDS::LocalObject_ptr obj);
   static DDS::LocalObject_ptr _nil();

   virtual void _add_ref();
   virtual void _remove_ref();
   virtual DDS::Boolean _is_a(const char * id);
   virtual DDS::ULong _hash(DDS::ULong maximum);

   virtual DDS::Boolean _is_equivalent(const DDS::Object * obj);

   virtual DDS::Boolean is_local_object();

   LocalObject();

protected:

   virtual ~LocalObject ();
};

inline DDS::LocalObject_ptr DDS::LocalObject::_nil()
{
   return NULL;
}

inline DDS::LocalObject_ptr
DDS::LocalObject::_narrow(DDS::LocalObject *obj)
{
   return (obj?_duplicate(obj):NULL);
}

inline DDS::LocalObject_ptr
DDS::LocalObject::_duplicate(DDS::LocalObject * p)
{
   if (p && (p->m_count != ~0U)) {
      p->m_count++;
   }

   return p;
}

#undef OS_API
#endif /* SACPP_LOCALOBJECT_H */
