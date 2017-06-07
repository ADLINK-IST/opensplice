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
#ifndef _tlist_hh
#define _tlist_hh

#include "StdList.h"

template <class T>

class TList : public StdList
{
public:

   class iterator : StdList::iterator
   {

   public:

      iterator()
      {}

      iterator(
         node * position)
            :
            StdList::iterator(position)
      {}

      unsigned char
      operator==(
         const iterator & it) const
      {
         return (unsigned char) (it.m_position == m_position);
      }

      unsigned char
      operator!=(
         const iterator & it) const
      {
         return (unsigned char) (it.m_position != m_position);
      }

      T& operator*()
      {
         return (T&) m_position->m_val;
      }

      iterator&
      operator++()
      {
         m_position = m_position->m_next;
         return *this;
      }

      iterator
      operator++(int dummy)
      {
         TYPENAME TList<T>::iterator tmp = *this;
         m_position = m_position->m_next;
         return tmp;
      }

      iterator&
      operator--()
      {
         m_position = m_position->m_prev;
         return *this;
      }

      iterator
      operator--(int dummy)
      {
         TYPENAME TList<T>::iterator tmp = *this;
         m_position = m_position->m_prev;
         return tmp;
      }

      pos_t
      idx()
      {
         return _idx();
      }
   };

   typedef iterator const_iterator;

public:

   void
   _release()
   {
      delete this;
   }

   void
   push_front(
      T val)
   {
      StdList::push_front((void*) val);
   }

   void
   push_back(
      T val)
   {
      StdList::push_back((void*) val);
   }

   void
   pop_front()
   {
      StdList::pop_front();
   }

   T
   front()
   {
      return (T) StdList::front();
   }

   T
   back()
   {
      return (T) StdList::back();
   }

   void
   insert(
      iterator & _it,
      T val)
   {
      StdList::iterator & it = (StdList::iterator&) _it;

      StdList::insert(it, (void*) val);
   }

   void

   remove
      (
         T val)
   {
      StdList::erase((void*) val);
   }

   void
   erase(
      iterator & it)
   {
      const StdList::iterator & stupid = (const StdList::iterator&) it;

      StdList::erase_iterator(stupid);
   }

   void
   erase()
   {
      StdList::erase();
   }

   iterator
   begin() const
   {
      return (StdList::node*) m_root.m_next;
   }

   iterator
   end() const
   {
      return (StdList::node*) &m_root;
   }
};


template <class T>
void release(
   T & it1,
   T & it2)
{
   while (it1 != it2)
   {
      DDS_release(*it1);
      ++it1;
   }
}


#endif
