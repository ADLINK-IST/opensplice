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
#include "os_stdlib.h"
#include "idl.h"
#include "idl_extern.h"
#include "xbe.h"
#include "xbe_literals.h"
#include "xbe_root.h"
#include "xbe_operation.h"
#include "xbe_attribute.h"
#include "xbe_interface.h"
#include "xbe_invoke.h"
#include "xbe_invoke.h"

void be_interface::GeneratePtrAndRef
(
   be_ClientHeader & source,
   const DDS_StdString & scope,
   const DDS_StdString & localName,
   pbbool needsForwardDecl
)
{
   ostream& os = source.Stream ();
   be_Tab tab (source);
   DDS_StdString ptrName = localName + "_ptr";

   // forward declare the class

   //if (not already forward declared at this scope)

   if (needsForwardDecl)
   {
      os << nl;
      os << tab << "class " << DLLMACRO << localName << ";" << nl;
   }

   // typedef the _ptr type

   os << nl;
   os << tab << "typedef " << localName << " * " << ptrName << ";" << nl;
}

void be_interface::GenerateVarOutAndMgr
(
   be_ClientHeader & source,
   const DDS_StdString & scope,
   const DDS_StdString & localName
)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   os << tab << "typedef DDS_DCPSInterface_var < " << localName << "> " << localName << "_var;" << nl;
   os << tab << "typedef DDS_DCPSInterface_out < " << localName << "> " << localName << "_out;" << nl;
   os << nl;
}

void be_interface::GenerateInheritanceInit
(
   ostream& os,
   String_map& ancestors,
   const char* args
)
{
   be_interface * ip;
   DDS_StdString ancestor = ScopedName ();

   for (int i = 0; i < n_inherits (); ++i)
   {
      ip = be_interface::_narrow (inherits()[i]);
      assert (ip);

      ip->GenerateInheritanceInit (os, ancestors, args);
      ancestor = ip->ScopedName ();
      if (ancestors.find (ancestor) == ancestors.end ())
      {
         os << "," << nl;
         os << "   DDS_IDLC_" << NoColons (ancestor) << "_stub (" << args
            << ")";
         ancestors[ancestor] = ancestor;
      }
   }
}

void be_interface::GenerateObjectVirtuals (be_ClientHeader& source)
{
}

void be_interface::GenerateObjectVirtuals (be_ClientImplementation & source)
{
}

void be_interface::DefineStubOperations (be_ClientHeader& source)
{
   ostream & os = source.Stream ();
   be_Tab tab(source);
   UTL_ScopeActiveIterator * it;
   AST_Decl * d;

   if (nmembers ())
   {
      os << nl << tab << "// Stub operations" << nl << nl;

      it = new UTL_ScopeActiveIterator(this, UTL_Scope::IK_decls);

      while (!(it->is_done()))
      {
         be_operation * op;
         be_attribute * at;

         d = it->item();

         op = (be_operation*)d->narrow((long) & be_operation::type_id);
         if (op)
         {
            os << tab << "virtual "
            << op->StubSignature(be_operation::OP_Declaration)
            << ";" << nl;
         }
         else if ((at = (be_attribute*)d->narrow((long) & be_attribute::type_id)))
         {
            os << tab << "virtual "
            << at->GetSignature(be_attribute::AT_Declaration,
                                Scope(baseClassname))
            << ";" << nl;

            if (!at->readonly())
            {
               os << tab << "virtual "
               << at->SetSignature(be_attribute::AT_Declaration,
                                   Scope(baseClassname), pbtrue)
               << ";" << nl;
            }
         }

         it->next();
      }

      delete it;
   }
}

