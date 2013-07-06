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
#include "cppgen_iostream.h"

#include "idl.h"
#include "idl_extern.h"

#include "xbe_globals.h"
#include "xbe_literals.h"
#include "xbe_attribute.h"
#include "xbe_argument.h"
#include "xbe_interface.h"
#include "xbe_utils.h"
#include "xbe_predefined.h"
#include "xbe_typedef.h"
#include "xbe_dispatcher.h"


// -------------------------------------------------
//  BE_ATTRIBUTE IMPLEMENTATION
// -------------------------------------------------
IMPL_NARROW_METHODS2(be_attribute, AST_Attribute, be_CodeGenerator)
IMPL_NARROW_FROM_DECL(be_attribute)

be_attribute * be_attribute::_narrow (AST_Decl * decl)
{
   be_attribute * ret = 0;

   if (decl)
   {
      ret = (be_attribute*)decl->narrow((long) & be_attribute::type_id);
   }

   return ret;
}

be_attribute::be_attribute ()
:
   m_getDispatchDone (FALSE),
   m_setDispatchDone (FALSE)
{
   isAtModuleScope (pbfalse);
}

be_attribute::be_attribute
(
   idl_bool ro,
   AST_Type *ft,
   UTL_ScopedName *n,
   const UTL_Pragmas &p
)
:
   AST_Decl (AST_Decl::NT_attr, n, p),
   AST_Field (AST_Decl::NT_attr, ft, n, p),
   AST_Attribute (ro, ft, n, p),
   fieldType (0),
   m_getDispatchDone (FALSE),
   m_setDispatchDone (FALSE)
{
   assert(field_type());

   isAtModuleScope(pbfalse);

   if (field_type())
   {
      fieldType =
         (be_DispatchableType*)field_type()->narrow((long) & be_Type::type_id);
      assert(fieldType);
      const char * typeName = fieldType->TypeName ();
      if (typeName && strcmp (typeName, "DDS::Any") == 0)
      {
         BE_Globals::inc_any = pbtrue;
      }
   }
   else
   {
      DDSError((DDS_StdString)"unknown field type for attribute " + LocalName());
   }

   enclosingScope = be_Type::EnclosingScopeString(this);
   setOpKey = (DDS_StdString) "_set_" + LocalName();
   getOpKey = (DDS_StdString) "_get_" + LocalName();
}

DDS_StdString
be_attribute::LocalName()
{
   DDS_StdString ret;

   assert(local_name());

   if (local_name())
   {
      ret = BE_Globals::KeywordToLabel(local_name()->get_string());
   }

   return ret;
}

void
be_attribute::Initialize(be_interface * owner, const DDS_StdString& className)
{
   interfaceBasename = className;

   if (fieldType)
   {
      fieldType->Initialize();
   }
}

void
be_attribute::GenerateImpureRequestCall(be_ClientImplementation& source)
{
   be_AttStubGenerator getStub
   (
      enclosingScope,
      getOpKey,
      LocalName (),
      GetSignature (AT_Implementation, InterfaceBasename (), pbfalse),
      fieldType,
      FALSE
   );

   getStub.Generate(source);

   if (!readonly())
   {
      DDS_StdString setArg ("_nval_");
      DDS_StdString setSignature = SetSignature
      (
         AT_Implementation,
         InterfaceBasename(),
         FALSE,
         setArg
      );
      be_AttStubGenerator setStub
      ( 
         enclosingScope,
         setOpKey,
         LocalName(),
         setSignature,
         fieldType,
         pbtrue
      );

      setStub.Generate(source);
   }
}

DDS_StdString
be_attribute::GetSignature(AT_SignatureType sigType,
                           const DDS_StdString& className,
                           pbbool pureVirtual)
{
   DDS_StdString ret;

   assert(fieldType);

   if (fieldType)
   {
      ostringstream os;

      // Output the type
      switch (sigType)
      {

            case AT_Implementation:
            os << fieldType->MakeSignature(VT_Return) << "\n";
            os << className << "::";
            break;

            case AT_Declaration:
            os << fieldType->MakeSignature(VT_Return, className) << "\t";
            break;

            case AT_TieImplementation:
            os << fieldType->MakeSignature(VT_Return, "") << " ";
            break;

            case AT_Invoke:

            case AT_TieInvoke:
            // nothing
            break;

            default:
            assert(0);
            break;
      }

      os << LocalName() << "(";

      switch (sigType)
      {

            case AT_Declaration:
            os << XBE_Ev::arg (XBE_ENV_ARG1);
            break;

            case AT_Implementation:

            case AT_TieImplementation:
            os << XBE_Ev::arg (XBE_ENV_ARG1);
            break;

            case AT_Invoke:
            os << XBE_Ev::arg (XBE_ENV_VAR1);
            break;

            case AT_TieInvoke:
            // nothing
            break;

            default:
            assert(0);
            break;
      }

      os << ")" << ends;

      ret = os.str().c_str();
   }

   return ret;
}

