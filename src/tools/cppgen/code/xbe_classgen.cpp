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
#include "sacpp_DDS_DCPS.h"

#include "idl.h"

#include "xbe_classgen.h"
#include "xbe_globals.h"
#include "xbe_argument.h"
#include "xbe_type.h"
#include "xbe_utils.h"
#include "xbe_exception.h"

os_ushort DDS_hash (const char * str)
{
   os_ushort hash = 5381;
   int c;
 
   while ((c = *str++))
   {
      hash = (hash * 33) ^ c;
   }     

   return hash & 0x7FFFFFFF;
}

// ------------------------------------------------------
//  BE_STUB_GENERATOR IMPLEMENTATION
// ------------------------------------------------------

be_OpStubGenerator::be_OpStubGenerator
(
   const DDS_StdString& scopedClassname,
   const DDS_StdString& opKey,
   const DDS_StdString& opName,
   const DDS_StdString& signature,
   be_Type * returnType,
   pbbool isOneWay,
   const ArgList &arguments,
   UTL_ExceptList *exceptions,
   UTL_StrList * context,
   const DDS_StdString & opDispatchName
)
   :  m_scopedClassname(scopedClassname),
      m_opKey(opKey),
      m_opName(opName),
      m_signature(signature),
      m_returnType(returnType),
      m_isOneWay(isOneWay),
      m_context(context),
      m_opDispatchName(opDispatchName)
{
   isAtModuleScope(pbfalse);
   TList<be_argument *>::iterator it = arguments.begin();
   TList<be_argument *>::iterator end_it = arguments.end();

   while (it != end_it)
   {
      m_arguments.push_back(*it);
      it++;
   }

   UTL_ExceptlistActiveIterator iter(exceptions);

   while (!iter.is_done ())
   {
      be_exception *exception =
      (be_exception *) iter.item ()->narrow ((long) &be_exception::type_id);
      m_exceptions.push_back (exception);
      iter.next ();
   }
}

be_OpStubGenerator::~be_OpStubGenerator()
{}

int
be_OpStubGenerator::InArgCount()
{
   TList<be_argument *>::iterator ait;
   int ret = 0;

   for (ait = m_arguments.begin(); ait != m_arguments.end() ; ait++)
   {
      if ((*ait)->IsInArg() || (*ait)->IsInOutArg())
      {
         ret++;
      }
   }

   return ret;
}

int
be_OpStubGenerator::OutArgCount()
{
   TList<be_argument *>::iterator ait;
   int ret = 0;

   for (ait = m_arguments.begin() ; ait != m_arguments.end() ; ait++)
   {
      if ((*ait)->IsInOutArg() || (*ait)->IsOutArg())
      {
         ret++;
      }
   }

   return ret;
}

pbbool
be_OpStubGenerator::HasReturn()
{
   pbbool ret = pbfalse;

   if (m_returnType)
   {
      assert ((const char *)m_returnType->TypeName());

      if ((const char *)m_returnType->TypeName())
      {
         ret = strcmp((const char *)m_returnType->TypeName(), "void") ? TRUE : FALSE;
      }
   }

   return ret;
}

