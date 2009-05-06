/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef SACPP_DDS_DCPS_H
#define SACPP_DDS_DCPS_H

#include "os_defs.h"
#include "sacpp_if.h"

#if !defined NULL
#define NULL ((void *)0)
#endif

#if !defined FALSE
#define FALSE (0)
#endif

#if !defined TRUE
#define TRUE (!FALSE)
#endif

class SACPP_API DDS_DCPSUStrSeq;
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
template <class T> class DDS_DCPSValueBase_mgr;
template <class T> class DDS_DCPS_var;
template <class T> class DDS_DCPS_out;
template <class T> class DDS_DCPS_mgr;

namespace DDS_DCPS
{
   // Primitive types

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
   class SACPP_API String_var;
   class SACPP_API String_mgr;
   class SACPP_API String_inout;
   class SACPP_API String_out;
   class SACPP_API String_for_seq;

   enum { enum32 = 0x7ffffff };

   enum CompletionStatus { COMPLETED_YES, COMPLETED_NO, COMPLETED_MAYBE };

   typedef String ObjectId;

   class SACPP_API     Object;
   typedef Object* Object_ptr;
   typedef DDS_DCPSInterface_var<Object> Object_var;
   typedef DDS_DCPSInterface_out<Object> Object_out;
   typedef DDS_DCPSInterface_mgr<Object> Object_mgr;

   class SACPP_API LocalObject;
   typedef LocalObject* LocalObject_ptr;
   typedef DDS_DCPSInterface_var<LocalObject> LocalObject_var;
   typedef DDS_DCPSInterface_out<LocalObject> LocalObject_out;
   typedef DDS_DCPSInterface_mgr<LocalObject> LocalObject_mgr;

   typedef DDS_DCPSUFLSeq<Octet> Octet_seq;
   typedef Octet_seq* Octet_seq_ptr;

   class SACPP_API Exception;
   class SACPP_API UserException;

   // Standard string functions
   static char* string_alloc(ULong len);
   static char* string_dup  (const char* str);
   static void  string_free (char * str);

   static void release(LocalObject_ptr p);
   static void release(Object_ptr p);

   class SACPP_API ValueBase;
   static void add_ref(ValueBase* vb);
   static void remove_ref(ValueBase* vb);
}

/* ************************************************************************** */
/*                           Inline Implementations                           */
/* ************************************************************************** */

inline char*
DDS_DCPS::string_alloc(DDS_DCPS::ULong len)
{
   return new char [len + 1];
}

inline void
DDS_DCPS::string_free(char * str)
{
   delete [] str;
}
 
inline char *
DDS_DCPS::string_dup(const char * s)
{
   char *ret = NULL;

   if (s) {
      ret = DDS_DCPS::string_alloc(strlen(s));
      strcpy(ret, s);
   }

   return ret;
}

#undef SACPP_API

#endif /* SACPP_DDS_DCPS_H */
