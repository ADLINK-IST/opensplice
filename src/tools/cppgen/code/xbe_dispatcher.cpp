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
#include "idl.h"
#include "xbe_dispatchable.h"
#include "xbe_utils.h"
#include "xbe_dispatcher.h"
#include "xbe_argument.h"
#include "xbe_argument2.h"

// constructor for ops whose invoke name is the same as their servant name

be_Dispatcher::be_Dispatcher
(
   const be_CppName& cppOpName,
   const be_CppName& implClassName,
   const be_ArgumentList& arguments,
   be_ServerImplementation& source,
   be_DispatchableType* returnType
)
:
   m_servantName(cppOpName),
   m_invokeName(cppOpName),
   m_implClassName(implClassName),
   m_arglist(arguments),
   m_os(source.Stream()),
   m_tab(source),
   m_returnType(returnType),
   m_opReturnsValue(returnType != NULL
      && !str_eq(returnType->TypeName(), "void")),
   m_getargCount(m_arglist.NumArgsSentFromClient()),
   m_putargCount(m_arglist.NumArgsSentFromServer() + m_opReturnsValue)
{}

// constructor for ops with invoke name different than servant name

be_Dispatcher::be_Dispatcher
(
   const be_CppName& servantName,
   const be_CppName& invokeName,
   const be_CppName& implClassName,
   const be_ArgumentList& arguments,
   be_ServerImplementation& source,
   be_DispatchableType* returnType
)
:
   m_servantName(servantName),
   m_invokeName(invokeName),
   m_implClassName(implClassName),
   m_arglist(arguments),
   m_os(source.Stream()),
   m_tab(source),
   m_returnType(returnType),
   m_opReturnsValue(returnType != NULL
      && !str_eq(returnType->TypeName(), "void")),
   m_getargCount(m_arglist.NumArgsSentFromClient()),
   m_putargCount(m_arglist.NumArgsSentFromServer() + m_opReturnsValue)
{}

void be_Dispatcher::Generate ()
{
   DeclareDispatcherBody ();
   m_os << m_tab << "{" << nl;
   m_tab.indent ();

   DeclareStackVariables ();
   m_os << nl;

   MakeGetargArray ();
   UnmarshalRequest ();
   m_os << nl;

   CallOperation ();
   m_os << nl;

   MakePutargArray ();
   MarshalReply ();

   m_tab.outdent ();
   m_os << m_tab << "}" << nl << nl;
}

void be_Dispatcher::DeclareDispatcherBody ()
{
   m_os << m_tab << "void " << m_implClassName << "::_dispatch_" << m_invokeName << nl;
   m_os << m_tab << "(" << nl;
   m_tab.indent ();
   m_os << m_tab << "void * _servant_," << nl;
   m_os << m_tab << "DDS::Codec::Request & _request_" << nl;
   if (XBE_Ev::generate ())
   {
      m_os << m_tab << XBE_Ev::arg (XBE_ENV_ARGN, false) << nl;
   }
   m_tab.outdent ();
   m_os << m_tab << ")" << nl;
}

void be_Dispatcher::DeclareStackVariables ()
{
   be_ArgumentList::const_iterator iter;

   m_os << m_tab << m_implClassName << " * _srv_ = (" 
        << m_implClassName << "*) _servant_;" << nl;

   for (iter = m_arglist.begin(); iter != m_arglist.end(); ++iter)
   {
      const be_Argument& arg = *iter;
      arg.DeclareForDispatcher (m_os, m_tab, m_implClassName);
      arg.InitializeInDispatcher (m_os, m_tab);
   }

   if (m_opReturnsValue)
   {
      const be_Argument returnValue ("_ret_", *m_returnType, VT_Return);
      returnValue.DeclareForDispatcher (m_os, m_tab, m_implClassName);
   }
}

