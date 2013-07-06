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
#ifndef _XBE_UTILS_HH
#define _XBE_UTILS_HH

#include "idl.h"
#include "Std.h"
#include "StdString.h"
#include "xps_string.h"
#include "xbe_globals.h"

class be_Source;

extern void DDSError(const DDS_StdString &message);

class DDSXBEException
{

public:

   DDSXBEException(
      const char * type,
      const char * function,
      int code);
   DDSXBEException(
      const char * type,
      const char * function,
      const char * message,
      int code);

   const DDS_StdString & type() const;
   const DDS_StdString & function() const;
   const DDS_StdString & message() const;
   int code() const;

   // DDSXBEException VIRTUALS
   virtual ~DDSXBEException();

private:

   friend ostream & operator<<(ostream & os, const DDSXBEException & exception);

   DDS_StdString m_type;
   DDS_StdString m_function;
   DDS_StdString m_message;
   int m_code;
};



class TypeIndex
{

public:

   enum TI_VAL { TI_long, TI_ulong, TI_longlong, TI_ulonglong, TI_short,
                 TI_ushort, TI_float, TI_double, TI_longdouble, TI_char,
                 TI_wchar, TI_boolean, TI_octet, TI_any, TI_void, TI_pseudo,
                 TI_object, TI_typecode, TI_string, TI_wstring,
                 TI_structured, TI_none };

private:

   TI_VAL val;

public:

   TypeIndex(AST_PredefinedType::PredefinedType pt)
         :
         val((TI_VAL)pt)
   {
      assert (val <= TI_none);
   }

   TypeIndex(AST_Type* tp);
   TypeIndex(AST_Expression::ExprType et);
   ~TypeIndex()
   {}

   inline TI_VAL
   Val() const
   {
      if (BE_Globals::map_wide)
      {
         if (val == TI_wchar)
         {
            return TI_char;
         }
         else if (val == TI_wstring)
         {
            return TI_string;
         }
      }

      return val;
   }
};

class CorbaTypesMap
{

public:

   struct NameMap
   {
      const char * nestingType;
      const char * no_nestingType;
      const char * typeCodeType;
   };

private:

   static NameMap CorbaTypeNames[TypeIndex::TI_none + 2];

public:

   static const char * TypeName(const TypeIndex& ti);
   static const char * TypeCodeName(const TypeIndex& ti);
};

void GeneratePtrStreamOps(
   be_Source& source,
   const DDS_StdString& streamOpTypeName);

DDS_StdString StripIDL(const DDS_StdString& idlname);
DDS_StdString StripExtension(const DDS_StdString& fname);

const char * FindFilename(const DDS_StdString& fullname);

DDS_StdString FilterFilename(const DDS_StdString& fullname);

DDS_StdString IStreamOpCast(const DDS_StdString& type);
DDS_StdString OStreamOpCast(const DDS_StdString& type);
void DotToBar(char * s);
void ColonToBar(char * s);
void ColonColonToBar(char * s);
DDS_StdString NoColons(const DDS_StdString &str);
DDS_StdString Ifndefize(const DDS_StdString &dirtyString);
DDS_StdString AlphaBarOnly(const DDS_StdString &dirtyString);

const char* LocalName(AST_Decl* d);
DDS_StdString NameToString(UTL_ScopedName* name, const char * sep = 0);
unsigned long ExprToULong(AST_Expression * expr);
long ExprToLong(AST_Expression::AST_ExprValue * val);
DDS_StdString BaseName(const char* fullpath);
void Sort(int len, bool (*compare)(int, int),
          void (*swap)(int, int));

ostream& operator<<(ostream& os, AST_Type& type);
ostream& operator<<(ostream& os, Identifier& name);
ostream& operator<<(ostream& os, UTL_ScopedName& name);
ostream& operator<<(ostream& os, AST_Expression& expr);

enum XBE_Env
{
   XBE_ENV_VAR1,
   XBE_ENV_VARN,
   XBE_ENV_ARG1,
   XBE_ENV_ARGN
};

/* This class wraps the generation of exception support code */

class XBE_Ev
{
public:

   enum Mode
   {
      XBE_ENV_PORTABLE,
      XBE_ENV_EXCEPTION,
      XBE_ENV_NO_EXCEPTION
   };

   static const char * arg (XBE_Env ev, bool space = true);
   static const char * declare ();
   static void throwex (ostream & os, const char * ex);
   static void check (ostream & os, const char * ret);
   static bool generate ();

   static Mode _mode;
};

class newline
{

public:
   newline()
   {}

   ~newline()
   {}

private:
   newline(const newline&);
   newline& operator=(const newline &);
   newline& operator==(const newline &);
};

extern newline nl;

inline ostream& operator<<(ostream& os, const newline &)
{
   return os << "\n";
}

inline bool
str_eq(const DDSString & s1, const DDSString & s2)
{
   return s1 == s2;
}

class UniqueString
{

public:
   static DDS_StdString unique_string();
   static unsigned long seed;
};

#endif
