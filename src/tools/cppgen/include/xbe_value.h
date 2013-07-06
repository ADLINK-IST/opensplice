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
#ifndef _XBE_VALUE_HH
#define _XBE_VALUE_HH

#include "xbe_interface.h"

class be_typedef;

class be_value
:
   public AST_Value,
   public be_interface
{

   public:

      typedef DDS::Boolean Boolean;
      typedef be_OpNameSet::Be_OpMap Be_OpMap;

      be_value();

      be_value
      (
         idl_bool abstract,
         idl_bool custom,
         idl_bool truncatable,
         UTL_ScopedName *n,
         AST_Value **ih,
         long nih,
         AST_Interface **supports,
         long nsupports,
         const UTL_Pragmas &p
      );

      ~be_value ();

      const DDS_StdString & BaseClassname();

      static be_value * _narrow(AST_Type * atype);

      static void GenerateRefAndVar
      (
         be_ClientHeader & source,
         const DDS_StdString & scope,
         const DDS_StdString & localName,
         pbbool needsForwardDecl
      );

      // BE_TYPE VIRTUALS
      virtual inline void Initialize()
      {}

      virtual Boolean IsPrimitiveType() const
      {
         return pbfalse;
      }

      virtual Boolean IsStructuredType() const
      {
         return pbfalse;
      }

      virtual Boolean IsArrayType() const
      {
         return pbfalse;
      }

      virtual Boolean IsStringType() const
      {
         return pbfalse;
      }

      virtual Boolean IsSequenceType() const
      {
         return pbfalse;
      }

      virtual Boolean IsInterfaceType() const
      {
         return pbfalse;
      }

      virtual pbbool IsReturnedByVar () const
      {
         return pbtrue;
      }

      virtual Boolean IsValueType() const
      {
         return pbtrue;
      }

      // BE_CODEGENERATOR VIRTUALS
      virtual void Generate(be_ClientHeader& clientHeader);
      virtual void Generate(be_ClientImplementation& clientImpl);
      virtual void Generate(be_ServerHeader& serverHeader);
      virtual void Generate(be_ServerImplementation& serverImpl);

      // AST_VALUE VIRTUALS
      virtual AST_StateMember *add_state_member(AST_StateMember *m);
      virtual AST_Initializer *add_initializer(AST_Initializer *i);

      DEF_NARROW_METHODS5(be_value, be_interface, AST_Value, AST_Interface,
                          be_CodeGenerator, be_Type);
      DEF_NARROW_FROM_DECL(be_value);
      DEF_NARROW_FROM_SCOPE(be_value);

   private:

      DDS_StdString baseClassname;
      const be_CppEnclosingScope m_cppScope; // scope enclosing this interface
      const be_CppEnclosingScope m_cppType;  // the scope that this interface is

      void GenerateStateMFs(be_Source& source, const DDS_StdString& implclassname, idl_bool public_access);
      void GenerateStaticMFs(be_ClientHeader& source);
      void GenerateDefaultConstructor(be_ClientHeader& source);
      void GenerateDestructor(be_ClientHeader& source);
      void GenerateCopyConstructor(be_ClientHeader& source);
      void GenerateAssignmentOperator(be_ClientHeader& source);
      void GenerateStaticMFs(be_ClientImplementation& source);
      void GenerateDefaultConstructor(be_ClientImplementation& source);
      void GenerateDestructor(be_ClientImplementation& source);
      void GenerateTypedefs
      (
         const DDS_StdString &scope,
         const be_typedef& alias,
         be_ClientHeader& source
      );

   protected:

      virtual void InitializeTypeMap(be_Type*);
};

class be_value_fwd: public AST_ValueFwd, public be_CodeGenerator
{
   public:
      be_value_fwd ();

      be_value_fwd
      (
         idl_bool abstract,
         UTL_ScopedName *n, 
         const UTL_Pragmas &p
      );

      virtual ~be_value_fwd ();

      virtual void Generate (be_ClientHeader& source);
      virtual void Generate(be_ClientImplementation& clientImpl);
      virtual void Generate(be_ServerHeader& serverHeader);
      virtual void Generate(be_ServerImplementation& serverImpl);

      DDS_StdString Scope(const DDS_StdString& name);

      static DDS::Boolean is_Generated(const DDS_StdString& declKey);
      static void Generated(const DDS_StdString& declKey);

      DEF_NARROW_METHODS2(be_value_fwd, AST_ValueFwd, be_CodeGenerator);
      DEF_NARROW_FROM_DECL(be_value_fwd);

   private:
      static String_map generated;
};

class be_state_member: public AST_StateMember
{
   public:
      be_state_member ();
      be_state_member
      (
         idl_bool public_access,
         AST_Type *ft,
         UTL_ScopedName *n,
         const UTL_Pragmas &p
      );
      virtual ~be_state_member ();

      void Initialize(be_value* owner);

      void GenerateAGetAccessor
      (
         be_Source& source, 
         const DDS_StdString& getReturn,
         pbbool isConst
      );

      void GenerateGetAccessor(be_Source& source);

      void GenerateASetAccessor
      (
         be_Source& source,
         const DDS_StdString& argType,
         const pbbool
      );

      void GenerateSetAccessor(be_Source& source);

      // Narrowing
      DEF_NARROW_METHODS1(be_state_member, AST_StateMember);
      DEF_NARROW_FROM_DECL(be_state_member);

   private:
      be_DispatchableType* fieldType;
};

class be_initializer: public AST_Initializer
{
   public:
      be_initializer ();

      be_initializer
      (
         UTL_ScopedName *n,
         const UTL_Pragmas &p
      );

      virtual ~be_initializer ();

      void Initialize(be_value* owner);

      DEF_NARROW_METHODS1(be_initializer, AST_Initializer);
      DEF_NARROW_FROM_DECL(be_initializer);
};

class be_boxed_valuetype: public AST_BoxedValue
{
   public:
      be_boxed_valuetype ();

      be_boxed_valuetype
      (
         UTL_ScopedName *n,
         AST_Type *t,
         const UTL_Pragmas &p
      );

      virtual ~be_boxed_valuetype ();

      DEF_NARROW_METHODS1(be_boxed_valuetype, AST_BoxedValue);
      DEF_NARROW_FROM_DECL(be_boxed_valuetype);
};

#endif
