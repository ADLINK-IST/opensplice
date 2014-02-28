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
#include "xbe_sequence.h"
#include "xbe_string.h"
#include "xbe_root.h"
#include "xbe_interface.h"
#include "xbe_utils.h"
#include "xbe_typedef.h"
#include "xbe_union.h"
#include "xbe_predefined.h"
#include "xbe_enum.h"

// -------------------------------------------------
//  BE_SEQUENCE IMPLEMENTATION
// -------------------------------------------------
String_map be_sequence::generatedSequences(idlc_hash_str);
String_map be_sequence::generatedSequencesInstantiations(idlc_hash_str);

IMPL_NARROW_METHODS4(be_sequence, AST_Sequence, be_CodeGenerator,
                     be_DispatchableType, be_Type)
IMPL_NARROW_FROM_DECL(be_sequence)

be_sequence::be_sequence ()
:
   initialized (pbfalse),
   anonymous (pbtrue),
   deferred (pbfalse)
{
   isAtModuleScope (pbfalse);
}

be_sequence::be_sequence (AST_Expression *v, AST_Type *t)
:
   AST_Decl
   (
      AST_Decl::NT_sequence,
      new UTL_ScopedName (new Identifier ("sequence", 1, 0, I_FALSE), NULL)
   ),
   AST_Sequence(v, t),
   initialized (pbfalse),
   maxSize (0),
   anonymous (pbtrue),
   deferred (pbfalse),
   isPrimitiveSeq (pbfalse),
   isStringSeq (pbfalse),
   isInterfaceSeq (pbfalse),
   baseType (0)
{
   // NOTE: BASE TYPE AND MAX SIZE ARE KNOWN
   assert(base_type());
   assert(max_size());

   static int sequence_count = 0;

   isAtModuleScope(pbfalse);
   idlType = this;

   if (base_type())
   {
      baseType = be_Type::_narrow(base_type());
      baseType = baseType->idlType;
      isStringSeq = baseType->IsStringType();
      isInterfaceSeq = baseType->IsInterfaceType();
      isValueSeq = baseType->IsValueType();
      isPrimitiveSeq = (baseType->IsPrimitiveType()
                        && baseType->IsFixedLength() // eliminates ANY, etc..
                        && !baseType->IsEnumeratedType());

      // Sequence of Typecode mem leak fix eCPP896
      be_predefined_type * pdt = be_predefined_type::_narrow(base_type());

      if (pdt && (pdt->pt() == AST_PredefinedType::PT_typecode))
      {
         isInterfaceSeq = TRUE;
      }
   }
   else
   {
      UTL_Error* oops = new UTL_Error;

      oops->error0(UTL_Error::EIDL_LOOKUP_ERROR);
      delete oops;
   }

   if (baseType->IsExceptionType())
   {
      UTL_Error* oops = new UTL_Error;

      oops->error0(UTL_Error::EIDL_ILLEGAL_USE);
      delete oops;
   }

   maxSize = ExprToULong(max_size());

   if (baseType)
   {
      be_Type * unaliasedBase = be_typedef::_beBase(base_type());

      baseTypeName = baseType->SequenceMemberTypeName();

      //
      // create type id (YO maybe should be the same as the equivalence id
      //
      localName = (DDS_StdString)"_s_" + baseType->TypeName() + "_";
      localName += BE_Globals::int_to_string(MaxSize());
      ColonToBar((char *)localName);

      //
      // create operational type equivalence id
      //
      m_any_op_id = (DDS_StdString)"_s_" + unaliasedBase->any_op_id() + "_";
      m_any_op_id += BE_Globals::int_to_string(MaxSize()) + "_";
      m_any_op_id += BE_Globals::int_to_string(sequence_count++);
      ColonToBar((char *)m_any_op_id);
   }

   // initialize typecode
   m_typecode->kind = DDS::tk_sequence;

   m_typecode->members.push_back(baseType->m_typecode);

   m_typecode->bounds = MaxSize();

   m_typecode->length = 0;

   if (!IsBounded())
   {
      m_typecode->bounds = 0;
   }

   if (isStringSeq)
   {
      m_typecode->id = "SEQ_DDS::String_" + BE_Globals::ulong_to_string(m_typecode->bounds);
   }

   init_type (enclosingScope, localName);
}

void
be_sequence::Initialize()
{}

