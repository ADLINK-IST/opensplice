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
#ifndef SACPP_CORBA_H
#define SACPP_CORBA_H

#include "sacpp_DDS_DCPS.h"

#include "sacpp_if.h"

#if !defined nil
#define nil (0)
#endif

namespace CORBA
{
    typedef DDS::Boolean Boolean;
    typedef DDS::Octet   Octet;
    typedef DDS::Long    Long;
    typedef DDS::ULong   ULong;
    typedef DDS::Object     Object;
    typedef DDS::Object_ptr Object_ptr;
    
    static void string_free(char* str);
    static char* string_dup(const char* s);
    static void release(Object_ptr p);

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
   DDS::release (p);
}

#undef SACPP_API

#endif /* SACPP_CORBA_H */
