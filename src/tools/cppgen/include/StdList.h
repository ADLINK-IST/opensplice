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
#ifndef _DDS_LIST_H_
#define _DDS_LIST_H_

#undef _CRTIMP

class StdList
{
public:

   typedef unsigned long pos_t;

   class node
   {

   public:

      node ();
      node (void * val);

      static void allocate_free_list (size_t size);
      void * operator new (size_t size);
      node * address () const;
      void append (node * n);
      inline ~node ()
      {
         m_prev->m_next = m_next;
         m_next->m_prev = m_prev;
      }

      // static const chunk allocation size
      static const size_t m_chunkSize;

      // static singly linked list of free nodes
      static node * m_freeList;

      // pointer to the value of this node
      void * m_val;

      // pointer to the next node in list
      node * m_next;

      // pointer to the previous node in list
      node * m_prev;

      #ifndef WINCE
      void operator delete (void * p, size_t size)
      #else
      void operator delete (void * p)
      #endif
      {
         ((node*) p)->m_next = m_freeList;
         m_freeList = (node*) p;
      }
   };

   class iterator
   {

   public:

      friend class StdList;

      iterator();

      inline iterator (node * position)
            :
            m_position (position)
      {}

      unsigned char operator== (const iterator & it) const;

      iterator & operator++ ();
      iterator operator++ (int);
      iterator & operator-- ();
      iterator operator-- (int);

      // iterator & operator= (const iterator & it);

      void * current ();
      pos_t _idx ();
      unsigned char operator!= (const iterator & it) const;
      void * operator* ();

   protected:

      node * m_position;
   };

public:


   StdList ();

   void * operator[] (pos_t idx) const;

   void push_front (void * val);

   void push_back (void * val);

   void insert (iterator & it,void * val);

   void pop_front ();

   void * front ();

   void * back ();

   void erase (void * val)
   {
      for (register node * n = m_root.m_next; n != &m_root; n = n->m_next)
      {
         if (n->m_val == val)
         {
            erase (n);
            break;
         }
      }
   }

   void erase_iterator (const iterator & it);

   void erase ();

   bool empty ();
   size_t size () const;

   iterator begin () const
   {
      return m_root.m_next;
   }

   iterator end () const
   {
      return (node*) &m_root;
   }

   ~StdList();

protected:

   void erase (node * n)
   {
      delete n;  // delete instead of DDS::release because node
      // does it's own pools
      m_size--;
   }

   node m_root;
   size_t m_size;

private:

   friend class iterator;

   StdList (const StdList &);
   StdList & operator= (const StdList&);
};



// -----------------------------------------
// StdList::node implementation
// -----------------------------------------
inline StdList::node::node ()
      : m_val (0)
{
   m_next = this;
   m_prev = this;
}


inline  StdList::node::node (void * val)
      :
      m_val (val)
{
   m_next = this;
   m_prev = this;
}


inline void  StdList::node::allocate_free_list (size_t size)
{
   register node * p;
   size_t chunk = m_chunkSize * size;

   m_freeList = p = (node*) new char[chunk];

   for (; p != &m_freeList[m_chunkSize - 1]; p->m_next = p + 1, p++)

      ;

   p->m_next = 0;
}


inline void*  StdList::node::operator new (size_t size)
{
   node * p;

   if (!m_freeList)
   {
      // THIS IS A SEPARATE FUNCTION TO KEEP THIS FUNCTION INLINE
      allocate_free_list (size);
   }

   p = m_freeList;
   m_freeList = m_freeList->m_next;
   return p;
}

inline StdList::node* StdList::node::address () const
{
   return (StdList::node*)this;
}


inline void StdList::node::append (node * n)
{
   n->m_prev = this;
   n->m_next = this->m_next;
   m_next->m_prev = n;
   m_next = n;
}



// -------------------------------------------------
// StdList::iterator implementation
// -------------------------------------------------
inline StdList::iterator::iterator ()
      :
      m_position (0)
{}


inline StdList::iterator& StdList::iterator::operator++ ()
{
   m_position = m_position->m_next;
   return *this;
}


inline StdList::iterator StdList::iterator::operator++ (int)
{
   StdList::iterator tmp = *this;
   m_position = m_position->m_next;
   return tmp;
}


inline StdList::iterator& StdList::iterator::operator-- ()
{
   m_position = m_position->m_prev;
   return *this;
}


inline StdList::iterator StdList::iterator::operator-- (int)
{
   StdList::iterator tmp = *this;
   m_position = m_position->m_prev;
   return tmp;
}

inline unsigned char StdList::iterator::operator== (
   const iterator & it) const
{
   return (m_position == it.m_position) ? 1 : 0;
}


// inline StdList::iterator& StdList::iterator::operator=
// (
//    const iterator & _it
// )
// {
//  iterator & it = (iterator&) _it;
 
//  m_position = it.m_position;
//  m_idx  = it.m_idx;
 
//  return *this;
// }


inline void* StdList::iterator::current ()
{
   return m_position->m_val;
}


inline StdList::pos_t StdList::iterator::_idx ()
{
   return 0;
}


inline unsigned char StdList::iterator::operator!=
(
   const iterator & it
) const
{
   return (unsigned char) (it.m_position != m_position);
}


inline void* StdList::iterator::operator* ()
{
   return m_position->m_val;
}



// -------------------------------------------------
// StdList implementation
// -------------------------------------------------
inline StdList::StdList ()
      :
      m_size (0)
{}


inline void StdList::push_front (
   void * val)
{
   m_root.append (new node (val));
   m_size++;
}


inline void StdList::push_back (void * val)
{
   m_root.m_prev->append (new node (val));
   m_size++;
}


inline void StdList::insert (iterator & it, void * val)
{
   pos_t idx = it._idx ();
   node * n;

   // check for special conditions
   if (idx == 0)
   {
      StdList::push_front (val);
   }
   else if (idx >= m_size) // idx is zero-based, hence >=
   {
      StdList::push_back (val);
   }
   else   // no special conditions
   {
      node * inserted = m_root.m_next;
      register unsigned i = 0;

      while ((i < (unsigned) (idx - 1)) && (inserted != &m_root))
      {
         i++;
         inserted = inserted->m_next;
      }

      n = new node (val);
      inserted->append (n);

      m_size++;
   }
}


inline void StdList::pop_front ()
{
   if (m_size)
   {
      erase (m_root.m_next);
   }
}


inline void* StdList::front ()
{
   return (m_root.m_next ? m_root.m_next->m_val : 0);
}

inline void* StdList::back ()
{
   return (m_root.m_prev ? m_root.m_prev->m_val : 0);
}


inline void StdList::erase_iterator (const iterator & _it)
{
   iterator & it = (iterator&) _it;

   if (it.m_position != &m_root)
   {
      node * target = it.m_position;

      ++it;
      erase (target);
   }
}


inline void StdList::erase ()
{
   node * n, * next;

   n = m_root.m_next;

   while (n != &m_root)
   {
      next = n->m_next;

      delete n;
      n = next;
   }

   m_size = 0;
}


inline void* StdList::operator[] (pos_t idx) const
{
   if (idx < m_size)
   {
      node * n = m_root.m_next;

      for (register pos_t i = 0; i < idx; i++)
      {
         n = n->m_next;
      }

      return n->m_val;
   }
   else
   {
      return 0;
   }
}


inline size_t StdList::size () const
{
   return m_size;
}


inline bool StdList::empty ()
{
   return (size() == 0 ? true : false);
}


inline StdList::~StdList()
{
   erase ();
}

#endif