void
be_sequence::InitializeTypeMap(be_Type* t)
{
   idlType = t;

   DDS_StdString outName;

   if (t)
   {
      t->TypeName(t->Scope(t->LocalName()));
      t->InTypeName("const " + t->TypeName() + "&");
      t->InOutTypeName(t->TypeName() + "&");

      t->OutTypeName(t->TypeName() + DDSOutExtension);
      t->ReturnTypeName(t->TypeName() + "*");

      t->DMFAdtMemberTypeName(t->TypeName() + "*");
      t->StructMemberTypeName(t->TypeName());
      t->UnionMemberTypeName(t->TypeName());
      t->SequenceMemberTypeName(t->TypeName());


      t->VarSignature(VT_InParam, t->TypeName(), VT_Const, VT_Var, VT_Reference);
      t->VarSignature(VT_InOutParam, t->TypeName(), VT_NonConst, VT_Var, VT_Reference);
      t->VarSignature(VT_OutParam, t->OutTypeName(), VT_NonConst, VT_Var, VT_NonReference);
      t->VarSignature(VT_Return, t->TypeName(), VT_NonConst, VT_Pointer, VT_NonReference);

      be_sequence * t_sequence = 0;

      if ((t_sequence = (be_sequence*)t->narrow((long) & be_sequence::type_id)))
      {
         t_sequence->OsOpSignature((DDS_StdString)"DDS::Codec::OutStream& operator<<(DDS::Codec::OutStream& os, const "
                                   + t->Scope(t->LocalName()) + "& s)");
         t_sequence->IsOpSignature((DDS_StdString)"DDS::Codec::InStream& operator>>(DDS::Codec::InStream& is, "
                                   + t->Scope(t->LocalName()) + "& s)");
      }

      if (BE_Globals::nesting_bug == pbtrue)
      {
         const char * streamopName = t->StreamOpTypeName();

         t->StreamOpTypeName(t->Scope(t->LocalName()));
         ColonToBar((char *)streamopName);
      }

      DDS_StdString scopedname = NoColons(t->EnclosingScope() + "_" + t->LocalName());
      t->TypeCodeTypeName(BE_Globals::TCPrefix + t->LocalName());
      t->TypeCodeBaseName(BE_Globals::TCBasePrefix + scopedname);
      t->TypeCodeRepName(BE_Globals::TCRepPrefix + scopedname);
      t->MetaTypeTypeName(BE_Globals::MTPrefix + scopedname);
   }
   else
   {
      assert (0);
   }
}

pbbool
be_sequence::IsFixedLength() const
{
   return pbfalse;
}

pbbool
be_sequence::IsFixedLengthPrimitiveType() const
{
   return pbfalse;
}

void
be_sequence::GenerateTypedefs(const DDS_StdString &scope,
                              const be_typedef& alias,
                              be_ClientHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);
   DDS_StdString relTypeName = BE_Globals::RelativeScope(scope, typeName);

   os << tab << "typedef " << relTypeName << " "
   << alias.LocalName() << ";" << nl;

   if (BE_Globals::isocpp_new_types)
   {
      /* no more */
   }
   else
   {
      os << tab << "typedef " << relTypeName << DDSVarExtension
      << " " << alias.LocalName() << DDSVarExtension << ";" << nl;
      os << tab << "typedef " << relTypeName << DDSOutExtension
      << " " << alias.LocalName() << DDSOutExtension << ";" << nl;
   }

   if (anonymous)
   {
      localName = alias.LocalName ();
      anonymous = pbfalse;
      InitializeTypeMap (this);
   }
}

DDS_StdString
be_sequence::Allocater(const DDS_StdString&) const
{
   return allocater;
}

DDS_StdString
be_sequence::Initializer(const DDS_StdString& arg, VarType vt) const
{
   DDS_StdString ret = arg;

   if (vt == VT_OutParam || vt == VT_Return)
   {
      ret = arg + " = 0";
   }

   return ret + ";";
}

DDS_StdString
be_sequence::Releaser(const DDS_StdString& arg) const
{
   return (DDS_StdString)"delete(" + arg + ");";
}

DDS_StdString
be_sequence::Assigner(const DDS_StdString&, const DDS_StdString&) const
{
   return assigner;
}

DDS_StdString be_sequence::Duplicater
(
   const DDS_StdString& arg,
   const DDS_StdString& val,
   const DDS_StdString & currentScope,
   const pbbool isConst
) const
{
   DDS_StdString relativeName = BE_Globals::RelativeScope(currentScope, typeName);
   DDS_StdString ret = arg + " = new " + relativeName + "(" + val + ");";
   return ret;
}

DDS_StdString
be_sequence::SyncStreamOut(const DDS_StdString& arg, const DDS_StdString& out, VarType vt) const
{
   DDS_StdString ret = out;

   if (!IsFixedLength() && (vt == VT_Return || vt == VT_OutParam))
   {
      ret += " << (" + ScopedName() + " *)" + arg + ";";
   }
   else
   {
      ret += " << " + arg + ";";
   }

   return ret;
}

DDS_StdString
be_sequence::NullReturnArg()
{
   DDS_StdString ret = (DDS_StdString)"(" + ScopedName() + "*)0";

   return ret;
}

