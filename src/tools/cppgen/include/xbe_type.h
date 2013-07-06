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
#ifndef _XBE_TYPE_HH
#define _XBE_TYPE_HH

#include "idl.h"
#include "ast.h"
#include "xbe_globals.h"
#include "xbe_source.h"
#include "xbe_cpptype.h"
#include "prototc.h"
#include "StdString.h"

enum VarType 
{
   VT_InParam,
   VT_OutParam,
   VT_InOutParam,
   VT_Return,
   VT_Attribute,
   VT_Max
};
VarType MakeVarType (AST_Argument::Direction dir);

extern const pbbool VT_Const;
extern const pbbool VT_NonConst;
extern const pbbool VT_Reference;
extern const pbbool VT_NonReference;
extern const int VT_Var;
extern const int VT_Pointer;

struct be_VarSignature
{
   DDS_StdString _varTypeName;
   pbbool _isConst;
   int _levelsOfIndirection;
   pbbool _isReference;
};

class be_Type;

class be_typedef;

// UGLINESS WARNING / EXPLANATION
//
// be_TypeMap is duct tape.  It allows objects created by the front end
// to be narrow'ed to be_Type, and still provide access to data that's
// specific to be_Type's descendants.  be_Type is a descendant of
// be_TypeMap, so every descendant of be_Type is guaranteed to offer all
// of be_TypeMap's pseudo-derived methods.  The way this pseudo-derivation
// works is each descendant of be_TypeMap contains an InitializeTypeMap()
// method, which fills in all of the variables in a way appropriate for
// the class.
//
// The reason that seemed like a good idea at some time is because when
// the front end passes the back end an object that represents an IDL type,
// the object is of class AST_Type.  We can't call back-end methods on
// objects declared as AST_Type.  For example, the notion of which C++ class
// represents a certain IDL type when it's stored inside a sequence is
// foreign to the front end.  Consequently, we can't ask an object of AST_Type
// to tell us the name of that C++ class.  The solution is that the
// constructors of descendants of be_Type explicitly narrow() each AST_Type
// pointer to be_Type.
//
// be_TypeMap has turned out to be a source of some nasty bugs.  Descendants of
// be_Type don't distinguish between IDL types and C++ types; they sort of
// represent both, using the same strings.  When you fix a bug involving
// one, you break stuff involving the other.  The confusion between IDL
// types and C++ types has made IDLC virtually unmaintainable.  There are
// casts all throughout the code.  That's because when you override a
// be_Type virtual function in a derived class, it won't get called unless
// you cast the be_Type* to the derived class!  If you remove the casts,
// you'll get tricky SEGVs in unexpected places.
//
// The long-term solution, called the NEW WORLD ORDER in comments, is to
// (of course) distinguish between IDL types and C++ types.  Code-generation
// classes should represent C++ code to generate, and should not inherit
// from front-end classes.  Back-end classes, in other words, should
// represent the structure of the generated C++ code, just as front-end
// classes represent the IDL source file.  As you refactor, look for
// opportunities to make things like be_CppName, be_CppType, and be_Dispatcher:
// classes that *solely* represent C++ code.
//
// The short-term solution is more duct tape: be_TypeMap now contains a
// member called idlType, which contains the "this" pointer of the actual
// be_Type descendant.  Now you can add virtual functions to be_Type,
// override them in derived classes, and they will be called, as long as
// the caller goes through idlType.

class be_TypeMap
{

public:

   // Duct tape to get us to the new world order (see big comment above)

   be_Type* idlType; // the actual be_Type descendant that we contain

   virtual void InitializeTypeMap (be_Type*) = 0;

   inline const DDS_StdString & TypeName () const
   {
      return typeName;
   }

   inline const DDS_StdString & InTypeName () const
   {
      return inTypeName;
   }

   inline const DDS_StdString & InOutTypeName () const
   {
      return inoutTypeName;
   }

   inline const DDS_StdString&
   OutTypeName() const
   {
      return outTypeName;
   }

   inline const DDS_StdString&
   ReturnTypeName() const
   {
      return returnTypeName;
   }

