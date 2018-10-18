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
