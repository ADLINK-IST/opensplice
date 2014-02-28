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
#ifndef _XBE_GLOBALS_HH
#define _XBE_GLOBALS_HH

#include "sacpp_DDS_DCPS.h"
#include "cppgen_iostream.h"
#include "xbe_hash.h"
#include "bool.h"
#include "xps_string.h"

#define DLLMACRO (const char*)BE_Globals::UserDLL << (const char*)BE_Globals::DLLExtension

extern unsigned long idlc_hash_str (const DDS_StdString &str);

typedef DDSMap<DDS_StdString, DDS_StdString> String_map;

extern const DDS_StdString NilString;

class BE_Globals
{
public:

   static DDS_StdString TCPrefix;
   static DDS_StdString MTPrefix;
   static DDS_StdString TCBasePrefix;
   static DDS_StdString TCRepPrefix;

   static DDS_StdString DLLExtension;

   static const int clientMax;
   static const int serverMax;

   static const char * IDLExtension;
   static const int MaxFileNameLen;

   static DDS_StdString ClientExtension;
   static DDS_StdString ServerExtension;
   static DDS_StdString TieExtension;

   // COMMAND LINE ARGUMENT RESULTS

   static DDS_StdString CExtension;
   static DDS_StdString hExtension;
   static DDS_StdString BaseFilename;

   static DDS_StdString UserDLL;
   static DDS_StdString UserDLLHeader;

   static DDS_StdString OutputDirectory;

   static DDS_StdString ClientHeaderFilename;
   static DDS_StdString ClientImplFilename;
   static DDS_StdString ServerHeaderFilename;
   static DDS_StdString ServerImplFilename;
   static DDS_StdString TieHeaderFilename;

   static DDS::Boolean flat_stack;
   static DDS::Boolean gen_gui_info;
   static DDS::Boolean generate_model;
   static DDS::Boolean client_only;
   static DDS::Boolean no_exceptions;
   static DDS::Boolean short_filenames;
   static DDS::Boolean implement_virtuals;
   static DDS::Boolean nesting_bug;
   static DDS::Boolean portable_exceptions;
   static DDS::Boolean per_request_attrs;
   static DDS::Boolean real_namespaces;
   static DDS::Boolean gen_externalization;
   static DDS::Boolean inc_any;
   static DDS::Boolean gen_onefile;
   static DDS::Boolean HFileOpen;
   static DDS::Boolean CFileOpen;
   static DDS::Boolean map_wide;
   static DDS::Boolean case_sensitive;
   static DDS::Boolean ignore_interfaces;
   static DDS::Boolean isocpp;
   static DDS::Boolean isocpp_new_types;
   static DDS::Boolean isocpp_test_methods;
   static DDS::Boolean gen_equality;
   static DDS::Boolean collocated_direct;

   static unsigned long max_char_per_line;

   static void Initialize ();
   static DDS_StdString OuterMostScope (const DDS_StdString& scope);
   static DDS_StdString ScopeOf (const DDS_StdString& scopedName);
   static DDS_StdString NameOf (const DDS_StdString& scopedName);
   static DDS_StdString CorbaScope (const DDS_StdString& name);
   static DDS_StdString Scope
   (
      const DDS_StdString & enclosingScope,
      const DDS_StdString & name
   );
   static DDS_StdString RelativeScope
   (
      const DDS_StdString & withinScope,
      const DDS_StdString & scopedName
   );
   static DDS::Boolean IsKeyword(const DDS_StdString& label);
   static DDS_StdString KeywordToLabel(const DDS_StdString& label);
   static DDS_StdString int_to_string(int num);
   static DDS_StdString ulong_to_string(unsigned long num);

   static inline DDSString ScopeSeparator ()
   {
      return "::";
   }

private:

   static DDS_StdString resolveScope
   (
      const DDS_StdString & withinScope,
      const DDS_StdString & enclosingScope,
      const DDS_StdString & name
   );

   static const char* _keywords[];
};

inline DDS_StdString&
operator+(DDS_StdString& str, unsigned long l)
{
   str += BE_Globals::ulong_to_string(l);

   return str;
}

inline DDS_StdString&
operator+=(DDS_StdString& str, unsigned long l)
{
   str += BE_Globals::ulong_to_string(l);

   return str;
}


#endif
