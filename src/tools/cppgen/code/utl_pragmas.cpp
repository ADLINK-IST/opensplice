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
#include "utl_pragmas.h"
#include <memory.h>
#include "os_stdlib.h"

// Constructors:
UTL_Pragmas::UTL_Pragmas ()
      :
      pd_pragma_prefix(NULL),
      pd_pragma_ID(NULL),
      pd_pragma_version(NULL),
      pd_pragma_client_synchronicity(false),
      pd_pragma_server_synchronicity(false),
      pd_name(NULL)
{}

UTL_Pragmas::UTL_Pragmas (const UTL_Pragmas &that)
      :
      pd_pragma_prefix(that.pd_pragma_prefix ? new UTL_String(that.pd_pragma_prefix) : NULL),
      pd_pragma_ID(that.pd_pragma_ID ? new UTL_String(that.pd_pragma_ID) : NULL),
      pd_pragma_version(that.pd_pragma_version ? new UTL_String(that.pd_pragma_version) : NULL),
      pd_pragma_client_synchronicity(that.pd_pragma_client_synchronicity),
      pd_pragma_server_synchronicity(that.pd_pragma_server_synchronicity),
      pd_name(NULL)
{}

UTL_Pragmas::~UTL_Pragmas ()
{
   delete pd_pragma_prefix;
   delete pd_pragma_ID;
   delete pd_pragma_version;
}

// Assignment:
UTL_Pragmas &UTL_Pragmas::operator = (const UTL_Pragmas &that)
{
   delete pd_pragma_prefix;
   delete pd_pragma_ID;
   delete pd_pragma_version;
   pd_pragma_prefix = that.pd_pragma_prefix ? new UTL_String(that.pd_pragma_prefix) : NULL;
   pd_pragma_ID = that.pd_pragma_ID ? new UTL_String(that.pd_pragma_ID) : NULL;
   pd_pragma_version = that.pd_pragma_version ? new UTL_String(that.pd_pragma_version) : NULL;
   pd_pragma_client_synchronicity = that.pd_pragma_client_synchronicity;
   pd_pragma_server_synchronicity = that.pd_pragma_server_synchronicity;
   return *this;
}

// Get the RepositoryID:
UTL_String * UTL_Pragmas::get_repositoryID ()
{
   if (!pd_name)
   {
      return NULL;
   }

   char * repID = new char [4096];
   UTL_String * ret;

   if (pd_pragma_ID)
   {
      // got an id already, don't need to build one.
      os_strcpy(repID, pd_pragma_ID->get_string());
   }
   else
   {
      // build id from version and prefix info.

      // standard id's have IDL:
      os_strcpy(repID, "IDL:");

      // add a prefix if one exists

      if (pd_pragma_prefix)
      {
         os_strcat(repID, pd_pragma_prefix->get_string());
         os_strcat(repID, "/");
      }

      // add the scope with '/' separators
      UTL_ScopedNameActiveIterator *i = new UTL_ScopedNameActiveIterator(pd_name);

      long firsttime = 1;

      for (;!i->is_done();i->next())  // skip first one cuz its blank
      {

         if (firsttime)
         {
            firsttime = 0;
         }
         else
         {
            os_strcat(repID, "/");
         }

         os_strcat(repID, i->item()->get_string());
      }

      delete i;

      // add ':' separator before version
      os_strcat(repID, ":");

      // add version, default is '1.0'

      if (pd_pragma_version)
      {
         os_strcat(repID, pd_pragma_version->get_string());
      }
      else
      {
         os_strcat(repID, "1.0");
      }
   }

   ret = new UTL_String(repID);
   delete [] repID;
   return ret;
}


// Accessors:
UTL_String *UTL_Pragmas::get_pragma_prefix ()
{
   return pd_pragma_prefix;
}

void UTL_Pragmas::set_pragma_prefix (UTL_String *prefix)
{
   delete pd_pragma_prefix;
   pd_pragma_prefix = prefix ? prefix : NULL;
}

UTL_String *UTL_Pragmas::get_pragma_ID ()
{
   return pd_pragma_ID;
}

void UTL_Pragmas::set_pragma_ID (UTL_String *ID)
{
   delete pd_pragma_ID;
   pd_pragma_ID = ID ? ID : NULL;
}

UTL_String *UTL_Pragmas::get_pragma_version ()
{
   return pd_pragma_version;
}

void UTL_Pragmas::set_pragma_version (UTL_String *version)
{
   delete pd_pragma_version;
   pd_pragma_version = version ? version : NULL;
}

bool
UTL_Pragmas::get_pragma_client_synchronicity ()
{
   return pd_pragma_client_synchronicity;
}

void
UTL_Pragmas::set_pragma_client_synchronicity (bool sync_type)
{
   pd_pragma_client_synchronicity = sync_type;
}

bool
UTL_Pragmas::get_pragma_server_synchronicity ()
{
   return pd_pragma_server_synchronicity;
}

void
UTL_Pragmas::set_pragma_server_synchronicity (bool sync_type)
{
   pd_pragma_server_synchronicity = sync_type;
}

void UTL_Pragmas::set_name (UTL_ScopedName *name)
{
   pd_name = name;
}

// Dump:
void UTL_Pragmas::dump (ostream &os)
{
   if (pd_name)
   {
      pd_name->dump (os);
   }

   if (pd_pragma_prefix)
   {
      os << " prefix: ";
      pd_pragma_prefix->dump (os);
   }

   if (pd_pragma_ID)
   {
      os << "id: ";
      pd_pragma_ID->dump (os);
   }

   if (pd_pragma_version)
   {
      os << " version : ";
      pd_pragma_version->dump (os);
   }

   os << " asynch client : " << pd_pragma_client_synchronicity;
   os << " asynch server : " << pd_pragma_server_synchronicity;
}
