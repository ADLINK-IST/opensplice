/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