void be_Dispatcher::MakeGetargArray ()
{
   if (m_getargCount == 0)
   {
      return ;
   }

   m_os << m_tab << "DDS::Codec::Param _out_[" << m_getargCount << "] =" << nl;
   m_os << m_tab << "{" << nl;
   m_tab.indent ();

   bool wroteOne = false;

   // make a getarg for each argument that's sent from the client

   be_ArgumentList::const_iterator iter;

   for (iter = m_arglist.begin(); iter != m_arglist.end(); iter++)
   {
      const be_Argument& arg = *iter;

      if (arg.IsSentFromClient())
      {
         if (wroteOne)
         {
            m_os << "," << nl;
         }

         // a getarg looks like this:
         //    { _tc_TypeName, &arg, DDS::PARAM_IN }
         //      m_typecode   m_value         m_mode

         m_os << m_tab << "{ "
            << arg.ScopedTypeCodeTypeName () << ", "
            << arg.GetargMvalue () << ", "
            << arg.IopParamMmode (TRUE)
            << " }";

         wroteOne = true;
      }
   }

   m_os << nl;
   m_tab.outdent();
   m_os << m_tab << "};" << nl;
}

void be_Dispatcher::UnmarshalRequest ()
{
   m_os << m_tab << "_request_.get_args ("
        << (m_getargCount ? "_out_" : "0") << ", " << m_getargCount
        << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   if (XBE_Ev::generate ())
   {
      m_os << m_tab;
      XBE_Ev::check (m_os, NULL);
      m_os << nl;
   }
}

void be_Dispatcher::CallOperation ()
{
   m_os << m_tab;

   if (m_opReturnsValue)
   {
      m_os << "_ret_ = ";
   }

   m_os << "_srv_->" << m_servantName << " (";

   // arguments

   bool wroteOne = false;
   int getargIndex = 0;
   be_ArgumentList::const_iterator iter;

   for (iter = m_arglist.begin(); iter != m_arglist.end(); iter++)
   {
      const be_Argument& arg = *iter;

      if (wroteOne)
      {
         m_os << ", ";
      }

      m_os << arg.PassToServantMethod (getargIndex);

      wroteOne = true;

      if (arg.IsSentFromClient())
      {
         // if arg is sent from client, then it has an element in the
         // getarg array
         getargIndex++;
      }
   }

   // env

   if (!wroteOne)
   {
      m_os << XBE_Ev::arg (XBE_ENV_VAR1);
   }
   else
   {
      m_os << XBE_Ev::arg (XBE_ENV_VARN);
   }

   m_os << ");" << nl;
   if (XBE_Ev::generate ())
   {
      m_os << m_tab;
      XBE_Ev::check (m_os, NULL);
      m_os << nl;
   }
}

void be_Dispatcher::MakePutarg
(
   ostream& os,
   be_Tab& tab,
   const be_Argument& arg,
   int getargIndex
) const
{
   // a Putarg looks like this:
   //    { _tc_TypeName, &arg, DDS::PARAM_IN }
   //      m_typecode   m_value         m_mode

   os << tab << "{ "
      << arg.ScopedTypeCodeTypeName () << ", "
      << arg.PutargMvalue (getargIndex) << ", "
      << arg.IopParamMmode (TRUE)
      << " }";
}

void be_Dispatcher::MakePutargArray ()
{
   if (m_putargCount == 0)
   {
      return ;
   }

   bool wroteOne = false;
   int getargIndex = 0;

   m_os << m_tab << "DDS::Codec::Param _in_[" << m_putargCount << "] =" << nl;
   m_os << m_tab << "{" << nl;
   m_tab.indent ();

   // return value

   if (m_opReturnsValue)
   {
      MakePutarg (m_os, m_tab, be_Argument("_ret_", *m_returnType, VT_Return), getargIndex);

      wroteOne = true;
   }

   // make a putarg for each argument sent from the server

   be_ArgumentList::const_iterator iter;

   for (iter = m_arglist.begin(); iter != m_arglist.end(); iter++)
   {
      const be_Argument& arg = *iter;

      if (arg.IsSentFromServer())
      {
         if (wroteOne)
         {
            m_os << "," << nl;
         }

         MakePutarg (m_os, m_tab, arg, getargIndex);

         wroteOne = true;
      }

      if (arg.IsSentFromClient())
      {
         // if arg is sent from client, then it has an element in the
         // getarg array
         getargIndex++;
      }
   }

   m_os << nl;
   m_tab.outdent ();
   m_os << m_tab << "};" << nl;
}

void be_Dispatcher::MarshalReply ()
{
   m_os << m_tab << "_request_.put_args ("
        << (m_putargCount ? "_in_" : "0") << ", " << m_putargCount
        << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
}
