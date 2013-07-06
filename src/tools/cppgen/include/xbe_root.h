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
#ifndef _XBE_ROOT_HH
#define _XBE_ROOT_HH

#include "xbe_codegen.h"
#include "xbe_type.h"
#include "xbe_classgen.h"
#include "tlist.h"

class be_exception;
class be_sequence;
class be_interface;

class be_root : public AST_Root, public be_CodeGenerator
{
private:

   static TList<be_Type *> &anyOps;
   static TList<be_Type *> &fwdDecls;
   static TList<be_Type *> &putGetOps;
   static TList<be_Type *> &streamOps;
   static TList<be_Type *> &typedefs;
   static TList<be_Type *> &typecodes;
   static TList<be_Type *> &typesThatNeedProtoTypeCodesFinished;
   static TList<be_exception *> &globalDecls;
   static TList<be_CodeGenerator *> &implementations;
   static TList<be_sequence*> &deferred_seqs;

   void GenerateGlobalTypes (be_ClientHeader& source);
   void GenerateGlobalDecls (be_ClientHeader& source);

public:

   be_root ();
   be_root (UTL_ScopedName *n, const UTL_Pragmas &p);

   static void AddAnyOps (be_Type& decl);
   static void AddFwdDecls (be_Type& decl);
   static void RemoveFwdDecls (be_Type& decl);
   static void AddStreamOps (be_Type& decl);
   static void AddPutGetOps (be_Type& decl);
   static void AddTypedef (be_Type& decl);
   static void AddTypecode (be_Type& decl);
   static void AddTypeThatNeedsProtoTypeCodeFinished (be_Type& decl);
   static void AddGlobalDeclarations (be_exception* except);
   static void AddImplementations (be_CodeGenerator& cg);
   static void DeferSequence (be_sequence* s);
   static void GenerateDependants (be_ClientHeader& src, const char* obj,
                                  const char* scope);

   void FinishAllProtoTypeCodes ();

   // Generator virtuals

   virtual void Generate (be_ClientHeader& clientHeader);
   virtual void Generate (be_ClientImplementation& clientImpl);
   virtual void Generate (be_ServerHeader& serverHeader);
   virtual void Generate (be_ServerImplementation& serverImpl);

   DEF_NARROW_METHODS2 (be_root, AST_root, be_CodeGenerator);
   DEF_NARROW_FROM_DECL (be_root);
   DEF_NARROW_FROM_SCOPE (be_root);
};

#endif