   inline const DDS_StdString&
   DMFAdtMemberTypeName() const
   {
      return dmfAdtMemberTypeName;
   }

   inline const DDS_StdString&
   StructMemberTypeName() const
   {
      return structMemberTypeName;
   }

   inline const DDS_StdString&
   UnionMemberTypeName() const
   {
      return unionMemberTypeName;
   }

   inline const DDS_StdString&
   SequenceMemberTypeName() const
   {
      return sequenceMemberTypeName;
   }

   inline const DDS_StdString&
   StreamOpTypeName() const
   {
      return streamOpTypeName;
   }

   inline const DDS_StdString&
   IStreamOpTypeName() const
   {
      return istreamOpTypeName;
   }

   inline const DDS_StdString&
   TypeCodeTypeName() const
   {
      return typeCodeTypeName;
   }

   inline const DDS_StdString&
   TypeCodeBaseName() const
   {
      return tcbaseName;
   }

   inline const DDS_StdString&
   TypeCodeRepName() const
   {
      return tcrepName;
   }

   inline const DDS_StdString&
   MetaTypeTypeName() const
   {
      return metaTypeTypeName;
   }

   // Set functions

   inline void
   TypeName(const DDS_StdString& _typename)
   {
      typeName = _typename;
   }

   inline void
   InTypeName(const DDS_StdString& intypename)
   {
      inTypeName = intypename;
   }

   inline void
   InOutTypeName(const DDS_StdString& inouttypename)
   {
      inoutTypeName = inouttypename;
   }

   inline void
   OutTypeName(const DDS_StdString& outtypename)
   {
      outTypeName = outtypename;
   }

   inline void
   ReturnTypeName(const DDS_StdString& returntypename)
   {
      returnTypeName = returntypename;
   }

   inline void
   DMFAdtMemberTypeName(const DDS_StdString& dmfadtmembertypename)
   {
      dmfAdtMemberTypeName = dmfadtmembertypename;
   }

   inline void
   StructMemberTypeName(const DDS_StdString& structmembertypename)
   {
      structMemberTypeName = structmembertypename;
   }

   inline void
   UnionMemberTypeName(const DDS_StdString& unionmembertypename)
   {
      unionMemberTypeName = unionmembertypename;
   }

   inline void
   SequenceMemberTypeName(const DDS_StdString& sequencemembertypename)
   {
      sequenceMemberTypeName = sequencemembertypename;
   }

   inline void
   StreamOpTypeName(const DDS_StdString& streamoptypename)
   {
      streamOpTypeName = streamoptypename;
   }

   inline void
   IStreamOpTypeName(const DDS_StdString& istreamoptypename)
   {
      istreamOpTypeName = istreamoptypename;
   }

   inline void
   TypeCodeTypeName(const DDS_StdString& typecodetypename)
   {
      typeCodeTypeName = typecodetypename;
   }

   inline void
   TypeCodeBaseName(const DDS_StdString& typecodetypename)
   {
      tcbaseName = typecodetypename;
   }

   inline void
   TypeCodeRepName(const DDS_StdString& typecodetypename)
   {
      tcrepName = typecodetypename;
   }

   inline void
   MetaTypeTypeName(const DDS_StdString& metatypetypename)
   {
      metaTypeTypeName = metatypetypename;
   }

   void VarSignature(VarType vt, const DDS_StdString& argTypeName, pbbool isConst,
                     int levelsOfIndirection, pbbool isReference);
   void VarSignature(VarType vt, const be_VarSignature& signature);

   const be_VarSignature& VarSignature(VarType vt) const;
   DDS_StdString VarTypeName(VarType vt) const;
   pbbool IsReference(VarType vt) const;
   int LevelsOfIndirection(VarType vt) const;
   pbbool IsConst(VarType vt) const;
   DDS_StdString MakeSignature(VarType vt, const DDS_StdString& className = NilString) const;

   //BE_TYPE_MAP VIRTUAL
   virtual pbbool IsFixedLengthPrimitiveType() const = 0;
 
   // BE_TYPE_MAP PURE VIRTUAL
   virtual pbbool IsFixedLength() const = 0;

