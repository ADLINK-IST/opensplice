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
#ifndef _XBE_ARGUMENT_HH
#define _XBE_ARGUMENT_HH

#include "idl.h"
#include "xbe_type.h"
#include "xbe_dispatchable.h"
#include "xbe_direction.h"
#include "StdString.h"

class be_argument
         :
         public virtual AST_Argument
{

public:

   be_argument();
   be_argument(
      AST_Argument::Direction d,
      AST_Type * ft,
      UTL_ScopedName * n,
      const UTL_Pragmas & p
   );


   //
   // the new way (BEN as of 25-Mar-2001)
   //

   const be_DispatchableType& ArgumentType() const;

   inline const be_ArgumentDirection& Direction() const
   {
      return m_direction;
   }

   //
   // deprecating the following ...
   //
   void Initialize();
   DDS_StdString LocalName() const;
   be_Type * BeType() const;
   pbbool IsInArg() const;
   pbbool IsInOutArg() const;
   pbbool IsOutArg() const;
   DDS_StdString ParamMode() const;
   DDS_StdString Releaser() const;

   const DDS_StdString & Signature(
      const DDS_StdString& className
   );
   pbbool PassPointer(
      AST_Argument::Direction dir
   ) const;
   DDS_StdString SyncStreamOut(
      const DDS_StdString &out,
      pbbool isStubSide = pbfalse
   ) const;
   DDS_StdString SyncStreamIn(
      const DDS_StdString &in,
      pbbool isStubSide = pbfalse
   ) const;

   void GenerateOpParameter(
      be_ClientHeader& clientHeader
   );
   //
   // in favor of these
   //
   be_Type * get_be_type() const;
   DDS::Boolean is_in_arg() const;
   DDS::Boolean is_inout_arg() const;
   DDS::Boolean is_out_arg() const;

   DDS::Boolean make_put_param_for_stub(
      ostream & os,
      be_Tab & tab
   );
   DDS::Boolean make_get_param_for_stub(
      ostream & os,
      be_Tab & tab
   );
   DDS::Boolean declare_for_stub
   (
      ostream & os,
      be_Tab & tab,
      const DDS_StdString& scope
   );
   DEF_NARROW_METHODS1(be_argument, AST_Argument);
   DEF_NARROW_FROM_DECL(be_argument);

private:

   DDS_StdString m_signature;
   be_Type * m_beType;
   be_ArgumentDirection m_direction;
};

inline be_Type *
be_argument::get_be_type() const
{
   if (!m_beType)
   {
      be_argument * ncarg = (be_argument *)this;

      ncarg->m_beType = be_Type::_narrow(ncarg->field_type());
   }

   return m_beType;
}

inline DDS::Boolean
be_argument::is_in_arg() const
{
   return (DDS::Boolean)(((be_argument*)this)->direction() == dir_IN);
}

inline DDS::Boolean
be_argument::is_inout_arg() const
{
   return (DDS::Boolean)(((be_argument*)this)->direction() == dir_INOUT);
}

inline DDS::Boolean
be_argument::is_out_arg() const
{
   return (DDS::Boolean)(((be_argument*)this)->direction() == dir_OUT);
}

inline DDS::Boolean
be_argument::make_put_param_for_stub(
   ostream & os,
   be_Tab & tab)
{
   return get_be_type()->make_put_param_for_stub(
             os,
             tab,
             LocalName(),
             MakeVarType(direction()));
}

inline DDS::Boolean
be_argument::make_get_param_for_stub(
   ostream & os,
   be_Tab & tab)
{
   return get_be_type()->make_get_param_for_stub(
             os,
             tab,
             LocalName(),
             MakeVarType(direction()));
}

inline DDS::Boolean
be_argument::declare_for_stub(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString& stubScope)
{
   return get_be_type()->declare_for_stub(os,
                                          tab,
                                          LocalName(),
                                          stubScope,
                                          MakeVarType(direction()));
}

#endif
