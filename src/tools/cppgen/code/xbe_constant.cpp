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
#include "global_extern.h"

#include "xbe_globals.h"
#include "xbe_literals.h"
#include "xbe_constant.h"
#include "xbe_utils.h"

#include "cppgen_iostream.h"

be_constant::be_constant ()
{
   isAtModuleScope (pbfalse);
}

be_constant::be_constant
(
   AST_Expression::ExprType et,
   AST_Expression *v,
   UTL_ScopedName *n,
   const UTL_Pragmas &p
)
:
   AST_Decl (AST_Decl::NT_const, n, p),
   AST_Constant (et, v, n, p),
   typeName (CorbaTypesMap::TypeName(et))
{
   isAtModuleScope (pbfalse);
}

const DDS_StdString & be_constant::TypeName ()
{
   return typeName;
}

pbbool be_constant::IsGlobalScope ()
{
   pbbool ret = pbfalse;
   AST_Root * root;
   UTL_Scope * rootscope;

   if (idl_global && (root = idl_global->root()) &&
         (rootscope = (UTL_Scope*)root->narrow((long) & UTL_Scope::type_id)))
   {
      ret = (pbbool)(defined_in() == rootscope);
   }

   return ret;
}

void be_constant::Generate (be_ClientHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);
   DDS_StdString quote;

   os << tab;

   if (! isAtModuleScope ())
   {
      os << "static ";
   }

   os << "const " << TypeName () << " " << *local_name ();

   if (IsGlobalScope () || isAtModuleScope ())
   {
      os << " = (" << TypeName () << ") "
         << (const char*) QuotedConstValue ()
         << (const char*) EndValueType () << ";" << nl;

   }
   else
   {
      os << "; // " << (const char*) QuotedConstValue () << nl;
   }
}

void
be_constant::Generate(be_ClientImplementation& source)
{
   if (!(IsGlobalScope() || isAtModuleScope ()))
   {
      source.Stream() << "const " << TypeName() << " "
                      << (const char*)NameToString(name()) << " = ("
                      << TypeName() << ")" << (const char*)QuotedConstValue() 
                      << (const char*)EndValueType() << ";" << nl;
   }
}

DDS_StdString be_constant::EndValueType ()
{
   DDS_StdString val = "";

   switch (et ())
   {
      case AST_Expression::EV_ulong:
      {
         val += "UL";
         break;
      }
      case AST_Expression::EV_long:
      {
         val += "L";
         break;
      }
      case AST_Expression::EV_ushort:
      {
         val += "U";
         break;
      }

      default:
         break;
   }

   return val;
}

DDS_StdString be_constant::QuotedConstValue ()
{
   DDS_StdString quote;
   DDS_StdString val = "";
   AST_Expression * expr = constant_value ();
   AST_Expression::AST_ExprValue *ast_val = 
      expr->eval (AST_Expression::EK_const);
   assert (ast_val);

   char buf[120];

   // setup quotes, if needed

   switch (et ())
   {
      case AST_Expression::EV_wstring:
      {
         val += "L";
      }
      case AST_Expression::EV_string:
      {
         quote = "\"";
      }
      break;

      case AST_Expression::EV_wchar:
      {
         val += "L";
      }
      case AST_Expression::EV_char:
      {
         quote = "'";
      }
      break;

      default:
         break;
   }

   val += quote;

   // get value and replace with meaningful const, if necessary

   switch (et ())
   {
      case AST_Expression::EV_short:
      {
         os_sprintf(buf, "%hd", ast_val->u.sval);
         val += buf;
      }
      break;

      case AST_Expression::EV_long:
      {
         os_sprintf (buf, "%d", (int) ast_val->u.lval);
         val += buf;
      }
      break;

      case AST_Expression::EV_longlong:
      {
         os_sprintf(buf, "0x%lx", ast_val->u.lval);
         val += buf;

      }
      break;

      case AST_Expression::EV_ushort:
      {
         os_sprintf(buf, "%hu", ast_val->u.usval);
         val += buf;
      }
      break;

      case AST_Expression::EV_ulong:
      case AST_Expression::EV_ulonglong:
      {
         os_sprintf(buf, "%lu", ast_val->u.ulval);
         val += buf;
      }
      break;

      case AST_Expression::EV_float:
      {
         if (expr->expr_str ())
         {
            val += expr->expr_str ();
         }
         else
         {
            os_sprintf (buf, "%1.12e", ast_val->u.fval);
            val += buf;
         }
      }
      break;

      case AST_Expression::EV_double:
      {
         if (expr->expr_str ())
         {
            val += expr->expr_str ();
         }
         else
         {
            os_sprintf (buf, "%1.22e", ast_val->u.dval);
            val += buf;
         }
      }
      break;

      case AST_Expression::EV_char:
      {
         // check if printable character - ascii only

         if (ast_val->u.cval > 32)
         {
            os_sprintf(buf, "%c", ast_val->u.cval);
         }
         else // print an escaped octal
         {
            val += "\\";
            os_sprintf(buf, "%o", ast_val->u.cval);
         }

         val += buf;
      }
      break;

      case AST_Expression::EV_wchar:
      {
         // check if printable character - ascii only

         if (ast_val->u.cwval > 32)
         {
            os_sprintf(buf, "%c", ast_val->u.cwval);
         }
         else // print an escaped octal
         {
            val += "\\";
            os_sprintf(buf, "%o", ast_val->u.cwval);
         }

         val += buf;
      }
      break;

      case AST_Expression::EV_bool:
      {
         val += ast_val->u.bval ? "TRUE" : "FALSE";
      }
      break;

      case AST_Expression::EV_octet:
      {
         os_sprintf(buf, "%hd", ast_val->u.oval);
         val += buf;
      }
      break;

      case AST_Expression::EV_string:
      {
         val += ast_val->u.strval->get_string();
      }
      break;

      case AST_Expression::EV_wstring:
      {
         val += ast_val->u.wstrval->get_string();
      }
      break;

      default: break;
   }

   val += quote;

   return val;
}


IMPL_NARROW_METHODS2(be_constant, AST_Constant, be_CodeGenerator)
IMPL_NARROW_FROM_DECL(be_constant)