void be_interface::GenerateStubDefinition (be_ClientHeader & source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   // declare stub class

   os << nl;
   os << tab << "class " << DLLMACRO << StubClassname() << nl;
   os << tab << ":" << nl;
   tab.indent();
   os << tab << "virtual public " << BaseClassname();

   // declare inheritance

   for (int i = 0; i < n_inherits(); ++i)
   {
      be_interface *ip;

      ip = be_interface::_narrow(inherits()[i]);
      assert(ip);
      os << ",\n" << tab << "virtual public ";
      os << BE_Globals::RelativeScope (Scope(BaseClassname()), ip->Scope(ip->StubClassname()));
   }

   os << nl;
   tab.outdent ();

   // open class definition

   os << tab << "{" << nl;
   os << tab << "public:" << nl;
   tab.indent ();

   // generate constructor/destructor

   os << tab << StubClassname () << " (IOP::IOR_ptr ior = 0);" << nl;
   os << tab << StubClassname () << " (DDS::Object_ptr p);" << nl;
   os << tab << "~" << StubClassname() << " () {}" << nl;

   // define stub operations

   DefineStubOperations (source);

   tab.outdent ();
   os << tab << "};" << nl << nl;
}

void be_interface::DeclareStaticDispatchers
(
   be_ServerHeader& source,
   const char * servantClass
)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);
   be_OpNameSet::Be_OpMap::iterator it;
   be_operation * op;

   LoadAllOps ();

   for (it = m_lops.begin (); it != m_lops.end (); it++)
   {
      if (it.valid ())
      {
         op = be_operation::_narrow ((AST_Decl*)(*it).m_opdecl);
         if (op)
         {
            os << tab << "static void _dispatch_" << op->LocalName ();
         }
         else
         {
            os << tab << "static void _dispatch_" << (*it).m_opname;
         }
         os << nl;
         os << tab << "(" << nl;
         tab.indent ();
         os << tab << "void * _servant_," << nl;
         os << tab << "DDS::Codec::Request & _request_" << nl;
         if (XBE_Ev::generate ())
         {
            os << tab << XBE_Ev::arg (XBE_ENV_ARGN, false) << nl;
         }
         tab.outdent ();
         os << tab << ");" << nl;
      }
   }
}

void be_interface::GenerateImplDefinition (be_ServerHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   os << tab << "class " << DLLMACRO;

   if (localName == Scope(localName))
   {
      os << DDSPOAImplPrefix;
   }

   os << localName << nl;
   os << tab << ":" << nl;
   source.Indent();

   if (n_inherits() < 1)
   {
      os << tab << "virtual public PortableServer::ServantBase" << nl;
   }
   else
   {
      for (int i = 0; i < n_inherits(); ++i)
      {
         be_interface *ip;
         assert(inherits()[i]);
         ip = be_interface::_narrow(inherits()[i]);
         assert(ip);
         os << tab << "virtual public ";
         os << ip->ImplClassname();

         if (i < (n_inherits() - 1))
         {
            os << ",";
         }

         os << nl;
      }
   }

   source.Outdent();
   os << tab << "{" << nl;
   source.Indent();
   source.SetAccess(be_Source::PublicAccess);
   os << nl;

   // _repository_id

   os << tab << "virtual const char * _repository_id () const;"
      << nl;

   // _is_a
   os << tab << "DDS::Boolean _is_a (const char * id);" << nl;

   // _this

   os << tab << ScopedName () << "_ptr _this ("
      << XBE_Ev::arg (XBE_ENV_ARG1) << ");" << nl;

   GenerateVirtualMFs (source, m_implClassname);

   source.SetAccess (be_Source::ProtectedAccess);

   // DECLARE static dispatchers
   DeclareStaticDispatchers(source, m_implClassname);

   // Declare _invoke ()

   os << tab << "virtual DDS::Codec::DispatchFN _invoke (DDS::Codec::Request & _req_"
      << XBE_Ev::arg (XBE_ENV_ARGN) << ");" << nl;

   source.Outdent ();
   os << tab << "};" << nl << nl;
}

