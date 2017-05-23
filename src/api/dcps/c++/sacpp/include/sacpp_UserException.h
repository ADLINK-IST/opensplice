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
#ifndef SACPP_USEREXCEPTION_H
#define SACPP_USEREXCEPTION_H

#include "sacpp_dds_basic_types.h"
#include "sacpp_Exception.h"
#include "cpp_dcps_if.h"

class OS_API DDS::UserException : public DDS::Exception
{
public:

   static DDS::UserException* _downcast (DDS::Exception* exc);
   static const DDS::UserException* _downcast (const DDS::Exception* exc);

   virtual UserException *_as_user();
};

#undef OS_API
#endif
