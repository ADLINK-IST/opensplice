/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#ifndef __UTL_PRAGMAS_HH__
#define __UTL_PRAGMAS_HH__

#include <string.h>
#include "utl_idlist.h"
#include "utl_scoped_name.h"
#include "utl_string.h"

class UTL_Pragmas
{

public:
   enum pragma_types
   {
      PT_prefix,
      PT_ID,
      PT_version,
      PT_async_client,
      PT_async_server
   };

   // Constructors:
   UTL_Pragmas();
   UTL_Pragmas(const UTL_Pragmas &that);

   // Destructor:
   virtual ~UTL_Pragmas();

   UTL_Pragmas &operator=(const UTL_Pragmas &that);

   UTL_String * get_repositoryID();

   UTL_String * get_pragma_prefix();
   void set_pragma_prefix(UTL_String *prefix);

   UTL_String * get_pragma_ID();
   void set_pragma_ID(UTL_String *ID);

   UTL_String * get_pragma_version();
   void set_pragma_version(UTL_String *version);

   bool get_pragma_client_synchronicity();
   void set_pragma_client_synchronicity(bool sync_type);

   bool get_pragma_server_synchronicity();
   void set_pragma_server_synchronicity(bool sync_type);

   void set_name(UTL_ScopedName *name);

   void dump(ostream &os);

private:

   UTL_String *pd_pragma_prefix;
   UTL_String *pd_pragma_ID;
   UTL_String *pd_pragma_version;
   bool pd_pragma_client_synchronicity;
   bool pd_pragma_server_synchronicity;
   UTL_ScopedName *pd_name;
};

#endif
