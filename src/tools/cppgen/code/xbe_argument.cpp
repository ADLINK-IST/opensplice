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
#include "Std.h"
#include "bool.h"
#include "xbe_argument.h"
#include "xbe_literals.h"
#include "xbe_typedef.h"
#include "xbe_type.h"
#include "xbe_structure.h"

// -------------------------------------------------
//  BE_ARGUMENT IMPLEMENTATION
// -------------------------------------------------

IMPL_NARROW_METHODS1(be_argument, AST_Argument)
IMPL_NARROW_FROM_DECL(be_argument)


// ***********************************************************
//  Type          in            inout      out      return
// -----------------------------------------------------------
// Primitive      P             P&         P&       P
// All others
//    (Interface, Struct/union, Array, Sequence, String, Any)
//
//                const T*      T_inout    T_out    T*
// ***********************************************************

be_argument::be_argument()
      :
      m_beType(0),
      m_direction(VT_InParam)
{
   assert(0);
}


be_argument::be_argument
(
   AST_Argument::Direction d,
   AST_Type *ft,
   UTL_ScopedName *n,
   const UTL_Pragmas &p
)
:
   AST_Decl (AST_Decl::NT_argument, n, p),
   AST_Field (AST_Decl::NT_argument, ft, n, p),
   AST_Argument (d, ft, n, p),
   m_beType (0),
   m_direction (d)
{
   AST_Type * at = field_type();
   be_Type * bt;

   if (at && (bt = (be_Type*)at->narrow((long) & be_Type::type_id)))
   {
      const char * typeName = bt->TypeName ();
      if (typeName && strcmp (typeName, "DDS::Any") == 0)
      {
         BE_Globals::inc_any = pbtrue;
      }
   }
}

void be_argument::Initialize ()
{
   AST_Type * at = field_type();
   be_Type * bt;

   if (at && (bt = (be_Type*)at->narrow((long) & be_Type::type_id)))
   {
      bt->Initialize ();
   }
}

DDS_StdString
be_argument::LocalName() const
{
   DDS_StdString ret;
   be_argument* me = (be_argument*)this;

   assert(me->local_name());

   if (me->local_name())
   {
      ret = BE_Globals::KeywordToLabel(me->local_name()->get_string());
   }

   return ret;
}

const DDS_StdString&
be_argument::Signature(const DDS_StdString& className)
{
   be_Type * btype = be_Type::_narrow(field_type());

   assert(btype);

   if (btype)
   {
      btype->Initialize();

      VarType paramdir = MakeVarType(direction());
      m_signature = btype->MakeSignature(paramdir, className);
   }

   return m_signature;
}

pbbool
be_argument::IsInArg() const
{
   return (pbbool)(((be_argument*)this)->direction() == dir_IN);
}

pbbool
be_argument::IsInOutArg() const
{
   return (pbbool)(((be_argument*)this)->direction() == dir_INOUT);
}

pbbool
be_argument::IsOutArg() const
{
   return (pbbool)(((be_argument*)this)->direction() == dir_OUT);
}

be_Type*
be_argument::BeType() const
{
   be_Type * ret = 0;
   AST_Type* atype = ((be_argument*)this)->field_type();

   if (atype)
   {
      ret = (be_Type*)atype->narrow((long) & be_Type::type_id);
      assert(ret);
   }

   return ret;
}

DDS_StdString be_argument::ParamMode () const
{
   DDS_StdString ret = "DDS::PARAM_OUT";

   if (direction() == AST_Argument::dir_IN)
   {
      ret = "DDS::PARAM_IN";
   }
   else if (direction() == AST_Argument::dir_INOUT)
   {
      ret = "DDS::PARAM_INOUT";
   }

   return ret;
}

pbbool be_argument::PassPointer (AST_Argument::Direction dir) const
{
   pbbool ret = pbfalse;
   be_Type * btype = BeType();

   if (dir == AST_Argument::dir_OUT && !btype->IsFixedLength() &&
         !btype->IsInterfaceType() && !btype->IsStringType())
   {
      ret = pbtrue;
   }

   return ret;
}

DDS_StdString
be_argument::SyncStreamOut(const DDS_StdString& out, pbbool isStubSide) const
{
   if (BeType()->IsOpaqueType() && isStubSide)
   {
      return BeType()->UnionStreamOut(LocalName(), out);
   }
   else
      return BeType()->SyncStreamOut(LocalName(), out, MakeVarType(((be_argument *)this)->direction()));
}

DDS_StdString
be_argument::SyncStreamIn(const DDS_StdString& in, pbbool isStubSide) const
{
   if (BeType()->IsOpaqueType() && isStubSide)
   {
      return BeType()->UnionStreamIn(LocalName(), in);
   }
   else
      return BeType()->SyncStreamIn(LocalName(), in, MakeVarType(((be_argument *)this)->direction()));
}

DDS_StdString
be_argument::Releaser() const
{
   return BeType()->Releaser(LocalName());
}

const be_DispatchableType&
be_argument::ArgumentType() const
{
   // YO BEN this cast should be unnecessary; a be_argument should only
   // be able to hold a be_DispatchableType
   be_DispatchableType* pDispatchable = (be_DispatchableType*)
                                        get_be_type()->narrow((long) & be_DispatchableType::type_id);

   if (pDispatchable != 0)
   {
      return *pDispatchable;
   }
   else
   {
      assert(0);
      // YO BEN this is just to please the compiler; delete when
      // m_beType is declared be_DispatchableType
      be_DispatchableType* result = new be_structure;
      return *result;
   }
}
