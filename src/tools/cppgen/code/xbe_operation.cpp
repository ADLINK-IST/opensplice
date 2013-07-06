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
#include "idl_extern.h"
#include "xbe_globals.h"
#include "xbe_literals.h"
#include "xbe_operation.h"
#include "xbe_argument.h"
#include "xbe_predefined.h"
#include "xbe_array.h"
#include "xbe_interface.h"
#include "xbe_utils.h"
#include "xbe_type.h"
#include "xbe_typedef.h"
#include "xbe_dispatchable.h"
#include "xbe_dispatcher.h"

#include "cppgen_iostream.h"

// -------------------------------------------------
//  BE_OPERATION IMPLEMENTATION
// -------------------------------------------------
IMPL_NARROW_METHODS2 (be_operation, AST_Operation, be_CodeGenerator)
IMPL_NARROW_FROM_DECL (be_operation) IMPL_NARROW_FROM_SCOPE (be_operation)
     be_operation *
     be_operation::_narrow (AST_Decl * decl)
{
   be_operation *ret = 0;

   if (decl)
   {
      ret = (be_operation *) decl->narrow ((long) &be_operation::type_id);
   }

   return ret;
}

be_operation::be_operation () :
   returnType (0),
   n_InArgs (0),
   n_OutArgs (0),
   n_InOutArgs (0),
   n_Args (0)
{
   isAtModuleScope (pbfalse);
}


be_operation::be_operation 
(
   AST_Type * rt,
   AST_Operation::Flags fl,
   UTL_ScopedName * n,
   const UTL_Pragmas & p
)
:
   AST_Decl (AST_Decl::NT_op, n, p),
   UTL_Scope (AST_Decl::NT_op, n, p),
   AST_Operation (rt, fl, n, p),
   returnType (0),
   n_InArgs (0),
   n_OutArgs (0),
   n_InOutArgs (0),
   n_Args (0)
{
   assert (return_type ());

   isAtModuleScope (pbfalse);

   if (return_type ())
   {
      returnType = (be_DispatchableType *) return_type ()->
         narrow ((long) &be_DispatchableType::type_id);

      const char * typeName = returnType->TypeName ();

      if (typeName && strcmp (typeName, "DDS::Any") == 0)
      {
         BE_Globals::inc_any = pbtrue;
      }
   }
   else
   {
      DDSError ((DDS_StdString) "unknown return type for operation " +
                LocalName ());
   }

   enclosingScope = be_Type::EnclosingScopeString (this);
   stubClassname = enclosingScope + DDSStubExtension;
   opKey = NameToString (name (), "_");
}


DDS_StdString be_operation::LocalName ()
{
   DDS_StdString ret;

   assert (local_name ());

   if (local_name ())
   {
      ret = BE_Globals::KeywordToLabel (local_name ()->get_string ());
   }

   return ret;
}


void
be_operation::Initialize (be_interface * owner)
{
  iface = owner;
  interfaceBasename = iface->ScopedName ();

  if (returnType)
    {
      returnType->Initialize ();
    }
}


const DDS_StdString &
be_operation::InterfaceBasename ()
{
  return interfaceBasename;
}


AST_Argument *
be_operation::add_argument (AST_Argument * arg)
{
   AST_Argument *ret = 0;

   assert (arg);

   if (AST_Operation::add_argument (arg))
   {
      be_argument *barg =
         (be_argument *) arg->narrow ((long) &be_argument::type_id);

      assert (barg);
      assert (arg->field_type ());
      assert (arg->field_type ()->narrow ((long) &be_Type::type_id));

      if (barg)
      {
         arguments.push_back (barg);

         if (barg->is_in_arg ())
         {
            ++n_InArgs;
         }
         else if (barg->is_out_arg ())
         {
            ++n_OutArgs;
         }
         else if (barg->is_inout_arg ())
         {
            ++n_InOutArgs;
         }

         ++n_Args;
      }

      ret = arg;
   }

   return ret;
}

