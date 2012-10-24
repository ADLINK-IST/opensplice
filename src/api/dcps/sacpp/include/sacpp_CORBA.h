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
#ifndef SACPP_CORBA_H
#define SACPP_CORBA_H

#include "sacpp_DDS_DCPS.h"
#include "sacpp_if.h"

namespace CORBA
{
    typedef DDS::Double Double;
    typedef DDS::Float Float;
    typedef DDS::Short Short;
    typedef DDS::LongLong LongLong;
    typedef DDS::Boolean Boolean;
    typedef DDS::Octet   Octet;
    typedef DDS::Long    Long;
    typedef DDS::ULong   ULong;
    typedef DDS::Object          Object;
    typedef DDS::Object_ptr      Object_ptr;
    typedef DDS::ValueBase       ValueBase;
    typedef DDS::LocalObject     LocalObject;
    typedef DDS::LocalObject_ptr LocalObject_ptr;
    typedef DDS::String_var      String_var;
    typedef DDS::Exception       Exception;
    typedef DDS::UserException   UserException;
    typedef DDS::SystemException SystemException;

    static void string_free(char* str);
    static char* string_dup(const char* s);
    static void release(Object_ptr p);
    static void release(LocalObject_ptr p);
    static void add_ref(ValueBase* vb);
    static void remove_ref(ValueBase* vb);
    static Boolean is_nil(Object_ptr p);
}

/* ************************************************************************** */
/*                           Inline Implementations                           */
/* ************************************************************************** */
inline void
CORBA::string_free(char * str)
{
   DDS::string_free(str);
}

inline char *
CORBA::string_dup(const char * s)
{
   return DDS::string_dup(s);
}

inline void CORBA::release(CORBA::Object * p)
{
   DDS::release(p);
}

inline void CORBA::release(CORBA::LocalObject * p)
{
   DDS::release(p);
}

inline CORBA::Boolean CORBA::is_nil(CORBA::Object * p)
{
   return (p == 0);
}


inline void
CORBA::add_ref(ValueBase* vb)
{
   DDS::add_ref(vb);
}

inline void
CORBA::remove_ref(ValueBase* vb)
{
   DDS::remove_ref(vb);
}

#undef SACPP_API

#endif /* SACPP_CORBA_H */
