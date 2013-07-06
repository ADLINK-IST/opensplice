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
#include "idl.h"
#include "xbe_dispatchable.h"
#include "xbe_literals.h"
#include "xbe_utils.h"

IMPL_NARROW_METHODS0 (be_DispatchableType)

void be_DispatchableType::DeclareForDispatcher
(
   ostream& os,
   be_Tab& tab,
   const be_CppName& implClassName,
   const be_CppName& argName,
   const be_ArgumentDirection& direction
) const
{
   switch (direction)
   {
      case VT_InParam:
      {
         DeclareIn (os, tab, implClassName, argName);
      }
      break;

      case VT_OutParam:
      case VT_Return:
      {
         DeclareOutOrReturn (os, tab, argName, direction);
      }
      break;

      case VT_InOutParam:
      {
         DeclareInout (os, tab, implClassName, argName);
      }
      break;

      default:
      {
         assert (0);
      }
   }
}

void be_DispatchableType::DeclareIn
(
   ostream& os,
   be_Tab& tab,
   const be_CppName& implClassName,
   const be_CppName& argName
) const
{
   switch (HowStoredInDispatcher (VT_InParam))
   {
      case STORED_AS_STACK_VARIABLE:
      {
         os << tab << TypeName () << " " << argName << ";" << nl;
      }
      break;

      case STORED_IN_VAR:
      case STORED_IN_ALLOCED_VAR:
      case STORED_IN_DESCENDANT_VAR:
      case STORED_IN_STRING_VAR:
      {
         os << tab << TypeName () << DDSVarExtension << " " << argName << ";" << nl;
      }
      break;

      case STORED_IN_IOR_VAR:
      {
         DDSString stubFactory = BE_Globals::RelativeScope
         (
            DDSString (implClassName),
            Scope (localName)
         );
         os << tab << ScopedName () << DDSVarExtension << " " << argName
         << " (new " << stubFactory;
         // Add extension
         os << DDSStubExtension << " ());" << nl;
      }
      break;

      default:
      {
         assert (0);
      }
   }
}

void be_DispatchableType::DeclareOutOrReturn
(
   ostream& os,
   be_Tab& tab,
   const be_CppName& argName,
   const be_ArgumentDirection& direction
) const
{
   switch (HowStoredInDispatcher(direction))
   {
      case STORED_AS_STACK_VARIABLE:
      {
         os << tab << TypeName () << " " << argName << ";" << nl;
      }
      break;

      case STORED_IN_VAR:
      case STORED_IN_STRING_VAR:
      case STORED_IN_ALLOCED_VAR:
      case STORED_IN_DESCENDANT_VAR:
      case STORED_IN_IOR_VAR:
      {
         os << tab << TypeName () << DDSVarExtension << " " << argName << ";" << nl;
      }
      break;

      default:
      {
         assert(0);
      }
   }
}

void be_DispatchableType::DeclareInout
(
   ostream& os,
   be_Tab& tab,
   const be_CppName& implClassName,
   const be_CppName& argName
) const
{
   switch (HowStoredInDispatcher (VT_InOutParam))
   {
      case STORED_AS_STACK_VARIABLE:
      {
         os << tab << TypeName () << " " << argName << ";" << nl;
      }
      break;

      case STORED_IN_VAR:
      case STORED_IN_STRING_VAR:
      case STORED_IN_DESCENDANT_VAR:
      {
         os << tab << TypeName () << DDSVarExtension << " " << argName << ";" << nl;
      }
      break;

      case STORED_IN_ALLOCED_VAR:
      {
         DDSString relTypeName =
            BE_Globals::RelativeScope(DDSString(implClassName), typeName);
         os << tab << relTypeName << DDSVarExtension << " " << argName << " = "
         << relTypeName << "_alloc ();" << nl;
      }
      break;

      case STORED_IN_IOR_VAR:
      {
         DDSString stubFactory =
            BE_Globals::RelativeScope(DDSString(implClassName),
                                      Scope(localName) + "");
         os << tab << ScopedName() << DDSVarExtension << " " << argName
            << " (new " << stubFactory << DDSStubExtension << " ());" << nl;
      }
      break;

      default:
      {
         assert(0);
      }
   }
}

DDSString be_DispatchableType::PassToServantMethod
(
   const be_CppName& argName,
   const be_ArgumentDirection& direction,
   int getargIndex
) const
{
   DDSString result;

   switch (direction)
   {
      case VT_InParam:
      {
         result = PassInToServantMethod (argName, getargIndex);
      }
      break;

      case VT_OutParam:
      {
         result = PassOutToServantMethod (argName, getargIndex);
      }
      break;

      case VT_InOutParam:
      {
         result = PassInoutToServantMethod (argName, getargIndex);
      }
      break;

      case VT_Return:
      default:
      {
         assert(0);
      }
   }

   return result;
}

// extract the m_value pointer from _out_[], cast it to the type of this,
// and dereference the pointer

DDSString be_DispatchableType::DereferenceMvalue
   (int getargIndex) const
{
   DDSString result = "*(";
   result += ScopedName ();
   result += (DDSString)"*)_out_[";
   result += getargIndex;
   result += "].m_value";
   return result;
}

DDSString be_DispatchableType::PassInToServantMethod
(
   const be_CppName& argName,
   int getargIndex
) const
{
   DDSString result;

   switch (HowStoredInDispatcher (VT_InParam))
   {
      case STORED_AS_STACK_VARIABLE:
      case STORED_IN_VAR:
      case STORED_IN_STRING_VAR:
      case STORED_IN_ALLOCED_VAR:
      {
         result = DereferenceMvalue (getargIndex);
      }
      break;

      case STORED_IN_DESCENDANT_VAR:
      {
         result += "*(";
         result += ScopedName();
         result += "**)_out_[";
         result += getargIndex;
         result += "].m_value";
      }
      break;

      case STORED_IN_IOR_VAR:
      {
         result = argName;
      }
      break;

      default:
      {
         assert(0);
      }
   }

   return result;
}

