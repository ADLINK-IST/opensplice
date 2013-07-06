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
