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
#include "sacpp_ValueBase.h"

DDS::ValueBase* DDS::ValueBase::_downcast(DDS::ValueBase* v)
{
   return v;
}

DDS::ValueBase::ValueBase()
{
}

DDS::ValueBase::ValueBase(const DDS::ValueBase&)
{
}

DDS::ValueBase::~ValueBase()
{
}

void DDS::add_ref(DDS::ValueBase* vb)
{
   if (vb)
   {
      vb->_add_ref();
   }
}

void DDS::remove_ref(DDS::ValueBase* vb)
{
   if (vb)
   {
      vb->_remove_ref();
   }
}