void be_sequence::GeneratePutGetOps (be_ClientHeader& source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);
   be_predefined_type * prtype = be_predefined_type::_narrow (base_type ());

   //
   // don't generate StreamOps for types that are already in
   // the core
   //

   if ( /* !baseType->IsStringType() && */ // note remove this when replace strseq
      !(prtype &&
        ( (prtype->pt() == AST_PredefinedType::PT_octet) ||
          (prtype->pt() == AST_PredefinedType::PT_char) ||
          (prtype->pt() == AST_PredefinedType::PT_wchar) ||
          (prtype->pt() == AST_PredefinedType::PT_boolean) ) ) )
   {
      os << tab << DLLMACRO << "void " << m_tc_put_val << nl;
      os << tab << "(" << nl;
      tab.indent ();
      os << tab << "DDS::Codec::OutStream & os," << nl;
      os << tab << "const void * arg," << nl;
      os << tab << "DDS::ParameterMode mode" << nl;
      if (XBE_Ev::generate ())
      {
         os << tab << XBE_Ev::arg (XBE_ENV_ARGN, false) << nl;
      }
      tab.outdent ();
      os << tab << ");" << nl << nl;
      os << tab << DLLMACRO << "void " << m_tc_get_val << nl;
      os << tab << "(" << nl;
      tab.indent ();
      os << tab << "DDS::Codec::InStream & is," << nl;
      os << tab << "void * arg," << nl;
      os << tab << "DDS::ParameterMode mode" << nl;
      if (XBE_Ev::generate ())
      {
         os << tab << XBE_Ev::arg (XBE_ENV_ARGN, false) << nl;
      }
      tab.outdent (); //2
      os << tab << ");" << nl << nl;
   }
}

void be_sequence::GenerateStreamOps (be_ClientHeader& source)
{
   be_predefined_type * prtype = be_predefined_type::_narrow(base_type());

   //
   // don't generate StreamOps for types that are already in
   // the core
   //

   if ( /* !baseType->IsStringType() && */ // note remove this when replace strseq
      !(prtype &&
        ( (prtype->pt() == AST_PredefinedType::PT_octet) ||
          (prtype->pt() == AST_PredefinedType::PT_char) ||
          (prtype->pt() == AST_PredefinedType::PT_wchar) ||
          (prtype->pt() == AST_PredefinedType::PT_boolean) ) ) )
   {
      //
      // Unbounded String sequence stream operators are defined in the
      // core, as are unbounded octet sequences (PR 10175)
      //

      if ((!isStringSeq || IsBounded()) && (TypeName() != "octetSeq"))
      {
         //
         // generic code
         //
         be_Type::GenerateStreamOps(source);
      }
   }
}

void
be_sequence::init_type(const DDS_StdString& scope,
                       const DDS_StdString& name)
{
   enclosingScope = scope;
   localName = name;

   m_tc_ctor_val = NoColons((DDS_StdString) Scope(localName) + "_ctor");
   m_tc_dtor_val = NoColons((DDS_StdString) Scope(localName) + "_dtor");
   if (local() != I_TRUE)
   {
      m_tc_put_val = NoColons((DDS_StdString) Scope(localName) + "_put");
      m_tc_get_val = NoColons((DDS_StdString) Scope(localName) + "_get");
   }
   else
   {
      m_tc_put_val = (DDS_StdString)"0";
      m_tc_get_val = (DDS_StdString)"0";
   }
   m_tc_assign_val = NoColons((DDS_StdString)
                              Scope(localName) + "_copy");

   InitializeTypeMap(this);
}

DDS::Boolean be_sequence::SetName
   (const DDS_StdString& _scope, const DDS_StdString& _name)
{
   DDS::Boolean ret = FALSE;

   if (anonymous)
   {
      anonymous = FALSE;

      init_type (_scope, _name);

      ret = TRUE;
   }

   return ret;
}

void be_sequence::GenerateAuxTypes (be_ClientHeader & source)
{
   if (BE_Globals::isocpp_new_types)
     return;

   ostream & os = source.Stream();
   be_Tab tab(source);
   DDS_StdString baseName = BaseTypeName ();
   DDS_StdString seqName = localName;
   DDS_StdString varName = localName + "_var";
   DDS_StdString outName = localName + "_out";
   DDS_StdString templateBase = "DDS_DCPSSequence";
   pbbool wideString = pbfalse;

   if (isStringSeq)
   {
      AST_Type* astbt = be_typedef::_astBase (base_type ());
      be_string* sbt = be_string::_narrow (astbt);
      wideString = sbt->IsWide ();
   }

   if (isStringSeq && !IsBounded ())
   {
      templateBase = wideString ? "DDS_DCPSUWStrSeq" : "DDS_DCPSUStrSeq";
   }

   os << tab << "typedef " << templateBase << "_var < " << seqName << "> "
      << varName << ";" << nl;
   os << tab << "typedef " << templateBase << "_out < " << seqName << "> "
      << outName << ";" << nl;
}

void be_sequence::Generate (be_ClientHeader& source)
{
   DDS_StdString scopedName = typeName;

   if (BE_Globals::ignore_interfaces && IsInterfaceDependant ())
   {
      return;
   }

   if (Generated ())
   {
      return;
   }

   if (anonymous && !isPrimitiveSeq && !isStringSeq
       && (!baseType->IsSequenceType ()
           || generatedSequences.find (baseType->TypeName ())
           == generatedSequences.end ()))
   {
      be_root::DeferSequence (this);
      deferred = pbtrue;
      return;
   }

   if (generatedSequences.find (scopedName) == generatedSequences.end ())
   {
      Generated (pbtrue);
      generatedSequences[scopedName] = scopedName;

      if (anonymous)
      {
         baseType->GenerateFwdDecls (source);
      }

      GenerateSequence (source);
      GenerateAuxTypes (source);

      be_root::AddStreamOps (*this);
      be_root::AddAnyOps (*this);
      be_root::AddPutGetOps (*this);
      be_root::AddImplementations (*this);
      be_root::AddTypedef (*this);
      be_root::AddTypecode (*this);
   }
}

