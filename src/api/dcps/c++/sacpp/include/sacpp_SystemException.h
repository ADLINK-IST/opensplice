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
#ifndef SACPP_SYSTEMEXCEPTION_H
#define SACPP_SYSTEMEXCEPTION_H

#include "sacpp_dds_basic_types.h"
#include "sacpp_Exception.h"
#include "cpp_dcps_if.h"

#ifdef minor
#undef minor
#endif

class OS_API DDS::SystemException : public DDS::Exception
{
public:

   SystemException ();
   SystemException (DDS::ULong minor, DDS::CompletionStatus status);

   static DDS::SystemException * _downcast (DDS::Exception*);
   static const DDS::SystemException * _downcast (const DDS::Exception*);

   DDS::ULong minor () const;
   void minor (DDS::ULong minor);

   DDS::CompletionStatus completed () const;
   void completed (DDS::CompletionStatus status);

   virtual SystemException *_as_system();

private:

   DDS::ULong m_minor;
   DDS::CompletionStatus m_status;
};

#undef OS_API
#endif