void be_OpStubGenerator::Generate (be_ClientImplementation& source)
{
   DDS::Boolean wroteOne;
   ostream & os = source.Stream ();
   be_Tab tab(source);
   DDS_StdString opret = "_ret_";
   DDS_StdString opType = "DDS::OP_NORMAL";
   DDS_StdString _in_ = "0";
   DDS_StdString _out_ = "0";
   int putArgCount = InArgCount ();
   int getArgCount = OutArgCount ();
   TList<be_argument *>::iterator ait;
   be_ExceptionList::iterator eit;
   DDS::ULong exception_count = m_exceptions.size ();
   DDS::ULong count = exception_count;

   // declare stub body

   os << nl;
   os << tab << m_signature << nl;
   os << tab << "{" << nl;
   tab.indent ();

   if (exception_count > 0)
   {
      os << tab << "static DDS::ULong _exception_hashes[] = {" << nl;

      for (eit = m_exceptions.begin (); eit != m_exceptions.end (); eit++)
      {
         be_exception *excep = *eit;
         const char *rep_id =
         excep->get_decl_pragmas().get_repositoryID()->get_string ();
         os << tab << tab << DDS_hash (rep_id);

         if (--count > 0)
         {
            os << ",";
         }

         os << tab << tab << "// " << rep_id << nl;
      }

      os << tab << "};" << nl;
   }

   // declare request arguments (pointer managers)

   for (ait = m_arguments.begin (); ait != m_arguments.end (); ait++)
   {
      (*ait)->declare_for_stub (os, tab, m_scopedClassname);
   }

   // declare return

   if (!m_isOneWay && HasReturn ())
   {
      getArgCount++;
      m_returnType->declare_for_stub
      (
         os,
         tab,
         opret,
         m_scopedClassname,
         VT_Return
      );
   }

   // populate put args

   if (putArgCount)
   {
      os << tab << "DDS::Codec::Param _in_[" << BE_Globals::int_to_string (putArgCount) << "] =" << nl;
      os << tab << "{" << nl;

      wroteOne = FALSE;

      tab.indent ();
      for (ait = m_arguments.begin(); ait != m_arguments.end(); ait++)
      {
         if ((*ait)->is_in_arg() || (*ait)->is_inout_arg())
         {
            if (wroteOne)
            {
               os << "," << nl;
            }

            wroteOne = (*ait)->make_put_param_for_stub (os, tab);
         }
      }
      tab.outdent ();

      os << nl << tab << "};" << nl;
      _in_ = "_in_";
   }

   //
   // populate get args
   //
   wroteOne = FALSE;

   if (getArgCount)
   {
      os << tab << "DDS::Codec::Param _out_[" << BE_Globals::int_to_string (getArgCount) << "] =" << nl;
      os << tab << "{" << nl;

      tab.indent ();
      if (HasReturn())
      {
         m_returnType->make_get_param_for_stub (os, tab, opret, VT_Return);
         wroteOne = TRUE;
      }

      for (ait = m_arguments.begin(); ait != m_arguments.end() ; ait++)
      {
         if ((*ait)->is_inout_arg() || (*ait)->is_out_arg())
         {
            if (wroteOne)
            {
               os << "," << nl;
            }

            wroteOne = (*ait)->make_get_param_for_stub (os, tab);
         }
      }
      tab.outdent ();

      os << nl << tab << "};" << nl;
      _out_ = "_out_";
   }

   if (m_isOneWay)
   {
      opType = "DDS::OP_ONEWAY";
   }

   if (getArgCount || putArgCount)
   {
      os << nl;
   }

   // invoke the request

   os << tab << "invoke_request" << nl;
   os << tab << "(" << nl;
   tab.indent ();
   os << tab << "\"" << m_opDispatchName << "\", "
      << m_opDispatchName.length () + 1 << "," << nl;
   os << tab << opType << "," << nl;
   os << tab << _in_ << ", " << putArgCount << "," << nl;
   os << tab << _out_ << ", " << getArgCount << "," << nl;

   if (exception_count > 0)
   {
      os << tab << "_exception_hashes, " << m_exceptions.size () << nl;
   }
   else
   {
      os << tab << "0, 0" << nl;
   }

   if (XBE_Ev::generate ())
   {
      os << tab << XBE_Ev::arg (XBE_ENV_VARN, false) << nl;
   }
   tab.outdent ();
   os << tab << ");" << nl;

   // return any return argument

   if (HasReturn ())
   {
      // When native exceptions are not supported and an
      // exception has been thrown, returned value must be
      // a null pointer for types returned by pointer that
      // have not been initialized to zero.

      if
      (
         m_returnType->IsSequenceType () ||
         m_returnType->IsInterfaceType () ||
         m_returnType->IsArrayType () ||
         (m_returnType->IsStructuredType () && !m_returnType->IsFixedLength ())
      )
      {
         if (XBE_Ev::generate ())
         {
            os << tab;
            XBE_Ev::check (os, "0");
            os << nl;
         }
      }
      os << tab << "return " << opret;
      if ((!m_isOneWay) && (m_returnType->IsReturnedByVar ()))
      {
         os << "._retn ();" << nl;
      }
      else
      {
         os << ";" << nl;
      }
   }

   tab.outdent ();
   os << tab << "}" << nl;
}

// ------------------------------------------------------
//  be_AttStubGenerator IMPLEMENTATION
// ------------------------------------------------------

be_AttStubGenerator::be_AttStubGenerator
(
   const DDS_StdString& scopedClassname,
   const DDS_StdString& opKey,
   const DDS_StdString& opName,
   const DDS_StdString& signature,
   be_Type * returnType,
   pbbool isSetAttribute
)
:
   m_scopedClassname(scopedClassname),
   m_opKey(opKey),
   m_opName(opName),
   m_signature(signature),
   m_returnType(returnType),
   m_isSetAttribute(isSetAttribute)
{}

be_AttStubGenerator::~be_AttStubGenerator()
{}


