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
#ifndef XBE_CPPFWD_H_INCLUDED
#define XBE_CPPFWD_H_INCLUDED 

#include "xbe_string.h"
#include "idl.h"
#include "tlist.h"
#include "xbe_source.h"
#include "xbe_utils.h"
#include "xbe_cppscope.h"
#include "xbe_type.h"

// a C++ forward declaration

class be_CppFwdDecl
{

public:
   enum DeclType { INTERFACE, STRUCT, UNION };
   // Add() creates a be_CppFwdDecl and appends it to the list
   // For example, be_CppFwdDecl::Add("struct", "blah", cppScope);
   static void Add
   (
      const DeclType declType,
      be_Type * beType,
      const be_CppEnclosingScope& cppScope
   );

   static void Remove
   (
      be_Type * beType
   );

   // generate all be_CppFwdDecls
   static void GenerateAllWithinScope(
      be_ClientHeader& source,
      const be_CppEnclosingScope& cppScope);

   static idl_bool IsAlreadyDeclared(
      const DDS_StdString& keyword,
      const DDS_StdString& name,
      const be_CppEnclosingScope& cppScope);

private:
   static const char * const keywords[3];
   const DeclType m_declType;
   be_Type * m_beType;
   const be_CppEnclosingScope m_cppScope;
   idl_bool m_generated;

   static TList<be_CppFwdDecl*> *sm_fwdDeclList;

   void Generate(be_ClientHeader& source) const;

   // construction happens only in Add()
   be_CppFwdDecl(const DeclType declType, 
                 be_Type * beType,
                 const be_CppEnclosingScope& cppScope)
         : m_declType (declType),
           m_beType (beType),
           m_cppScope (cppScope), 
           m_generated (I_FALSE)
   {
   }

   // assignment, copying, and destruction are not allowed
   be_CppFwdDecl& operator=(const be_CppFwdDecl&);
   be_CppFwdDecl(const be_CppFwdDecl&);
   ~be_CppFwdDecl();
};

#endif // XBE_CPPFWD_H_INCLUDED
