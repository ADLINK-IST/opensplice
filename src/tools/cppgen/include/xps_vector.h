#ifndef _XPS_VECTOR_H
#define _XPS_VECTOR_H

#include "sacpp_DDS_DCPS.h"

// A subset implementation of STL vector.
//
// Includes only a bare minimum of functionality.  However, you can be
// sure that elements that you store in an DDSVector won't move in memory,
// nor will assignment occur on unconstructed objects.  All you need to
// provide is a copy constructor and/or = function.  To use sort(), > must
// also be defined on the base type.

class DDSVectorBase
{
   friend class test_DDSVectorBase;

public:

   DDSVectorBase (DDS::ULong blockSize);
   DDSVectorBase (const DDSVectorBase & that);

   ~DDSVectorBase ();

   inline void*& operator[](DDS::ULong index) const
   {
      return m_pointerBlock[index];
   }

   void push_back (void* pointer);

   // zero all pointers from index to the end

   inline void chop (DDS::ULong index)
   {
      memset(m_pointerBlock + index, 0, sizeof(void*) * (m_blockSize - index));
      m_size = index;
   }

   inline DDS::ULong size () const
   {
      return m_size;
   }

private:

   DDS::ULong m_blockSize;
   DDS::ULong m_size; // current number of elements
   void ** m_pointerBlock;
};

template <class T, class V> class DDSVectorIterator
{
public:

   inline DDSVectorIterator () : m_vector (NULL), m_index (0)
   {}

   inline DDSVectorIterator (V & vector, DDS::ULong index = 0)
      : m_vector (&vector), m_index (index)
   {}

   inline DDSVectorIterator (const DDSVectorIterator & that)
      : m_vector (that.m_vector), m_index (that.m_index)
   {}

   inline T& operator*() const
   {
      assert (m_vector);
      assert (m_index < m_vector->size());
      return (*m_vector)[m_index];
   }

   inline DDSVectorIterator & operator = (const DDSVectorIterator & that)
   {
      if (this != &that)
      {
         m_vector = that.m_vector;
         m_index = that.m_index;
      }

      return *this;
   }

   inline bool operator == (const DDSVectorIterator & that) const
   {
      return m_vector == that.m_vector && m_index == that.m_index;
   }

   inline bool operator != (const DDSVectorIterator & that) const
   {
      return m_vector != that.m_vector || m_index != that.m_index;
   }

   inline DDSVectorIterator & operator ++ ()
   {
      ++m_index;
      return *this;
   }

   inline DDSVectorIterator operator ++ (int)
   {
      DDSVectorIterator <T, V> tmp = *this;
      ++m_index;
      return tmp;
   }

   inline DDSVectorIterator & operator--()
   {
      --m_index;
      return *this;
   }

   inline DDSVectorIterator operator -- (int)
   {
      DDSVectorIterator <T, V> tmp = *this;
      --m_index;
      return tmp;
   }

   // should be private:

public:

   V* m_vector; // can be NULL, if iterator is not init'ed
   DDS::ULong m_index;
};

