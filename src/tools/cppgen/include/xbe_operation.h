/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#ifndef _XBE_OPERATION_HH
#define _XBE_OPERATION_HH

#include "xbe_codegen.h"
#include "xbe_type.h"
#include "xbe_exception.h"
#include "xbe_classgen.h"
#include "tlist.h"

class be_interface;
class be_DispatchableType;

class be_operation
         :
         public virtual AST_Operation,
         public be_CodeGenerator
{

public:

   enum RequestType { OP_Request, OP_Return };
   enum RefType { OP_Client, OP_Server };
   enum ExceptionStatus { OP_NoException, OP_Exception, OP_Retry };
   enum OP_SignatureType { OP_Declaration, OP_Implementation, OP_Invoke };
   enum { signatureBufLen = 1024 };

   static char signatureBuf[signatureBufLen];

public:

   static be_operation * _narrow(AST_Decl * decl);

   be_operation();
   be_operation(
      AST_Type *rt,
      AST_Operation::Flags fl,
      UTL_ScopedName *n,
      const UTL_Pragmas &p);

   DDS::Boolean is_oneway() const;
   DDS_StdString Scope(const DDS_StdString& name);
   DDS_StdString LocalName();
   void Initialize(be_interface* owner);
   const DDS_StdString& ReturnTypeName();
   DDS::Boolean HasReturn();
   DDS_StdString BaseSignature(const DDS_StdString & implclassname);
   DDS_StdString ScopedBaseRequestCall();
   DDS_StdString StubSignature(OP_SignatureType sigType);
   DDS_StdString ReplySignature(
      ExceptionStatus excStat,
      OP_SignatureType sigType);
   DDS_StdString TieSignature(OP_SignatureType sigType);
   DDS_StdString DirectSignature(
      OP_SignatureType sigType,
      const DDS_StdString & implclassname);

   const DDS_StdString& InterfaceBasename();
   void GenerateImpureRequestCall(
      be_ClientImplementation &source);
   void GenerateVirtual(be_Source& source, const DDS_StdString& implclassname);
   void GenerateStub(
      be_ClientImplementation& source);
   void GenerateServerImplementation(
      be_ClientImplementation& source);
   void GenerateSyncCall(
      be_ServerImplementation &source);
   void GenerateDispatcher(
      be_ServerImplementation& source,
      const DDS_StdString& implbasename);
   void GenerateSyncDispatcher(
      be_ServerImplementation& source,
      const DDS_StdString& implbasename);

   // BE_CODEGENERATOR VIRTUALS
   virtual void Generate(be_ClientHeader& baseHeader);
   virtual void Generate(be_ClientImplementation& baseImpl);
   virtual void Generate(be_ServerHeader&)
   {}

   virtual void Generate(be_ServerImplementation& dispatchImpl);

   // AST_OPERATION VIRTUALS
   virtual AST_Argument * add_argument(AST_Argument *a);

   DEF_NARROW_METHODS2(be_operation, AST_Operation, be_CodeGenerator);
   DEF_NARROW_FROM_DECL(be_operation);
   DEF_NARROW_FROM_SCOPE(be_operation);

private:

   friend class be_interface;

   be_interface * iface;
   DDS_StdString stubClassname;
   DDS_StdString interfaceBasename;
   DDS_StdString opKey;
   DDS_StdString enclosingScope;
   TList<be_argument *> arguments;
   be_DispatchableType* returnType;
   long n_InArgs;
   long n_OutArgs;
   long n_InOutArgs;
   long n_Args;

   void InitializeOpRequest();
};

inline DDS::Boolean
be_operation::is_oneway() const
{
   be_operation * ncop = (be_operation*)this;

   return (DDS::Boolean)(ncop->flags() == OP_oneway);
}

inline DDS_StdString
be_operation::Scope(const DDS_StdString& name)
{
   DDS_StdString ret = enclosingScope;

   if (ret.length())
   {
      ret += "::";
   }

   return ret + name;
}

#endif
