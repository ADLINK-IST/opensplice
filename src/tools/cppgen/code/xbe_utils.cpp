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
#include "os_heap.h"
#include "xbe_utils.h"
#include "xbe_string.h"

#include "cppgen_iostream.h"

#if defined(_WIN32)
const char pathSep = '\\';
#else

const char pathSep = '/';
#endif

newline nl;

DDS_StdString
StripIDL(const DDS_StdString& idlname)
{
   static char buf[256];
   DDS_StdString ret;
   char * p;

   os_strcpy(buf, (const char *)idlname);

   p = strstr(buf, BE_Globals::IDLExtension);
   if (p)
   {
      *p = (char)0;
   }

   ret = buf;

   return ret;
}

DDS_StdString
StripExtension(const DDS_StdString& fname)
{
   static char buf[256];
   DDS_StdString ret;
   char * p;

   os_strcpy(buf, (const char *)fname);

   p = strrchr (buf, '.');
   if (p)
   {
      *p = (char)0;
   }

   ret = buf;

   return ret;
}

const char *
FindFilename(const DDS_StdString& fullname)
{

   const char * t = (const char*)strrchr((const char *)fullname, pathSep);

#if defined(_WIN32)

   if (!t)
   {
      t = (const char*) strrchr((const char *)fullname, ':');
   }

#endif

   // Now point at the filename, not the separator
   if (t && ++t)
   {
      return t;
   }
   else
   {
      return (const char *)fullname;
   }
}

DDS_StdString
FilterFilename(const DDS_StdString& fullname)
{
   DDS_StdString ret = fullname;    // make our own copy

   if (BE_Globals::short_filenames)
   {
      int len = BE_Globals::clientMax;

      ret = FindFilename(fullname);

      if (ret == (const char *)"")
      {
         ret = fullname;
      }

      if (ret != (const char*)"")
      {
         if (ret.length() < (unsigned int)len)
         {
            len = ret.length();
         }

         ((char *)ret)[len] = (char)0;
      }
   }

   return ret;
}


void
DDSError(const DDS_StdString& message)
{
   cerr << "cppgen: error: " << message << nl;
}

void
GeneratePtrStreamOps(be_Source& source, const DDS_StdString& streamOpTypeName)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   // OSTREAM POINTER INSERTION
   os << tab << "DDS::Codec::OutStream&" << nl;
   os << tab << "operator<<(DDS::Codec::OutStream& os, const " << streamOpTypeName << "* _s_)" << nl;
   os << tab << "{" << nl;
   source.Indent();
   os << tab << "if (_s_) os << * _s_;" << nl;
   os << tab << "return os;" << nl;
   source.Outdent();
   os << tab << "}" << nl;

   // ISTREAM POINTER EXTRACTION
   os << tab << "DDS::Codec::InStream&" << nl;
   os << tab << "operator>>(DDS::Codec::InStream& is, " << streamOpTypeName << "*& _s_)" << nl;
   os << tab << "{" << nl;
   source.Indent();
   os << tab << "_s_ = new " << streamOpTypeName << ";" << nl;
   os << tab << "is >> * _s_;" << nl;
   os << tab << "return is;" << nl;
   source.Outdent();
   os << tab << "}" << nl;

   // ISTREAM VAR EXTRACTION
   os << tab << "DDS::Codec::InStream&" << nl;
   os << tab << "operator>>(DDS::Codec::InStream& is, " << streamOpTypeName << "_var& _s_)" << nl;
   os << tab << "{" << nl;
   source.Indent();
   os << tab << streamOpTypeName << " * _p_ = 0;" << nl;
   os << tab << "is >> _p_;" << nl;
   os << tab << "_s_ = _p_;" << nl;
   os << tab << "return is;" << nl;
   source.Outdent();
   os << tab << "}" << nl;

}


ostream&
operator<<(ostream& os, AST_Type& type)
{
   assert(type.name());

   if (type.name())
   {
      type.name()->dump(os);
   }
   else
   {
      DDSError("unnamed type");
   }

   return os;
}

ostream&
operator<<(ostream& os, Identifier& name)
{
   name.dump(os);
   return os;
}

ostream&
operator<<(ostream& os, UTL_ScopedName& name)
{
   name.dump(os);
   return os;
}

ostream&
operator<<(ostream& os, AST_Expression& expr)
{
   expr.dump(os);
   return os;
}

