/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    IDENTIFIER = 258,
    CONST = 259,
    MODULE = 260,
    LOCAL = 261,
    INTERFACE = 262,
    TYPEDEF = 263,
    IDL_LONG = 264,
    IDL_SHORT = 265,
    UNSIGNED = 266,
    IDL_DOUBLE = 267,
    IDL_FLOAT = 268,
    IDL_CHAR = 269,
    IDL_WCHAR = 270,
    IDL_OCTET = 271,
    IDL_BOOLEAN = 272,
    ANY = 273,
    STRUCT = 274,
    UNION = 275,
    SWITCH = 276,
    ENUM = 277,
    SEQUENCE = 278,
    STRING = 279,
    WSTRING = 280,
    EXCEPTION = 281,
    CASE = 282,
    DEFAULT = 283,
    READONLY = 284,
    ATTRIBUTE = 285,
    ONEWAY = 286,
    IDEMPOTENT = 287,
    VOID = 288,
    IN = 289,
    OUT = 290,
    INOUT = 291,
    RAISES = 292,
    CUSTOM = 293,
    VALUETYPE = 294,
    TRUNCATABLE = 295,
    SUPPORTS = 296,
    IDL_PUBLIC = 297,
    IDL_PRIVATE = 298,
    FACTORY = 299,
    ABSTRACT = 300,
    IDL_CONTEXT = 301,
    OPAQUE = 302,
    VERSION = 303,
    INTEGER_LITERAL = 304,
    STRING_LITERAL = 305,
    CHARACTER_LITERAL = 306,
    FLOATING_PT_LITERAL = 307,
    TRUETOK = 308,
    FALSETOK = 309,
    SCOPE_DELIMITOR = 310,
    LEFT_SHIFT = 311,
    RIGHT_SHIFT = 312,
    PRAGMA = 313,
    PRAGMA_INCLUDE = 314,
    PFROM = 315,
    PRAGMA_ASYNC_CLIENT = 316,
    PRAGMA_ASYNC_SERVER = 317,
    PRAGMA_ID = 318,
    PRAGMA_PREFIX = 319,
    PRAGMA_VERSION = 320,
    PRAGMA_ANY = 321,
    PRAGMA_END = 322
  };
#endif
/* Tokens.  */
#define IDENTIFIER 258
#define CONST 259
#define MODULE 260
#define LOCAL 261
#define INTERFACE 262
#define TYPEDEF 263
#define IDL_LONG 264
#define IDL_SHORT 265
#define UNSIGNED 266
#define IDL_DOUBLE 267
#define IDL_FLOAT 268
#define IDL_CHAR 269
#define IDL_WCHAR 270
#define IDL_OCTET 271
#define IDL_BOOLEAN 272
#define ANY 273
#define STRUCT 274
#define UNION 275
#define SWITCH 276
#define ENUM 277
#define SEQUENCE 278
#define STRING 279
#define WSTRING 280
#define EXCEPTION 281
#define CASE 282
#define DEFAULT 283
#define READONLY 284
#define ATTRIBUTE 285
#define ONEWAY 286
#define IDEMPOTENT 287
#define VOID 288
#define IN 289
#define OUT 290
#define INOUT 291
#define RAISES 292
#define CUSTOM 293
#define VALUETYPE 294
#define TRUNCATABLE 295
#define SUPPORTS 296
#define IDL_PUBLIC 297
#define IDL_PRIVATE 298
#define FACTORY 299
#define ABSTRACT 300
#define IDL_CONTEXT 301
#define OPAQUE 302
#define VERSION 303
#define INTEGER_LITERAL 304
#define STRING_LITERAL 305
#define CHARACTER_LITERAL 306
#define FLOATING_PT_LITERAL 307
#define TRUETOK 308
#define FALSETOK 309
#define SCOPE_DELIMITOR 310
#define LEFT_SHIFT 311
#define RIGHT_SHIFT 312
#define PRAGMA 313
#define PRAGMA_INCLUDE 314
#define PFROM 315
#define PRAGMA_ASYNC_CLIENT 316
#define PRAGMA_ASYNC_SERVER 317
#define PRAGMA_ID 318
#define PRAGMA_PREFIX 319
#define PRAGMA_VERSION 320
#define PRAGMA_ANY 321
#define PRAGMA_END 322

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 94 "idl.yy" /* yacc.c:1909  */

  AST_Decl              *dcval;         /* Decl value           */
  UTL_StrList           *slval;         /* String list          */
  UTL_NameList          *nlval;         /* Name list            */
  UTL_ExprList          *elval;         /* Expression list      */
  UTL_LabelList         *llval;         /* Label list           */
  UTL_DeclList          *dlval;         /* Declaration list     */
  FE_InterfaceHeader    *ihval;         /* Interface header     */
  FE_ValueHeader        *vhval;         /* Value header         */
  FE_ValueInheritanceSpec *visval;      /* Value inheritance    */
  AST_Expression        *exval;         /* Expression value     */
  AST_UnionLabel        *ulval;         /* Union label          */
  AST_Field             *ffval;         /* Field value          */
  AST_Expression::ExprType etval;       /* Expression type      */
  AST_Argument::Direction dival;        /* Argument direction   */
  AST_Operation::Flags  ofval;          /* Operation flags      */
  FE_Declarator         *deval;         /* Declarator value     */
  bool                   bval;           /* Boolean value        */
  long                  ival;           /* Long value           */
  double                dval;           /* Double value         */
  float                 fval;           /* Float value          */
  char                  cval;           /* Char value           */
  UTL_String            *sval;          /* String value         */
  char                  *strval;        /* char * value         */
  Identifier            *idval;         /* Identifier           */
  UTL_IdList            *idlist;        /* Identifier list      */

#line 216 "y.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