DDS_StdString be_operation::BaseSignature 
   (const DDS_StdString & implclassname)
{
   UTL_ScopeActiveIterator *
      i = new UTL_ScopeActiveIterator (this, IK_decls);
   ostringstream os;
   pbbool first = pbtrue;
   DDS_StdString ret;
   AST_Decl * d;
   bool fullScope = (iface->BaseClassname () != implclassname);

   // GENERATE RETURN TYPE
   assert (returnType);

   if (fullScope)
   {
      os << returnType->MakeSignature (VT_Return, "") << " ";
   }
   else
   {
      os << returnType->MakeSignature (VT_Return,
                                       iface->ScopedName ()) << " ";
   }

   os << LocalName ();

   // GENERATE PARAMETER LIST
   os << " (";

   for (; !(i->is_done ()); i->next ())
   {
      be_argument *arg;

      d = i->item ();

      arg = (be_argument *) d->narrow ((long) &be_argument::type_id);
      if (arg)
      {
         if (!first)
            os << ", ";

         first = pbfalse;

         if (fullScope)
         {
            os << arg->Signature ("");
         }
         else
         {
            os << arg->Signature (iface->ScopedName ());
         }

         os << " " << *arg->local_name ();
      }
   }

   delete i;

#if defined(CONTEXT_SUPPORT)
   //
   // add context
   //

   if (context ())
   {
      if (!first)
      {
         os << ", ";
      }

      first = pbfalse;

      os << BE_Globals::CorbaScope ("Context_ptr");
      os << " " << DDSCtxVar;
   }

#endif

   if (!first)
   {
      os << XBE_Ev::arg (XBE_ENV_ARGN);
   }
   else
   {
      os << XBE_Ev::arg (XBE_ENV_ARG1);
   }

   os << ")" << ends;

   ret = os.str().c_str();
   return ret;
}


DDS_StdString be_operation::ScopedBaseRequestCall ()
{
  UTL_ScopeActiveIterator *
    i = new UTL_ScopeActiveIterator (this, IK_decls);
  ostringstream
    os;
  pbbool
    first = pbtrue;
  DDS_StdString
    ret;
  AST_Decl *
    d;

  DDS_StdString
    ifaceBasename_nocolons = NoColons (iface->ScopedName ());

  if (ifaceBasename_nocolons.length ())
    {
      os << ifaceBasename_nocolons << "::" << LocalName ();
    }
  else
    {
      os << LocalName ();
    }

  // GENERATE PARAMETER LIST
  os << "(";

  for (; !(i->is_done ()); i->next ())
    {
      be_argument *
        arg;

      d = i->item ();

      arg = (be_argument *) d->narrow ((long) &be_argument::type_id);
      if (arg)
        {
          if (!first)
            {
              os << ", ";
            }
          first = pbfalse;

          os << " " << arg->LocalName ();
        }
    }

  delete
    i;

  // ADD CONTEXT

  if (context ())
    {
      if (!first)
        os << ", ";

      first = pbfalse;

      os << " " << DDSCtxVar;
    }

  // THIS IS FOR PROPERTIES
  if (!first)
    os << ", ";

  os << "0";

  first = pbfalse;

  os << ")" << ends;

  ret = os.str().c_str();
  return ret;
}


DDS_StdString be_operation::StubSignature (OP_SignatureType sigType)
{
   UTL_ScopeActiveIterator * i = new UTL_ScopeActiveIterator (this, IK_decls);
   ostringstream os;
   pbbool first = pbtrue;
   DDS_StdString ret;
   AST_Decl * d;

   // Generate return type

   assert (returnType);

   if (sigType == OP_Implementation)
   {
      os << returnType->MakeSignature (VT_Return, "") << " " << nl;
   }
   else
   {
      os << returnType->MakeSignature (VT_Return, iface->ScopedName ()) << " ";
   }

   // Op name

   if (sigType == OP_Implementation)
   {
      os << iface->Scope (iface->StubClassname ()) << "::" << LocalName ();
   }
   else
   {
      os << LocalName ();
   }

   // Generate parameter list

   os << " (";

   for (; !(i->is_done ()); i->next ())
   {
      be_argument * arg;

      d = i->item ();

      arg = (be_argument *) d->narrow ((long) &be_argument::type_id);
      if (arg)
      {
         if (!first)
         {
            os << ", ";
         }

         first = pbfalse;
         os << arg->Signature (iface->ScopedName ());
         os << " " << *arg->local_name ();
      }
   }

   delete i;

#if defined (CONTEXT_SUPPORT)

   // Add context

   if (context ())
   {
      if (!first)
      {
         os << ", ";
      }

      first = pbfalse;

      os << BE_Globals::CorbaScope ("Context_ptr");
      os << " " << DDSCtxVar;
   }

#endif

   if (!first)
   {
      os << XBE_Ev::arg (XBE_ENV_ARGN);
   }
   else
   {
      os << XBE_Ev::arg (XBE_ENV_ARG1);
   }

   os << ")" << ends;

   ret = os.str().c_str();
   return ret;
}