TypeIndex::TypeIndex(AST_Type* tp)
{
   if (tp)
   {
      AST_PredefinedType * ptp;

      ptp = (AST_PredefinedType*)tp->narrow((long) & AST_PredefinedType::type_id);
      if (ptp)
      {
         val = (TI_VAL) ptp->pt();
      }
      else
      {
         DDS_StdString typeName(NameToString(tp->name()));

         if (tp->narrow((long)&be_string::type_id))
         {
            val = TI_string;
         }
         else
         {
            val = TI_structured;
         }
      }
   }
   else
   {
      val = TI_none;
   }

   assert(val <= TI_none);
}

TypeIndex::TypeIndex (AST_Expression::ExprType et)
{
   switch (et)
   {
      case AST_Expression::EV_short :
         val = TI_short;
         break;

      case AST_Expression::EV_ushort :
         val = TI_ushort;
         break;

      case AST_Expression::EV_long :
         val = TI_long;
         break;

      case AST_Expression::EV_ulong :
         val = TI_ulong;
         break;

      case AST_Expression::EV_longlong :
         val = TI_longlong;
         break;

      case AST_Expression::EV_ulonglong :
         val = TI_ulonglong;
         break;

      case AST_Expression::EV_float :
         val = TI_float;
         break;

      case AST_Expression::EV_double :
         val = TI_double;
         break;

      case AST_Expression::EV_longdouble :
         val = TI_longdouble;
         break;

      case AST_Expression::EV_char :
         val = TI_char;
         break;

      case AST_Expression::EV_wchar :
         val = TI_wchar;
         break;

      case AST_Expression::EV_octet :
         val = TI_octet;
         break;

      case AST_Expression::EV_bool :
         val = TI_boolean;
         break;

      case AST_Expression::EV_string :
         val = TI_string;
         break;

      case AST_Expression::EV_wstring :
         val = TI_wstring;
         break;

      case AST_Expression::EV_any :
         val = TI_any;
         break;

      case AST_Expression::EV_void :
         val = TI_void;
         break;

      case AST_Expression::EV_none :
         val = TI_none;
         break;

      default:
         assert(pbfalse);
   }

   assert(val <= TI_none);
}

const char * CorbaTypesMap::TypeName (const TypeIndex & ti)
{
   return CorbaTypeNames[ti.Val()].nestingType;
}

const char * CorbaTypesMap::TypeCodeName(const TypeIndex& ti)
{
   return CorbaTypeNames[ti.Val()].typeCodeType;
}


CorbaTypesMap::NameMap CorbaTypesMap::CorbaTypeNames[TypeIndex::TI_none + 2] =
{
   { "DDS::Long", "DDS::Long", "_tc_long" },
   { "DDS::ULong", "DDS::ULong", "_tc_ulong" },
   { "DDS::LongLong", "DDS::LongLong", "_tc_longlong" },
   { "DDS::ULongLong", "DDS::ULongLong", "_tc_ulonglong" },
   { "DDS::Short", "DDS::Short", "_tc_short" },
   { "DDS::UShort", "DDS::UShort", "_tc_ushort" },
   { "DDS::Float", "DDS::Float", "_tc_float" },
   { "DDS::Double", "DDS::Double", "_tc_double" },
   { "DDS::LongDouble", "DDS::LongDouble", "_tc_longdouble" },
   { "DDS::Char", "DDS::Char", "_tc_char" },
   { "DDS::WChar", "DDS::WChar", "_tc_wchar" },
   { "DDS::Boolean", "DDS::Boolean", "_tc_boolean" },
   { "DDS::Octet", "DDS::Octet", "_tc_octet" },
   { "DDS::Any", "DDS::Any", "_tc_any" },
   { "void", "void", "_tc_void" },
   { "pseudo", "pseudo", "_tc_pseudo" },
   { "DDS::Object", "DDS::Object", "_tc_Object" },
   { "DDS::TypeCode", "DDS::TypeCode", "_tc_TypeCode" },
   { "DDS::String", "DDS::String", "_tc_ubstring" },
   { "DDS::WString", "DDS::WString", "_tc_wubstring" },
   { "DDS::Octet*" "DDS::Octet*", "" },
   { "structured", "structured", "" },
   { "none", "", "" }
};

DDS_StdString
IStreamOpCast(const DDS_StdString& type)
{
   DDS_StdString ret;

   if (type.length())
   {
      ret = (DDS_StdString)"(" + type + ")";
   }

   return ret;
}

DDS_StdString
OStreamOpCast(const DDS_StdString& type)
{
   DDS_StdString ret;

   if (type.length())
   {
      ret = (DDS_StdString)"(" + type + ")";
   }

   return ret;
}

void
DotToBar(char * s)
{
   char * p;

   for (p = s; *p; p++)
   {
      if (*p == '.')
         *p = '_';
   }
}

