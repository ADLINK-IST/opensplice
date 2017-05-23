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

#include "idl.h"
#include "idl_extern.h"

#include "fe_private.h"

/*
 * Constructor(s) and destructor
 */

FE_ValueHeader::FE_ValueHeader
(
   bool custom,
   UTL_ScopedName *n, 
   FE_ValueInheritanceSpec *inherits
)
   : pd_value_name (n),
     pd_inherits (inherits),
     pd_custom (custom)
{
}

/*
 * Data Accessors
 */

UTL_ScopedName *
FE_ValueHeader::value_name()
{
   return pd_value_name;
}

AST_Value **
FE_ValueHeader::inherits()
{
   return pd_inherits ? pd_inherits->inherits () : NULL;
}

long
FE_ValueHeader::n_inherits()
{
   return pd_inherits ? pd_inherits->n_inherits () : 0;
}

AST_Interface **
FE_ValueHeader::supports()
{
   return pd_inherits ? pd_inherits->supports () : NULL;
}

long
FE_ValueHeader::n_supports()
{
   return pd_inherits ? pd_inherits->n_supports () : 0;
}

bool
FE_ValueHeader::custom()
{
   return pd_custom;
}

bool
FE_ValueHeader::truncatable ()
{
   return pd_inherits ? pd_inherits->truncatable () : FALSE;
}