void be_sequence::GenerateType (be_ClientHeader& source)
{
   Generate (source);
}

void be_sequence::Generate (be_ClientImplementation& source)
{
   DDS_StdString scopedName = typeName;

   if (BE_Globals::ignore_interfaces && IsInterfaceDependant ())
   {
      return;
   }

   if ( ! ( anonymous &&
            !isPrimitiveSeq &&
            !isStringSeq &&
            !baseType->Generated() )
        && (generatedSequencesInstantiations.find(scopedName)
            == generatedSequencesInstantiations.end()))
   {
      GenerateSequence(source);
   }
}

void be_sequence::isAtModuleScope (bool is_at_module)
{
   be_CodeGenerator *generator;

   be_CodeGenerator::isAtModuleScope (is_at_module);

   if (baseType)
   {
      generator = (be_CodeGenerator*)
         baseType->narrow((long) & be_CodeGenerator::type_id);

      if (generator)
      {
         generator->isAtModuleScope (is_at_module);
      }
   }
}

bool be_sequence::isAtModuleScope () const
{
   return be_CodeGenerator::isAtModuleScope ();
}

be_sequence * be_sequence::_narrow (AST_Type * atype)
{
   be_sequence * ret = 0;

   if (atype)
   {
      ret = (be_sequence*)atype->narrow((long) & be_sequence::type_id);
   }

   return ret;
}

pbbool be_sequence::IsInterfaceDependant () const
{
   return baseType->IsInterfaceDependant ();
}

