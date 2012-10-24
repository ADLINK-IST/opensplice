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
#ifndef SACPP_MAPPING_ARRAY_H
#define SACPP_MAPPING_ARRAY_H
#include "sacpp_if.h"

template <typename Type, typename SliceType, typename Unique> class DDS_DCPS_ArrayHelper
{
   public:
      static SliceType* alloc();
      static void copy(SliceType *to, const SliceType* from);
      static void free(SliceType *);
};

template <typename Type, typename SliceType, typename Unique>
class DDS_DCPS_BaseArray_var
{
      typedef DDS_DCPS_ArrayHelper<Type, SliceType, Unique> Helper;

   public:
      DDS_DCPS_BaseArray_var()
         : m_ptr(NULL)
      {
      }

      DDS_DCPS_BaseArray_var(SliceType* _slice)
         : m_ptr(_slice)
      {
      }

      ~DDS_DCPS_BaseArray_var()
      {
         Helper::free(m_ptr);
      }

      operator SliceType * ()
      {
         return m_ptr;
      }

      operator const SliceType * () const
      {
         return (const SliceType*) m_ptr;
      }

      const SliceType * in () const
      {
         return m_ptr;
      }

      SliceType * inout ()
      {
         return m_ptr;
      }

      SliceType* _retn()
      {
         SliceType* ret = m_ptr;
         m_ptr = NULL;
         return ret;
      }

      SliceType*& val()
      {
         return m_ptr;
      }

      SliceType * m_ptr;

   protected:

      inline void _copy(SliceType* s)
      {
         m_ptr = Helper::alloc();
         Helper::copy(m_ptr, s);
      }
};

template <typename Type, typename SliceType, typename Unique>
class DDS_DCPS_VArray_var: public DDS_DCPS_BaseArray_var<Type, SliceType, Unique>
{
      typedef DDS_DCPS_ArrayHelper<Type, SliceType, Unique> Helper;

   public:
      DDS_DCPS_VArray_var()
         : DDS_DCPS_BaseArray_var<Type, SliceType, Unique> (NULL)
      {
      }

      DDS_DCPS_VArray_var(SliceType* slice)
         : DDS_DCPS_BaseArray_var<Type, SliceType, Unique> (slice)
      {
      }

      DDS_DCPS_VArray_var(const DDS_DCPS_VArray_var<Type, SliceType, Unique>& that)
      {
         _copy(that.m_ptr);
      }

      DDS_DCPS_VArray_var<Type, SliceType, Unique>& operator= (SliceType* s)
      {
         Helper::free(this->m_ptr);
         this->m_ptr = s;
         return *this;
      }

      DDS_DCPS_VArray_var<Type, SliceType, Unique>& operator=(const DDS_DCPS_VArray_var<Type, SliceType, Unique>& v)
      {
         Helper::free(this->m_ptr);
         _copy(v.m_ptr);
         return *this;
      }

      const SliceType & operator [] (DDS::ULong index) const
      {
         return this->m_ptr[index];
      }

      SliceType & operator [] (DDS::ULong index)
      {
         return this->m_ptr[index];
      }

      SliceType*& out()
      {
         Helper::free(this->m_ptr);
         this->m_ptr = NULL;
         return this->m_ptr;
      }
};

template <typename Type, typename SliceType, typename Unique>
class DDS_DCPS_MArray_var: public DDS_DCPS_BaseArray_var<Type, SliceType, Unique>
{
      typedef DDS_DCPS_ArrayHelper<Type, SliceType, Unique> Helper;

   public:
      DDS_DCPS_MArray_var()
      {
      }

      DDS_DCPS_MArray_var(SliceType* slice)
         : DDS_DCPS_BaseArray_var<Type, SliceType, Unique> (slice)
      {
      }

      DDS_DCPS_MArray_var(const DDS_DCPS_MArray_var<Type, SliceType, Unique>& that)
      {
         _copy(that.m_ptr);
      }

      DDS_DCPS_MArray_var<Type, SliceType, Unique>& operator= (SliceType* s)
      {
         Helper::free(this->m_ptr);
         this->m_ptr = s;
         return *this;
      }

      DDS_DCPS_MArray_var<Type, SliceType, Unique>& operator=(const DDS_DCPS_MArray_var<Type, SliceType, Unique>& v)
      {
         Helper::free(this->m_ptr);
         _copy(v.m_ptr);
         return *this;
      }

      const SliceType & operator [] (DDS::ULong index) const
      {
         return (const SliceType&)*this->m_ptr[index];
      }

      SliceType & operator [] (DDS::ULong index)
      {
         return (SliceType&)*this->m_ptr[index];
      }

      SliceType*& out()
      {
         Helper::free(this->m_ptr);
         this->m_ptr = NULL;
         return this->m_ptr;
      }
};

template <typename Type, typename SliceType, typename Unique>
class DDS_DCPS_FArray_var: public DDS_DCPS_BaseArray_var<Type, SliceType, Unique>
{
      typedef DDS_DCPS_ArrayHelper<Type, SliceType, Unique> Helper;

   public:
      DDS_DCPS_FArray_var()
      {
      }

      DDS_DCPS_FArray_var(SliceType* slice)
         : DDS_DCPS_BaseArray_var<Type, SliceType, Unique> (slice)
      {
      }