void
ColonToBar(char * s)
{
   char * p;

   for (p = s; *p; p++)
   {
      if (*p == ':')
         *p = '_';
   }
}

void
ColonColonToBar(char * s)
{
   char * p, *i;

   for (p = s; *p; p++)
   {
      if (*p == ':' && *(p + 1) == ':')
      {
         *p = '_';

         for (i = p + 1; *(i + 1);++i)
         {
            *i = *(i + 1);
         }

         *i = '\0';
      }
   }
}

DDS_StdString
NoColons(const DDS_StdString &colons)
{
   DDS_StdString nocolons = (const char *)colons;
   ColonColonToBar((char *)nocolons);
   return nocolons;
}

DDS_StdString
Ifndefize(const DDS_StdString& name)
{
   const DDS_StdString Line = "_";
   DDS_StdString ret;

   ret = Line + name + Line;

   for (char *p = (char *)ret; *p; p++)
   {
      *p = isalnum(*p) ? toupper(*p) : '_';
   }

   return ret;
}


DDS_StdString
AlphaBarOnly(const DDS_StdString &dirtyString)
{
   DDS_StdString alphabar = (const char *)dirtyString;

   for (char *p = (char *)alphabar; *p; p++)
   {
      *p = isalnum(*p) ? *p : '_';
   }

   return alphabar;
}

const char*
LocalName(AST_Decl* d)
{
   const char * ret = 0;

   if (d->local_name())
   {
      ret = d->local_name()->get_string();
   }

   return ret;
}

DDS_StdString
NameToString(UTL_ScopedName* name, const char * sep)
{
   UTL_ScopedNameActiveIterator* it = new UTL_ScopedNameActiveIterator(name);
   pbbool first = pbtrue;
   DDS_StdString fs = sep;
   DDS_StdString ret;

   if (!fs.length())
   {
      fs = "::";
   }

   if (name)
   {
      ostringstream os;

      while (!(it->is_done()))
      {
         if (!first)
            os << (const char*)fs;

         first = pbfalse;

         os << it->item()->get_string();

         it->next();
      }

      os.flush();
      os.put ((char) 0);
      ret = os.str().c_str();
   }

   delete it;
   return ret;
}

unsigned long
ExprToULong(AST_Expression * expr)
{
   unsigned long ret = 0;

   if (expr)
   {
      AST_Expression::AST_ExprValue * val = expr->eval(AST_Expression::EK_const);

      if (val)
      {
         ret = val->u.ulval;
      }
      else
      {
         ostringstream os;

         expr->dump(os);
         os << ends;

         DDSError((DDS_StdString)"Expression failed to evaluate to const: " + os.str().c_str());
      }
   }

   return ret;
}


long
ExprToLong(AST_Expression::AST_ExprValue * val)
{
   long ret = 0;

   if (val)
   {
      switch (val->et)
      {

            case AST_Expression::EV_short:
            {
               ret = (long) val->u.sval;
            }

            break;

            case AST_Expression::EV_long:
            {
               ret = (long) val->u.lval;
            }

            break;

            case AST_Expression::EV_ushort:
            {
               ret = (long) val->u.usval;
            }

            break;

            case AST_Expression::EV_ulong:
            {
               ret = (long) val->u.ulval;
            }

            break;

            case AST_Expression::EV_float:
            {
               ret = (long) val->u.fval;
            }

            break;

            case AST_Expression::EV_double:
            {
               ret = (long) val->u.dval;
            }

            break;

            case AST_Expression::EV_char:
            {
               ret = (long) val->u.cval;
            }

            break;

            case AST_Expression::EV_wchar:
            {
               ret = (long) val->u.cwval;
            }

            break;

            case AST_Expression::EV_bool:
            {
               ret = (long) val->u.bval;
            }

            break;

            case AST_Expression::EV_octet:
            {
               ret = (long) val->u.oval;
            }

            break;

            default:
            break;
      }
   }

   return ret;
}


DDSXBEException::DDSXBEException(const char * type, const char * function, int code)
      :
      m_type(type),
      m_function(function),
      m_code(code)
{
   char * message = strerror (code);

   if (message)
   {
      m_message = message;
   }
   else
   {
      m_message = "<unknown>";
   }
}


DDSXBEException::DDSXBEException(const char * type, const char * function, const char * message, int code)
      :
      m_type(type),
      m_function(function),
      m_message(message),
      m_code(code)
{}


ostream&
operator<<(ostream & os, const DDSXBEException & exception)
{
   os << "idlc error: type = " << exception.type()
   << ", function = " << exception.function()
   << ", message = " << exception.message()
   << ", code = " << exception.code() << endl;

   return os;
}


