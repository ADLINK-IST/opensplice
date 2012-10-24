#ifndef AST_OPAQUE_HH
#define AST_OPAQUE_HH
/*
*  AST_Opaque --  AST node for opaque types
*  
*  The contents of this file are the property of Expersoft Corporation.
*  Any unauthorized access, distribution, or copying of the information
*  contained herein, is forbidden and subject to prosecution under 
*  International copyright laws.  In addition to extensive prosecution,
*  Expersoft Corporation has the option of placing a curse upon any 
*  person using this file illegally.  The curse may, but is not herein
*  expressly limited to, cause your wobbly bits to shrivel up and fall off.
*  You have been warned.   
*  
*  Coder: Derek Lee  
*  For:   Expersoft Corporation
*  Initial Coding:  11 December 1996
*
*/

#include "ast_concrete_type.h"
#include "idl.h"

class
         AST_Opaque : virtual public AST_Type
{

public:
   // Constructors
   AST_Opaque();
   virtual ~AST_Opaque();
   AST_Opaque(UTL_IdList* base_name, const UTL_Pragmas& p);

   virtual void dump(ostream&);
   DEF_NARROW_METHODS1(AST_Opaque, AST_Type);
   DEF_NARROW_FROM_DECL(AST_Opaque);

   void opaqueName(UTL_IdList* namen);
   UTL_IdList* opaqueName() const;

private:
   friend int yyparse();
   UTL_IdList* opaque_name;
};

#endif