   virtual ~be_TypeMap()
   {
      if (!typeName.length() || !inTypeName.length() || !inoutTypeName.length()
            || !outTypeName.length() || !returnTypeName.length()
                                               || !dmfAdtMemberTypeName.length() || !structMemberTypeName.length()
                                               || !sequenceMemberTypeName.length())
      {
         cerr << "Type map for " << (const char*)typeName << " not complete" << endl;
      }
   }

   DEF_NARROW_METHODS0(be_TypeMap);

protected:

   friend class be_Type;

   DDS_StdString typeName;
   DDS_StdString inTypeName;
   DDS_StdString inoutTypeName;
   DDS_StdString outTypeName;
   DDS_StdString returnTypeName;
   DDS_StdString dmfAdtMemberTypeName;
   DDS_StdString structMemberTypeName;
   DDS_StdString unionMemberTypeName;
   DDS_StdString sequenceMemberTypeName;
   DDS_StdString istreamOpTypeName;
   DDS_StdString streamOpTypeName;
   DDS_StdString typeCodeTypeName;
   DDS_StdString metaTypeTypeName;
   DDS_StdString tcrepName;
   DDS_StdString tcbaseName;

   be_VarSignature _signatures[VT_Max];
};


class be_Type : public be_TypeMap
{

public:

   // PUBLIC METHODS FOR THE NEW WORLD ORDER

   virtual be_CppType CppTypeWhenSequenceMember() const
   {
      return be_CppType(TypeName());
   }

   // ALL THE REST IS OLD WORLD ORDER

   inline pbbool Generated () const
   {
      return generated;
   }

   inline void Generated (pbbool val)
   {
      generated = val;
   }

   inline const DDS_StdString & EnclosingScope () const
   {
      return enclosingScope;
   }

   inline const DDS_StdString & LocalName () const
   {
      return localName;
   }

   inline void HasTypeDef (pbbool val)
   {
      _hasTypeDef = val;
   }

   inline pbbool HasTypeDef ()
   {
      return _hasTypeDef;
   }

   pbbool IsObjectType ();
   pbbool IsLocalObjectType ();
   pbbool IsTypeCodeType ();
   pbbool IsPseudoObject ();

   DDS_StdString TypedefName ();

   DDS_StdString Scope (const DDS_StdString& name) const;
   DDS_StdString ScopedName () const;

   static DDS_StdString EnclosingScopeString(AST_Decl* type);
   static be_Type * _narrow(AST_Type * type);

   // BE_TYPE VIRTUALS

   virtual void Initialize() = 0;
   virtual pbbool IsPrimitiveType() const = 0;
   virtual pbbool IsEnumeratedType() const
   {
      return pbfalse;
   }

   virtual pbbool IsStructuredType() const = 0;
   virtual pbbool IsStringType() const = 0;
   virtual pbbool IsArrayType() const = 0;
   virtual pbbool IsSequenceType() const = 0;
   virtual pbbool IsInterfaceType() const = 0;
   virtual pbbool IsReturnedByVar () const = 0;
   virtual pbbool IsExceptionType() const
   {
      return pbfalse;
   }

   virtual pbbool IsOpaqueType() const
   {
      return pbfalse;
   }

   virtual DDS_StdString ConstCastAlias()
   {
      return "";
   }

   virtual pbbool IsInterfaceDependant () const
   {
      return pbfalse;
   }

   virtual pbbool IsValueType() const
   {
      return pbfalse;
   }

