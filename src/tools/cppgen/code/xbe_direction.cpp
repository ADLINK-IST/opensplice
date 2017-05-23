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
#include "xbe_direction.h"

be_ArgumentDirection::be_ArgumentDirection (VarType varType)
   : m_varType (varType)
{
   assert(m_varType == VT_InParam
               || m_varType == VT_OutParam
               || m_varType == VT_InOutParam
               || m_varType == VT_Return);
}

be_ArgumentDirection::be_ArgumentDirection (AST_Argument::Direction d)
{
   switch (d)
   {
      case AST_Argument::dir_IN:
      m_varType = VT_InParam;
      break;

      case AST_Argument::dir_OUT:
      m_varType = VT_OutParam;
      break;

      case AST_Argument::dir_INOUT:
      m_varType = VT_InOutParam;
      break;

      default:
      assert(0);
      break;
   }
}

DDS_StdString be_ArgumentDirection::IopParamMmode (DDS::Boolean server) const
{
   DDS_StdString result;

   switch (m_varType)
   {
      case VT_InParam:
      result = "DDS::PARAM_IN";
      break;

      case VT_OutParam:
      case VT_Return:
      result = "DDS::PARAM_OUT";
      break;

      case VT_InOutParam:
      result = "DDS::PARAM_INOUT";
      break;

      default:
      assert (0);
   }

   return result;
}