void be_interface::GenerateDirectImplDefinition (be_ServerHeader & source)
{  
   ostream & os = source.Stream ();
   be_Tab tab (source);
   DDS_StdString tmplt = localName + "_direct_stub";

   if (localName == Scope(localName))
   {
      tmplt = DDSPOAImplPrefix + tmplt;
   }

   os << tab << "class " << DLLMACRO;

   os << tmplt << nl;
   os << tab << ":" << nl;
   source.Indent();
   os << tab << "virtual public " << ScopedName();
   os << nl;

   source.Outdent();
   os << tab << "{" << nl;
   source.Indent();
   source.SetAccess(be_Source::PublicAccess);
   os << nl;

   // Direct Stub Public Constructors
   os <<  tab << tmplt
      << " (IOP::IOR_ptr ior);" << nl;
   os << tab << tmplt
      << " (DDS::Object_ptr p);" << nl;
   os << tab << tmplt
      << " (DDS::Object_ptr p, " << m_implClassname << " * c);" << nl;
   
   // Direct Stub Destructor
   os <<  tab << "~" << tmplt << " ();" << nl;

   // Direct Stub operations
   DefineDirectStubOperations(source);
 
   source.SetAccess (be_Source::ProtectedAccess);

   // Direct Stub Protected Constructors
   os << tab << tmplt << " () {};" << nl;

   source.SetAccess (be_Source::PrivateAccess);

   // Direct Stub Private data
   os << tab << m_implClassname << " * m_servant;" << nl;

   source.Outdent ();
   os << tab << "};" << nl << nl;
}  

void be_interface::DefineDirectStubOperations (be_ServerHeader& source)
{
   ostream & os = source.Stream ();
   be_Tab tab(source);
   Be_OpMap::iterator it;
   AST_Decl * d;

   for (it = m_ops.begin(); it != m_ops.end(); it++)
   {
      if (it.valid())
      {
         be_operation * op;
         be_attribute * at;

         d = (AST_Decl*)(*it).m_opdecl;

         op = (be_operation*)d->narrow((long) & be_operation::type_id);
         if (op)
         {
            os << tab << "virtual "
               << op->DirectSignature
                      (
                         be_operation::OP_Declaration,
                         m_dirstubClassname
                      )
               << ";" << nl;
         }
         else if ((at = (be_attribute*)d->narrow((long) & be_attribute::type_id)))
         {
            if ((*it).m_opname[(unsigned)1] == 'g')   // get attribute
            {
               os << tab << "virtual "
                  << at->GetSignature(be_attribute::AT_TieImplementation,
                                      Scope(baseClassname))
                  << ";" << nl;
            }
            else if ((*it).m_opname[(unsigned)1] == 's')  // set attribute
            {
               os << tab << "virtual "
                  << at->SetSignature(be_attribute::AT_TieImplementation,
                                      Scope(baseClassname), pbtrue)
                  << ";" << nl;
            }
         }
      }
   }
}

void be_interface::GeneratePOATypedef (be_ServerHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   os << tab << "typedef " << m_implClassname << " " << localName << ";" << nl;
}

unsigned short be_interface::CountDispatchTableEntries ()
{
   unsigned short opCount = 0;
   UTL_ScopeActiveIterator *it;

   it = new UTL_ScopeActiveIterator(this, UTL_Scope::IK_decls);

   while (!(it->is_done()))
   {
      be_operation * op;
      be_attribute * at;
      AST_Decl * d;

      d = it->item();

      op = (be_operation*)d->narrow((long) & be_operation::type_id);
      if (op)
      {
         opCount++;
      }
      else if ((at = (be_attribute*)d->narrow((long) & be_attribute::type_id)))
      {
         opCount++;

         if (!at->readonly())
         {
            opCount++;
         }
      }

      it->next();
   }

   delete it;

   return opCount;
}

void be_interface::GenerateInvoke (be_ServerImplementation & source)
{
   LoadAllOps ();
   be_Invoke invoke (m_implClassname, m_ops);
   invoke.Generate (source);
}

void be_interface::LoadAllOps ()
{
   if (m_allOpsLoaded)
   {
      return;
   }

   LoadOpTable (m_implClassname, m_ops, pbtrue);
   LoadOpTable (m_implClassname, m_lops, pbfalse);
   m_allOpsLoaded = true;
}

