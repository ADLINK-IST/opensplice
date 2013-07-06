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
#include "xbe_globals.h"
#include "xbe_generator.h"
#include "os_version.h"

/*
 * Initialize the BE. The protocol requires only that this routine
 * return an instance of AST_Generator (or a subclass thereof).
 *
 * Remember that none of the FE initialization has been done, when you
 * add stuff here.
 */

AST_Generator * BE_init ()
{
   return (new be_generator ());
}

/*
   Print out product and version string. Note we can tell
   if running licensed compiler from the v/V prior to the
   actual version.
*/

void BE_version ()
{
   cout << "OpenSplice C++ IDL Compiler ";
   cout << OSPL_VERSION_STR << " (" __DATE__ ")" << endl;
}
