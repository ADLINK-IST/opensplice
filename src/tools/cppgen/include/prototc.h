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
#ifndef _PROTOTC_H_
#define _PROTOTC_H_

#include "StdString.h"
#include "sacpp_DDS_DCPS.h"
#include "tlist.h"

struct ProtoTypeCode
{
   typedef DDS_StdString String;
   typedef TList<ProtoTypeCode*> memlist;
   typedef TList<const char*> namelist;


   DDS::TCKind kind;
   String id;
   String name_of_type;
   memlist members;
   namelist member_names;
   int has_default;
   ProtoTypeCode * disc_type;
   TList<void*> disclist;
   unsigned short fixed_size;
   unsigned short fixed_scale;
   unsigned long bounds;
   unsigned long length;

   ProtoTypeCode() : has_default(0), disc_type(0), fixed_size(0),
         fixed_scale(0), bounds(0), length(0)
   {}

}

;

typedef ProtoTypeCode * ProtoTypeCode_ptr;
typedef const ProtoTypeCode* const_ProtoTypeCode_ptr;
typedef ProtoTypeCode * const ProtoTypeCode_ptr_const;
typedef const ProtoTypeCode* const const_ProtoTypeCode_ptr_const;

#endif
