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
#ifndef _XBE_TYPEMAP_HH
#define _XBE_TYPEMAP_HH

#include "ast.h"

class be_TypeMap
{

protected:

   DDS_StdString typeName;
   DDS_StdString inTypeName;
   DDS_StdString inoutTypeName;
   DDS_StdString outTypeName;
   DDS_StdString returnTypeName;
   DDS_StdString dmfAdtMemberTypeName;
   DDS_StdString structMemberTypeName;
   DDS_StdString sequenceMemberTypeName;
   DDS_StdString istreamOpTypeName;
   DDS_StdString streamOpTypeName;

   // BE_TYPE_MAP PURE VIRTUAL
   virtual void InitializeTypeMap(be_Type*) = 0;

public:

   inline const DDS_StdString&
   TypeName() const
   {
      return typeName;
   }

   inline const DDS_StdString&
   InTypeName() const
   {
      return inTypeName;
   }

   inline const DDS_StdString&
   InOutTypeName() const
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

   // BE_TYPE_MAP PURE VIRTUAL
   virtual pbbool IsFixedLength() const = 0;

   virtual ~be_TypeMap()
   {
      if 
      (
         !typeName.Length() || !inTypeName.Length() || !inoutTypeName.Length()
         || !outTypeName.Length() || !returnTypeName.Length()
         || !dmfAdtMemberTypeName.Length() || !structMemberTypeName.Length()
         || !sequenceMemberTypeName.Length()
      )
      {
         cerr << "Type map for " << typeName << " not complete" << endl;
      }
   }

   DEF_NARROW_METHODS0(be_TypeMap);
};

#endif // _XBE_TYPEMAP_HH
