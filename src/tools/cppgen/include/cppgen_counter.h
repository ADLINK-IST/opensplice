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
#ifndef _DDS_COUNTER_H_
#define _DDS_COUNTER_H_

#include "os_abstract.h"

class Counter
{
   public:

      Counter (os_uint32 val = 0) { m_value = val; };

      operator os_uint32 () { return m_value; };

      Counter & operator = (os_uint32 val) 
      { 
         m_value = val;
         return *this;
      };

      os_uint32 operator ++ (); // prefix (++a)
      os_uint32 operator -- (); // prefix (--a)

      os_uint32 operator ++ (int); // postfix (a++)
      os_uint32 operator -- (int); // postfix (a--)

   private:

      Counter (const Counter&);
      Counter &operator= (const Counter&);

      os_uint32 m_value;
};


/******************************************************************************/
/*                               Inline Methods                               */
/******************************************************************************/

inline os_uint32 Counter::operator ++ (int)
{
   return m_value++;
}

inline os_uint32 Counter::operator -- (int)
{
   return m_value--;
}

inline os_uint32 Counter::operator ++ ()
{
   return ++m_value;
}

inline os_uint32 Counter::operator -- ()
{
   return --m_value;
}

#undef EXPORT
#endif
