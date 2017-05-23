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
#ifndef _XBE_DISPATCHABLE_H
#define _XBE_DISPATCHABLE_H

#include "xbe_type.h"
#include "xbe_cppname.h"
#include "xbe_direction.h"

// a be_DispatchableType is a be_Type that can be sent from a dispatcher
// to a servant method

class be_DispatchableType : public be_Type 
{

public:
   DEF_NARROW_METHODS2(be_DispatchableType, be_Type, be_TypeMap);

   // STRING GENERATORS

   void DeclareForDispatcher(
      ostream& os,
      be_Tab& tab,
      const be_CppName& implClassName,
      const be_CppName& argName,
      const be_ArgumentDirection& direction) const;

   DDSString PassToServantMethod(
      const be_CppName& argName,
      const be_ArgumentDirection& direction,
      int getargIndex) const;

   DDSString GetargMvalue(
      const be_CppName& argName,
      const be_ArgumentDirection& direction) const;

   DDSString PutargMvalue(
      const be_CppName& argName,
      const be_ArgumentDirection& direction,
      int getargIndex) const;

   enum en_HowStoredInDispatcher
   {
      STORED_AS_STACK_VARIABLE,  // Normal stack variable
      STORED_IN_VAR,             // Class _var
      STORED_IN_ALLOCED_VAR,     // a var initialized by calling ##Type_alloc()
      STORED_IN_IOR_VAR,         // retrieved by ->get_ior()
      STORED_IN_DESCENDANT_VAR,  // like a typecode
      STORED_IN_STRING_VAR       // String vars
   };

   // VIRTUALS

   virtual void      // code to initialize a variable, comes
   InitializeInDispatcher(   // right after declaration
      ostream& os,
      be_Tab& tab,
      const be_CppName& argName,
      const be_ArgumentDirection& direction) const;

   // PURE VIRTUALS

   virtual en_HowStoredInDispatcher
   HowStoredInDispatcher(const be_ArgumentDirection& direction) const = 0;

private:

   // PRIVATE HELPERS

   DDSString DereferenceMvalue(int index) const;

   void DeclareIn(
      ostream& os,
      be_Tab& tab,
      const be_CppName& implClassName,
      const be_CppName& argName) const;

   void DeclareOutOrReturn(
      ostream& os,
      be_Tab& tab,
      const be_CppName& argName,
      const be_ArgumentDirection& direction) const;

   void DeclareInout(
      ostream& os,
      be_Tab& tab,
      const be_CppName& implClassName,
      const be_CppName& argName) const;

   DDSString PassInToServantMethod(
      const be_CppName& argName,
      int getargIndex) const;

   DDSString PassOutToServantMethod(
      const be_CppName& argName,
      int getargIndex) const;

   DDSString PassInoutToServantMethod(
      const be_CppName& argName,
      int getargIndex) const;

   DDSString PutargMvalueInout(
      const be_CppName& argName,
      int getargIndex) const;

   DDSString PutargMvalueOutOrReturn(
      const be_CppName& argName,
      const be_ArgumentDirection& direction,
      int getargIndex) const;
};

#endif // _XBE_DISPATCHABLE_H
