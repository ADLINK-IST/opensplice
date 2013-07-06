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
#ifndef _XBE_ARGUMENT2_H
#define _XBE_ARGUMENT2_H

#include "idl.h"
#include "xbe_cppname.h"
#include "xbe_argument.h"

// a be_Argument is an argument passed in an IDL operation

class be_Argument
{

public:
   be_Argument(
      const be_CppName& cppName,
      const be_DispatchableType& type,
      const be_ArgumentDirection& direction);

   be_Argument(const be_argument& argument);

   be_Argument(const be_Argument& that);

   be_Argument& operator=(const be_Argument& that);

   inline bool operator==(const be_Argument& that)
   {
      return m_cppName == that.m_cppName
             && m_type == that.m_type
             && m_direction == that.m_direction;
   }

   inline bool operator!=(const be_Argument& that)
   {
      return !(*this == that);
   }

   // STRING GENERATORS

   inline DDS_StdString ScopedTypeCodeTypeName() const
   {
      return m_type->Scope(m_type->TypeCodeTypeName());
   }

   inline DDS_StdString GetargMvalue() const
   {
      return m_type->GetargMvalue(m_cppName, m_direction);
   }

   inline void DeclareForDispatcher(
      ostream& os,
      be_Tab& tab,
      const be_CppName& implClassName) const
   {
      m_type->DeclareForDispatcher(os, tab, implClassName, m_cppName,
                                   m_direction);
   }

   inline void InitializeInDispatcher(ostream& os, be_Tab& tab) const
   {
      m_type->InitializeInDispatcher(os, tab, m_cppName, m_direction);
   }

   inline DDS_StdString PassToServantMethod(int getargIndex) const
   {
      return m_type->PassToServantMethod(
                m_cppName, m_direction, getargIndex);
   }

   inline DDS_StdString PutargMvalue(int getargIndex) const
   {
      return m_type->PutargMvalue(m_cppName, m_direction, getargIndex);
   }

   inline DDS_StdString IopParamMmode (DDS::Boolean server) const
   {
      return m_direction.IopParamMmode (server);
   }

   // HELPERS

   inline bool IsSentFromClient() const
   {
      return m_direction.IsSentFromClient();
   }

   inline bool IsSentFromServer() const
   {
      return m_direction.IsSentFromServer();
   }

private:
   be_CppName m_cppName;
   const be_DispatchableType* m_type; // cannot be NULL
   be_ArgumentDirection m_direction;
};

#endif // _XBE_ARGUMENT2_H
