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
#ifndef SACPP_VALUEBASE_H
#define SACPP_VALUEBASE_H

#include "sacpp_dds_basic_types.h"
#include "cpp_dcps_if.h"

namespace DDS
{
   class OS_API ValueBase
   {
      public:
         virtual ValueBase* _add_ref() = 0;

         virtual void _remove_ref() = 0;

         virtual ValueBase* _copy_value() = 0;

         virtual ULong _refcount_value() = 0;

         static ValueBase* _downcast(ValueBase*);

      protected:
         ValueBase();

         ValueBase(const ValueBase&);

         virtual ~ValueBase();

      private:
         void operator=(const ValueBase&);
   };
}
#undef OS_API

#endif /* SACPP_VALUEBASE_H */
