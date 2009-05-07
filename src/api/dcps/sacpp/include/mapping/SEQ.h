#ifndef SACPP_MAPPING_SEQ_H
#define SACPP_MAPPING_SEQ_H

template <class T> class DDS_DCPSSequence_var
{
public:

   DDS_DCPSSequence_var () : m_ptr (nil) {}

   DDS_DCPSSequence_var (T * p) : m_ptr (p) {}

   DDS_DCPSSequence_var (const DDS_DCPSSequence_var<T> & that)
   {
      m_ptr = (that.m_ptr) ? new T (*(that.m_ptr)) : nil;
   }

   ~DDS_DCPSSequence_var () { delete m_ptr; }

   DDS_DCPSSequence_var<T> & operator = (T * p)
   {
      delete m_ptr;
      m_ptr = p;
      return *this;
   }

   DDS_DCPSSequence_var<T> & operator = 
   (
      const DDS_DCPSSequence_var<T> & that
   )
   {
      if (this != &that)
      {
         delete m_ptr;
         m_ptr = (that.m_ptr) ? new T (*(that.m_ptr)) : nil;
      }
      return *this;
   }

   typename T::_subscript_type operator [] (DDS::ULong index)
   {
      assert (m_ptr);
      return m_ptr->operator[] (index);
   }

   typename T::_const_subscript_type operator [] (DDS::ULong index) const
   {
      assert (m_ptr);
      return ((const T *)m_ptr)->operator[] (index);
   }

   typename T::_subscript_type operator [] (int index)
   {
      assert (m_ptr);
      assert (index >= 0);
      return m_ptr->operator[] (index);
   }

   T * operator -> () { return m_ptr; }

   operator T & () { return *m_ptr; }

   operator const T & () const { return *m_ptr; }

   operator T * () { return m_ptr; }

   operator const T * () const { return m_ptr; }

   const T & in () const { return *m_ptr; }

   T & inout () { return *m_ptr; }

   T *& val () { return m_ptr; }

   T *& out ()
   {
      delete m_ptr;
      m_ptr = nil;
      return m_ptr;
   }

   T * _retn ()
   {
      T * ret = m_ptr;
      m_ptr = nil;
      return ret;
   }

public:

   T * m_ptr;
};

template <class T> class DDS_DCPSSequence_out
{
public:

   DDS_DCPSSequence_out (T *& p) : m_ptr (p) {}

   DDS_DCPSSequence_out (DDS_DCPSSequence_var<T> & v)
      : m_ptr (v.m_ptr)
   {
      delete m_ptr;
   }

   DDS_DCPSSequence_out (const DDS_DCPSSequence_out<T> & v)
      : m_ptr (v.m_ptr)
   {}

   DDS_DCPSSequence_out<T>& operator=(T * p)
   {
      m_ptr = p;
      return *this;
   }

   typename T::_subscript_type operator [] (DDS::ULong index)
   {
      assert (m_ptr);
      return m_ptr->operator[] (index);
   }

   typename T::_subscript_type operator [] (int index)
   {
      assert (m_ptr);
      assert (index >= 0);
      return m_ptr->operator[] (index);
   }

   T * operator -> () { return m_ptr; }

   T *& out () { return m_ptr; }

   operator T * () const { return m_ptr; }

public:

   T *& m_ptr;
};

#endif /* SACPP_MAPPING_SEQ_H */
