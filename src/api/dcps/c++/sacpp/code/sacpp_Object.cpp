/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
