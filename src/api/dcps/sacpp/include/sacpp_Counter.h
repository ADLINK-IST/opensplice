/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef SACPP_COUNTER_H
#define SACPP_COUNTER_H

#include "os_abstract.h"
#include "sacpp_if.h"

class SACPP_API DDS_DCPS_Counter
{
   public:
      DDS_DCPS_Counter(os_uint32 val = 0) {
          m_value = val;
      };

      operator os_uint32 () {
          return m_value;
      };

      DDS_DCPS_Counter & operator = (os_uint32 val) { 
         m_value = val;
         return *this;
      };

      os_uint32 operator ++ (); // prefix (++a)
      os_uint32 operator -- (); // prefix (--a)

      os_uint32 operator ++ (int); // postfix (a++)
      os_uint32 operator -- (int); // postfix (a--)

   private:

      DDS_DCPS_Counter(const DDS_DCPS_Counter&);
      DDS_DCPS_Counter &operator= (const DDS_DCPS_Counter&);

      os_uint32 m_value;
};


/******************************************************************************/
/*                               Inline Methods                               */
/******************************************************************************/

inline os_uint32 DDS_DCPS_Counter::operator ++ (int)
{
   os_uint32 result = m_value;
   pa_increment ((os_uint32*)&m_value);
   return result;
}

inline os_uint32 DDS_DCPS_Counter::operator -- (int)
{
   os_uint32 result = m_value;
   pa_decrement ((os_uint32*)&m_value);
   return result;
}

inline os_uint32 DDS_DCPS_Counter::operator ++ ()
{
   return (pa_increment((os_uint32*)&m_value));
}

inline os_uint32 DDS_DCPS_Counter::operator -- ()
{
   return (pa_decrement((os_uint32*)&m_value));
}

#undef SACPP_API
#endif /* SACPP_COUNTER_H */
