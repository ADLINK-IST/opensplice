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
