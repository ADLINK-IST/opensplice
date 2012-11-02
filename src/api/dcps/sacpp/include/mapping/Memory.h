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
#ifndef SACPP_MAPPING_MEMORY_H
#define SACPP_MAPPING_MEMORY_H

#include "sacpp_if.h"

#define DDS_DCPS_MEM_ALIGN 8

class SACPP_API DDS_DCPS_Memory
{
public:

   static inline void * _vec_alloc (unsigned long tsize, unsigned long nelems)
   {
      char * buffer = 0;
      buffer = new char [DDS_DCPS_MEM_ALIGN + (nelems * tsize)];

      if (buffer)
      {
         *(unsigned long*) buffer = nelems;
         return (void*) (buffer + DDS_DCPS_MEM_ALIGN);
      }
      return 0;
   }

   static inline unsigned long _vec_size (void * buffer)
   {
      return *(unsigned long*) (((char*) buffer) - DDS_DCPS_MEM_ALIGN);
   }

   static inline void _vec_dealloc (void * buffer)
   {
      delete [] (((char*) buffer) - DDS_DCPS_MEM_ALIGN);
   }
};

#undef SACPP_API
#endif /* SACPP_MAPPING_MEMORY_H */
