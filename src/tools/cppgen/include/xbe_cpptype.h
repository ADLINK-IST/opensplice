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
#ifndef _XBE_CPPTYPE_H
#define _XBE_CPPTYPE_H

#include "xbe_cppname.h"
#include "xbe_cppscope.h"

// A be_CppType is a C++ type.
//
// A be_CppType is not to be confused with an IDL type, even though both
// may have the same name.  There is not a simple one-to-one mapping
// between IDL types and generated C++ types.

class be_CppType
{

public:
   be_CppType(const be_CppName& name) : m_name(name), m_enclosingScope()
   { }

   be_CppType(UTL_ScopedName& utlScopedName)
         : m_name(LowestLevelName(utlScopedName)),
         m_enclosingScope(utlScopedName)
   { }

   be_CppType(const be_CppType& that)
         : m_name(that.m_name),
         m_enclosingScope(that.m_enclosingScope)
   { }

   inline const be_CppName& Name() const
   {
      return m_name;
   }

   be_CppName ScopedName() const;

private:
   const be_CppName m_name;
   const be_CppEnclosingScope m_enclosingScope;

   // DO NOT ASSIGN
   // having a public copy constructor is not a good idea, either, but we need
   // it for a while, to make use of be_CppType in be_Type feasible for now

   be_CppType& operator=(const be_CppType& that);

   // PRIVATE HELPERS

   static be_CppName LowestLevelName(UTL_ScopedName& utlScopedName);
};

#endif // _XBE_CPPTYPE_H