void be_interface::LoadOpTable
(
   DDS_StdString & mdiImpl,
   be_OpNameSet & ops,
   pbbool all
)
{
   UTL_ScopeActiveIterator* it;
   be_operation* op;
   be_attribute* at;
   AST_Decl* decl;
   DDS_StdString opname;
   DDS_StdString opdisp;
   DDS_StdString opdispName;
   DDS_StdString dispatchPrefix = mdiImpl + os_strdup ("::_dispatch_");

   //
   // iterate through all operations and attributes in this scope
   //
   it = new UTL_ScopeActiveIterator (this, UTL_Scope::IK_decls);

   while (!(it->is_done()))
   {
      decl = it->item();

      op = be_operation::_narrow (decl);
      if (op)
      {
         opname = op->LocalName();
         opdisp = dispatchPrefix + op->LocalName();
         opdispName = op->local_name()->get_string();
         ops.AddOpName (mdiImpl, opname, opdisp, opdispName, decl);
      }
      else if ((at = be_attribute::_narrow (decl)))
      {
         opname = (DDS_StdString)"_get_" + at->LocalName();
         opdispName = (DDS_StdString)"_get_" + at->local_name()->get_string();
         opdisp = dispatchPrefix + "_get_" + at->LocalName();

         ops.AddOpName (mdiImpl, opname, opdisp, opdispName, decl);

         if (!at->readonly())
         {
            opname = (DDS_StdString)"_set_" + at->LocalName();
            opdispName = (DDS_StdString)"_set_" + at->local_name()->get_string();

            opdisp = dispatchPrefix + "_set_" + at->LocalName();

            ops.AddOpName (mdiImpl, opname, opdisp, opdispName, decl);
         }
      }

      it->next();
   }

   delete it;
 
   //
   // now add all operations and attributes from parents' scopes
   //

   if (all)
   {
      for (int i = 0; i < n_inherits(); i++)
      {
         be_interface * ip;

         ip = be_interface::_narrow (inherits()[i]);
         assert(ip);
         ip->LoadOpTable (ip->m_implClassname, ops, all);
      }
   }
}

void be_interface::GenerateImplImplementations (be_ServerImplementation & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   // get_repository_id

   os << "const char * " << m_implClassname << "::_repository_id () const" << nl
      << "{" << nl;
   if (local() == I_TRUE)
   {
      os << "   return " << m_implClassname << "::_local_id;" << nl;
   }
   else
   {
      UTL_String * repID = get_decl_pragmas().get_repositoryID();
      assert(repID);

      os << "   return (const char*) \"" << repID->get_string() << "\";" << nl;
   }
   os << "}" << nl << nl;

   // _this

   os << ScopedName () << "_ptr " << m_implClassname << "::_this ("
      << XBE_Ev::arg (XBE_ENV_ARG1) << ")" << nl
      << "{" << nl
      << "   DDS::Object_var obj (_this_ ("
      << XBE_Ev::arg (XBE_ENV_VAR1) << "));" << nl
      << "   return (" << ScopedName() << "::_narrow (obj.in ()"
      << XBE_Ev::arg (XBE_ENV_VARN) << "));" << nl
      << "}" << nl << nl;

   GenerateLocalIsA (source);

   GenerateRequestDispatchers (source, m_implClassname);
   GenerateInvoke (source);
}

void be_interface::GenerateLocalIsA (be_ServerImplementation& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   os << tab << "DDS::Boolean" << " " << m_implClassname <<
   "::_is_a (const char * _id)" << nl;
   os << tab << "{" << nl;
   os << tab << "   if (strcmp (_id, " << m_implClassname
      << "::_local_id) == 0)" << nl;
   os << tab << "   {" << nl;
   os << tab << "      return true;" << nl;
   os << tab << "   }" << nl;
   os << tab << nl;
   tab.outdent ();
   
   for (int i = 0; i < n_inherits(); ++i)
   {
      be_interface *ip;

      ip = be_interface::_narrow(inherits()[i]);
      assert(ip);

      os << tab << "   if (";
      os << ip->ImplClassname();
      os << "::_is_a (_id))" << nl;
      os << tab << "   {" << nl;
      os << tab << "      return true;" << nl;
      os << tab << "   }" << nl;
      os << nl;
   }
   
   os << tab << "   return false;" << nl;
   os << tab << "}" << nl << nl;
}