template <class T, class V> class DDSVectorConstIterator
{
public:

   inline DDSVectorConstIterator () : m_vector(NULL), m_index(0)
   { }

   inline DDSVectorConstIterator (const V & vector, DDS::ULong index = 0)
         : m_vector(&vector), m_index(index)
   { }

   inline DDSVectorConstIterator (const DDSVectorConstIterator & that)
         : m_vector(that.m_vector), m_index(that.m_index)
   { }

   inline const T& operator*() const
   {
      assert (m_vector);
      assert (m_index < m_vector->size());
      return (*m_vector)[m_index];
   }

   inline DDSVectorConstIterator & operator=(const DDSVectorConstIterator & that)
   {
      m_vector = that.m_vector;
      m_index = that.m_index;
      return *this;
   }

   inline DDSVectorConstIterator & operator = (const DDSVectorIterator<T, V> & that)
   {
      m_vector = that.m_vector;
      m_index = that.m_index;
      return *this;
   }

   inline bool operator==(const DDSVectorConstIterator & that) const
   {
      return m_vector == that.m_vector && m_index == that.m_index;
   }

   inline bool operator!=(const DDSVectorConstIterator & that) const
   {
      return m_vector != that.m_vector || m_index != that.m_index;
   }

   inline bool operator==(const DDSVectorIterator<T, V> & that) const
   {
      return m_vector == that.m_vector && m_index == that.m_index;
   }

   inline bool operator!=(const DDSVectorIterator<T, V> & that) const
   {
      return m_vector != that.m_vector || m_index != that.m_index;
   }

   inline DDSVectorConstIterator & operator++()
   {
      ++m_index;
      return *this;
   }

   inline DDSVectorConstIterator operator++(int)
   {
      DDSVectorConstIterator<T, V> tmp = *this;
      ++m_index;
      return tmp;
   }

   inline DDSVectorConstIterator & operator -- ()
   {
      --m_index;
      return *this;
   }

   inline DDSVectorConstIterator operator -- (int)
   {
      DDSVectorConstIterator<T, V> tmp = *this;
      --m_index;
      return tmp;
   }

private:

   const V* m_vector; // can be NULL, if iterator is not init'ed
   DDS::ULong m_index;
};

template <class T> class DDSVector
{
public:

   typedef DDSVectorIterator <T, DDSVector<T> > iterator;
   typedef DDSVectorConstIterator <T, DDSVector<T> > const_iterator;

   DDSVector(DDS::ULong blockSize = 16) : m_base(blockSize)
   { }

   DDSVector(
      const DDSVector<T>& that)
         :
         m_base(that.m_base)
   {
      for (unsigned int i = 0; i < that.size(); i++)
      {
         m_base.operator[](i) = new T(that[i]);
      }
   }

   virtual ~DDSVector()
   {
      erase();
   }

   inline void push_back(const T& val)
   {
      m_base.push_back(new T(val));
   }

   inline T pop_back()
   {
      T result = *(T*)(m_base[m_base.size() - 1]);
      erase(m_base.size() - 1);
      return result;
   }

   inline T& front()
   {
      return operator[](0);
   }

   inline const T& front() const
   {
      return operator[](0);
   }

   inline T& back()
   {
      return operator[](m_base.size() - 1);
   }

   inline const T& back() const
   {
      return operator[](m_base.size() - 1);
   }

   inline DDS::ULong size() const
   {
      return m_base.size();
   }

   inline T& operator[](DDS::ULong index)
   {
      return *(T*)(m_base.operator[](index));
   }

   inline const T& operator[](DDS::ULong index) const
   {
      return *(T*)(m_base.operator[](index));
   }

   void erase(DDS::ULong index = 0)
   {
      if (index >= m_base.size())
      {
         return ;
      }

      for (DDS::ULong i = index; i < m_base.size(); i++)
      {
         delete (T*)m_base[i];
      }

      m_base.chop(index);
   }

   // FUNCTIONS THAT USE OR CREATE ITERATORS

   iterator begin()
   {
      return iterator(*this, 0);
   }

   const_iterator begin() const
   {
      return const_iterator(*this, 0);
   }

   iterator end()
   {
      return iterator(*this, m_base.size());
   }

   const_iterator end() const
   {
      return const_iterator(*this, m_base.size());
   }

   void sort()
   {
      iterator it = begin();
      iterator end_it = end();
      iterator max_it;
      iterator find_it;

      while (it != end_it)
      {
         max_it = it;
         find_it = it;
         ++find_it;

         while (find_it != end_it)
         {
            if (*find_it < *max_it)
            {
               max_it = find_it;
            }

            ++find_it;
         }

         if (it != max_it)
         {
            // swap
            T tmp = *it;
            *it = *max_it;
            *max_it = tmp;
         }

         ++it;
      }
   }

private:

   DDSVectorBase m_base;
};

#endif // _XPS_VECTOR_H
