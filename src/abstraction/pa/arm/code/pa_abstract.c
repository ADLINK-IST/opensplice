/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2014 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/*
 * Implements PA abstraction for arm. Currently only gcc/linux
 */

/* The oldest version of gcc on our supported platforms is 4.4, and the __sync
   operations are available there. They may also be present on earlier versions
   but we don't know.
*/

#if (__GNUC__ * 100 + __GNUC_MINOR__) >= 404

   #ifdef pa_increment
      #undef pa_increment
   #endif

   #ifdef pa_decrement
      #undef pa_decrement
   #endif

   os_uint32 pa_increment(os_uint32 *count)
   {
      return __sync_add_and_fetch (count, 1);
   }

   os_uint32 pa_decrement(os_uint32 *count)
   {
      return __sync_sub_and_fetch (count, 1);
   }

#else

   #error atomic increment and decrement operations not implemented for this platform

#endif