void be_interface::GenerateStubImplementations 
   (be_ClientImplementation & source)
{
   ostream & os = source.Stream();
   DDS_StdString rel;
   String_map ancestors(idlc_hash_str);

   os << nl;

   // stub constructor

   os << Scope (stubClassname) << "::" << stubClassname 
      << " (IOP::IOR_ptr ior) :" << nl;
   os << "   DDS_IDLC_DDS_Object (ior)";

   GenerateInheritanceInit (os, ancestors, "ior");

   os << nl;
   os << "{" << nl;
   os << "}" << nl;
   os << nl;

   os << nl;

   // stub constructor

   os << Scope (stubClassname) << "::" << stubClassname 
      << " (DDS::Object_ptr p) :" << nl;
   os << "   DDS_IDLC_DDS_Object (*p)";

   ancestors.erase ();
   GenerateInheritanceInit (os, ancestors, "p");

   os << nl;
   os << "{" << nl;
   os << "}" << nl;
   os << nl;
}

void be_interface::GenerateDirectStubImplImplementations
   (be_ServerImplementation & source)
{  
   ostream & os = source.Stream();
   DDS_StdString rel;
   String_map ancestors(idlc_hash_str);
   DDS_StdString dirstub = localName + "_direct_stub";
   be_Tab tab(source);

   if (localName == Scope(localName))
   {
      dirstub = DDSPOAImplPrefix + dirstub;
   }

   os << nl;

   // stub constructor

   os << (const char*) m_dirstubClassname << "::" << (const char*) dirstub
      << " (IOP::IOR_ptr ior) :" << nl;
   os << "   DDS_IDLC_DDS_Object (ior), " << nl;
   os << "   m_servant(NULL)";
   os << nl;
   os << "{" << nl;
   os << "}" << nl;
   os << nl;

   os << nl;

   // stub constructor

   os << (const char*) m_dirstubClassname << "::" << (const char*) dirstub
      << " (DDS::Object_ptr p) :" << nl;
   os << "   DDS_IDLC_DDS_Object (*p)," << nl;
   os << "   m_servant(NULL)";
   os << nl;
   os << "{" << nl;
   os << "}" << nl;
   os << nl;

   // stub constructor

   os << (const char*) m_dirstubClassname << "::" << (const char*) dirstub
      << " (DDS::Object_ptr p, " << m_implClassname << " * servant) :"
      << nl;
   os << "   DDS_IDLC_DDS_Object (*p)," << nl;
   os << "   m_servant(servant)";
   os << nl;
   os << "{" << nl;
   os << "}" << nl;
   os << nl;

   // stub destructor

   os << (const char*) m_dirstubClassname
      << "::~"
      << (const char*) dirstub
      << " ()"
      << nl;
   os << "{" << nl;
   source.Indent();
   os << tab << "if (m_servant)" << nl;
   os << tab << "{" << nl;
   source.Indent();
   os << tab << "m_servant->_remove_ref ();" << nl;
   source.Outdent();
   os << tab << "}" << nl;
   source.Outdent();
   os << "}" << nl;
   os << nl;

   // Stub operations

   Be_OpMap::iterator it;

   // 
   // iterate through all supported (and inherited) operations
   // and attributes
   //

   for (it = m_ops.begin(); it != m_ops.end(); it++)
   {
      if (it.valid())
      {
         be_operation * op;
         be_attribute * at;
         AST_Decl * d = (AST_Decl*)(*it).m_opdecl;

         if (d)
         {
            op = be_operation::_narrow (d);
            if (op)
            {
               os << tab
                  << op->DirectSignature(be_operation::OP_Implementation,
                                         m_dirstubClassname)
                  << nl; 
               os << tab << "{" << nl;
               source.Indent();

               os << tab;

               if (op->HasReturn())
               {
                  os << "return ";
               }

               os << "m_servant->" << op->TieSignature(be_operation::OP_Invoke) << ";" << nl;
               source.Outdent();

               os << tab << "}" << nl << nl;
            }
            else if ((at = be_attribute::_narrow (d)))
            {
               if ((*it).m_opname[(unsigned)1] == 'g')   // get attribute
               {
                  os << tab << at->GetSignature(be_attribute::AT_Implementation, m_dirstubClassname) << nl;
                  os << tab << "{" << nl;
                  source.Indent();

                  os << tab << "return m_servant->" << at->GetSignature(be_attribute::AT_Invoke, "") << ";" << nl;
                  source.Outdent();
                  os << tab << "}" << nl;
               }
               else if ((*it).m_opname[(unsigned)1] == 's')  // set attribute
               {
                  os << tab << at->SetSignature(be_attribute::AT_Implementation, m_dirstubClassname, pbtrue, "_attribute_") << nl;
                  os << tab << "{" << nl;
                  source.Indent();

                  os << tab << "m_servant->" << at->SetSignature(be_attribute::AT_Invoke, "", pbtrue, "_attribute_") << ";" << nl;
                  source.Outdent();
                  os << tab << "}" << nl;
               }
            }
         }
         else
         {
            // NOTE: object operations will not have decls 
            // cerr << "no decl for " << (*it).m_opname << endl;
         }
      }
   }
}

