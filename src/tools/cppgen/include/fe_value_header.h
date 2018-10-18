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
#ifndef _FE_VALUE_HEADER_FE_VALUE_HH
#define _FE_VALUE_HEADER_FE_VALUE_HH


// FE_ValueHeader
//
// Internal class for FE to describe value headers

/*
** DEPENDENCIES: utl_scoped_name.hh, ast_value.hh, utl_scope.hh,
**   ast_decl.hh
**
** USE: Included from fe.hh
*/

class FE_ValueHeader
{

public:
   // Operations

   // Constructor(s)
   FE_ValueHeader
   (
      bool custom,
      UTL_ScopedName *n,
      FE_ValueInheritanceSpec *inherits
   );
   virtual ~FE_ValueHeader()
   {}

   // Data Accessors
   UTL_ScopedName *value_name();
   AST_Value **inherits();
   long n_inherits();
   AST_Interface **supports();
   long n_supports();
   bool custom();
   bool truncatable();

private:
   // Data
   UTL_ScopedName *pd_value_name;
   FE_ValueInheritanceSpec *pd_inherits;
   bool pd_custom;
};

#endif           // _FE_VALUE_HEADER_FE_VALUE_HH
