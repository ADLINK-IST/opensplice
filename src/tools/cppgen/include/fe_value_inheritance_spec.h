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
#ifndef _FE_VALUE_INHERITANCE_SPEC_HH
#define _FE_VALUE_INHERITANCE_SPEC_HH


// FE_ValueInheritance_Spec
//
// Internal class for FE to describe value inheritance_specs

/*
** DEPENDENCIES: utl_scoped_name.hh, ast_value.hh, utl_scope.hh,
**   ast_decl.hh
**
** USE: Included from fe.hh
*/

class FE_ValueInheritanceSpec
{

public:
   // Operations

   // Constructor(s)
   FE_ValueInheritanceSpec
   (
      bool truncatable,
      UTL_NameList *ihl,
      UTL_NameList *supl
   );

   virtual ~FE_ValueInheritanceSpec()
   {   
      if (pd_supports)
      {
         delete pd_supports;
      }
      if (pd_inherits)
      {
         delete pd_inherits;
      }
   }

   // Data Accessors
   AST_Value **inherits();
   long n_inherits();
   AST_Interface **supports();
   long n_supports();
   bool truncatable();

private:
   // Data
   AST_Value **pd_inherits;  // Inherited values
   long pd_n_inherits;  // How many
   AST_Interface **pd_supports;  // Supported Interfaces
   long pd_n_supports;  // How many
   bool pd_truncatable;

   // Operations

   // Compile the flattened unique list of values which this
   // value inherits from
   void compile_value_inheritance(UTL_NameList *l);
   void compile_inheritance(UTL_NameList *l);
};

#endif           // _FE_VALUE_INHERITANCE_SPEC_HH