void be_AttStubGenerator::Generate(be_ClientImplementation& source)
{
   const DDS_StdString opret = "_ret_";
   const DDS_StdString argName = "_nval_";
   ostream & os = source.Stream();
   be_Tab tab(source);

   //
   // declare stub body
   //
   os << nl;
   os << tab << m_signature << nl;
   os << tab << "{" << nl;
   tab.indent ();

   os << tab << "static DDS::String _onam_ = (char*)\"" << m_opName << "\";" << nl;
   os << tab << "static const int _olen_  = ";
   os << m_opName.length () + 1 << ";" << nl;

   //
   // declare request argument (pointer manager)
   //
   if (m_isSetAttribute)
   {
      //
      // declare put arg
      //
      m_returnType->declare_for_stub (os, tab, argName, m_scopedClassname, VT_InParam);

      // populate put args

      os << tab << "DDS::Codec::Param _in_[1] =" << nl;
      os << tab << "{" << nl;
      tab.indent ();
      m_returnType->make_put_param_for_stub (os, tab, argName, VT_InParam);
      tab.outdent ();
      os << tab << "};" << nl;

      // invoke the request
      //
      os << nl;
      os << tab << "invoke_request (_onam_, _olen_, DDS::OP_NORMAL, "
         << "_in_,  1, 0, 0, 0, 0"
         << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   }
   else
   {
      m_returnType->declare_for_stub (os, tab, opret, m_scopedClassname, VT_Return);

      // populate get args

      os << tab << "DDS::Codec::Param _out_[1] =" << nl;
      os << tab << "{" << nl;
      tab.indent ();
      m_returnType->make_get_param_for_stub (os, tab, opret, VT_Return);
      tab.outdent ();
      os << tab << "};" << nl;

      // invoke the request

      os << nl;
      os << tab << "invoke_request (_onam_, _olen_, DDS::OP_NORMAL, 0, "
         << "0, _out_, 1, 0, 0"
         << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

      // return any return argument

      os << nl;

      os << tab << "return " << opret;
      if (m_returnType->IsReturnedByVar ())
      {
         os << "._retn ();" << nl;
      }
      else
      {
         os << ";" << nl;
      }
   }

   tab.outdent();
   os << tab << "}" << nl;
}


// ------------------------------------------------------
//  BE_CLASS_GENERATOR IMPLEMENTATION
// ------------------------------------------------------
const char* be_ClassGenerator::ClassAccesses[be_ClassGenerator::CA_UNDEFINED] = 
{
   "private:",
   "protected:",
   "public:"
};

be_ClassGenerator::be_ClassGenerator()
      :
      access(CA_UNDEFINED)
{}


be_ClassGenerator::be_ClassGenerator(const DDS_StdString& _name_)
      :
      access(CA_UNDEFINED),
      className(_name_)
{}

be_ClassGenerator::be_ClassGenerator(const DDS_StdString& _name_,
                                     const TList<be_ClassParent *> &_parents_,
                                     const TList<be_ClassMember *> &_members_)
      :
      access(CA_UNDEFINED),
      className(_name_)
{
   SetParents(_parents_);
   SetMembers(_members_);
}


be_ClassGenerator::~be_ClassGenerator()
{}

DDS_StdString
be_ClassGenerator::ClassScope() const
{
   return BE_Globals::ScopeOf(scopedClassName);
}

void
be_ClassGenerator::GenerateConstructor(be_ClientHeader& source)
{
   ostream & os = source.Stream();
   pbbool first = pbtrue;
   be_Tab tab(source);
   TList<be_ClassMember *>::iterator mit;

   os << tab << LocalClassName() << "(";

   // PARAMETER LIST

   for (mit = members.begin() ; mit != members.end() ; mit++)
   {
      if (!first)
         os << ", ";

      first = pbfalse;

      os << BE_Globals::RelativeScope(ScopedClassName(), (*mit)->MemberType())
      << "& " << (*mit)->Name();
   }

   os << ")";

   if (members.size())
   {
      // INITIALIZER LIST
      os << nl << tab << ": ";

      first = pbtrue;

      for (mit = members.begin() ; mit != members.end() ; mit++)
      {
         if (!first)
            os << ",";

         first = pbfalse;

         os << (*mit)->name << "(" << (*mit)->name << ")";
      }
   }

   os << "{" << nl;
   os << "}" << nl;
}


void
be_ClassGenerator::GenerateAccessors(be_ClientHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);
   be_ClassMember * member;
   TList<be_ClassMember *>::iterator mit;

   for (mit = members.begin() ; mit != members.end() ; mit++)
   {
      member = (*mit);
      os << tab << "inline " << member->type << " " << member->accessorName
      << "() const { return " << member->name << "; }"
      << nl;
      os << tab << "inline void " << member->accessorName << "("
      << member->type << " _" << member->name << ") { "
      << member->name << " = _" << member->name << "; } " << nl;
   }
}

void
be_ClassGenerator::SetParents(const TList<be_ClassParent *> &_parents_)
{
   TList<be_ClassParent *>::const_iterator pit;

   parents.erase();

   for (pit = _parents_.begin() ; pit != _parents_.end() ; pit++)
   {
      parents.push_back(new be_ClassParent(*(*pit)));
   }
}

