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
#ifndef SACPP_DDS_BASIC_TYPES_H
#define SACPP_DDS_BASIC_TYPES_H

#include "os_defs.h"
#include "os_stdlib.h"
#include "cpp_dcps_if.h"

/**
 * @file
 * @bug OSPL-2272 We've no business/should have no need to define NULL, TRUE, & FALSE. Wrongly. */
#if !defined NULL
#define NULL ((void *)0)
#endif

#if !defined FALSE
#define FALSE (0)
#endif

#if !defined TRUE
#define TRUE (!FALSE)
#endif

class OS_API DDS_DCPSUStrSeq;
template <class T = DDS_DCPSUStrSeq> class DDS_DCPSUStrSeq_var;
template <class T = DDS_DCPSUStrSeq> class DDS_DCPSUStrSeq_out;
template <class T, typename X = T> class DDS_DCPSUVLSeq;
template <class T, typename X = T> class DDS_DCPSUFLSeq;
template <class T, typename X = T> class DDS_DCPSUObjSeq;
template <typename X> class DDS_DCPSUStrSeqT;
template <class T> class DDS_DCPSSequence_var;
template <class T> class DDS_DCPSSequence_out;
template <class T> class DDS_DCPSInterface_var;
template <class T> class DDS_DCPSInterface_out;
template <class T> class DDS_DCPSInterface_mgr;
template <class T> class DDSValueBase_mgr;
template <class T> class DDS_DCPS_var;
template <class T> class DDS_DCPS_out;
template <class T> class DDS_DCPS_mgr;

namespace DDS
{
   // Primitive types
   /**
    * @bug OSPL-918 This should be bool */
   typedef os_uchar Boolean;
   typedef Boolean& Boolean_out;
   typedef Boolean* Boolean_ptr;

   typedef os_char Char;
   typedef Char& Char_out;
   typedef Char* Char_ptr;

   typedef os_uchar Octet;
   typedef Octet& Octet_out;
   typedef Octet* Octet_ptr;

   typedef os_short Short;
   typedef Short& Short_out;
   typedef Short* Short_ptr;

   typedef os_ushort UShort;
   typedef UShort& UShort_out;
   typedef UShort* UShort_ptr;

   typedef os_int32 Long;
   typedef Long& Long_out;
   typedef Long* Long_ptr;

   typedef os_uint32 ULong;
   typedef ULong& ULong_out;
   typedef ULong* ULong_ptr;

   typedef os_float Float;
   typedef Float& Float_out;
   typedef Float* Float_ptr;

   typedef os_double Double;
   typedef Double& Double_out;
   typedef Double* Double_ptr;

   typedef void* Opaque;

   typedef os_int64 LongLong;
   typedef LongLong& LongLong_out;
   typedef LongLong* LongLong_ptr;

   typedef os_uint64 ULongLong;
   typedef ULongLong& ULongLong_out;
   typedef ULongLong* ULongLong_ptr;

   typedef Char* String;
   class OS_API String_var;
   class OS_API String_mgr;
   class OS_API String_inout;
   class OS_API String_out;
   class OS_API String_for_seq;

   enum { enum32 = 0x7ffffff };

   enum CompletionStatus { COMPLETED_YES, COMPLETED_NO, COMPLETED_MAYBE };

   typedef String ObjectId;

   enum TCKind
   {
      tk_null, tk_void, tk_short, tk_long,
      tk_ushort, tk_ulong, tk_float, tk_double,
      tk_boolean, tk_char, tk_octet, tk_any,
      tk_TypeCode, tk_Principal, tk_objref, tk_struct,
      tk_union, tk_enum, tk_string, tk_sequence,
      tk_array, tk_alias, tk_except, tk_longlong,
      tk_ulonglong, tk_longdouble, tk_wchar, tk_wstring,
      tk_fixed, tk_local_interface, _TCKind = enum32
   };

   class OS_API Object;
   typedef Object* Object_ptr;
   typedef DDS_DCPSInterface_var<Object> Object_var;
   typedef DDS_DCPSInterface_out<Object> Object_out;
   typedef DDS_DCPSInterface_mgr<Object> Object_mgr;
   OS_API void release(Object_ptr p);
   OS_API Boolean is_nil(Object_ptr p);

   class OS_API LocalObject;
   typedef LocalObject* LocalObject_ptr;
   typedef DDS_DCPSInterface_var<LocalObject> LocalObject_var;
   typedef DDS_DCPSInterface_out<LocalObject> LocalObject_out;
   typedef DDS_DCPSInterface_mgr<LocalObject> LocalObject_mgr;
   OS_API void release(LocalObject_ptr p);

   class OS_API Exception;
   typedef Exception* Exception_ptr;

   class OS_API UserException;
   typedef UserException* UserException_ptr;

   class OS_API SystemException;
   typedef SystemException* SystemException_ptr;

   class OS_API ExceptionInitializer;

   // Standard string functions
   OS_API char * string_alloc (ULong len);
   OS_API char * string_dup   (const char* str);
   OS_API void string_free  (char * str);
   OS_API Long string_cmp (const char * str1, const char * str2);
   OS_API Boolean is_nil (Object_ptr ptr);

   class OS_API ValueBase;
   OS_API void add_ref(ValueBase* vb);
   OS_API void remove_ref(ValueBase* vb);

   class OS_API Counter;
}

/* ************************************************************************** */
/*                           Inline Implementations                           */
/* ************************************************************************** */

OS_API inline char * DDS::string_alloc (DDS::ULong len)
{
   char * ret = new char [len + 1];
   ret[0] = '\0';
   return ret;
}

OS_API inline void DDS::string_free (char * str)
{
   delete [] str;
}

OS_API inline char * DDS::string_dup (const char * s)
{
   char * ret = 0;

   if (s)
   {
      ret = DDS::string_alloc ((DDS::ULong)strlen (s));
      os_strcpy (ret, s);
   }

   return ret;
}

OS_API inline DDS::Long DDS::string_cmp (const char* str1, const char* str2)
{
   DDS::Long ret = 0;

   if ((str1 != 0) || (str2 != 0))
   {
      if ((str1 == 0) && (str2 != 0))
      {
         ret = 1;
      }
      else
      {
         if ((str1 != 0) && (str2 == 0))
         {
            ret = -1;
         }
         else
         {
            ret = strcmp (str1, str2);
         }
      }
   }

   return ret;
}

OS_API inline DDS::Boolean DDS::is_nil(DDS::Object_ptr ptr)
{
    return (ptr == NULL);
}


namespace CORBA {
    /*
     * By using 'using' instead of actual functions and typedefs,
     * we prevent these compiler errors:
     *      - call of overloaded ‘function’ is ambiguous
     *      - reference to ‘type’ is ambiguous
     */

    /* Types */
    using DDS::Boolean;
    using DDS::Char;
    using DDS::Octet;
    using DDS::Short;
    using DDS::UShort;
    using DDS::Long;
    using DDS::ULong;
    using DDS::Float;
    using DDS::Double;
    using DDS::LongLong;
    using DDS::ULongLong;

    using DDS::String_var;

    using DDS::Object;
    using DDS::Object_ptr;
    using DDS::LocalObject;
    using DDS::LocalObject_ptr;
    using DDS::Exception;
    using DDS::UserException;
    using DDS::SystemException;
    using DDS::ValueBase;

    /* Functions */
    using DDS::release;
    using DDS::string_alloc;
    using DDS::string_free;
    using DDS::string_dup;
    using DDS::is_nil;
}


#undef OS_API

#endif  /* SACPP_DDS_BASIC_TYPES_H */
