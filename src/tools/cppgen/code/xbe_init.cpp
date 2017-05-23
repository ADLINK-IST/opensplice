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
