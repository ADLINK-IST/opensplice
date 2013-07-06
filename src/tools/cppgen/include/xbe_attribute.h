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
#ifndef _XBE_ATTRIBUTE_HH
#define _XBE_ATTRIBUTE_HH

#include "xbe_codegen.h"
#include "xbe_type.h"
#include "xbe_classgen.h"
#include "xbe_interface.h"

class be_attribute :
   public virtual AST_Attribute,
   public be_CodeGenerator
{

public:

   enum AT_SignatureType {
      AT_Declaration,     // signature in function declaration
      AT_Implementation,    // signature in beginning of definition
      // of attribute function
      AT_Invoke,      // signature in statement that calls
      // attribute function
      AT_TieImplementation,  // signature of beginning of definition
      // inside a tie template
      AT_TieInvoke    // signature of statement inside a tie
      // template that calls the tied function
   };

public:

   static be_attribute * _narrow(AST_Decl * decl);

   be_attribute ();
   be_attribute
   (
      idl_bool ro,
      AST_Type *ft,
      UTL_ScopedName *n,
      const UTL_Pragmas &p
   );

   DDS_StdString Scope(const DDS_StdString& name);
   DDS_StdString LocalName();
   void Initialize(be_interface* owner, const DDS_StdString& className);
   DDS_StdString StubClassname();
   const DDS_StdString& InterfaceBasename();

   //APV
   DDS_StdString GetSignature(
      AT_SignatureType sigType,
      const DDS_StdString& className,
      DDS::Boolean pureVirtual = pbtrue);
   //APV
   DDS_StdString SetSignature(
      AT_SignatureType sigType,
      const DDS_StdString& className,
      DDS::Boolean pureVirtual,
      const DDS_StdString& argName = NilString);
   void GenerateImpureRequestCall(be_ClientImplementation&);
   void GenerateGetDispatcher(be_ServerImplementation& source,
      const DDS_StdString& implbasename);
   void GenerateSetDispatcher(be_ServerImplementation& source,
      const DDS_StdString& implbasename);
   void GenerateVirtual(be_Source& source, const DDS_StdString& className);
   void GenerateGetStub(be_ClientImplementation& source);
   void GenerateSetStub(be_ClientImplementation& source);

   // BE_CODEGENERATOR VIRTUALS
   virtual void Generate(be_ClientHeader& source);
   virtual void Generate(be_ClientImplementation& source);
   virtual void Generate(be_ServerHeader&)
   {}

   virtual void Generate(be_ServerImplementation& dispatchImpl);

   DEF_NARROW_METHODS2(be_attribute, AST_Attribute, be_CodeGenerator);
   DEF_NARROW_FROM_DECL(be_attribute);

private:

   friend class be_interface;

   DDS_StdString interfaceBasename;
   DDS_StdString enclosingScope;
   DDS_StdString setOpKey;
   DDS_StdString getOpKey;
   be_DispatchableType* fieldType;
   DDS::Boolean m_getDispatchDone;
   DDS::Boolean m_setDispatchDone;
};

inline DDS_StdString
be_attribute::Scope(const DDS_StdString& name)
{
   DDS_StdString ret = enclosingScope;

   if (ret.length())
   {
      ret += "::";
   }

   return ret + name;
}

#endif
