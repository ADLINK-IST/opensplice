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
#ifndef _XBE_DISPATCHER_H
#define _XBE_DISPATCHER_H

#include "idl.h"
#include "xbe_arglist.h"

class be_argument;
class be_ServerImplementation;
class be_DispatchableType;
class be_Tab;

class be_Dispatcher
{
public:

   be_Dispatcher
   (
      const be_CppName & opName,
      const be_CppName & implClassName,
      const be_ArgumentList & arguments,
      be_ServerImplementation & source,
      be_DispatchableType* returnType = NULL
   );

   be_Dispatcher
   (
      const be_CppName & servantName,  // impl func that dispatcher calls
      const be_CppName & invokeName,   // name that _invoke() calls
      const be_CppName & implClassName,
      const be_ArgumentList & arguments,
      be_ServerImplementation & source,
      be_DispatchableType * returnType = NULL
   );

   void Generate ();

protected:

   void DeclareDispatcherBody ();
   void DeclareStackVariables ();
   void MakeGetargArray ();
   void UnmarshalRequest ();
   void MarshalReply ();
   void CallOperation ();
   void MakePutargArray ();
   void MakePutarg 
   ( 
      ostream & os,
      be_Tab & tab,
      const be_Argument & arg, 
      int getargIndex
   ) const;

   const be_CppName m_servantName; // name of func that dispatcher calls
   const be_CppName m_invokeName; // name of dispatcher (called by _invoke())
   const be_CppName m_implClassName; // interface that owns the operation
   const be_ArgumentList m_arglist;   // the arguments of the operation
   ostream& m_os;     // where to output the code to
   be_Tab m_tab;     // current tab setting
   be_DispatchableType* m_returnType; // type of return value (might be NULL)
   const bool m_opReturnsValue; // does the op return a value?
   const unsigned short m_getargCount; // number of args to unmarshal
   const unsigned short m_putargCount; // number of args to marshal on reply
};

#endif // _XBE_DISPATCHER_H
