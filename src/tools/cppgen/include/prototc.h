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
#ifndef _PROTOTC_H_
#define _PROTOTC_H_

#include "StdString.h"
#include "sacpp_dds_basic_types.h"
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