void
be_interface::GenerateTie(be_ServerHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   DDS_StdString tmplt = (DDS_StdString) "POA_" + NoColons(ScopedName()) + "_tie";

   // DECLARE CLASS AND INHERITANCE
   os << nl;
   os << tab << "template<class T> ";
   os << tab << "class " << (const char*)tmplt << nl;
   os << tab << ":" << nl;
   source.Indent();

   os << tab << "virtual public " << m_implClassname << nl;
   source.Outdent();
   os << tab << "{" << nl;
   source.Indent();

   source.SetAccess(be_Source::PublicAccess);

   // PUBLIC CONSTRUCTORS
   os << tab << (const char*)tmplt << "() {}" << nl;

   GenerateTieConstructor(source, tmplt,
                          "T& t",
                          "m_tie(&t), m_poa(PortableServer::POA::_nil()), m_rel(0)");

   GenerateTieConstructor(source, tmplt,
                          "T& t, PortableServer::POA_ptr poa",
                          "m_tie(&t), m_poa(PortableServer::POA::_duplicate(poa)), m_rel(0)");

   GenerateTieConstructor(source, tmplt,
                          "T* tp, DDS::Boolean release = 1",
                          "m_tie(tp), m_poa(PortableServer::POA::_nil()), m_rel(release)");

   GenerateTieConstructor(source, tmplt,
                          "T* tp, PortableServer::POA_ptr poa, DDS::Boolean release = 1",
                          "m_tie(tp), m_poa(PortableServer::POA::_duplicate(poa)), m_rel(release)");

   if (n_inherits() == 0)
   {
      // Destructor

      os << nl << tab << "~" << (const char*)tmplt << " ()" << nl;
      os << tab << "{" << nl;
      source.Indent();
      os << tab << "DDS::release (m_poa);" << nl;
      os << tab << "if (m_rel) delete m_tie;" << nl;
      source.Outdent();
      os << tab << "}" << nl;

      // _default_POA

      os << tab << "virtual PortableServer::POA_ptr _default_POA ()" << nl;
      os << tab << "{" << nl;
      source.Indent ();
      os << tab << "typedef PortableServer::ServantBase base;" << nl;
      os << tab << "return (m_poa) ? m_poa : base::_default_POA ();" << nl;
      source.Outdent ();
      os << tab << "}" << nl;

      // Tie specific functions

      os << nl << tab << "// tie-specific functions" << nl;
      os << tab << "T* _tied_object() { return m_tie; }" << nl;
      os << tab << "void _tied_object(T& obj)" << nl;
      os << tab << "{" << nl;
      source.Indent ();
      os << tab << "if (m_rel) delete m_tie;" << nl;
      os << tab << "m_tie = &obj;" << nl;
      os << tab << "m_rel = 0;" << nl;
      source.Outdent ();
      os << tab << "}" << nl;

      os << tab << "void _tied_object(T* obj, DDS::Boolean release = 1)" << nl;
      os << tab << "{" << nl;
      source.Indent();
      os << tab << "if (m_rel) delete m_tie;" << nl;
      os << tab << "m_tie = obj;" << nl;
      os << tab << "m_rel = release;" << nl;
      source.Outdent();
      os << tab << "}" << nl;

      os << tab << "DDS::Boolean is_owner() { return m_rel; }" << nl;
      os << tab << "void _is_owner(DDS::Boolean b) { m_rel = b; }" << nl;
   }

   // Virtuals

   os << nl << tab << "// IDL functions" << nl;

   DefineTieOperations (source);

   // Data members

   os << nl;

   source.SetAccess(be_Source::PrivateAccess);

   os << tab << "T* m_tie;" << nl;
   os << tab << "PortableServer::POA_ptr m_poa;" << nl;
   os << tab << "DDS::Boolean m_rel;" << nl;

   // Private copy constructor and assignment operator

   os << nl;

   os << tab << "// copy constructor and assignment not allowed" << nl;
   os << tab << (const char*)tmplt << "(" << (const char*)tmplt << "&);" << nl;
   os << tab << "void operator=(const " << (const char*)tmplt << "&);" << nl;

   // VC++2.2 DOMINANCE BUG
   // doesn't like m-inherited tie implementations.
   // macro does nothing for other platforms.
   // os << nl << tab << "WIN32_DOMINANCE_BUG_WORKAROUND("
   //  << m_implClassname << ")" << nl;

   source.Outdent();
   os << tab << "};" << nl;
   os << nl;
}

