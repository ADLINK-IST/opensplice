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
    ++m_count;
}

void DDS::LocalObject::_remove_ref()
{
    if (m_count != ~0U && --m_count == 0) {
       delete this;
    }
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
