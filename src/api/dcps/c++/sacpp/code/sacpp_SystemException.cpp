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
#include "sacpp_SystemException.h"

DDS::SystemException::SystemException() :
   m_minor (0),
   m_status (DDS::COMPLETED_NO)
{
}

DDS::SystemException::SystemException (DDS::ULong minor, DDS::CompletionStatus status) :
   m_minor (minor),
   m_status (status)
{
}

DDS::SystemException * DDS::SystemException::_downcast 
   (DDS::Exception * e)
{
   return e ? e->_as_system() : 0;
}

const DDS::SystemException * DDS::SystemException::_downcast 
   (const DDS::Exception * e)
{
   return _downcast ((DDS::Exception*) e);
}

DDS::ULong DDS::SystemException::minor () const
{
   return m_minor;
}

void DDS::SystemException::minor (DDS::ULong minor)
{
   m_minor = minor;
}

DDS::CompletionStatus DDS::SystemException::completed () const
{
   return m_status;
}

void DDS::SystemException::completed (DDS::CompletionStatus status)
{
   m_status = status;
}

DDS::SystemException *DDS::SystemException::_as_system()
{
   return this;
}
