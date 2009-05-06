#ifndef SACPP_COUNTER_H
#define SACPP_COUNTER_H

#include "sacpp_dcps.h"
#include "os_abstract.h"
#include "sacpp_if.h"

class SACPP_API SACPP_DCPS::Counter
{
   public:
      Counter(os_uint32 val = 0) {
          m_value = val;
      };

      operator os_uint32 () {
          return m_value;
      };

      Counter & operator = (os_uint32 val) { 
         m_value = val;
         return *this;
      };

      os_uint32 operator ++ (); // prefix (++a)
      os_uint32 operator -- (); // prefix (--a)

      os_uint32 operator ++ (int); // postfix (a++)
      os_uint32 operator -- (int); // postfix (a--)

   private:

      Counter(const Counter&);
      Counter &operator= (const Counter&);

      os_uint32 m_value;
};


/******************************************************************************/
/*                               Inline Methods                               */
/******************************************************************************/

inline os_uint32 SACPP_DCPS::Counter::operator ++ (int)
{
   os_uint32 result = m_value;
   pa_increment ((os_uint32*)&m_value);
   return result;
}

inline os_uint32 SACPP_DCPS::Counter::operator -- (int)
{
   os_uint32 result = m_value;
   pa_decrement ((os_uint32*)&m_value);
   return result;
}

inline os_uint32 SACPP_DCPS::Counter::operator ++ ()
{
   return (pa_increment((os_uint32*)&m_value));
}

inline os_uint32 SACPP_DCPS::Counter::operator -- ()
{
   return (pa_decrement((os_uint32*)&m_value));
}

#undef SACPP_API
#endif