   virtual DDS_StdString Allocater(const DDS_StdString&) const = 0;
   virtual DDS_StdString Initializer(const DDS_StdString&, VarType) const = 0;
   virtual DDS_StdString InRequestArgumentDeclaration(
      be_Type& btype,
      const DDS_StdString&,
      VarType vt);
   virtual DDS::Boolean is_core_marshaled() = 0;
   virtual DDS::Boolean declare_for_stub
   (
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & arg,
      const DDS_StdString & stubScope,
      VarType vt
   );
   virtual DDS::Boolean declare_for_struct_put(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual DDS::Boolean declare_for_struct_get(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual DDS::Boolean declare_for_union_put(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual DDS::Boolean declare_for_union_get(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual DDS::Boolean make_get_param_for_stub(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & argname,
      VarType vt) const;
   virtual DDS::Boolean make_put_param_for_stub(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & argname,
      VarType vt) const;
   virtual DDS::Boolean make_put_param_for_union(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual DDS::Boolean make_get_param_for_union(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual DDS::Boolean make_put_param_for_struct(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual DDS::Boolean make_get_param_for_struct(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual ostream & put_for_struct(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual ostream & get_for_struct(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual ostream & put_for_union(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid) = 0;
   virtual ostream & get_for_union(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid) = 0;
   virtual ostream & put_for_sequence(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & arg,
      const DDS_StdString & index,
      unsigned long uid);
   virtual ostream & get_for_sequence(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & arg,
      const DDS_StdString & index,
      unsigned long uid);
   virtual ostream & put_for_array(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & arg,
      const DDS_StdString & index,
      unsigned long uid);
   virtual ostream & get_for_array(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & arg,
      const DDS_StdString & index,
      unsigned long uid);
   virtual void generate_tc_ctor_val(
      be_Source & source);
   virtual void generate_tc_dtor_val
   (
       be_Source & source,
       pbbool isCounted
   );
   virtual void generate_tc_put_val(
      be_Source & source);
   virtual void generate_tc_get_val(
      be_Source & source);
   virtual void generate_tc_assign_val(
      be_Source & source);
   virtual DDS_StdString Releaser(const DDS_StdString&) const = 0;
   virtual DDS_StdString Assigner(
      const DDS_StdString&,
      const DDS_StdString&) const = 0;
   virtual DDS_StdString Duplicater(
      const DDS_StdString&,
      const DDS_StdString&,
      const DDS_StdString&,
      const pbbool) const = 0;
   virtual DDS_StdString SyncStreamIn(
      const DDS_StdString& arg,
      const DDS_StdString& out, VarType vt) const;
   virtual DDS_StdString SyncStreamOut(
      const DDS_StdString& arg ,
      const DDS_StdString& out, VarType vt) const;
   virtual DDS_StdString StructStreamOut(
      const DDS_StdString& arg,
      const DDS_StdString& out) const;
   virtual DDS_StdString StructStreamIn(
      const DDS_StdString& arg,
      const DDS_StdString& in) const;
   virtual DDS_StdString UnionStreamOut(
      const DDS_StdString& arg,
      const DDS_StdString& out) const;
   virtual DDS_StdString UnionStreamIn(
      const DDS_StdString& arg,
      const DDS_StdString& in) const;
   virtual DDS_StdString NullReturnArg() = 0;
   virtual void GenerateType(be_ClientHeader& source) = 0;
   virtual void GenerateTypedefs(
      const DDS_StdString &scope,
      const be_typedef & alias,
      be_ClientHeader& source) = 0;
   virtual void GeneratePutGetOps(be_ClientHeader& source);
   virtual void GenerateStreamOps(be_ClientHeader&);

   virtual void GenerateGlobalTypedef(be_ClientHeader &);
   virtual void GenerateFwdDecls(be_ClientHeader &)
   {}

   virtual DDS_StdString kind_string() = 0;
   virtual DDS::ULong get_elem_size();
   virtual DDS::ULong get_elem_alignment();
   virtual DDS::ULong get_OS_elem_alignment();

   virtual const DDS_StdString&
   any_op_id();

   virtual pbbool IsOptimizable ()
   {
      return IsFixedLength() && (get_elem_size() > 0);
   }

   DEF_NARROW_METHODS1(be_Type, be_TypeMap);

public:

   ProtoTypeCode* m_typecode;

   virtual void FinishProtoTypeCode();

protected:

   be_Type();

protected:

   DDS_StdString enclosingScope;
   DDS_StdString localName;
   DDS_StdString m_tc_ctor_val;
   DDS_StdString m_tc_dtor_val;
   DDS_StdString m_tc_put_val;
   DDS_StdString m_tc_get_val;
   DDS_StdString m_tc_assign_val;
   DDS_StdString m_any_op_id;

private:

   pbbool generated;
   pbbool _hasTypeDef;
};

#endif
