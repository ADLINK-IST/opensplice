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

#include "idl.h"
#include "idl_extern.h"

#include "fe_private.h"

/*
 * Constructor(s) and destructor
 */

FE_ValueHeader::FE_ValueHeader
(
   idl_bool custom,
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

idl_bool
FE_ValueHeader::custom()
{
   return pd_custom;
}

idl_bool
FE_ValueHeader::truncatable ()
{
   return pd_inherits ? pd_inherits->truncatable () : FALSE;
}
