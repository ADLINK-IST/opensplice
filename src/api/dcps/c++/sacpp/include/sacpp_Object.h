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
#ifndef SACPP_OBJECT_H
#define SACPP_OBJECT_H

#include "sacpp_dds_basic_types.h"
#include "sacpp_Counter.h"
#include "cpp_dcps_if.h"

class OS_API DDS::Object
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

#undef OS_API

#endif /* SACPP_OBJECT_H */