DDS_StdString
be_operation::ReplySignature
  (ExceptionStatus excStat, OP_SignatureType sigType)
{
  UTL_ScopeActiveIterator *i = new UTL_ScopeActiveIterator (this, IK_decls);
  ostringstream os;
  DDS_StdString ret = "";
  AST_Decl *d;
  DDS_StdString op_suffix = "";

  // VOID RETURN

  if (sigType == OP_Declaration || sigType == OP_Implementation)
    {
      os << "void ";
    }

  if (excStat == OP_NoException)
    {
      // no suffix
    }
  else if (excStat == OP_Exception)
    {
      op_suffix = "_exc";
    }
  else
    {
      op_suffix = "_retry";
    }

  // OP NAME
  os << "handle_" << LocalName () << op_suffix;

  // GENERATE PARAMETER LIST
  os << "(";

  os << "DDS::Codec::RequestId rid, ";

  os << "DDS::Stub_ptr stub";

  if (excStat == OP_NoException)
    {
      // GENERATE RETURN

      if (HasReturn ())
        {
          assert (returnType);
          os << ", ";

          if (returnType->IsFixedLength () && returnType->IsStructuredType ())
            {
              os << returnType->MakeSignature (VT_InOutParam, "");
            }
          else
            {
              os << returnType->MakeSignature (VT_Return, "");
            }
        }

      for (; !(i->is_done ()); i->next ())
        {
          be_argument *arg;

          d = i->item ();

          arg = (be_argument *) d->narrow ((long) &be_argument::type_id);
          if (arg)
            {
              if (arg->is_out_arg () || arg->is_inout_arg ())
                {
                  be_Type *argtype = be_Type::_narrow (arg->field_type ());

                  os << ", ";

                  if (sigType != OP_Invoke)
                    {
                      if (argtype->IsFixedLength ()
                          && argtype->IsStructuredType ())
                        {
                          os << argtype->MakeSignature (VT_InOutParam, "");
                        }
                      else
                        {
                          os << argtype->MakeSignature (VT_Return, "");
                        }
                    }

                  os << " " << *arg->local_name ();
                }
            }
        }

      delete i;
      os << ")" << ends;
    }
  else if (excStat == OP_Exception)
  {
     os << ", DDS::Exception & exception)" << ends;
  }
  else                          // (excStat == OP_Retry)
  {
     os << ")" << ends;
  }

  ret = os.str ().c_str();
  return ret;
}


DDS_StdString be_operation::TieSignature (OP_SignatureType sigType)
{
  UTL_ScopeActiveIterator *
    i = new UTL_ScopeActiveIterator (this, IK_decls);
  ostringstream
    os;
  pbbool
    first = pbtrue;
  DDS_StdString
    ret;
  AST_Decl *
    d;

  // GENERATE RETURN TYPE

  if (sigType != OP_Invoke)
    {
      assert (returnType);
      os << returnType->MakeSignature (VT_Return, "") << " ";
    }

  // OP NAME
  os << LocalName ();

  // GENERATE PARAMETER LIST
  os << "(";

  for (; !(i->is_done ()); i->next ())
    {
      be_argument *
        arg;

      d = i->item ();

      arg = (be_argument *) d->narrow ((long) &be_argument::type_id);
      if (arg)
        {
          if (!first)
            os << ", ";

          first = pbfalse;

          if (sigType != OP_Invoke)
            {
              os << arg->Signature ("");
            }

          os << " " << *arg->local_name ();
        }
    }

  delete
    i;

#if defined(CONTEXT_SUPPORT)
  // ADD CONTEXT

  if (context ())
    {
      if (!first)
        {
          os << ", ";
        }

      first = pbfalse;

      if (sigType != OP_Invoke)
        {
          os << BE_Globals::CorbaScope ("Context_ptr");
        }

      os << " " << DDSCtxVar;
    }

#endif

  if (sigType == OP_Implementation)
    {
      if (!first)
      {
         os << XBE_Ev::arg (XBE_ENV_ARGN);
      }
      else
      {
         os << XBE_Ev::arg (XBE_ENV_ARG1);
      }
    }
  else if (sigType == OP_Invoke)
    {
      if (!first)
      {
         os << XBE_Ev::arg (XBE_ENV_VARN);
      }
      else
      {
         os << XBE_Ev::arg (XBE_ENV_VAR1);
      }
    }

  os << ")" << ends;

  ret = os.str ().c_str();
  return ret;
}