void
be_ClassGenerator::SetMembers(const TList<be_ClassMember *> &_members_)
{
   TList<be_ClassMember *>::const_iterator mit;

   members.erase();

   for (mit = _members_.begin() ; mit != _members_.end() ; mit++)
   {
      members.push_back((*mit)->Duplicate());
   }
}

void
be_ClassGenerator::GenerateOpenClassDefinition(be_ClientHeader& source)
{
   ostream & os = source.Stream();
   pbbool first = pbtrue;
   be_Tab tab(source);

   os << tab << "class " << DLLMACRO << LocalClassName() << nl;
   os << tab << ":" << nl;

   source.Indent();

   // DECLARE INHERITANCE
   TList<be_ClassParent *>::iterator pit;

   for (pit = parents.begin() ; pit != parents.end() ; pit++)
   {
      if (!first)
         os << ",\n";

      first = pbfalse;

      os << tab;

      if ((*pit)->isVirtual)
      {
         os << "virtual ";
      }

      os << "public " << BE_Globals::RelativeScope(ScopedClassName(), (*pit)->scopedName);
   }

   os << nl;
   source.Outdent();
   os << tab << "{ " << nl;
   source.Indent();
}


//Insert here to generate function to return hierachy for class


void be_ClassGenerator::GenerateHierachySearch (be_ClientImplementation& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);
   DDS_StdString objectTest;

   // DECLARE INHERITANCE
   TList<be_ClassParent *>::iterator pit;
    
    pit = parents.begin();
    objectTest = BE_Globals::RelativeScope(ScopedClassName(),(*pit)->scopedName);

   if(strcmp((char *)objectTest,"DDS::Stub") != 0 && 
      strcmp((char *)objectTest,"DDS::LocalObject") != 0 )
    {
       int NestedBaseNum;
       NestedBaseNum = 0;

      for (pit = parents.begin() ; pit != parents.end() ; pit++)
      { 
         
      //Produce the hierachy chart here
      //In this form where Class is calculated from the hierachy
      
           //generate hierachy here for the local interface
           
          DDS_StdString name = BE_Globals::RelativeScope(ScopedClassName(),(*pit)->scopedName);

          if (strcmp (name,"DDS::LocalObject") != 0)
          {
            NestedBaseNum++;
            os << tab << "   typedef " << name << " NestedBase_" << NestedBaseNum << ";" << nl;           
            os << tab << nl;
           
            os << tab << "   if (NestedBase_" << NestedBaseNum << "::";
            if ((*pit)->local ())
            {
               os << "_local";
            }
            os << "_is_a (_id))" << nl;
            os << tab << "   {" << nl;
            os << tab << "      return true;" << nl;
            os << tab << "   }" << nl;
            os << tab << nl;
          }
        }
      }
}

//end insert

void be_ClassGenerator::GenerateClassDeclarations (be_ClientHeader& source)
{
}

void be_ClassGenerator::SetAccess (be_Source& source, ClassAccess newAccess)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   if (newAccess != access)
   {
      tab.outdent ();
      os << tab << ClassAccesses[newAccess] << nl;
      tab.indent ();

      access = newAccess;
   }
}


void
be_ClassGenerator::GenerateMemberDeclarations(be_ClientHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);
   TList<be_ClassMember *>::iterator mit;

   for (mit = members.begin() ; mit != members.end() ; mit++)
   {
      os << tab << BE_Globals::RelativeScope(ScopedClassName(), (*mit)->type)
      << " " << (*mit)->name << ";" << nl;
   }
}


void
be_ClassGenerator::GenerateCloseClassDefinition(be_ClientHeader& source)
{
   be_Tab tab(source);

   tab.outdent();
   source.Stream() << tab << "};" << nl;
}


void
be_ClassGenerator::GenerateDestructor(be_ClientHeader& source)
{
   be_Tab tab(source);

   source.Stream() << tab << "~" << LocalClassName()
   << "()" << "{" << nl << "}" << nl;
}


void
be_ClassGenerator::Generate(be_ClientHeader& source)
{
   GenerateOpenClassDefinition(source);
   GenerateClassDeclarations(source);
   SetAccess(source, CA_PRIVATE);
   GenerateMemberDeclarations(source);
   SetAccess(source, CA_PUBLIC);
   GenerateConstructor(source);
   GenerateAccessors(source);
   GenerateDestructor(source);
   GenerateCloseClassDefinition(source);
}

void
be_ClassGenerator::GenerateDestructor(be_ClientImplementation& source)
{
   be_Tab tab(source);

   source.Stream() << tab << ScopedClassName() << "::~" << LocalClassName() << "(){}" << nl;
}