ostream & be_sequence::put_for_sequence
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & data,
   const DDS_StdString & index,
   unsigned long uid
)
{
   DDS_StdString put_val = BE_Globals::ScopeOf(ScopedName()) + "::"
      + TypeCodeTypeName() + "->put_val";

   os << tab << put_val << "(os, (void*)&"
      << data << "[" << index << "], DDS::PARAM_IN"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_sequence::get_for_sequence
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & data,
   const DDS_StdString & index,
   unsigned long uid
)
{
   DDS_StdString get_val = BE_Globals::ScopeOf(ScopedName()) + "::"
      + TypeCodeTypeName() + "->get_val";

   os << tab << get_val << "(is, (void*)&"
      << data << "[" << index << "], DDS::PARAM_OUT"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

void be_sequence::putter
(
   ostream& os,
   be_Tab& tab,
   const DDS_StdString& seqptr,   // pointer to a sequence
   unsigned long uid
)
{
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString length = (DDS_StdString)"len" + uids;
   DDS_StdString data = (DDS_StdString)"data" + uids;
   DDS_StdString index = (DDS_StdString)"i" + uids;

   // first, let's put the length

   os << tab << "DDS::ULong " << length << " = " << seqptr << "->length ();" << nl;
   os << tab << "DDS::Codec::Param lenPutArg = { DDS::_tc_ulong, &" << length
      << ", DDS::PARAM_IN };" << nl;
   os << tab << baseType->CppTypeWhenSequenceMember().ScopedName() << "* "
      << data << " = (" << baseType->CppTypeWhenSequenceMember().ScopedName()
      << "*)" << seqptr << "->get_buffer (FALSE);" << nl << nl;

   os << tab << "os.put (&lenPutArg, 1"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl << nl;

   os << tab << "for (DDS::ULong " << index << " = 0; " << index
      << " < " << length << "; " << index << "++)" << nl;
   os << tab << "{" << nl;
   tab.indent ();
   baseType->put_for_sequence (os, tab, data, index, uid);
   tab.outdent ();
   os << tab << "}" << nl;
}

void be_sequence::getter
(
   ostream& os,
   be_Tab& tab,
   const DDS_StdString& seqptr,   // sequence pointer
   unsigned long uid
)
{
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString length = (DDS_StdString)"len" + uids;
   DDS_StdString size = (DDS_StdString)"size" + uids;
   DDS_StdString data = (DDS_StdString)"data" + uids;
   DDS_StdString index = (DDS_StdString)"i" + uids;
   DDS_StdString maxArg;
   DDS_StdString swapArg = data;
   DDS_StdString incData = data + "++";
   DDS_StdString swapCall;

   if (!IsBounded ())
   {
      maxArg = length + ", ";
   }

   //
   // now, let's declare our get args
   //
   os << tab << "DDS::ULong " << length << ";" << nl;

   //
   // then get the length
   //
   os << tab << "DDS::Codec::Param lenGetArg = { DDS::_tc_ulong, &" << length
      << ", DDS::PARAM_OUT };" << nl;

   os << tab << baseType->CppTypeWhenSequenceMember().ScopedName()
      << "* " << data << ";" << nl << nl;

   os << tab << "is.get (&lenGetArg, 1"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   if (isInterfaceSeq)
   {
      os << tab << seqptr << "->length (" << length << ");" << nl;

      os << tab << data << " = " << seqptr << "->get_buffer (FALSE);" << nl << nl;
   }
   else
   {
      os << tab << data << " = " << ScopedName() << "::allocbuf ("
         << length << ");" << nl << nl;
   }

   os << tab << "for (DDS::ULong " << index << " = 0; "
      << index << " < " << length << "; " << index << "++)" << nl;

   os << tab << "{" << nl;

   tab.indent ();

   if (isInterfaceSeq)
   {
      os << tab << "if(" << data << "[" << index << "])" << nl;
      os << tab << "{" << nl;
      tab.indent ();
      os << tab << "DDS::release(" << data << "[" << index << "]);" << nl;
      os << tab << data << "[" << index << "] = 0;" << nl;
      tab.outdent ();
      os << tab << "}" << nl;
   }

   baseType->get_for_sequence (os, tab, data, index, uid);
   tab.outdent ();

   os << tab << "}" << nl << nl;

   if ( IsBounded() && !isInterfaceSeq )
   {
      os << tab << seqptr << "->replace (" << length << "," << data << ",TRUE);" << nl;
   }
   if ( !IsBounded() && !isInterfaceSeq )
   {
      os << tab << seqptr << "->replace (" << length << "," << length << "," << data << ",TRUE);" << nl;
   }
}

void be_sequence::generate_tc_ctor_val (be_Source & source)
{
   be_Type::generate_tc_ctor_val (source);
}

void be_sequence::generate_tc_dtor_val
(
   be_Source & source,
   pbbool isCounted
)
{
   be_Type::generate_tc_dtor_val (source, FALSE);
}

void be_sequence::generate_tc_put_val (be_Source & source)
{
   be_predefined_type * prtype = be_predefined_type::_narrow (base_type ());
   ostream & os = source.Stream ();
   be_Tab tab (source);

   // declare writer body

   os << tab << "void " << m_tc_put_val << nl;
   os << tab << "(" << nl;
   tab.indent ();
   os << tab << "DDS::Codec::OutStream & os," << nl;
   os << tab << "const void * arg," << nl;
   os << tab << "DDS::ParameterMode mode" << nl;
   if (XBE_Ev::generate ())
   {
      os << tab << XBE_Ev::arg (XBE_ENV_ARGN, false) << nl;
   }
   tab.outdent ();
   os << tab << ")" << nl;
   os << tab << "{" << nl;
   tab.indent ();

   /* Sequences of 1/2/4 byte types handled as special cases */

   if
   (
      prtype &&
      ! IsBounded () &&
      (
         (prtype->pt () == AST_PredefinedType::PT_octet) ||
         (prtype->pt () == AST_PredefinedType::PT_char) ||
         (prtype->pt () == AST_PredefinedType::PT_long) ||
         (prtype->pt () == AST_PredefinedType::PT_ulong) ||
         (prtype->pt () == AST_PredefinedType::PT_float) ||
         (prtype->pt () == AST_PredefinedType::PT_short) ||
         (prtype->pt () == AST_PredefinedType::PT_ushort)
      )
   )
   {
      os << tab << "DDS::Codec::Param p = { ";

      if
      (
         (prtype->pt () == AST_PredefinedType::PT_octet) ||
         (prtype->pt () == AST_PredefinedType::PT_char)
      )
      {
         os << "DDS::_tc_sequence";
      }
      else if
      (
         (prtype->pt () == AST_PredefinedType::PT_short) ||
         (prtype->pt () == AST_PredefinedType::PT_ushort)
      )
      {
         os << "DDS::_tc_sequence2";
      }
      else
      {
         os << "DDS::_tc_sequence4";
      }

      os << ", (void*) arg, mode };" << nl
         << tab << "os.put (&p, 1"
         << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   }
   else
   {
      // since the put param must be a sequence pointer pointer,
      // that's what the writer takes.  The putter, however,

      os << tab << ScopedName () << " * p = ("
         << ScopedName () << "*) arg;" << nl;

      // now put the data

      putter (os, tab, "p", 0);
   }

   tab.outdent ();
   os << tab << "}" << nl << nl;
}

void be_sequence::generate_tc_get_val (be_Source & source)
{
   be_predefined_type * prtype = be_predefined_type::_narrow (base_type ());
   ostream & os = source.Stream ();
   be_Tab tab (source);

   // declare reader body

   os << tab << "void " << m_tc_get_val << nl;
   os << tab << "(" << nl;
   tab.indent ();
   os << tab << "DDS::Codec::InStream & is," << nl;
   os << tab << "void * arg," << nl;
   os << tab << "DDS::ParameterMode mode" << nl;
   if (XBE_Ev::generate ())
   {
      os << tab << XBE_Ev::arg (XBE_ENV_ARGN, false) << nl;
   }
   tab.outdent ();
   os << tab << ")" << nl;
   os << tab << "{" << nl;
   tab.indent ();

   /* Sequences of 1/2/4 byte types handled as special cases */

   if
   (
      prtype &&
      ! IsBounded () &&
      (
         (prtype->pt () == AST_PredefinedType::PT_octet) ||
         (prtype->pt () == AST_PredefinedType::PT_char) ||
         (prtype->pt () == AST_PredefinedType::PT_long) ||
         (prtype->pt () == AST_PredefinedType::PT_ulong) ||
         (prtype->pt () == AST_PredefinedType::PT_float) ||
         (prtype->pt () == AST_PredefinedType::PT_short) ||
         (prtype->pt () == AST_PredefinedType::PT_ushort)
      )
   )
   {
      os << tab << "DDS::Codec::Param p = {";

      if
      (
         (prtype->pt () == AST_PredefinedType::PT_octet) ||
         (prtype->pt () == AST_PredefinedType::PT_char)
      )
      {
         os << "DDS::_tc_sequence";
      }
      else if
      (
         (prtype->pt () == AST_PredefinedType::PT_short) ||
         (prtype->pt () == AST_PredefinedType::PT_ushort)
      )
      {
         os << "DDS::_tc_sequence2";
      }
      else
      {
         os << "DDS::_tc_sequence4";
      }

      os << ", arg, mode };" << nl
         << tab << "is.get (&p, 1"
         << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   }
   else
   {
      os << tab << ScopedName () << " * p = ("
         << ScopedName () << "*) arg;" << nl;

      getter (os, tab, "p", 0);
   }
   tab.outdent ();
   os << tab << "}" << nl << nl;
}

void be_sequence::generate_tc_assign_val (be_Source & source)
{
   be_Type::generate_tc_assign_val (source);
}

unsigned long be_sequence::isRecursive ()
{
   UTL_Scope * scope = 0;
   be_Type * btype = 0;
   AST_Decl * adecl = (AST_Decl *)this->narrow((long) & AST_Decl::type_id);
   assert(adecl);
   be_Type * base = (be_Type *)base_type()->narrow((long) & be_Type::type_id);
   assert(base);
   unsigned long offset = 1;

   for (
      offset = 1;
      adecl &&
      (scope = adecl->defined_in()) &&
      (btype = (be_Type *)scope->narrow((long) & be_Type::type_id));
      ++offset
   )
   {
      adecl = (AST_Decl *)scope->narrow((long) & AST_Decl::type_id);

      if (btype->TypeCodeTypeName() == base->TypeCodeTypeName())
      {
         break;
      }
   }

   if (scope && btype)
   {
      return offset;
   }

   return 0;
}

DDS_StdString
be_sequence::kind_string()
{
   return "DDS::tk_sequence";
}

DDS::ULong
be_sequence::get_elem_size()
{
   return 0;
}

DDS::ULong
be_sequence::get_elem_alignment()
{
   return 4;
}

DDS::Boolean be_sequence::declare_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & stubScope,
   VarType vt
)
{
   DDS::Boolean ret = TRUE;
   DDS_StdString relTypeName = BE_Globals::RelativeScope(
                                  stubScope,
                                  typeName);

   switch (vt)
   {
      case VT_InParam:
      case VT_InOutParam:
      break;

      case VT_OutParam:
      {
         os << tab << arg << " = new " << relTypeName << ";" << nl;
      }

      break;

      case VT_Return:
      {
         os << tab << relTypeName << "_var " << arg
            << " = new " << relTypeName << ";" << nl;
      }

      break;

      default:   // VT_OutParam
      ret = FALSE;
   }

   return ret;
}

