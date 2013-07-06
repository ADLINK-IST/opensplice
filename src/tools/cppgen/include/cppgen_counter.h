/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
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
   os_uint32 result = m_value;
   pa_increment ((os_uint32*)&m_value);
   return result;
}

inline os_uint32 Counter::operator -- (int)
{
   os_uint32 result = m_value;
   pa_decrement ((os_uint32*)&m_value);
   return result;
}

inline os_uint32 Counter::operator ++ ()
{
   return (pa_increment ((os_uint32*)&m_value));
}

inline os_uint32 Counter::operator -- ()
{
   return (pa_decrement ((os_uint32*)&m_value));
}

#undef EXPORT
#endif