DDS_StdString
be_attribute::SetSignature(AT_SignatureType sigType,
                           const DDS_StdString& className,
                           pbbool pureVirtual,
                           const DDS_StdString& argName)
{
   DDS_StdString ret;

   if (fieldType)
   {
      ostringstream os;

      if ( sigType == AT_Implementation ||
            sigType == AT_Declaration ||
            sigType == AT_TieImplementation )
      {
         os << "void ";
      }

      if (sigType == AT_Implementation)
      {
         os << className << "::";
      }


      os << LocalName() << "(";

      if ( sigType == AT_TieImplementation ||
            sigType == AT_Declaration ||
            sigType == AT_Implementation )
      {
         os << fieldType->MakeSignature(VT_InParam, className);
         os << " ";
      }

      os << argName;

      switch (sigType)
      {
         case AT_Declaration:
         case AT_Implementation:
         case AT_TieImplementation:
            os << XBE_Ev::arg (XBE_ENV_ARGN);
            break;

         case AT_Invoke:
            os << XBE_Ev::arg (XBE_ENV_VARN);
            break;

         case AT_TieInvoke:
            // nothing
            break;

         default:
            assert(0);
      }

      os << ")" << ends;

      ret = os.str().c_str();
   }

   return ret;
}

DDS_StdString
be_attribute::StubClassname()
{
   return be_Type::EnclosingScopeString(this) + DDSStubExtension;
}

const DDS_StdString&
be_attribute::InterfaceBasename()
{
   return interfaceBasename;
}

void
be_attribute::GenerateVirtual
(
   be_Source& source,
   const DDS_StdString& className
)
{
   ostream & os = source.Stream();
   be_Tab tab(source);
   DDS_StdString responsibility = NameToString(name());

   if (fieldType)
   {
      // PURE VIRTUAL GET ATTRIBUTE
      os << tab << "virtual " << GetSignature(AT_Declaration, className);
      os << " = 0;" << nl;

      // PURE VIRTUAL SET ATTRIBUTE
      if (!readonly())
      {
         os << tab << "virtual " << SetSignature(AT_Declaration, className, pbtrue);
         os << " = 0;" << nl;
      }
   }
}

void be_attribute::GenerateGetStub (be_ClientImplementation & source)
{
   be_AttStubGenerator stub
   (
      enclosingScope,
      getOpKey,
      getOpKey,
      GetSignature (AT_Implementation, StubClassname ()),
      fieldType,
      FALSE
   );

   stub.Generate (source);
}

void be_attribute::GenerateSetStub (be_ClientImplementation & source)
{
   DDS_StdString setArg ("_nval_");
   be_AttStubGenerator stub
   (
      enclosingScope,
      setOpKey,
      setOpKey,
      SetSignature
      (
         AT_Implementation,
         StubClassname(),
         TRUE,
         setArg
      ),
      fieldType,
      pbtrue
   );

   stub.Generate (source);
}


void
be_attribute::Generate(be_ClientHeader& source)
{}

void
be_attribute::Generate(be_ServerImplementation& source)
{}

void be_attribute::GenerateGetDispatcher
(
   be_ServerImplementation & source,
   const DDS_StdString & implClassName
)
{
   be_CppName name (LocalName ());
   DDS_StdString strName ("_get_" + LocalName ());
   be_CppName getName (strName);

   be_Dispatcher dispatcher
   (
      name,
      getName,
      implClassName,
      be_ArgumentList (),
      source,
      fieldType
   );
   dispatcher.Generate ();
}

void be_attribute::GenerateSetDispatcher
(
   be_ServerImplementation & source,
   const DDS_StdString & implClassName
)
{
   be_CppName name (LocalName ());
   DDS_StdString strName ("_set_" + LocalName ());
   be_CppName setName (strName);
   be_Argument arg ("_nval_", *fieldType, VT_InParam);
   be_ArgumentList list (arg);

   be_Dispatcher dispatcher
   (
      name,
      setName,
      implClassName,
      list,
      source
   );
   dispatcher.Generate ();
}

void be_attribute::Generate (be_ClientImplementation & source)
{
   GenerateGetStub (source);

   if (!readonly ())
   {
      GenerateSetStub (source);
   }
}