DDSXBEException::~DDSXBEException()
{}

const DDS_StdString &
DDSXBEException::message() const
{
   return m_message;
}

const DDS_StdString &
DDSXBEException::function() const
{
   return m_function;
}

int
DDSXBEException::code() const
{
   return m_code;
}

const DDS_StdString &
DDSXBEException::type() const
{
   return m_type;
}

// BaseName() -- returns just the base filename portion of fullpath
// fullpath: a filename, possibly including directories and a drive letter

DDS_StdString BaseName (const char * fullpath)
{
   assert (fullpath);

   char * s = os_strdup (fullpath);
   char * t = s + strlen (s);

   while (t >= s)
   {
      if (*t == '/' || *t == '\\' || *t == ':')
      {
         break;
      }

      --t;
   }

   ++t;
   assert (t >= s);
   assert (t <= s + strlen(s));

   DDS_StdString result (t);
   os_free (s);
   return result;
}

// Sort() -- Don't laugh, it's a bubble sort
void
Sort(int len, bool (*compare)(int, int), void (*swap)(int, int))
// compare() and swap() each take array indices
{
   assert(len > 0);
   assert(compare);
   assert(swap);

   bool swappedOnThisPass;

   do
   {
      swappedOnThisPass = false;

      for (int i = 0; i < len - 1; i++)
      {
         if (!(*compare)(i, i + 1))
         {
            swap(i, i + 1);
            swappedOnThisPass = true;
         }
      }
   }
   while (swappedOnThisPass);
}

unsigned long UniqueString::seed = 0;

DDS_StdString UniqueString::unique_string()
{
   UniqueString::seed++;
   return (BE_Globals::ulong_to_string(UniqueString::seed));
}

/*
#if DDS_USE_EXCEPTIONS
XBE_Ev::Mode XBE_Ev::_mode = XBE_Ev::XBE_ENV_EXCEPTION;
#else
XBE_Ev::Mode XBE_Ev::_mode = XBE_Ev::XBE_ENV_NO_EXCEPTION;
#endif
*/
XBE_Ev::Mode XBE_Ev::_mode = XBE_Ev::XBE_ENV_EXCEPTION;

bool XBE_Ev::generate ()
{
   return (_mode != XBE_ENV_EXCEPTION);
}

const char * XBE_Ev::declare ()
{
   if (_mode == XBE_ENV_PORTABLE)
   {
      return "DDS_DECLARE_ENV;";
   }
   else if (_mode == XBE_ENV_NO_EXCEPTION)
   {
      return "DDS::Environment _env_;";
   }
   return "";
}

const char * XBE_Ev::arg (XBE_Env ev, bool space)
{
   if (_mode == XBE_ENV_PORTABLE)
   {
      switch (ev)
      {
         case XBE_ENV_VAR1: return "DDS_ENV_VAR1";
         case XBE_ENV_VARN: return space ? " DDS_ENV_VARN" : "DDS_ENV_VARN";
         case XBE_ENV_ARG1: return "DDS_ENV_ARG1";
         case XBE_ENV_ARGN: return space ? " DDS_ENV_ARGN" : "DDS_ENV_ARGN";
      }
   }
   else if (_mode == XBE_ENV_NO_EXCEPTION)
   {
      switch (ev)
      {
         case XBE_ENV_VAR1: return "_env_";
         case XBE_ENV_VARN: return ", _env_";
         case XBE_ENV_ARG1: return "DDS::Environment & _env_";
         case XBE_ENV_ARGN: return ", DDS::Environment & _env_";
      }
   }
   return "";
}

void XBE_Ev::throwex (ostream & os, const char * ex)
{
   if (_mode == XBE_ENV_PORTABLE)
   {
      os << "DDS_THROW_RETURN_VOID (" << ex << ");";
   }
   else if (_mode == XBE_ENV_NO_EXCEPTION)
   {
      os << "_env_.exception (" << ex << ".clone ()); return;";
   }
   else
   {
      os << "throw " << ex << ";";
   }
}

void XBE_Ev::check (ostream & os, const char * ret)
{
   if (_mode == XBE_ENV_PORTABLE)
   {
      if (ret)
      {
         os << "DDS_CHECK_ENV_RETURN (" << ret << ");";
      }
      else
      {
         os << "DDS_CHECK_ENV_RETURN_VOID;";
      }
   }
   else if (_mode == XBE_ENV_NO_EXCEPTION)
   {
      os << "if (_env_.exception()) { return";
      if (ret)
      {
         os << " " << ret;
      }
      os << ";";
   }
}