void be_interface::GenerateTieConstructor
(
   be_ServerHeader& source,
   DDS_StdString& tmplt,     // name of tie template class
   const char* signature,     // arguments to constructor
   const char* uninheritedInits // mem-initializers if tie class does
) 
{
   ostream& os = source.Stream();
   be_Tab tab(source);

   os << nl << tab << tmplt << "(" << signature << ")";
   source.Indent();
   os << nl << tab << ": " << uninheritedInits << " {}" << nl;
   source.Outdent();
}

void be_interface::DefineTieOperations (be_ServerHeader & source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);
   Be_OpMap::iterator it;

   //
   // iterate through all supported (and inherited) operations
   // and attributes
   //

   for (it = m_ops.begin(); it != m_ops.end(); it++)
   {
      if (it.valid())
      {
         be_operation * op;
         be_attribute * at;
         AST_Decl * d = (AST_Decl*)(*it).m_opdecl;

         if (d)
         {
            op = be_operation::_narrow (d);
            if (op)
            {
               os << tab << "virtual " << op->TieSignature(be_operation::OP_Implementation) << nl;
               os << tab << "{" << nl;
               source.Indent();

               os << tab;

               if (op->HasReturn())
               {
                  os << "return ";
               }

               os << "m_tie->" << op->TieSignature(be_operation::OP_Invoke) << ";" << nl;
               source.Outdent();

               os << tab << "}" << nl;
            }
            else if ((at = be_attribute::_narrow (d)))
            {
               if ((*it).m_opname[(unsigned)1] == 'g')   // get attribute
               {
                  os << tab << "virtual " << at->GetSignature(be_attribute::AT_TieImplementation, "") << nl;
                  os << tab << "{" << nl;
                  source.Indent();

                  os << tab << "return m_tie->" << at->GetSignature(be_attribute::AT_TieInvoke, "") << ";" << nl;
                  source.Outdent();
                  os << tab << "}" << nl;
               }
               else if ((*it).m_opname[(unsigned)1] == 's')  // set attribute
               {
                  os << tab << "virtual " << at->SetSignature(be_attribute::AT_TieImplementation, "", pbtrue, "_attribute_") << nl;
                  os << tab << "{" << nl;
                  source.Indent();

                  os << tab << "m_tie->" << at->SetSignature(be_attribute::AT_TieInvoke, "", pbtrue, "_attribute_") << ";" << nl;
                  source.Outdent();
                  os << tab << "}" << nl;
               }
            }
         }
         else
         {
            // NOTE: object operations will not have decls
            // cerr << "no decl for " << (*it).m_opname << endl;
         }
      }
   }
}
