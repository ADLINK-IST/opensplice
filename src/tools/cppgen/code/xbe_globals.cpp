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
#include "idl_extern.h"
#include "xbe_globals.h"
#include "xbe_utils.h"
#include "xbe_scopestack.h"

#define DDS_DEBUG_SCOPING 0

be_CppScopeStack g_cppScopeStack;
be_CppScopeStack g_feScopeStack;

const DDS_StdString NilString;

DDS_StdString BE_Globals::TCPrefix = "_tc_";
DDS_StdString BE_Globals::MTPrefix = "xps_mt_";
DDS_StdString BE_Globals::TCBasePrefix = "_tcbase_";
DDS_StdString BE_Globals::TCRepPrefix = "_tcrep_";
DDS_StdString BE_Globals::UserDLL = "";
DDS_StdString BE_Globals::UserDLLHeader = "";
DDS_StdString BE_Globals::DLLExtension = "";
DDS_StdString BE_Globals::hExtension = "h";
DDS_StdString BE_Globals::CExtension = "cpp";
DDS_StdString BE_Globals::ClientExtension = "";
DDS_StdString BE_Globals::ServerExtension = "_s";
DDS_StdString BE_Globals::TieExtension = "_t";
DDS_StdString BE_Globals::OutputDirectory = "";
DDS_StdString BE_Globals::ClientHeaderFilename;
DDS_StdString BE_Globals::ClientImplFilename;
DDS_StdString BE_Globals::ServerHeaderFilename;
DDS_StdString BE_Globals::ServerImplFilename;
DDS_StdString BE_Globals::TieHeaderFilename;
DDS_StdString BE_Globals::BaseFilename;

const int BE_Globals::clientMax = 6;
const int BE_Globals::serverMax = 6; // WINDOWS BASE (8) - _s
const int BE_Globals::MaxFileNameLen = 80;

const char * BE_Globals::IDLExtension = ".idl";

pbbool BE_Globals::gen_gui_info = pbfalse;
pbbool BE_Globals::generate_model = pbfalse;
pbbool BE_Globals::client_only = pbtrue;
pbbool BE_Globals::no_exceptions = pbtrue;
pbbool BE_Globals::nesting_bug = pbfalse;
pbbool BE_Globals::short_filenames = pbfalse;
pbbool BE_Globals::portable_exceptions = pbfalse;
pbbool BE_Globals::per_request_attrs = pbfalse;
pbbool BE_Globals::gen_externalization = pbfalse;
pbbool BE_Globals::inc_any = pbfalse;
pbbool BE_Globals::gen_onefile = pbfalse;
pbbool BE_Globals::HFileOpen = pbfalse;
pbbool BE_Globals::CFileOpen = pbfalse;
pbbool BE_Globals::map_wide = pbfalse;
pbbool BE_Globals::case_sensitive = pbfalse;
pbbool BE_Globals::ignore_interfaces = pbfalse;
pbbool BE_Globals::isocpp = pbfalse;
pbbool BE_Globals::isocpp_new_types = pbfalse;
pbbool BE_Globals::gen_equality = pbfalse;
pbbool BE_Globals::isocpp_test_methods = pbfalse;
pbbool BE_Globals::collocated_direct = pbfalse;

unsigned long BE_Globals::max_char_per_line = 1024;

const char* BE_Globals::_keywords[] =
{
   "and",
   "and_eq",
   "asm ",
   "auto",
   "bitand",
   "bitor",
   "bool",
   "break",
   "case",
   "catch",
   "char",
   "class",
   "compl",
   "const",
   "const_cast",
   "continue",
   "default",
   "delete",
   "do",
   "double",
   "dynamic_cast",
   "else",
   "enum",
   "export",
   "extern",
   "false",
   "float",
   "for",
   "friend",
   "goto",
   "if",
   "inline",
   "int",
   "long",
   "mutable",
   "namespace",
   "new",
   "not",
   "not_eq",
   "operator",
   "or",
   "or_eq",
   "private",
   "protected",
   "public",
   "register",
   "reinterpret_cast",
   "return",
   "short",
   "signed",
   "sizeof",
   "static",
   "static_cast",
   "struct",
   "switch",
   "template",
   "this",
   "throw",
   "true",
   "try",
   "typedef",
   "typeid",
   "union",
   "unsigned",
   "using",
   "virtual",
   "void",
   "volatile",
   "wchar_t",
   "while",
   "xor",
   "xor_eq",
   ""
};

void BE_Globals::Initialize ()
{
}

DDS_StdString BE_Globals::CorbaScope (const DDS_StdString& name)
{
   return Scope ("DDS", name);
}

DDS_StdString BE_Globals::Scope
(
   const DDS_StdString & enclosingScope,
   const DDS_StdString & name
)
{
   DDS_StdString ret (enclosingScope);
   DDS_StdString sep = "::";

   if (ret.length ())
   {
      ret += sep;
   }

   return ret + name;
}

DDS_StdString BE_Globals::OuterMostScope (const DDS_StdString& scope)
{
   DDS_StdString ret = (const char *) scope;

   if (scope.length ())
   {
      char * pos = (char*)strchr((const char *)ret, ':');

      if (pos)
      {
         *pos = (char)0;
      }
   }

   return ret;
}

