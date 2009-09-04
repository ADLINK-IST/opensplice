/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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

      Counter (uint32_t val = 0) { m_value = val; };

      operator uint32_t () { return m_value; };

      Counter & operator = (uint32_t val) 
      { 
         m_value = val;
         return *this;
      };

      uint32_t operator ++ (); // prefix (++a)
      uint32_t operator -- (); // prefix (--a)

      uint32_t operator ++ (int); // postfix (a++)
      uint32_t operator -- (int); // postfix (a--)

   private:

      Counter (const Counter&);
      Counter &operator= (const Counter&);

      uint32_t m_value;
};


/******************************************************************************/
/*                               Inline Methods                               */
/******************************************************************************/

inline uint32_t Counter::operator ++ (int)
{
   uint32_t result = m_value;
   pa_increment ((os_uint32*)&m_value);
   return result;
}

inline uint32_t Counter::operator -- (int)
{
   uint32_t result = m_value;
   pa_decrement ((os_uint32*)&m_value);
   return result;
}

inline uint32_t Counter::operator ++ ()
{
   return (pa_increment ((os_uint32*)&m_value));
}

inline uint32_t Counter::operator -- ()
{
   return (pa_decrement ((os_uint32*)&m_value));
}

#undef EXPORT
#endif