DDS::Boolean be_sequence::declare_for_struct_put
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   return TRUE;
}

DDS::Boolean be_sequence::declare_for_struct_get
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   return TRUE;
}

DDS::Boolean be_sequence::declare_for_union_put
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   return TRUE;
}

DDS::Boolean be_sequence::declare_for_union_get
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << sptr << "->" << fld << " = new " << ScopedName() << ";" << nl;

   return TRUE;
}

DDS::Boolean be_sequence::make_get_param_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   VarType vt
) const
{
   DDS::Boolean ret = FALSE;

   if (vt == VT_InOutParam || vt == VT_OutParam || vt == VT_Return)
   {
      os << tab << "{ " << Scope(TypeCodeTypeName()) << ", ";

      switch (vt)
      {
         case VT_InOutParam:
         os << "&" << arg << ", DDS::PARAM_OUT ";
         break;

         case VT_OutParam:
         os << arg << ".out(), DDS::PARAM_OUT ";
         break;

         case VT_Return:
         os << arg << ", DDS::PARAM_OUT ";
         break;

         default:
         assert (0);
      }

      os << "}";
      ret = TRUE;
   }

   return ret;
}

DDS::Boolean be_sequence::make_put_param_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   VarType vt
) const
{
   DDS::Boolean ret = FALSE;

   if (vt == VT_InParam || vt == VT_InOutParam)
   {
      os << tab << "{ " << Scope(TypeCodeTypeName()) << ", ";

      switch (vt)
      {
         case VT_InParam:
         case VT_InOutParam:
         os << "(" << TypeName() << "*)" << "&" << arg << ", DDS::PARAM_IN ";
         break;

         default:
         assert (0);
      }

      os << "}";
      ret = TRUE;
   }

   return ret;
}

DDS::Boolean be_sequence::make_put_param_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << "{ " << Scope(TypeCodeTypeName())
   << ", " << sptr << "->" << fld << ", DDS::PARAM_IN }";
   return TRUE;
}

