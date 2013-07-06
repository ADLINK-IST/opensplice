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