DDS_StdString be_operation::DirectSignature
(
   OP_SignatureType sigType,
   const DDS_StdString & implclassname
)
{
   UTL_ScopeActiveIterator *i =
      new UTL_ScopeActiveIterator (this, IK_decls);
   ostringstream os;
   pbbool first = pbtrue;
   DDS_StdString ret;
   AST_Decl *d;

   // GENERATE RETURN TYPE

   if (sigType != OP_Invoke)
   {
      assert (returnType);
      os << returnType->MakeSignature (VT_Return, "") << " ";
   }

   if (sigType != OP_Declaration)
   {
      os << implclassname << "::";
   }

   // OP NAME
   os << LocalName ();

   // GENERATE PARAMETER LIST
   os << "(";

   for (; !(i->is_done ()); i->next ())
   {
      be_argument *arg;

      d = i->item ();

      arg = (be_argument *) d->narrow ((long) &be_argument::type_id);
      if (arg)
      {
         if (!first)
            os << ", ";

         first = pbfalse;

         if (sigType != OP_Invoke)
         {
            os << arg->Signature ("");
         }

         os << " " << *arg->local_name ();
      }
   }

   delete i;

#if defined(CONTEXT_SUPPORT)
   // ADD CONTEXT

   if (context ())
   {
      if (!first)
      {
         os << ", ";
      }

      first = pbfalse;

      if (sigType != OP_Invoke)
      {
         os << BE_Globals::CorbaScope ("Context_ptr");
      }

      os << " " << DDSCtxVar;
   }

#endif

   if (sigType == OP_Invoke)
   {
      if (!first)
      {
         os << XBE_Ev::arg (XBE_ENV_VARN);
      }
      else
      {
         os << XBE_Ev::arg (XBE_ENV_VAR1);
      }
   }
   else
   {
      if (!first)
      {
         os << XBE_Ev::arg (XBE_ENV_ARGN);
      }
      else
      {
         os << XBE_Ev::arg (XBE_ENV_ARG1);
      }
   }

   os << ")" << ends;

   ret = os.str ().c_str ();
   return ret;
}

pbbool be_operation::HasReturn ()
{
  pbbool
    ret = pbfalse;

  if (returnType)
    {
      assert (returnType->TypeName ());

      if (returnType->TypeName ())
        {
          ret = strcmp (returnType->TypeName (), "void") ? TRUE : FALSE;
        }
    }

  return ret;
}

void
be_operation::GenerateVirtual
(
   be_Source & source, 
   const DDS_StdString & implclassname
)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);
   be_Type *rettype =
      (be_Type *) return_type ()->narrow ((long) &be_Type::type_id);

   (void) rettype;
   assert (rettype);

   os << tab << "virtual " << BaseSignature (implclassname);
   os << " = 0;" << nl;
}


void be_operation::GenerateStub (be_ClientImplementation & source)
{
   be_OpStubGenerator stub
   (
      stubClassname,
      opKey,
      LocalName (),
      StubSignature (OP_Implementation),
      returnType,
      is_oneway (),
      arguments,
      exceptions(),
      context (),
      local_name ()->get_string ()
   );

   stub.Generate (source);
}


void
be_operation::GenerateSyncCall (be_ServerImplementation & source)
{
  const DDS_StdString opret ("_ret_");
  pbbool firstArg = pbtrue;
  ostream & os = source.Stream ();
  be_Tab tab (source);
  TList < be_argument * >::iterator ait;

  os << tab;

  if (HasReturn ())
  {
     os << opret << " = ";
  }

  os << "_servant_->" << LocalName () << "(";

  // PASS ALL ARGS

   for (ait = arguments.begin (); ait != arguments.end (); ait++)
   {
      if (!firstArg)
      {
         os << ", ";
      }

      firstArg = pbfalse;

      os << (*ait)->LocalName ();
   }

   if (firstArg)
   {
      os << XBE_Ev::arg (XBE_ENV_VAR1);
   }
   else
   {
      os << XBE_Ev::arg (XBE_ENV_VARN);
   }

   os << ");" << nl;
}

void be_operation::GenerateImpureRequestCall (be_ClientImplementation & source)
{
   be_OpStubGenerator stub
   (
      stubClassname,
      opKey,
      LocalName (),
      StubSignature (OP_Implementation),
      returnType,
      is_oneway (),
      arguments,
      exceptions (),
      context (),
      local_name ()->get_string ()
   );

   stub.Generate (source);
}

void
be_operation::GenerateDispatcher (be_ServerImplementation & source,
                                  const DDS_StdString & implclassname)
{
   GenerateSyncDispatcher (source, implclassname);
}


void
be_operation::GenerateSyncDispatcher (be_ServerImplementation & source,
                                      const DDS_StdString & implclassname)
{
   be_Dispatcher dispatcher
   (
      LocalName (),
      implclassname,
      arguments,
      source,
      returnType
   );
   dispatcher.Generate ();
}

void
be_operation::Generate (be_ServerImplementation & source)
{
}


void
be_operation::Generate (be_ClientHeader & source)
{
}


void
be_operation::Generate (be_ClientImplementation & source)
{
  GenerateStub (source);
}
