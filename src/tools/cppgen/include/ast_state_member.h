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
#ifndef _AST_STATE_MEMBER_HH
#define _AST_STATE_MEMBER_HH


// Representation of attribute declaration:
//
// An attribute is a field with a readonly property

/*
** DEPENDENCIES: ast_field.hh, ast_type.hh, utl_scoped_name.hh,
**   utl_strlist.hh, ast_decl.hh
**
** USE: Included from ast.h
*/

class AST_StateMember : public virtual AST_Field
{

public:
   // Operations

   // Constructor(s)
   AST_StateMember();
   AST_StateMember
   (
      bool public_access,
      AST_Type *ft,
      UTL_ScopedName *n,
      const UTL_Pragmas &p
   );
   virtual ~AST_StateMember()
   {}

   // Data Accessors
   bool public_access ();

   // Narrowing
   DEF_NARROW_METHODS1(AST_StateMember, AST_Field);
   DEF_NARROW_FROM_DECL(AST_StateMember);

   // AST Dumping
   virtual void dump(ostream &o);

private:
   // Data
   const bool pd_public_access; // Is public or private
};

#endif           // _AST_STATE_MEMBER_HH
