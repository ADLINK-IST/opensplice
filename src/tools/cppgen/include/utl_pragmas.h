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
#ifndef __UTL_PRAGMAS_HH__
#define __UTL_PRAGMAS_HH__

#include <string.h>
#include "idl_bool.h"
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

   idl_bool get_pragma_client_synchronicity();
   void set_pragma_client_synchronicity(idl_bool sync_type);

   idl_bool get_pragma_server_synchronicity();
   void set_pragma_server_synchronicity(idl_bool sync_type);

   void set_name(UTL_ScopedName *name);

   void dump(ostream &os);

private:

   UTL_String *pd_pragma_prefix;
   UTL_String *pd_pragma_ID;
   UTL_String *pd_pragma_version;
   idl_bool pd_pragma_client_synchronicity;
   idl_bool pd_pragma_server_synchronicity;
   UTL_ScopedName *pd_name;
};

#endif