DDSString be_DispatchableType::PassOutToServantMethod
(
   const be_CppName& argName,
   int getargIndex
) const
{
   DDSString result;

   switch (HowStoredInDispatcher (VT_OutParam))
   {
      case STORED_AS_STACK_VARIABLE:
      case STORED_IN_VAR:
      case STORED_IN_STRING_VAR:
      case STORED_IN_ALLOCED_VAR:
      {
         result = argName;
      }
      break;

      case STORED_IN_DESCENDANT_VAR:
      case STORED_IN_IOR_VAR:
      {
         result = DDSString (argName) + DDSString (".out ()");
      }
      break;

      default:
      {
         assert(0);
      }
   }

   return result;
}

DDSString be_DispatchableType::PassInoutToServantMethod
(
   const be_CppName& argName,
   int getargIndex
) const
{
   DDSString result;

   switch (HowStoredInDispatcher (VT_InOutParam))
   {
      case STORED_AS_STACK_VARIABLE:
      case STORED_IN_VAR:
      case STORED_IN_STRING_VAR:
      case STORED_IN_ALLOCED_VAR:
      {
         result = DereferenceMvalue (getargIndex);
      }
      break;

      case STORED_IN_DESCENDANT_VAR:
      {
         result = "*(";
         result += ScopedName();
         result += "**&)_out_[";
         result += getargIndex;
         result += "].m_value";
      }
      break;

      case STORED_IN_IOR_VAR:
      {
         result = DDSString(argName) + DDSString(".inout()");
      }
      break;

      default:
      {
         assert(0);
      }
   }

   return result;
}

DDSString be_DispatchableType::GetargMvalue
(
   const be_CppName & argName,
   const be_ArgumentDirection & direction
) const
{
   DDSString result;

   switch (HowStoredInDispatcher (direction))
   {
      case STORED_AS_STACK_VARIABLE:
      {
         result = "&";
         result += argName;
         break;
      }
      case STORED_IN_VAR:
      case STORED_IN_STRING_VAR:
      case STORED_IN_DESCENDANT_VAR:
      {
         result = "&";
         result += DDSString(argName) + DDSString(".val ()");
         break;
      }
      case STORED_IN_ALLOCED_VAR:
      {
         result = DDSString (argName) + DDSString (".val ()");
         break;
      }
      case STORED_IN_IOR_VAR:
      {
         result = "&";
         result += DDSString (argName) + DDSString ("->get_ior ()");
         break;
      }
      default:
      {
         assert (0);
      }
   }

   return result;
}

DDSString be_DispatchableType::PutargMvalue
(
   const be_CppName& argName,
   const be_ArgumentDirection& direction,
   int getargIndex
) const
{
   DDSString result;

   switch (direction)
   {
      case VT_InOutParam:
      result = PutargMvalueInout (argName, getargIndex);
      break;

      case VT_Return:
      case VT_OutParam:
      result = PutargMvalueOutOrReturn (argName, direction, getargIndex);
      break;

      case VT_InParam:
      default:
      assert(0);
   }

   return result;
}

DDSString be_DispatchableType::PutargMvalueInout
(
   const be_CppName& argName,
   int getargIndex
) const
{
   DDSString result;

   switch (HowStoredInDispatcher (VT_InParam))
   {
      case STORED_AS_STACK_VARIABLE:
      case STORED_IN_VAR:
      case STORED_IN_STRING_VAR:
      case STORED_IN_ALLOCED_VAR:
      {
         result = "_out_[";
         result += getargIndex;
         result += "].m_value";
      }
      break;

      case STORED_IN_DESCENDANT_VAR:
      {
         result += "*(";
         result += ScopedName();
         result += "**)_out_[";
         result += getargIndex;
         result += "].m_value";
      }
      break;

      case STORED_IN_IOR_VAR:
      {
         result = "(";
         result += argName;
         result += ".in()) ? ";
         result += argName;
         result += "->get_ior() : 0";
      }
      break;

      default:
      {
         assert(0);
      }
   }

   return result;
}

DDSString be_DispatchableType::PutargMvalueOutOrReturn
(
   const be_CppName & argName,
   const be_ArgumentDirection & direction,
   int getargIndex
) const
{
   DDSString result;

   switch (HowStoredInDispatcher (direction))
   {
      case STORED_AS_STACK_VARIABLE:
      {
         result = "&";
         result += argName;
         break;
      }
      case STORED_IN_STRING_VAR:
      {
         result = "&";
         result += DDSString (argName) + DDSString (".val ()");
         break;
      }
      case STORED_IN_VAR:
      case STORED_IN_ALLOCED_VAR:
      case STORED_IN_DESCENDANT_VAR:
      {
         result = DDSString (argName) + DDSString (".val ()");
         break;
      }
      case STORED_IN_IOR_VAR:
      {
         result = DDSString ("(");
         result += DDSString (argName);
         result += DDSString (".in()) ? ");
         result += DDSString (argName);
         result += DDSString ("->get_ior () : 0");
         break;
      }
      default:
      {
         assert (0);
      }
   }

   return result;
}

void be_DispatchableType::InitializeInDispatcher
(
   ostream&,
   be_Tab&,
   const be_CppName&,
   const be_ArgumentDirection&
) const
{
}
