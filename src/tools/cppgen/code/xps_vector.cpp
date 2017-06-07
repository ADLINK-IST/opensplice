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
#include <string.h>
#include "xps_vector.h"

DDSVectorBase::DDSVectorBase (DDS::ULong blockSize) :
   m_blockSize (blockSize),
   m_size (0),
   m_pointerBlock (new void*[blockSize])
{
   assert (blockSize > 0);
   memset (m_pointerBlock, 0, sizeof(void*) * blockSize);
}

/* Copy constructor: caller is responsible for copying pointer vector */

DDSVectorBase::DDSVectorBase (const DDSVectorBase & that) :
   m_blockSize (that.m_blockSize),
   m_size (that.m_size),
   m_pointerBlock (new void*[m_blockSize])
{
   assert(m_blockSize > 0);
   memset (m_pointerBlock, 0, sizeof(void*) * m_blockSize);
}

DDSVectorBase::~DDSVectorBase ()
{
   for (unsigned int i = 0; i < m_size; i++)
   {
      delete (char*) m_pointerBlock[i];
   }

   delete[] m_pointerBlock;
}

void DDSVectorBase::push_back (void * pointer)
{
   if (m_size >= m_blockSize)
   {
      DDS::ULong oldBlockSize = m_blockSize;

      do  // double block size until it's big enough
      {
         m_blockSize *= 2;
      }
      while (m_size >= m_blockSize);

      void ** newBlock = new void * [m_blockSize];

      memcpy (newBlock, m_pointerBlock, sizeof(void*) * oldBlockSize);

      memset (newBlock + oldBlockSize, 0, sizeof(void*) * (m_blockSize-oldBlockSize));

      delete[] m_pointerBlock;
      m_pointerBlock = newBlock;
   }

   operator[](m_size) = pointer;
   m_size++;
}