DDS::Boolean be_sequence::make_get_param_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << "{ " << Scope(TypeCodeTypeName())
   << ", " << sptr << "->" << fld << ", DDS::PARAM_OUT }";
   return TRUE;
}

DDS::Boolean be_sequence::make_put_param_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   //
   // only called for primitive sequences
   //
   os << tab << "{ " << Scope (TypeCodeTypeName ())
      << ", &" << sptr << "->" << fld << ", DDS::PARAM_IN " << "}";

   return TRUE;
}

DDS::Boolean be_sequence::make_get_param_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   //
   // only called for primitive sequences
   //
   os << tab << "{ " << Scope (TypeCodeTypeName ())
      << ", &" << sptr << "->" << fld << ", DDS::PARAM_OUT " << "}";

   return TRUE;
}

DDS::Boolean
be_sequence::is_core_marshaled()
{
   DDS::Boolean ret = FALSE;

   if (isPrimitiveSeq)
   {
      ret = TRUE;
   }

   return ret;
}

ostream &
be_sequence::put_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   //
   // the struct has a sequence member, we'll need a pointer to it
   //
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString seqptr = (DDS_StdString)"_seqptr_" + uids;

   os << nl;
   os << " " << ScopedName() << " * " << seqptr << " = ("
      << ScopedName() << "*)&("
      << sptr << "->" << fld << ");" << nl;

   os << tab << m_tc_put_val << "(os, (void*)" << seqptr
      << ", DDS::PARAM_OUT" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream &
be_sequence::get_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   //
   // the struct has a sequence member, we'll need a pointer to it
   //
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString seqptr = (DDS_StdString)"seqptr" + uids;

   os << " " << ScopedName() << " * " << seqptr << " = &("
      << sptr << "->" << fld << ");" << nl;
   os << tab << m_tc_get_val << "(is, (void*&)" << seqptr
      << ", DDS::PARAM_OUT" <<  XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream &
be_sequence::put_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   //
   // the union has a void pointer member, we'll need a sequence pointer to it
   //
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString seqptr = (DDS_StdString)"_seqptr_" + uids;

   os << nl;
   os << " " << ScopedName() << " * " << seqptr << " = ("
      << ScopedName() << "*)("
      << sptr << "->" << fld << ");" << nl;

   os << tab << m_tc_put_val << "(os, (void*)" << seqptr
      << ", DDS::PARAM_OUT" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream &
be_sequence::get_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   //
   // the struct has a sequence member, we'll need a pointer to it
   //
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString seqptr = (DDS_StdString)"seqptr" + uids;

   os << " " << ScopedName() << " * " << seqptr << " = ("
      << ScopedName() << "*)"
      << sptr << "->" << fld << ";" << nl;

   os << tab << m_tc_get_val << "(is, (void*&)" << seqptr
      << ", DDS::PARAM_OUT" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_sequence::put_for_array
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   return put_for_sequence (os, tab, arg, index, uid);
}

ostream & be_sequence::get_for_array
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   return get_for_sequence (os, tab, arg, index, uid);
}

be_DispatchableType::en_HowStoredInDispatcher be_sequence::HowStoredInDispatcher
   (const be_ArgumentDirection& direction) const
{
   switch (direction)
   {
      case VT_InParam:
      case VT_InOutParam:
      return STORED_AS_STACK_VARIABLE;

      case VT_OutParam:
      case VT_Return:
      return STORED_IN_VAR;

      default:
      break;
   }

   return STORED_AS_STACK_VARIABLE;
}

DDS_StdString
be_sequence::UnionStreamOut(const DDS_StdString& arg, const DDS_StdString& out) const
{
   return out + " << (" + UnionMemberTypeName() + "*)" + arg + ";";
}

DDS_StdString
be_sequence::UnionStreamIn(const DDS_StdString& arg, const DDS_StdString& in) const
{
   return in + " >> (" + UnionMemberTypeName() + "*&)" + arg + ";";
}

