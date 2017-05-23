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
#ifndef SACPP_COUNTER_H
#define SACPP_COUNTER_H

#include "os_abstract.h"
#include "os_atomics.h"
#include "cpp_dcps_if.h"

class OS_API DDS_DCPS_Counter
{
   public:
      DDS_DCPS_Counter(os_uint32 val = 0) {
          pa_st32(&m_value, val);
      };

      operator os_uint32 () {
          return pa_ld32(&m_value);
      };

      DDS_DCPS_Counter & operator = (os_uint32 val) { 
         pa_st32(&m_value, val);
         return *this;
      };

      os_uint32 operator ++ (); // prefix (++a)
      os_uint32 operator -- (); // prefix (--a)

      os_uint32 operator ++ (int); // postfix (a++)
      os_uint32 operator -- (int); // postfix (a--)

   private:

      DDS_DCPS_Counter(const DDS_DCPS_Counter&);
      DDS_DCPS_Counter &operator= (const DDS_DCPS_Counter&);

      pa_uint32_t m_value;
};


/******************************************************************************/
/*                               Inline Methods                               */
/******************************************************************************/

inline os_uint32 DDS_DCPS_Counter::operator ++ (int)
{
   return pa_inc32_nv (&m_value) - 1;
}

inline os_uint32 DDS_DCPS_Counter::operator -- (int)
{
   return pa_dec32_nv (&m_value) + 1;
}

inline os_uint32 DDS_DCPS_Counter::operator ++ ()
{
   return pa_inc32_nv (&m_value);
}

inline os_uint32 DDS_DCPS_Counter::operator -- ()
{
   return pa_dec32_nv (&m_value);
}

#undef OS_API
#endif /* SACPP_COUNTER_H */
