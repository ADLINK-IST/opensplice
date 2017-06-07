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

#ifndef _SACPP_MEMORY_H_
#define _SACPP_MEMORY_H_

#include "vortex_os.h"

#define DDS_MEM_ALIGN 8

class DDS_Memory
{
public:

   static inline void * _vec_alloc (unsigned long tsize, unsigned long nelems)
   {
      char * buffer = 0;
      buffer = new char [DDS_MEM_ALIGN + (nelems * tsize)];

      if (buffer)
      {
         *(unsigned long*) buffer = nelems;
         return (void*) (buffer + DDS_MEM_ALIGN);
      }
      return 0;
   }

   static inline unsigned long _vec_size (void * buffer)
   {
      return *(unsigned long*) (((char*) buffer) - DDS_MEM_ALIGN);
   }

   static inline void _vec_dealloc (void * buffer)
   {
      delete [] (((char*) buffer) - DDS_MEM_ALIGN);
   }
};

#endif