void be_sequence::GenerateSequence (be_ClientHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);
   DDS_StdString elemName;
   pbbool wideString = pbfalse;

   if (isInterfaceSeq || isValueSeq)
   {
      elemName = BE_Globals::RelativeScope(enclosingScope, baseType->ScopedName());
   }
   else
   {
      elemName = BE_Globals::RelativeScope(enclosingScope, baseTypeName);

      if (isStringSeq)
      {
         AST_Type* astbt = be_typedef::_astBase(base_type());
         be_string* sbt = be_string::_narrow(astbt);
         wideString = sbt->IsWide();
      }
   }

   DDS_StdString FileClassname;
   DDS_StdString charFileClassname;
   DDS_StdString boolFileClassname;

   FileClassname = elemName;
   charFileClassname = DDS_StdString("DDSChar");
   boolFileClassname = DDS_StdString("DDSBoolean");

   if (BE_Globals::isocpp_new_types)
   {
      if (isStringSeq)
      {
          elemName = "::std::string";
      }
      os << tab << "typedef ::std::vector < " << elemName << "> " << localName << ";" << nl;
      return;
   }

   os << tab << "struct " << localName << "_uniq_ {};" << nl;

   if (IsBounded())
   {
      // Bounded sequence types

      if (baseType->IsFixedLength())
      {
         be_predefined_type * pdt = be_predefined_type::_narrow(base_type());

         os << tab << "typedef DDS_DCPSBFLSeq < " << elemName
            << ", " << FileClassname << ", " << maxSize << "> " << localName << ";" << nl;
      }
      else if (isStringSeq)
      {
         if (wideString)
         {
            os << tab << "typedef DDS_DCPSBWStrSeq < " << maxSize << "> "
            << localName << ";" << nl;
         }
         else
         {
            os << tab << "typedef DDS_DCPSBStrSeq < " << maxSize << "> "
            << localName << ";" << nl;
         }
      }
      else if (isInterfaceSeq)
      {
         os << tab << "typedef DDS_DCPSBObjSeq < " << elemName << ", struct "
            << localName << "_uniq_, " << maxSize << "> " << localName << ";"
            << nl;
      }
      else if (isValueSeq)
      {
         os << tab << "typedef DDS_DCPSBValSeq < " << elemName
         << ", " << maxSize << "> " << localName << ";" << nl;
      }
      else
      {
         os << tab << "typedef DDS_DCPSBVLSeq < " << elemName
         << ", " << maxSize << "> " << localName << ";" << nl;
      }
   }
   else
   {
      // Unbounded sequence types

      if (baseType->IsFixedLength ())
      {
         os << tab << "typedef DDS_DCPSUFLSeq < " << elemName << ", struct "
            << localName << "_uniq_> " << localName << ";" << nl;
      }
      else if (isStringSeq)
      {
         if (wideString)
         {
            os << tab << "typedef DDS_DCPSUWStrSeqT <struct " << localName
               << "_uniq_> " << localName << ";" << nl;
         }
         else
         {
            os << tab << "typedef DDS_DCPSUStrSeqT <struct " << localName
               << "_uniq_> " << localName << ";" << nl;
         }
      }
      else if (isInterfaceSeq)
      {
         os << tab << "typedef DDS_DCPSUObjSeq < " << elemName << ", struct "
            << localName << "_uniq_> " << localName << ";" << nl;
      }
      else if (isValueSeq)
      {
         os << tab << "typedef DDS_DCPSUValSeq < " << elemName
            << "> " << localName << ";" << nl;
      }
      else
      {
        if (BE_Globals::isocpp_new_types)
        {
          os << tab << "typedef ::std::vector <" << elemName << "> " << localName << ";" << nl;
        }
        else
        {
          os << tab << "typedef DDS_DCPSUVLSeq < " << elemName << ", struct "
            << localName << "_uniq_> " << localName << ";" << nl;
        }
      }
   }
}

void be_sequence::GenerateSequence (be_ClientImplementation & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);
   DDS_StdString elemName;

   elemName = baseType->ScopedName();

   DDS_StdString FileClassname;
   DDS_StdString charFileClassname;
   DDS_StdString boolFileClassname;

   FileClassname = elemName;
   charFileClassname = DDS_StdString("DDSChar");
   boolFileClassname = DDS_StdString("DDSBoolean");

   os << "#if DDS_USE_EXPLICIT_TEMPLATES" << nl;

   if (IsBounded())
   {
      // Bounded sequence types

      if (baseType->IsFixedLength())
      {
         be_predefined_type * pdt = be_predefined_type::_narrow(base_type());

         os << tab << "template class DDS_DCPSBFLSeq < " << elemName
            << ", " << FileClassname << ", " << maxSize << ">;" << nl;
      }
      else if (isInterfaceSeq)
      {
         os << tab << "template class DDS_DCPSBObjSeq < " << elemName << ", struct "
            << localName << "_uniq_, " << maxSize << ">;" << nl;
      }
      else if (isValueSeq)
      {
         os << tab << "template class DDS_DCPSBValSeq < " << elemName
         << ", " << maxSize << ">;" << nl;
      }
      else
      {
         os << tab << "template class DDS_DCPSBVLSeq < " << elemName
         << ", " << maxSize << ">;" << nl;
      }
   }
   else
   {
      // Unbounded sequence types

      if (baseType->IsFixedLength ())
      {
         os << tab << "template class DDS_DCPSUFLSeq < " << elemName
            << ", struct " << localName << "_uniq_>;" << nl;
      }
      else if (isInterfaceSeq)
      {
         os << tab << "template class DDS_DCPSUObjSeq < " << elemName
            << ", struct " << localName << "_uniq_>;" << nl;
      }
      else if (isValueSeq)
      {
         os << tab << "template class DDS_DCPSUValSeq < " << elemName
            << ">;" << nl;
      }
      else
      {
        if (BE_Globals::isocpp_new_types)
        {
          os << tab << "template class ::std::vector <" << elemName << ">" << nl;
        }
        else
        {
          os << tab << "template class DDS_DCPSUVLSeq < " << elemName << ", struct "
            << localName << "_uniq_>;" << nl;
        }
      }
   }

   os << "#endif" << nl;
}

void be_sequence::GenerateGlobalTypedef (be_ClientHeader &source)
{
}