// Remove overlapping scope

DDS_StdString BE_Globals::resolveScope
(
   const DDS_StdString & withinScope,
   const DDS_StdString & enclosingScope,
   const DDS_StdString & name
)
{
   DDS_StdString scope;
   DDS_StdString wScope (withinScope);
   DDS_StdString eScope (enclosingScope);
   bool common = false;

#if DDS_DEBUG_SCOPING
   cout << "Name: " << name
        << " Within: " << withinScope
        << " Enclosing: " << enclosingScope << endl;
#endif

   char * wp = (char*) wScope;
   char * ep = (char*) eScope;

   // Check for common scoping

   if (wp && ep)
   {
      DDS_StdString wscope = OuterMostScope (wp);
      DDS_StdString escope = OuterMostScope (ep);

      while ((wscope == escope) && wscope.length ())
      {
         common = true;

#if defined(_WIN32)
#pragma warning( disable : 4390 )
#endif

         // Advance ptrs to next label or end of string

         if ((wp = strchr(wp, ':')) && *(wp++) && *(wp++)) ;
         if ((ep = strchr(ep, ':')) && *(ep++) && *(ep++)) ;

         wscope = OuterMostScope (wp);
         escope = OuterMostScope (ep);
      }

      /*
         The following test is actually too strict. If we have type
         AA::BB::CC::Type used in module AA::BB if can be referred to
         as CC::Type, but when used in module AA::CC it must be fully
         scoped. It is not enough to simply check for a common scoping
         prefix. TBD. - Steve
      */

      // Are the last parts the same?

      if (wp && ep && (strcmp (wp, ep) != 0))
      {
         common = false;
         ep = (char*) eScope;
      }
   }
   scope = Scope (ep, name);

   /*
      To disambiguate types from differently scoped modules with the
      same name. Scoped types need to be made absolute with a :: prefix
      if they cannot be resolved within a common containing scope.

      Note: Some type names have 'const' embedded in them which needs
      to be checked. This needs fixing. - Steve
   */

   if
   (
      !common&&
      (withinScope != "") &&
      (enclosingScope != "") &&
      (enclosingScope != "DDS") &&
      (enclosingScope != "const DDS") &&
      (strchr ((const char *) scope, ':') != NULL) &&
      (strncmp ((const char *) scope, "::", 2) != 0) &&
      (strncmp ((const char *) scope, "const ", 6) != 0)
   )
   {
      scope = "::" + scope;
   }

#if DDS_DEBUG_SCOPING
   cout << "C++ Scope: " << scope << endl;
#endif

   return scope;
}

// Remove trailing "::name"

DDS_StdString BE_Globals::ScopeOf (const DDS_StdString & scopedName)
{
   DDS_StdString ret = (const char *)scopedName;

   if (scopedName.length())
   {
      char * pos = strrchr(ret, ':');

      if (pos && --pos)
      {
         *pos = (char)0;
      }
      else
      {
         ret = "";
      }
   }

   return ret;
}

// Remove preceding "scope::"

DDS_StdString BE_Globals::NameOf (const DDS_StdString & scopedName)
{
   DDS_StdString buf = (const char *)scopedName;
   DDS_StdString ret = (const char *)scopedName;

   if (scopedName.length ())
   {
      char * pos = strrchr(buf, ':');

      if (pos && *(++pos))
      {
         ret = pos;
      }
   }

   return ret;
}

DDS_StdString BE_Globals::RelativeScope
(
   const DDS_StdString & withinScope,
   const DDS_StdString & scopedName
)
{
   DDS_StdString enclosingScope = ScopeOf (scopedName);
   DDS_StdString name = NameOf (scopedName);
   return resolveScope (withinScope, enclosingScope, name);
}

pbbool
BE_Globals::IsKeyword(const DDS_StdString& label)
{
   pbbool ret = pbfalse;

   if ((const char*)label)
   {
      const char* const * p = BE_Globals::_keywords;

      while (p && *p && **p)
      {
         if (!strcmp(*p, (const char*)label))
         {
            ret = pbtrue;
            break;
         }

         p++;
      }
   }

   return ret;
}

DDS_StdString
BE_Globals::KeywordToLabel(const DDS_StdString& label)
{
   DDS_StdString ret = label;

   if (IsKeyword(label))
   {
      ret = (DDS_StdString)"_cxx_" + label;
   }

   return ret;
}

DDS_StdString
BE_Globals::int_to_string(int i)
{
   DDS_StdString result;
   char buf[20];

   os_sprintf(buf, "%d", i);
   result = buf;

   return result;
}

DDS_StdString
BE_Globals::ulong_to_string(unsigned long ul)
{
   DDS_StdString result;
   char buf[256];

   os_sprintf(buf, "%lu", ul);
   result = buf;

   return result;
}

extern unsigned long idlc_hash_str (const DDS_StdString &str)
{
   const long p = 1073741827L;  // prime
   int n = str.length();
   const char * d = (const char*)str;
   long h = 0;
   unsigned retval = 0;

   for ( int i = 0; i < n; ++i, ++d)
      h = (h << 2) + *d;

   retval = ((h >= 0) ? (h % p) : ( -h % p));

   return retval;
}