      DDS_DCPS_FArray_var(const DDS_DCPS_FArray_var<Type, SliceType, Unique>& that)
      {
         _copy(that.m_ptr);
      }

      DDS_DCPS_FArray_var<Type, SliceType, Unique>& operator= (SliceType* s)
      {
         Helper::free(this->m_ptr);
         this->m_ptr = s;
         return *this;
      }

      DDS_DCPS_FArray_var<Type, SliceType, Unique>& operator=(const DDS_DCPS_FArray_var<Type, SliceType, Unique>& v)
      {
         Helper::free(this->m_ptr);
         _copy(v.m_ptr);
         return *this;
      }

      const SliceType & operator [] (DDS::ULong index) const
      {
         return this->m_ptr[index];
      }

      SliceType & operator [] (DDS::ULong index)
      {
         return this->m_ptr[index];
      }

      SliceType* out()
      {
         Helper::free (this->m_ptr);
         this->m_ptr = Helper::alloc ();
         return this->m_ptr;
      }
};

template <typename Type, typename SliceType, typename Unique>
class DDS_DCPS_Array_forany
{
      typedef DDS_DCPS_ArrayHelper<Type, SliceType, Unique> Helper;

   public:

      DDS_DCPS_Array_forany()
         : m_ptr(NULL)
      {
      }

      DDS_DCPS_Array_forany(SliceType* _slice, DDS::Boolean nocopy = FALSE)
         : m_ptr(_slice)
      {
         if (nocopy)
         {
            m_ptr = _slice;
         }
         else
         {
            _copy(_slice);
         }
      }

      DDS_DCPS_Array_forany(const DDS_DCPS_Array_forany& that)
         : m_ptr(NULL)
      {
         _copy(that.m_ptr);
      }

      ~DDS_DCPS_Array_forany()
      {
      }

      DDS_DCPS_Array_forany&
      operator=(SliceType* s)
      {
         m_ptr = s;
         return *this;
      }

      DDS_DCPS_Array_forany&
      operator=(const DDS_DCPS_Array_forany& v)
      {
         _copy(v.m_ptr);
         return *this;
      }

      const SliceType&
      operator[](DDS::ULong index) const
      {
         return m_ptr[index];
      }

      SliceType&
      operator[](DDS::ULong index)
      {
         return m_ptr[index];
      }


      operator SliceType*()
      {
         return m_ptr;
      }

      operator const SliceType*() const
      {
         return (const SliceType*)m_ptr;
      }

      SliceType * m_ptr;

   private:

      inline void
      _copy(SliceType* s)
      {
         m_ptr = Helper::alloc();
         Helper::copy(m_ptr, s);
      }
};

template <typename Type, typename SliceType, typename Unique>
class DDS_DCPS_MArray_forany
{
      typedef DDS_DCPS_ArrayHelper<Type, SliceType, Unique> Helper;

   public:

      DDS_DCPS_MArray_forany()
         : m_ptr(NULL)
      {
      }

      DDS_DCPS_MArray_forany(SliceType* _slice, DDS::Boolean nocopy = FALSE)
         : m_ptr(_slice)
      {
         if (nocopy)
         {
            m_ptr = _slice;
         }
         else
         {
            _copy(_slice);
         }
      }

      DDS_DCPS_MArray_forany(const DDS_DCPS_MArray_forany& that)
         : m_ptr(NULL)
      {
         _copy(that.m_ptr);
      }

      ~DDS_DCPS_MArray_forany()
      {
      }

      DDS_DCPS_MArray_forany&
      operator=(SliceType* s)
      {
         m_ptr = s;
         return *this;
      }

      DDS_DCPS_MArray_forany&
      operator=(const DDS_DCPS_MArray_forany& v)
      {
         _copy(v.m_ptr);
         return *this;
      }

      const SliceType&
      operator[](DDS::ULong index) const
      {
         return (const SliceType&)*m_ptr[index];
      }

      SliceType&
      operator[](DDS::ULong index)
      {
         return (SliceType&)*m_ptr[index];
      }


      operator SliceType*()
      {
         return m_ptr;
      }

      operator const SliceType*() const
      {
         return (const SliceType*)m_ptr;
      }

      SliceType * m_ptr;

   private:

      inline void
      _copy(SliceType* s)
      {
         m_ptr = Helper::alloc();
         Helper::copy(m_ptr, s);
      }
};

template <typename Type, typename SliceType, class Var, typename Unique>
class DDS_DCPS_VLArray_out
{
      typedef DDS_DCPS_ArrayHelper<Type, SliceType, Unique> Helper;

   public:
      DDS_DCPS_VLArray_out(SliceType*& p)
         : m_ptr(p)
      {
      }

      DDS_DCPS_VLArray_out(Var& v)
         : m_ptr(v.m_ptr)
      {
         Helper::free(m_ptr);
      }

      DDS_DCPS_VLArray_out(const DDS_DCPS_VLArray_out & v)
         : m_ptr(v.m_ptr)
      {
      }

      DDS_DCPS_VLArray_out& operator=(SliceType* p)
      {
         m_ptr = p;
         return *this;
      }

      operator SliceType*()
      {
         return m_ptr;
      }

      SliceType& operator[](DDS::ULong index)
      {
         return m_ptr[index];
      }

   private:

      SliceType *& m_ptr;
};

#undef SACPP_API
#endif /* SACPP_MAPPING_ARRAY_H */
