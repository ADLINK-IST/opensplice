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
#ifndef _XBE_DIRECTION_H
#define _XBE_DIRECTION_H

#include "idl.h"
#include "ast.h"
#include "ast_argument.h"
#include "xbe_type.h"

// a be_ArgumentDirection is which way an argument to an operation is
// going: in, out, inout, or return

class be_ArgumentDirection
{

public:
   // refactoring: ultimately, ArgumentDirection should replace VarType
   be_ArgumentDirection(VarType varType);
   be_ArgumentDirection(AST_Argument::Direction d);

   // CONVERSIONS

   inline operator VarType() const
   {
      return m_varType;
   }

   // STRING GENERATORS

   // returns the string that should populate an IOP::Param's m_mode member
   // to represent this be_ArgumentDirection

   DDS_StdString IopParamMmode (DDS::Boolean server) const;

   // HELPERS

   inline bool IsSentFromClient () const
   {
      return m_varType == VT_InParam || m_varType == VT_InOutParam;
   }

   inline bool IsSentFromServer () const
   {
      return m_varType == VT_OutParam
             || m_varType == VT_InOutParam
             || m_varType == VT_Return;
   }

private:
   VarType m_varType; // may only be VT_InParam, VT_OutParam, VT_InOutParam,
   // or VT_Return
};

#endif // _XBE_DIRECTION_H
