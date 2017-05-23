/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 71 "idl.yy"

#include <os_stdlib.h>
#include <os_heap.h>
#include <idl.h>
#include <idl_extern.h>

#include <fe_private.h>
#include <utl_incl.h>
#include <xbe_scopestack.h>

#include <stdio.h>

void yyunput (int c);
extern int yylex (void);
extern void yyerror (const char *);
int yywrap (void);



/* Line 268 of yacc.c  */
#line 91 "y.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
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




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 94 "idl.yy"

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



/* Line 293 of yacc.c  */
#line 291 "y.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 303 "y.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (To)[yyi] = (From)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  71
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   866

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  89
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  212
/* YYNRULES -- Number of rules.  */
#define YYNRULES  339
/* YYNRULES -- Number of states.  */
#define YYNSTATES  497

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   322

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    83,    78,     2,
      73,    74,    81,    79,    72,    80,     2,    82,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    71,    68,
      86,    75,    85,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    87,     2,    88,    77,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    69,    76,    70,    84,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,     9,    10,    14,    15,    19,
      20,    24,    25,    29,    30,    34,    35,    39,    40,    45,
      46,    50,    52,    58,    62,    66,    71,    75,    79,    84,
      88,    89,    90,    91,    92,   102,   104,   106,   107,   108,
     109,   117,   118,   122,   126,   127,   131,   132,   134,   136,
     138,   140,   143,   146,   150,   151,   159,   162,   163,   169,
     173,   178,   182,   187,   189,   192,   196,   197,   200,   201,
     203,   205,   207,   209,   212,   213,   218,   220,   222,   223,
     224,   234,   238,   240,   241,   246,   247,   251,   254,   255,
     256,   260,   261,   265,   266,   270,   271,   275,   276,   280,
     281,   285,   288,   289,   294,   295,   297,   298,   302,   303,
     308,   310,   313,   314,   315,   316,   317,   327,   329,   331,
     333,   335,   337,   339,   341,   343,   345,   347,   349,   353,
     355,   359,   361,   365,   367,   371,   375,   377,   381,   385,
     387,   391,   395,   399,   401,   404,   407,   410,   412,   414,
     418,   420,   422,   424,   426,   428,   430,   433,   435,   437,
     438,   442,   444,   446,   448,   450,   452,   454,   455,   459,
     460,   464,   465,   469,   471,   473,   475,   477,   479,   481,
     483,   485,   487,   489,   491,   493,   495,   497,   499,   501,
     503,   506,   507,   512,   513,   515,   517,   519,   521,   523,
     525,   527,   530,   532,   535,   539,   542,   544,   546,   549,
     551,   553,   555,   557,   559,   560,   561,   568,   571,   574,
     575,   576,   577,   583,   584,   588,   589,   590,   591,   592,
     593,   594,   609,   611,   613,   615,   617,   619,   621,   624,
     627,   628,   629,   630,   636,   637,   641,   644,   647,   648,
     649,   653,   654,   655,   661,   662,   666,   667,   668,   669,
     670,   680,   683,   684,   689,   690,   692,   693,   694,   701,
     704,   705,   706,   712,   713,   714,   721,   723,   725,   726,
     727,   734,   736,   738,   739,   743,   746,   749,   750,   751,
     752,   758,   759,   760,   767,   769,   770,   772,   774,   775,
     776,   777,   778,   779,   789,   790,   791,   792,   793,   804,
     806,   808,   809,   811,   813,   814,   818,   819,   824,   827,
     828,   833,   834,   835,   836,   842,   844,   846,   848,   849,
     850,   857,   858,   859,   860,   867,   868,   871,   872,   877
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      90,     0,    -1,    91,    -1,    92,    91,    -1,    -1,    -1,
     176,    93,    68,    -1,    -1,   157,    94,    68,    -1,    -1,
     270,    95,    68,    -1,    -1,   107,    96,    68,    -1,    -1,
     102,    97,    68,    -1,    -1,   117,    98,    68,    -1,    -1,
      47,     3,    99,    68,    -1,    -1,     1,   100,    68,    -1,
     101,    -1,    59,    50,    60,    50,    67,    -1,    61,   152,
      67,    -1,    62,   152,    67,    -1,    63,   152,    50,    67,
      -1,    64,    50,    67,    -1,    66,   152,    67,    -1,    65,
     152,    48,    67,    -1,    58,     1,    67,    -1,    -1,    -1,
      -1,    -1,     5,   103,     3,   104,    69,   105,    91,   106,
      70,    -1,   108,    -1,   156,    -1,    -1,    -1,    -1,   114,
     109,    69,   110,   141,   111,    70,    -1,    -1,     7,   113,
     155,    -1,   269,   112,   115,    -1,    -1,    71,   116,   149,
      -1,    -1,   123,    -1,   120,    -1,   119,    -1,   118,    -1,
      39,   155,    -1,   122,   155,    -1,    39,   155,   185,    -1,
      -1,   122,   155,   126,   121,    69,   141,    70,    -1,    45,
      39,    -1,    -1,   125,   124,    69,   132,    70,    -1,    39,
     155,   126,    -1,    38,    39,   155,   126,    -1,    71,   127,
     129,    -1,    71,    40,   127,   129,    -1,   129,    -1,   130,
     128,    -1,   128,    72,   130,    -1,    -1,    41,   149,    -1,
      -1,   152,    -1,   142,    -1,   133,    -1,   135,    -1,   132,
     131,    -1,    -1,   134,   185,   190,    68,    -1,    42,    -1,
      43,    -1,    -1,    -1,    44,   155,   136,    73,   138,    74,
     139,   137,    68,    -1,   138,    72,   140,    -1,   140,    -1,
      -1,    37,    73,   149,    74,    -1,    -1,    34,   186,   194,
      -1,   141,   142,    -1,    -1,    -1,   176,   143,    68,    -1,
      -1,   157,   144,    68,    -1,    -1,   270,   145,    68,    -1,
      -1,   265,   146,    68,    -1,    -1,   275,   147,    68,    -1,
      -1,     1,   148,    68,    -1,   152,   150,    -1,    -1,   150,
      72,   151,   152,    -1,    -1,   155,    -1,    -1,    55,   153,
     155,    -1,    -1,   152,    55,   154,   155,    -1,     3,    -1,
     269,   112,    -1,    -1,    -1,    -1,    -1,     4,   158,   162,
     159,   155,   160,    75,   161,   163,    -1,   196,    -1,   200,
      -1,   201,    -1,   202,    -1,   199,    -1,   250,    -1,   254,
      -1,   152,    -1,   164,    -1,   165,    -1,   166,    -1,   165,
      76,   166,    -1,   167,    -1,   166,    77,   167,    -1,   168,
      -1,   167,    78,   168,    -1,   169,    -1,   168,    56,   169,
      -1,   168,    57,   169,    -1,   170,    -1,   169,    79,   170,
      -1,   169,    80,   170,    -1,   171,    -1,   170,    81,   171,
      -1,   170,    82,   171,    -1,   170,    83,   171,    -1,   172,
      -1,    79,   172,    -1,    80,   172,    -1,    84,   172,    -1,
     152,    -1,   173,    -1,    73,   164,    74,    -1,    49,    -1,
     174,    -1,    51,    -1,    52,    -1,    53,    -1,    54,    -1,
     174,    50,    -1,    50,    -1,   164,    -1,    -1,     8,   177,
     183,    -1,   204,    -1,   213,    -1,   235,    -1,   178,    -1,
     179,    -1,   181,    -1,    -1,    19,   180,   155,    -1,    -1,
      20,   182,   155,    -1,    -1,   185,   184,   190,    -1,   186,
      -1,   189,    -1,   187,    -1,   188,    -1,   152,    -1,   196,
      -1,   199,    -1,   200,    -1,   202,    -1,   201,    -1,   203,
      -1,   244,    -1,   250,    -1,   254,    -1,   204,    -1,   213,
      -1,   235,    -1,   193,   191,    -1,    -1,   191,    72,   192,
     193,    -1,    -1,   194,    -1,   195,    -1,   155,    -1,   258,
      -1,   197,    -1,   198,    -1,     9,    -1,     9,     9,    -1,
      10,    -1,    11,     9,    -1,    11,     9,     9,    -1,    11,
      10,    -1,    12,    -1,    13,    -1,     9,    12,    -1,    14,
      -1,    15,    -1,    16,    -1,    17,    -1,    18,    -1,    -1,
      -1,   179,    69,   205,   207,   206,    70,    -1,   209,   208,
      -1,   208,   209,    -1,    -1,    -1,    -1,   185,   210,   190,
     211,    68,    -1,    -1,     1,   212,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   181,    21,   214,    73,   215,   220,
     216,    74,   217,    69,   218,   221,   219,    70,    -1,   196,
      -1,   200,    -1,   201,    -1,   202,    -1,   235,    -1,   152,
      -1,   223,   222,    -1,   222,   223,    -1,    -1,    -1,    -1,
     227,   224,   233,   225,    68,    -1,    -1,     1,   226,    68,
      -1,   229,   228,    -1,   228,   229,    -1,    -1,    -1,    28,
     230,    71,    -1,    -1,    -1,    27,   231,   164,   232,    71,
      -1,    -1,   185,   234,   193,    -1,    -1,    -1,    -1,    -1,
      22,   236,   155,   237,    69,   238,   240,   239,    70,    -1,
     243,   241,    -1,    -1,   241,    72,   242,   243,    -1,    -1,
       3,    -1,    -1,    -1,   247,    72,   245,   175,   246,    85,
      -1,   247,    85,    -1,    -1,    -1,    23,   248,    86,   249,
     186,    -1,    -1,    -1,   253,    86,   251,   175,   252,    85,
      -1,   253,    -1,    24,    -1,    -1,    -1,   257,    86,   255,
     175,   256,    85,    -1,   257,    -1,    25,    -1,    -1,   155,
     259,   260,    -1,   262,   261,    -1,   261,   262,    -1,    -1,
      -1,    -1,    87,   263,   175,   264,    88,    -1,    -1,    -1,
     268,    30,   266,   186,   267,   190,    -1,    29,    -1,    -1,
       6,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    26,
     271,   155,   272,    69,   273,   208,   274,    70,    -1,    -1,
      -1,    -1,    -1,   280,   281,   276,     3,   277,   282,   278,
     292,   279,   295,    -1,    31,    -1,    32,    -1,    -1,   186,
      -1,    33,    -1,    -1,    73,   283,    74,    -1,    -1,    73,
     284,   285,    74,    -1,   288,   286,    -1,    -1,   286,    72,
     287,   288,    -1,    -1,    -1,    -1,   291,   289,   186,   290,
     193,    -1,    34,    -1,    35,    -1,    36,    -1,    -1,    -1,
      37,   293,    73,   294,   149,    74,    -1,    -1,    -1,    -1,
      46,   296,    73,   297,   298,    74,    -1,    -1,    50,   299,
      -1,    -1,   299,    72,   300,    50,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   257,   257,   261,   262,   267,   266,   275,   274,   283,
     282,   291,   290,   299,   298,   304,   303,   312,   311,   329,
     328,   338,   342,   347,   369,   391,   414,   420,   435,   457,
     464,   468,   509,   513,   463,   528,   529,   534,   634,   638,
     533,   654,   653,   665,   683,   682,   691,   697,   698,   699,
     700,   704,   720,   740,   767,   766,   864,   869,   868,   964,
     969,   977,   981,   985,   992,   999,  1014,  1020,  1025,  1031,
    1035,  1036,  1037,  1041,  1042,  1046,  1112,  1116,  1124,  1145,
    1123,  1152,  1153,  1154,  1158,  1159,  1163,  1184,  1185,  1190,
    1189,  1198,  1197,  1206,  1205,  1214,  1213,  1222,  1221,  1230,
    1229,  1241,  1250,  1248,  1265,  1271,  1278,  1277,  1288,  1287,
    1300,  1308,  1333,  1337,  1341,  1345,  1332,  1378,  1379,  1380,
    1381,  1382,  1383,  1387,  1391,  1430,  1432,  1434,  1435,  1442,
    1443,  1450,  1451,  1458,  1459,  1463,  1470,  1471,  1475,  1482,
    1483,  1487,  1491,  1498,  1499,  1505,  1511,  1520,  1529,  1530,
    1537,  1541,  1545,  1549,  1553,  1558,  1566,  1579,  1586,  1595,
    1594,  1599,  1600,  1601,  1602,  1606,  1621,  1639,  1638,  1651,
    1650,  1663,  1662,  1696,  1697,  1701,  1705,  1706,  1720,  1721,
    1722,  1723,  1724,  1725,  1729,  1730,  1731,  1735,  1736,  1737,
    1741,  1749,  1747,  1764,  1770,  1771,  1775,  1783,  1792,  1793,
    1797,  1801,  1805,  1812,  1816,  1820,  1827,  1831,  1835,  1842,
    1846,  1853,  1860,  1867,  1875,  1906,  1874,  1929,  1932,  1933,
    1938,  1942,  1937,  2009,  2008,  2021,  2025,  2029,  2033,  2077,
    2081,  2020,  2103,  2107,  2111,  2115,  2119,  2120,  2183,  2186,
    2187,  2192,  2196,  2191,  2228,  2227,  2239,  2246,  2256,  2263,
    2262,  2275,  2279,  2274,  2293,  2292,  2358,  2362,  2385,  2389,
    2357,  2407,  2412,  2410,  2416,  2420,  2447,  2451,  2445,  2484,
    2519,  2527,  2518,  2540,  2544,  2538,  2565,  2583,  2592,  2596,
    2590,  2617,  2635,  2643,  2642,  2660,  2667,  2677,  2684,  2688,
    2683,  2709,  2713,  2707,  2749,  2755,  2769,  2774,  2780,  2787,
    2791,  2811,  2815,  2786,  2831,  2835,  2866,  2870,  2829,  2898,
    2903,  2909,  2915,  2916,  2926,  2925,  2934,  2933,  2944,  2949,
    2947,  2953,  2958,  2962,  2957,  2986,  2990,  2994,  3002,  3006,
    3001,  3016,  3023,  3027,  3022,  3037,  3043,  3052,  3050,  3065
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "CONST", "MODULE", "LOCAL",
  "INTERFACE", "TYPEDEF", "IDL_LONG", "IDL_SHORT", "UNSIGNED",
  "IDL_DOUBLE", "IDL_FLOAT", "IDL_CHAR", "IDL_WCHAR", "IDL_OCTET",
  "IDL_BOOLEAN", "ANY", "STRUCT", "UNION", "SWITCH", "ENUM", "SEQUENCE",
  "STRING", "WSTRING", "EXCEPTION", "CASE", "DEFAULT", "READONLY",
  "ATTRIBUTE", "ONEWAY", "IDEMPOTENT", "VOID", "IN", "OUT", "INOUT",
  "RAISES", "CUSTOM", "VALUETYPE", "TRUNCATABLE", "SUPPORTS", "IDL_PUBLIC",
  "IDL_PRIVATE", "FACTORY", "ABSTRACT", "IDL_CONTEXT", "OPAQUE", "VERSION",
  "INTEGER_LITERAL", "STRING_LITERAL", "CHARACTER_LITERAL",
  "FLOATING_PT_LITERAL", "TRUETOK", "FALSETOK", "SCOPE_DELIMITOR",
  "LEFT_SHIFT", "RIGHT_SHIFT", "PRAGMA", "PRAGMA_INCLUDE", "PFROM",
  "PRAGMA_ASYNC_CLIENT", "PRAGMA_ASYNC_SERVER", "PRAGMA_ID",
  "PRAGMA_PREFIX", "PRAGMA_VERSION", "PRAGMA_ANY", "PRAGMA_END", "';'",
  "'{'", "'}'", "':'", "','", "'('", "')'", "'='", "'|'", "'^'", "'&'",
  "'+'", "'-'", "'*'", "'/'", "'%'", "'~'", "'>'", "'<'", "'['", "']'",
  "$accept", "start", "definitions", "definition", "$@1", "$@2", "$@3",
  "$@4", "$@5", "$@6", "$@7", "$@8", "pragma", "module", "$@9", "$@10",
  "$@11", "$@12", "interface_def", "interface", "$@13", "$@14", "$@15",
  "interface_decl", "$@16", "interface_header", "inheritance_spec", "$@17",
  "value", "value_forward_dcl", "value_box_dcl", "value_abs_dcl", "$@18",
  "abstract_valuetype", "value_dcl", "$@19", "value_header",
  "value_inheritance_spec", "at_least_one_value_name", "value_names",
  "support_dcl", "value_name", "value_element", "value_elements",
  "state_member", "access_decl", "init_dcl", "$@20", "$@21",
  "init_param_dcls", "raises_expr", "init_param_dcl", "exports", "export",
  "$@22", "$@23", "$@24", "$@25", "$@26", "$@27",
  "at_least_one_scoped_name", "scoped_names", "$@28", "scoped_name",
  "$@29", "$@30", "id", "forward", "const_dcl", "$@31", "$@32", "$@33",
  "$@34", "const_type", "expression", "const_expr", "or_expr", "xor_expr",
  "and_expr", "shift_expr", "add_expr", "mult_expr", "unary_expr",
  "primary_expr", "literal", "string_literal", "positive_int_expr",
  "type_dcl", "$@35", "constr_forward_decl", "struct_decl", "$@36",
  "union_decl", "$@37", "type_declarator", "$@38", "type_spec",
  "simple_type_spec", "base_type_spec", "template_type_spec",
  "constr_type_spec", "at_least_one_declarator", "declarators", "$@39",
  "declarator", "simple_declarator", "complex_declarator", "integer_type",
  "signed_int", "unsigned_int", "floating_pt_type", "char_type",
  "octet_type", "boolean_type", "any_type", "struct_type", "$@40", "$@41",
  "at_least_one_member", "members", "member", "$@42", "$@43", "$@44",
  "union_type", "$@45", "$@46", "$@47", "$@48", "$@49", "$@50",
  "switch_type_spec", "at_least_one_case_branch", "case_branches",
  "case_branch", "$@51", "$@52", "$@53", "at_least_one_case_label",
  "case_labels", "case_label", "$@54", "$@55", "$@56", "element_spec",
  "$@57", "enum_type", "$@58", "$@59", "$@60", "$@61",
  "at_least_one_enumerator", "enumerators", "$@62", "enumerator",
  "sequence_type_spec", "$@63", "$@64", "seq_head", "$@65", "$@66",
  "string_type_spec", "$@67", "$@68", "string_head", "wstring_type_spec",
  "$@69", "$@70", "wstring_head", "array_declarator", "$@71",
  "at_least_one_array_dim", "array_dims", "array_dim", "$@72", "$@73",
  "attribute", "$@74", "$@75", "opt_readonly", "opt_local_or_abstract",
  "exception", "$@76", "$@77", "$@78", "$@79", "operation", "$@80", "$@81",
  "$@82", "$@83", "opt_op_attribute", "op_type_spec", "parameter_list",
  "$@84", "$@85", "at_least_one_parameter", "parameters", "$@86",
  "parameter", "$@87", "$@88", "direction", "opt_raises", "$@89", "$@90",
  "opt_context", "$@91", "$@92", "at_least_one_string_literal",
  "string_literals", "$@93", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,    59,   123,
     125,    58,    44,    40,    41,    61,   124,    94,    38,    43,
      45,    42,    47,    37,   126,    62,    60,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,    89,    90,    91,    91,    93,    92,    94,    92,    95,
      92,    96,    92,    97,    92,    98,    92,    99,    92,   100,
      92,    92,   101,   101,   101,   101,   101,   101,   101,   101,
     103,   104,   105,   106,   102,   107,   107,   109,   110,   111,
     108,   113,   112,   114,   116,   115,   115,   117,   117,   117,
     117,   118,   118,   119,   121,   120,   122,   124,   123,   125,
     125,   126,   126,   126,   127,   128,   128,   129,   129,   130,
     131,   131,   131,   132,   132,   133,   134,   134,   136,   137,
     135,   138,   138,   138,   139,   139,   140,   141,   141,   143,
     142,   144,   142,   145,   142,   146,   142,   147,   142,   148,
     142,   149,   151,   150,   150,   152,   153,   152,   154,   152,
     155,   156,   158,   159,   160,   161,   157,   162,   162,   162,
     162,   162,   162,   162,   162,   163,   164,   165,   165,   166,
     166,   167,   167,   168,   168,   168,   169,   169,   169,   170,
     170,   170,   170,   171,   171,   171,   171,   172,   172,   172,
     173,   173,   173,   173,   173,   173,   174,   174,   175,   177,
     176,   176,   176,   176,   176,   178,   178,   180,   179,   182,
     181,   184,   183,   185,   185,   186,   186,   186,   187,   187,
     187,   187,   187,   187,   188,   188,   188,   189,   189,   189,
     190,   192,   191,   191,   193,   193,   194,   195,   196,   196,
     197,   197,   197,   198,   198,   198,   199,   199,   199,   200,
     200,   201,   202,   203,   205,   206,   204,   207,   208,   208,
     210,   211,   209,   212,   209,   214,   215,   216,   217,   218,
     219,   213,   220,   220,   220,   220,   220,   220,   221,   222,
     222,   224,   225,   223,   226,   223,   227,   228,   228,   230,
     229,   231,   232,   229,   234,   233,   236,   237,   238,   239,
     235,   240,   242,   241,   241,   243,   245,   246,   244,   244,
     248,   249,   247,   251,   252,   250,   250,   253,   255,   256,
     254,   254,   257,   259,   258,   260,   261,   261,   263,   264,
     262,   266,   267,   265,   268,   268,   269,   269,   269,   271,
     272,   273,   274,   270,   276,   277,   278,   279,   275,   280,
     280,   280,   281,   281,   283,   282,   284,   282,   285,   287,
     286,   286,   289,   290,   288,   291,   291,   291,   293,   294,
     292,   292,   296,   297,   295,   295,   298,   300,   299,   299
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     0,     0,     3,     0,     3,     0,
       3,     0,     3,     0,     3,     0,     3,     0,     4,     0,
       3,     1,     5,     3,     3,     4,     3,     3,     4,     3,
       0,     0,     0,     0,     9,     1,     1,     0,     0,     0,
       7,     0,     3,     3,     0,     3,     0,     1,     1,     1,
       1,     2,     2,     3,     0,     7,     2,     0,     5,     3,
       4,     3,     4,     1,     2,     3,     0,     2,     0,     1,
       1,     1,     1,     2,     0,     4,     1,     1,     0,     0,
       9,     3,     1,     0,     4,     0,     3,     2,     0,     0,
       3,     0,     3,     0,     3,     0,     3,     0,     3,     0,
       3,     2,     0,     4,     0,     1,     0,     3,     0,     4,
       1,     2,     0,     0,     0,     0,     9,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       3,     1,     3,     1,     3,     3,     1,     3,     3,     1,
       3,     3,     3,     1,     2,     2,     2,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     2,     1,     1,     0,
       3,     1,     1,     1,     1,     1,     1,     0,     3,     0,
       3,     0,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     0,     4,     0,     1,     1,     1,     1,     1,     1,
       1,     2,     1,     2,     3,     2,     1,     1,     2,     1,
       1,     1,     1,     1,     0,     0,     6,     2,     2,     0,
       0,     0,     5,     0,     3,     0,     0,     0,     0,     0,
       0,    14,     1,     1,     1,     1,     1,     1,     2,     2,
       0,     0,     0,     5,     0,     3,     2,     2,     0,     0,
       3,     0,     0,     5,     0,     3,     0,     0,     0,     0,
       9,     2,     0,     4,     0,     1,     0,     0,     6,     2,
       0,     0,     5,     0,     0,     6,     1,     1,     0,     0,
       6,     1,     1,     0,     3,     2,     2,     0,     0,     0,
       5,     0,     0,     6,     1,     0,     1,     1,     0,     0,
       0,     0,     0,     9,     0,     0,     0,     0,    10,     1,
       1,     0,     1,     1,     0,     3,     0,     4,     2,     0,
       4,     0,     0,     0,     5,     1,     1,     1,     0,     0,
       6,     0,     0,     0,     6,     0,     2,     0,     4,     0
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,    19,   112,    30,   296,   159,   167,   169,   256,   299,
       0,     0,   297,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     2,     0,    21,    13,    11,    35,    37,
      15,    50,    49,    48,     0,    47,    57,    36,     7,     5,
     164,   165,   166,   161,   162,   163,     0,     9,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   110,    51,    56,
      17,     0,     0,   106,     0,   105,     0,     0,     0,     0,
       0,     1,     3,     0,     0,     0,     0,    52,     0,     0,
       0,   214,   225,    41,    46,     0,    20,   200,   202,     0,
     206,   207,   209,   210,   211,   212,   277,   282,   124,   113,
     117,   198,   199,   121,   118,   119,   120,   122,   276,   123,
     281,    31,   213,   270,   177,     0,     0,   160,   171,   173,
     175,   176,   174,   178,   179,   180,   182,   181,   183,   187,
     188,   189,   184,     0,   185,   186,   168,   170,   257,   300,
      68,     0,     0,    59,    63,    53,     0,    29,     0,     0,
     108,    23,    24,     0,    26,     0,    27,    14,    12,    38,
      16,    54,    74,     8,     6,     0,     0,     0,    44,    43,
      10,   201,   208,   203,   205,     0,   273,   278,     0,     0,
       0,   266,   269,     0,     0,    60,    67,   104,     0,    68,
      66,    69,    18,     0,   107,     0,    25,    28,    88,     0,
       0,   223,   220,   215,   219,   226,    42,     0,   204,   114,
       0,     0,    32,   271,   196,   172,   193,   194,   195,   197,
       0,   258,   301,   101,    68,    61,    64,    22,   109,     0,
      88,    99,   294,   309,   310,    76,    77,     0,    58,    73,
      71,     0,    72,    70,    91,    89,    95,     0,    93,    97,
       0,     0,     0,     0,     0,     0,    45,     0,   150,   157,
     152,   153,   154,   155,     0,     0,     0,     0,   147,   158,
     126,   127,   129,   131,   133,   136,   139,   143,   148,   151,
     274,   279,     0,     0,     0,   190,   267,     0,   219,   102,
      62,     0,     0,    87,     0,     0,    78,     0,     0,     0,
       0,   291,     0,     0,   313,   312,   304,   224,   221,   216,
     218,   200,   237,   232,   233,   234,   235,   227,   236,   115,
       0,   144,   145,   146,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   156,     0,     0,    33,   272,   288,
     284,   287,   191,     0,   265,   259,   264,     0,     0,    65,
      40,    55,   100,     0,     0,    92,    90,    96,     0,    94,
      98,     0,     0,     0,     0,   149,   128,   130,   132,   134,
     135,   137,   138,   140,   141,   142,   275,   280,     0,     0,
     285,     0,   268,     0,   261,     0,   103,    83,    75,   292,
     305,   222,   228,   116,   125,    34,   289,   286,   192,   260,
     262,   303,     0,     0,    82,     0,     0,     0,     0,     0,
       0,     0,    85,   293,   316,   306,   229,   290,   263,   196,
      86,    81,     0,    79,     0,     0,   331,     0,     0,     0,
     315,   325,   326,   327,     0,   321,   322,   328,   307,   244,
     251,   249,   230,   240,   241,   248,     0,    80,   317,   318,
       0,     0,   335,     0,     0,     0,     0,     0,     0,   246,
      84,   319,   323,   329,   332,   308,   245,   252,   250,   231,
     239,   254,   242,   247,     0,     0,     0,     0,     0,     0,
       0,   320,   324,     0,   333,   253,   255,   243,   330,     0,
     339,     0,   336,   334,   337,     0,   338
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    22,    23,    24,    80,    79,    85,    74,    73,    76,
     146,    48,    25,    26,    50,   178,   282,   378,    27,    28,
      75,   198,   292,    84,   167,    29,   169,   207,    30,    31,
      32,    33,   199,    34,    35,    78,    36,   143,   189,   226,
     144,   190,   239,   200,   240,   241,   242,   353,   429,   403,
     423,   404,   229,   293,   299,   298,   302,   300,   303,   295,
     186,   223,   348,   268,   149,   195,    65,    37,    38,    49,
     175,   257,   364,    99,   393,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,    39,    51,    40,
     115,    52,   116,    53,   117,   180,   202,   119,   120,   121,
     122,   215,   285,   381,   216,   217,   218,   123,   101,   102,
     124,   125,   126,   127,   128,   129,   165,   253,   203,   254,
     310,   252,   362,   251,   130,   166,   255,   363,   407,   427,
     456,   317,   442,   457,   443,   458,   480,   453,   444,   459,
     445,   455,   454,   478,   472,   479,   131,    54,   183,   287,
     383,   345,   384,   409,   346,   132,   220,   343,   133,   179,
     283,   134,   210,   335,   108,   135,   211,   336,   110,   219,
     284,   340,   380,   341,   379,   408,   246,   358,   405,   247,
      46,    47,    55,   184,   288,   385,   249,   361,   406,   426,
     452,   250,   306,   415,   424,   425,   434,   449,   474,   435,
     450,   475,   436,   438,   451,   476,   465,   477,   489,   491,
     492,   495
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -358
static const yytype_int16 yypact[] =
{
     674,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,
      18,    68,    69,    70,    91,    51,    34,    34,    34,    60,
      34,    34,   124,  -358,   310,  -358,  -358,  -358,  -358,  -358,
    -358,  -358,  -358,  -358,    68,  -358,  -358,  -358,  -358,  -358,
    -358,    93,   147,  -358,  -358,  -358,   163,  -358,   103,   793,
     170,   758,    68,    68,    68,    68,    68,  -358,   647,  -358,
    -358,   109,   115,  -358,    -5,  -358,    -4,    45,   110,     0,
      36,  -358,  -358,   111,   112,   113,   117,    -1,   114,   119,
     120,  -358,  -358,  -358,     1,   125,  -358,    90,  -358,   106,
    -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,   123,  -358,
    -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,   118,  -358,
     122,  -358,  -358,  -358,   123,    93,   147,  -358,  -358,  -358,
    -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,
    -358,  -358,  -358,   -33,  -358,  -358,  -358,  -358,  -358,  -358,
      19,    34,    56,  -358,  -358,  -358,   126,  -358,   131,    68,
    -358,  -358,  -358,   128,  -358,   129,  -358,  -358,  -358,  -358,
    -358,  -358,  -358,  -358,  -358,   740,   127,    68,  -358,  -358,
    -358,  -358,  -358,   183,  -358,    68,  -358,  -358,   130,   135,
      68,  -358,  -358,   132,   141,  -358,  -358,   123,    34,   156,
    -358,   123,  -358,   145,  -358,    68,  -358,  -358,  -358,   146,
     468,  -358,  -358,  -358,  -358,  -358,  -358,    34,  -358,  -358,
      25,    25,  -358,  -358,   136,  -358,  -358,  -358,  -358,  -358,
      25,  -358,  -358,   142,   156,  -358,   150,  -358,  -358,   517,
    -358,  -358,  -358,  -358,  -358,  -358,  -358,    68,  -358,  -358,
    -358,   758,  -358,  -358,  -358,  -358,  -358,   168,  -358,  -358,
     134,   149,    68,   154,   585,   811,  -358,   152,  -358,  -358,
    -358,  -358,  -358,  -358,    25,    33,    33,    33,   123,  -358,
     153,   151,   155,    64,    62,    31,  -358,  -358,  -358,   180,
    -358,  -358,   359,   776,   157,   162,  -358,   233,  -358,  -358,
    -358,    34,   167,  -358,   551,   174,  -358,    68,   175,   177,
     178,  -358,   179,   181,  -358,  -358,  -358,  -358,  -358,  -358,
    -358,   241,   123,  -358,  -358,  -358,  -358,  -358,  -358,  -358,
     182,  -358,  -358,  -358,    25,    25,    25,    25,    25,    25,
      25,    25,    25,    25,  -358,   166,   169,  -358,  -358,  -358,
    -358,  -358,  -358,   173,  -358,  -358,  -358,   613,    34,  -358,
    -358,  -358,  -358,   186,   187,  -358,  -358,  -358,   776,  -358,
    -358,   257,   193,   188,    25,  -358,   151,   155,    64,    62,
      62,    31,    31,  -358,  -358,  -358,  -358,  -358,   194,    25,
     157,    68,  -358,   195,   196,   197,   123,   232,  -358,  -358,
    -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,
    -358,  -358,   776,    53,  -358,    68,   198,   200,   184,   233,
      68,   232,   237,  -358,   201,  -358,  -358,  -358,  -358,  -358,
    -358,  -358,   203,  -358,   204,    95,   240,    66,    34,   202,
    -358,  -358,  -358,  -358,   205,  -358,  -358,  -358,  -358,  -358,
    -358,  -358,  -358,  -358,  -358,  -358,   206,  -358,  -358,   210,
     776,   214,   244,   215,    25,   221,   224,    37,   758,   108,
    -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,
    -358,  -358,  -358,  -358,    95,    68,    34,   222,   227,    68,
     231,  -358,  -358,   228,  -358,  -358,  -358,  -358,  -358,   251,
    -358,   230,   234,  -358,  -358,   258,  -358
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -358,  -358,   -19,  -358,  -358,  -358,  -358,  -358,  -358,  -358,
    -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,
    -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,
    -358,  -358,  -358,  -358,  -358,  -358,  -358,   -43,   121,  -358,
    -143,    16,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,
    -358,   -99,    83,   133,  -358,  -358,  -358,  -358,  -358,  -358,
    -203,  -358,  -358,    -2,  -358,  -358,   -11,  -358,  -175,  -358,
    -358,  -358,  -358,  -358,  -358,  -247,  -358,    -3,    -6,     5,
    -167,  -164,  -199,  -112,  -358,  -358,  -189,  -171,  -358,  -358,
       3,  -358,     6,  -358,  -358,  -358,   -50,  -230,  -358,  -358,
    -358,  -231,  -358,  -358,  -357,   -90,  -358,   -42,  -358,  -358,
     273,   -39,   -37,   -36,  -358,     9,  -358,  -358,  -358,    35,
     159,  -358,  -358,  -358,    11,  -358,  -358,  -358,  -358,  -358,
    -358,  -358,  -358,  -358,  -132,  -358,  -358,  -358,  -358,  -358,
    -133,  -358,  -358,  -358,  -358,  -358,     2,  -358,  -358,  -358,
    -358,  -358,  -358,  -358,   -82,  -358,  -358,  -358,  -358,  -358,
    -358,   279,  -358,  -358,  -358,   285,  -358,  -358,  -358,  -358,
    -358,  -358,  -358,   -45,  -358,  -358,  -358,  -358,  -358,  -358,
    -358,  -168,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,
    -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -137,
    -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,  -358,
    -358,  -358
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -315
static const yytype_int16 yytable[] =
{
      58,   118,    45,    41,   256,    72,    42,   100,   145,    43,
     104,    44,   105,   106,    64,    66,    67,   320,    69,    70,
     305,   308,   281,    77,   398,   244,    45,    41,    57,   245,
      42,   286,   248,    43,   161,    44,    57,    57,   439,   181,
     141,   136,   137,   138,   139,   140,   225,    98,   155,   114,
     150,   150,   182,   338,   244,   150,   114,    56,   245,    57,
     141,   248,   151,   152,   440,   441,   354,   439,   -68,  -111,
     142,    57,   168,    60,   258,   259,   260,   261,   262,   263,
      63,   290,   258,   259,   260,   261,   262,   263,    63,    63,
     142,   150,    61,   440,   441,   153,   188,   185,   264,   171,
     150,    62,   172,   156,   265,   266,   264,  -238,    59,   267,
      68,    63,   331,   332,   333,   173,   174,   394,   482,   244,
     327,   328,   486,   245,    71,   411,   248,   412,   389,   431,
     432,   433,   373,   374,   375,   440,   441,    57,   194,   187,
     191,   329,   330,    87,    88,    89,    90,    91,    92,    93,
      94,    95,   112,   321,   322,   323,   206,   113,    96,    97,
     369,   370,    81,   114,   209,   371,   372,   304,    82,   214,
      83,    86,   410,   111,   413,   148,   147,   154,   150,   157,
     158,   193,   159,   162,   228,   160,   191,   163,   164,    63,
     396,   297,   208,   170,   192,   196,   197,   141,   301,   212,
     205,   221,    45,    41,   176,   187,    42,   467,   177,    43,
     222,    44,   227,   313,   289,   230,   314,   307,   315,   316,
     462,   213,   291,  -283,   309,   446,   296,   319,   325,   324,
     334,    45,    41,   326,   342,    42,   344,   350,    43,   114,
      44,   214,   352,   355,   339,   356,   357,   359,   114,   360,
     171,   376,   114,   312,   377,   388,   365,   318,   382,   387,
     390,   391,   392,   337,   395,   399,   402,   401,   400,   416,
     447,   414,   417,   483,   422,  -314,   428,   437,   430,   448,
     460,   114,   461,   466,    45,    41,   214,   463,    42,   191,
     464,    43,   468,    44,   469,   484,    45,    41,   485,   487,
      42,   490,   488,    43,   493,    44,   494,   349,   496,   224,
      -4,     1,   421,   294,     2,     3,     4,  -298,     5,   367,
     420,   366,   103,   347,   204,   470,   473,   418,   107,     6,
       7,   368,     8,   243,   109,   397,     9,   481,     0,     0,
       0,     0,     0,     0,     0,   114,   386,     0,    10,    11,
       0,     0,     0,     0,     0,    12,   114,    13,     0,     0,
       1,     0,     0,     2,     3,     4,  -298,     5,    14,    15,
     214,    16,    17,    18,    19,    20,    21,     0,     6,     7,
      -4,     8,     0,     0,     0,     9,     0,     0,     0,     0,
       0,     0,     0,     0,   214,     0,     0,    10,    11,   419,
     114,     0,     0,     0,    12,     0,    13,     0,   471,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
      16,    17,    18,    19,    20,    21,   187,     0,     0,    -4,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   114,     0,
       0,     0,     0,     0,     0,     0,   114,     0,     0,     0,
       0,     0,     0,     0,   214,     0,     0,     0,   214,   231,
       0,  -311,     2,     0,   187,     0,     5,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,     6,     7,     0,
       8,  -311,  -311,  -311,     9,     0,     0,   232,  -295,   233,
     234,  -311,     0,     0,     0,     0,     0,     0,     0,     0,
     235,   236,   237,     0,     0,     0,     0,     0,   231,     0,
    -311,     2,     0,  -311,     0,     5,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,     6,     7,   238,     8,
    -311,  -311,  -311,     9,     0,     0,   232,  -295,   233,   234,
    -311,     0,   231,     0,  -311,     2,     0,     0,     0,     5,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
       6,     7,  -311,     8,  -311,  -311,  -311,     9,     0,     0,
     232,  -295,   233,   234,  -311,     0,   201,   -39,    57,     0,
       0,     0,     0,     0,    87,    88,    89,    90,    91,    92,
      93,    94,    95,   112,     6,     7,  -311,     8,   113,    96,
      97,     0,     0,     0,   201,     0,    57,     0,     0,     0,
       0,   351,    87,    88,    89,    90,    91,    92,    93,    94,
      95,   112,     6,     7,     0,     8,   113,    96,    97,     0,
      63,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      57,     0,     0,     0,     0,  -217,    87,    88,    89,    90,
      91,    92,    93,    94,    95,   112,     6,     7,    63,     8,
     113,    96,    97,     0,    -4,     1,     0,     0,     2,     3,
       4,  -298,     5,  -302,     0,     0,     0,     0,   141,     0,
       0,     0,     0,     6,     7,     0,     8,     0,     0,     0,
       9,     0,    63,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    10,    11,     0,     0,   -68,     0,   142,    12,
       0,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,    16,    17,    18,    19,    20,
      21,   201,     0,    57,     0,     0,     0,     0,     0,    87,
      88,    89,    90,    91,    92,    93,    94,    95,   112,     6,
       7,    57,     8,   113,    96,    97,     0,    87,    88,    89,
      90,    91,    92,    93,    94,    95,   112,     6,     7,    57,
       8,   113,    96,    97,     0,    87,    88,    89,    90,    91,
      92,    93,    94,    95,   112,    63,    57,     0,     0,   113,
      96,    97,    87,    88,    89,    90,    91,    92,    93,    94,
      95,     0,     0,    63,    57,     0,     0,    96,    97,     0,
     311,    88,    89,     0,     0,    92,    93,    94,    95,     0,
       0,    63,     0,     8,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    63,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    63
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-358))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      11,    51,     0,     0,   207,    24,     0,    49,    58,     0,
      49,     0,    49,    49,    16,    17,    18,   264,    20,    21,
     250,   252,   211,    34,   381,   200,    24,    24,     3,   200,
      24,   220,   200,    24,    77,    24,     3,     3,     1,    72,
      41,    52,    53,    54,    55,    56,   189,    49,    48,    51,
      55,    55,    85,   283,   229,    55,    58,    39,   229,     3,
      41,   229,    67,    67,    27,    28,   297,     1,    69,    68,
      71,     3,    71,     3,    49,    50,    51,    52,    53,    54,
      55,   224,    49,    50,    51,    52,    53,    54,    55,    55,
      71,    55,     1,    27,    28,    50,    40,   140,    73,     9,
      55,    50,    12,    67,    79,    80,    73,    70,    39,    84,
      50,    55,    81,    82,    83,     9,    10,   364,   475,   294,
      56,    57,   479,   294,     0,    72,   294,    74,   358,    34,
      35,    36,   331,   332,   333,    27,    28,     3,   149,   141,
     142,    79,    80,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,   265,   266,   267,   167,    23,    24,    25,
     327,   328,    69,   165,   175,   329,   330,    33,    21,   180,
       7,    68,   402,     3,   405,    60,    67,    67,    55,    68,
      68,    50,    69,    69,   195,    68,   188,    68,    68,    55,
     379,   241,     9,    68,    68,    67,    67,    41,    30,    69,
      73,    69,   200,   200,    86,   207,   200,   454,    86,   200,
      69,   200,    67,   255,    72,    69,   255,    68,   255,   255,
     450,    86,    72,    87,    70,   428,   237,    75,    77,    76,
      50,   229,   229,    78,    72,   229,     3,    70,   229,   241,
     229,   252,    68,    68,    87,    68,    68,    68,   250,    68,
       9,    85,   254,   255,    85,    68,    74,   255,    85,    73,
       3,    68,    74,   282,    70,    70,    34,    70,    72,    69,
      68,    73,    88,   476,    37,    74,    73,    37,    74,    74,
      74,   283,    72,    68,   282,   282,   297,    73,   282,   291,
      46,   282,    71,   282,    70,    73,   294,   294,    71,    68,
     294,    50,    74,   294,    74,   294,    72,   291,    50,   188,
       0,     1,   411,   230,     4,     5,     6,     7,     8,   325,
     410,   324,    49,   288,   165,   457,   459,   409,    49,    19,
      20,   326,    22,   200,    49,   380,    26,   474,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   347,   348,    -1,    38,    39,
      -1,    -1,    -1,    -1,    -1,    45,   358,    47,    -1,    -1,
       1,    -1,    -1,     4,     5,     6,     7,     8,    58,    59,
     381,    61,    62,    63,    64,    65,    66,    -1,    19,    20,
      70,    22,    -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   405,    -1,    -1,    38,    39,   410,
     402,    -1,    -1,    -1,    45,    -1,    47,    -1,   458,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,    59,    -1,
      61,    62,    63,    64,    65,    66,   428,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   450,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   458,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   475,    -1,    -1,    -1,   479,     1,
      -1,     3,     4,    -1,   476,    -1,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    -1,
      22,    23,    24,    25,    26,    -1,    -1,    29,    30,    31,
      32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,     1,    -1,
       3,     4,    -1,    55,    -1,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    70,    22,
      23,    24,    25,    26,    -1,    -1,    29,    30,    31,    32,
      33,    -1,     1,    -1,     3,     4,    -1,    -1,    -1,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    55,    22,    23,    24,    25,    26,    -1,    -1,
      29,    30,    31,    32,    33,    -1,     1,    70,     3,    -1,
      -1,    -1,    -1,    -1,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    55,    22,    23,    24,
      25,    -1,    -1,    -1,     1,    -1,     3,    -1,    -1,    -1,
      -1,    70,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    -1,    22,    23,    24,    25,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,    -1,    -1,    -1,    -1,    70,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    55,    22,
      23,    24,    25,    -1,     0,     1,    -1,    -1,     4,     5,
       6,     7,     8,    70,    -1,    -1,    -1,    -1,    41,    -1,
      -1,    -1,    -1,    19,    20,    -1,    22,    -1,    -1,    -1,
      26,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    39,    -1,    -1,    69,    -1,    71,    45,
      -1,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    58,    59,    -1,    61,    62,    63,    64,    65,
      66,     1,    -1,     3,    -1,    -1,    -1,    -1,    -1,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,     3,    22,    23,    24,    25,    -1,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,     3,
      22,    23,    24,    25,    -1,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    55,     3,    -1,    -1,    23,
      24,    25,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    -1,    -1,    55,     3,    -1,    -1,    24,    25,    -1,
       9,    10,    11,    -1,    -1,    14,    15,    16,    17,    -1,
      -1,    55,    -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    55
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     1,     4,     5,     6,     8,    19,    20,    22,    26,
      38,    39,    45,    47,    58,    59,    61,    62,    63,    64,
      65,    66,    90,    91,    92,   101,   102,   107,   108,   114,
     117,   118,   119,   120,   122,   123,   125,   156,   157,   176,
     178,   179,   181,   204,   213,   235,   269,   270,   100,   158,
     103,   177,   180,   182,   236,   271,    39,     3,   155,    39,
       3,     1,    50,    55,   152,   155,   152,   152,    50,   152,
     152,     0,    91,    97,    96,   109,    98,   155,   124,    94,
      93,    69,    21,     7,   112,    95,    68,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    24,    25,   152,   162,
     196,   197,   198,   199,   200,   201,   202,   250,   253,   254,
     257,     3,    18,    23,   152,   179,   181,   183,   185,   186,
     187,   188,   189,   196,   199,   200,   201,   202,   203,   204,
     213,   235,   244,   247,   250,   254,   155,   155,   155,   155,
     155,    41,    71,   126,   129,   185,    99,    67,    60,   153,
      55,    67,    67,    50,    67,    48,    67,    68,    68,    69,
      68,   126,    69,    68,    68,   205,   214,   113,    71,   115,
      68,     9,    12,     9,    10,   159,    86,    86,   104,   248,
     184,    72,    85,   237,   272,   126,   149,   152,    40,   127,
     130,   152,    68,    50,   155,   154,    67,    67,   110,   121,
     132,     1,   185,   207,   209,    73,   155,   116,     9,   155,
     251,   255,    69,    86,   155,   190,   193,   194,   195,   258,
     245,    69,    69,   150,   127,   129,   128,    67,   155,   141,
      69,     1,    29,    31,    32,    42,    43,    44,    70,   131,
     133,   134,   135,   142,   157,   176,   265,   268,   270,   275,
     280,   212,   210,   206,   208,   215,   149,   160,    49,    50,
      51,    52,    53,    54,    73,    79,    80,    84,   152,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   175,   105,   249,   259,   191,   175,   238,   273,    72,
     129,    72,   111,   142,   141,   148,   155,   185,   144,   143,
     146,    30,   145,   147,    33,   186,   281,    68,   190,    70,
     209,     9,   152,   196,   200,   201,   202,   220,   235,    75,
     164,   172,   172,   172,    76,    77,    78,    56,    57,    79,
      80,    81,    82,    83,    50,   252,   256,    91,   186,    87,
     260,   262,    72,   246,     3,   240,   243,   208,   151,   130,
      70,    70,    68,   136,   190,    68,    68,    68,   266,    68,
      68,   276,   211,   216,   161,    74,   166,   167,   168,   169,
     169,   170,   170,   171,   171,   171,    85,    85,   106,   263,
     261,   192,    85,   239,   241,   274,   152,    73,    68,   186,
       3,    68,    74,   163,   164,    70,   175,   262,   193,    70,
      72,    70,    34,   138,   140,   267,   277,   217,   264,   242,
     186,    72,    74,   190,    73,   282,    69,    88,   243,   155,
     194,   140,    37,   139,   283,   284,   278,   218,    73,   137,
      74,    34,    35,    36,   285,   288,   291,    37,   292,     1,
      27,    28,   221,   223,   227,   229,   149,    68,    74,   286,
     289,   293,   279,   226,   231,   230,   219,   222,   224,   228,
      74,    72,   186,    73,    46,   295,    68,   164,    71,    70,
     223,   185,   233,   229,   287,   290,   294,   296,   232,   234,
     225,   288,   193,   149,    73,    71,   193,    68,    74,   297,
      50,   298,   299,    74,    72,   300,    50
};

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL          goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY && yylen == 1)                          \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (1);                                           \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (YYID (0))


#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (YYID (N))                                                    \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
        break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 5:

/* Line 1806 of yacc.c  */
#line 267 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_TypeDeclSeen);
        }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 271 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_NoState);
        }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 275 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_ConstDeclSeen);
        }
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 279 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_NoState);
        }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 283 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_ExceptDeclSeen);
        }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 287 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_NoState);
        }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 291 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_InterfaceDeclSeen);
        }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 295 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_NoState);
        }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 299 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_ModuleDeclSeen);
        }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 304 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_ValueDeclSeen);
        }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 308 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_NoState);
        }
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 312 "idl.yy"
    {
           idl_global->set_parse_state(IDL_GlobalData::PS_OpaqueDeclSeen);
           UTL_Scope *s = idl_global->scopes()->top_non_null();
           UTL_ScopedName *n = new UTL_ScopedName (new Identifier ((yyvsp[(2) - (2)].strval)) ,NULL);
           AST_Opaque *o = NULL;
        
           if(s != NULL)
           {
              o = idl_global->gen()->create_opaque(n,s->get_pragmas());
              (void) s->fe_add_opaque(o);
           }
        }
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 325 "idl.yy"
    {
           idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 329 "idl.yy"
    {
           idl_global->err()->syntax_error(idl_global->parse_state());
        }
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 333 "idl.yy"
    {
           idl_global->set_parse_state (IDL_GlobalData::PS_NoState);
           yyerrok;
        }
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 343 "idl.yy"
    {
           UTL_IncludeFiles::AddIncludeFile((yyvsp[(2) - (5)].sval)->get_string(),(yyvsp[(4) - (5)].sval)->get_string());
        }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 348 "idl.yy"
    {
           UTL_Scope *s = idl_global->scopes()->top_non_null();
           AST_Decl *d = s->lookup_by_name((yyvsp[(2) - (3)].idlist),false);
           if(d)
           {
              if((AST_Interface *)d->narrow((long)&AST_Interface::type_id) ||
                 (AST_Operation *)d->narrow((long)&AST_Operation::type_id)) 
              {
                 d->get_decl_pragmas().set_pragma_client_synchronicity(false);
              }
              else
              {
                 idl_global->err()->warning_msg("Only operations and interfaces can be asynchronous.  Pragma ignored.");
              }
           }
           else
           {
              idl_global->err()->warning_msg("Identifier not found.");
           }
        }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 370 "idl.yy"
    {
           UTL_Scope *s = idl_global->scopes()->top_non_null();
           AST_Decl *d = s->lookup_by_name((yyvsp[(2) - (3)].idlist),false);
           if(d)
           {
              if((AST_Interface *)d->narrow((long)&AST_Interface::type_id) ||
                 (AST_Operation *)d->narrow((long)&AST_Operation::type_id)) 
              {
                 d->get_decl_pragmas().set_pragma_server_synchronicity(true);
              }
              else
              {
                 idl_global->err()->warning_msg("Only operations and interfaces can be asynchronous.  Pragma ignored.");
              }
           }
           else
           {
              idl_global->err()->warning_msg("Identifier not found.");
           }
        }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 392 "idl.yy"
    {
           UTL_Scope * s = idl_global->scopes()->top_non_null ();
           AST_Decl * d = s->lookup_by_name ((yyvsp[(2) - (4)].idlist), false);
           if(d)
           {
              if(!d->get_decl_pragmas().get_pragma_ID())
              {
                 d->get_decl_pragmas().set_pragma_ID
                    (new UTL_String ((yyvsp[(3) - (4)].sval)->get_string ()));
              }
              else
              {
                 idl_global->err()->warning_msg
                    ("Identifier already has ID. Pragma ignored.");
              }
           }
           else
           {
              idl_global->err()->warning_msg ("Identifier not found.");
           }
        }
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 415 "idl.yy"
    {
           UTL_Scope * s = idl_global->scopes()->top_non_null();
           s->get_pragmas().set_pragma_prefix ((yyvsp[(2) - (3)].sval));
        }
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 421 "idl.yy"
    {
           UTL_Scope * s = idl_global->scopes()->top_non_null ();
           AST_Decl * d = s->lookup_by_name ((yyvsp[(2) - (3)].idlist), false);

           if (d)
           {
              d->set_gen_any ();
           }
           else
           {
              idl_global->err()->error_msg ("Identifier not found.");
           }
        }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 436 "idl.yy"
    {
           UTL_Scope * s = idl_global->scopes()->top_non_null ();
           AST_Decl * d = s->lookup_by_name ((yyvsp[(2) - (4)].idlist), false);
           if (d)
           {
              if (!d->get_decl_pragmas().get_pragma_version ())
              {
                 d->get_decl_pragmas().set_pragma_version (new UTL_String ((yyvsp[(3) - (4)].strval)));
              }
              else
              {
                 idl_global->err()->warning_msg
                    ("Identifier already has version. Pragma ignored.");
              }
           }
           else
           {
              idl_global->err()->error_msg ("Identifier not found.");
           }
        }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 458 "idl.yy"
    {
           idl_global->err()->warning_msg ("Unrecognized pragma ignored.");
        }
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 464 "idl.yy"
    {
            idl_global->set_parse_state(IDL_GlobalData::PS_ModuleSeen);
          }
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 468 "idl.yy"
    {
            if (idl_global->valid_identifier((yyvsp[(3) - (3)].strval)))
            {
              Identifier * id = new Identifier ((yyvsp[(3) - (3)].strval));
              UTL_ScopedName *n = new UTL_ScopedName (id, NULL);
              AST_Module *m = NULL;
              UTL_Scope  *s = idl_global->scopes()->top_non_null();

              idl_global->set_parse_state(IDL_GlobalData::PS_ModuleIDSeen);
              /*
               * Make a new module and add it to the enclosing scope
               */
              if (s != NULL)
              {
                 AST_Module * m2;
                 m = idl_global->gen()->create_module (n, s->get_pragmas ());
                 bool inMainFile = m->in_main_file();
                 m2 = s->fe_add_module (m);

                 /* Check if have re-opened existing module */

                 if (m2 != m)
                 {
                    delete m;
                    m = m2;

                    if (inMainFile)
                    {
                       m->set_imported (false);
                    }
                 }
              }
              /*
               * Push it on the stack
               */
              idl_global->scopes()->push(m);
              g_feScopeStack.Push(be_CppEnclosingScope(*n,
                be_CppEnclosingScope::NameIsScope()));
            }
          }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 509 "idl.yy"
    {
            idl_global->set_parse_state(IDL_GlobalData::PS_ModuleSqSeen);
          }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 513 "idl.yy"
    {
            idl_global->set_parse_state(IDL_GlobalData::PS_ModuleBodySeen);
          }
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 517 "idl.yy"
    {
            idl_global->set_parse_state(IDL_GlobalData::PS_ModuleQsSeen);
            /*
             * Finished with this module - pop it from the scope stack
             */
            idl_global->scopes()->pop();
            g_feScopeStack.Pop();
          }
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 534 "idl.yy"
    {
          UTL_Scope     *s = idl_global->scopes()->top_non_null();
          AST_Interface *i = NULL;
          AST_Decl      *d = NULL;
          AST_Interface *fd = NULL;

          /*
           * Make a new interface node and add it to its enclosing scope
           */
          if (s != NULL && (yyvsp[(1) - (1)].ihval) != NULL) {
            i = idl_global->gen()->create_interface
            (
               (yyvsp[(1) - (1)].ihval)->local(),
               (yyvsp[(1) - (1)].ihval)->abstract(),
               (yyvsp[(1) - (1)].ihval)->interface_name(),
               (yyvsp[(1) - (1)].ihval)->inherits(),
               (yyvsp[(1) - (1)].ihval)->n_inherits(),
               s->get_pragmas()
            );
            if (i != NULL &&
                (d = s->lookup_by_name(i->name(), false)) != NULL) {
              /*
               * See if we're defining a forward declared interface.
               */
              if (d->node_type() == AST_Decl::NT_interface) {
                /*
                 * Narrow to an interface
                 */
                fd = AST_Interface::narrow_from_decl(d);
                /*
                 * Successful?
                 */
                if (fd == NULL) {
                  /*
                   * Should we give an error here?
                   */
                }
                /*
                 * If it is a forward declared interface..
                 */
                else if (!fd->is_defined()) {
                  /*
                   * Check if redefining in same scope
                   */
                  if (fd->defined_in() != s) {
                    idl_global->err()
                       ->error3(UTL_Error::EIDL_SCOPE_CONFLICT,
                                i,
                                fd,
                                ScopeAsDecl(s));
                  }
                  /*
                   * Check if fwd and interface not in same file
                   */

/*
                  if (fd->file_name() != idl_global->filename()) {
                    idl_global->err()
                       ->error1(UTL_Error::EIDL_FWD_DECL_LOOKUP,i);
                  }
*/

                  /*
                   * All OK, do the redefinition
                   */
                  else 
                  {
                     fd->set_local((yyvsp[(1) - (1)].ihval)->local());
                     fd->set_inherits((yyvsp[(1) - (1)].ihval)->inherits());
                     fd->set_n_inherits((yyvsp[(1) - (1)].ihval)->n_inherits());
                    /*
                     * Update place of definition
                     */
                     fd->set_imported(idl_global->imported());
                     fd->set_in_main_file(idl_global->in_main_file());
                     fd->set_line(idl_global->lineno());
                     fd->set_file_name(idl_global->filename());
          //fd->set_decl_pragmas(s->get_pragmas());
                     /*
                      * Use full definition node
                      */
                     delete i;
                     i = fd;
                  }
                }
              }
            }
            /*
             * Add the interface to its definition scope
             */
            (void) s->fe_add_interface (i);
          }
          /*
           * Push it on the scope stack
           */
          idl_global->scopes()->push(i);
          g_feScopeStack.Push(be_CppEnclosingScope(*((yyvsp[(1) - (1)].ihval)->interface_name()),
            be_CppEnclosingScope::NameIsScope()));
      }
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 634 "idl.yy"
    {
            idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceSqSeen);
        }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 638 "idl.yy"
    {
            idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceBodySeen);
        }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 642 "idl.yy"
    {
            idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceQsSeen);
            /*
             * Done with this interface - pop it off the scopes stack
             */
            g_feScopeStack.Pop();
            idl_global->scopes()->pop();
        }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 654 "idl.yy"
    {
            idl_global->set_parse_state (IDL_GlobalData::PS_InterfaceSeen);
         }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 658 "idl.yy"
    {
            idl_global->set_parse_state (IDL_GlobalData::PS_InterfaceIDSeen);
            (yyval.idval) = (yyvsp[(3) - (3)].idval);
         }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 666 "idl.yy"
    {
            bool local = ((yyvsp[(1) - (3)].cval) == 'L');
            bool abstract = ((yyvsp[(1) - (3)].cval) == 'A');

            idl_global->set_parse_state (IDL_GlobalData::PS_InheritSpecSeen);
            /*
             * Create an AST representation of the information in the header
             * part of an interface - this representation contains a computed
             * list of all interfaces which this interface inherits from,
             * recursively
             */
            (yyval.ihval) = new FE_InterfaceHeader (local, abstract, new UTL_ScopedName ((yyvsp[(2) - (3)].idval), NULL), (yyvsp[(3) - (3)].nlval));
        }
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 683 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_InheritColonSeen);
        }
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 687 "idl.yy"
    {
          (yyval.nlval) = (yyvsp[(3) - (3)].nlval);
        }
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 691 "idl.yy"
    {
          (yyval.nlval) = NULL;
        }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 705 "idl.yy"
    {
              UTL_Scope         *s = idl_global->scopes()->top_non_null();
              UTL_ScopedName    *n = new UTL_ScopedName((yyvsp[(2) - (2)].idval), NULL);
              AST_ValueFwd      *f = NULL;

              idl_global->set_parse_state(IDL_GlobalData::PS_ValueForwardDeclSeen);
              /*
               * Create a node representing a forward declaration of a
               * value type. Store it in the enclosing scope
               */
              if (s != NULL) {
                f = idl_global->gen()->create_valuetype_fwd (false, n, s->get_pragmas());
                (void) s->fe_add_valuetype_fwd(f);
              }
        }
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 721 "idl.yy"
    {
              UTL_Scope         *s = idl_global->scopes()->top_non_null();
              UTL_ScopedName    *n = new UTL_ScopedName((yyvsp[(2) - (2)].idval), NULL);
              AST_ValueFwd      *f = NULL;

              idl_global->set_parse_state(IDL_GlobalData::PS_ValueForwardDeclSeen);
              /*
               * Create a node representing a forward declaration of a
               * value type. Store it in the enclosing scope
               */
              if (s != NULL) {
                f = idl_global->gen()->create_valuetype_fwd
                   (true, n, s->get_pragmas());
                (void) s->fe_add_valuetype_fwd(f);
              }
        }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 741 "idl.yy"
    {
              UTL_Scope         *s = idl_global->scopes()->top_non_null();
              UTL_ScopedName    *n = new UTL_ScopedName((yyvsp[(2) - (3)].idval), NULL);
              AST_BoxedValue    *b = NULL;
              AST_Type          *tp = AST_Type::narrow_from_decl((yyvsp[(3) - (3)].dcval));

              idl_global->set_parse_state(IDL_GlobalData::PS_BoxedValueDeclSeen);
              if (tp == NULL)
              {
                 idl_global->err()->not_a_type((yyvsp[(3) - (3)].dcval));
              }

              /*
               * Create a node representing a forward declaration of a
               * value type. Store it in the enclosing scope
               */
              else if (s != NULL)
              {
                 b = idl_global->gen()->create_boxed_valuetype(n, tp, s->get_pragmas());
                 (void) s->fe_add_boxed_valuetype(b);
              }
        }
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 767 "idl.yy"
    {
         UTL_Scope      *s = idl_global->scopes()->top_non_null();
         AST_Value      *i = NULL;
         AST_Decl       *d = NULL;
         AST_Value     *fd = NULL;
         UTL_ScopedName *n = new UTL_ScopedName((yyvsp[(2) - (3)].idval), NULL);

         /*
          * Make a new interface node and add it to its enclosing scope
          */
         if (s != NULL)
         {
            i = idl_global->gen()->create_valuetype
            (
               true,
               false,
               (yyvsp[(3) - (3)].visval)->truncatable(),
               n,
               (yyvsp[(3) - (3)].visval)->inherits(),
               (yyvsp[(3) - (3)].visval)->n_inherits(),
               (yyvsp[(3) - (3)].visval)->supports(),
               (yyvsp[(3) - (3)].visval)->n_supports(),
               s->get_pragmas()
            );

            if (i != NULL &&
                (d = s->lookup_by_name(i->name(), false)) != NULL) {
               /*
                * See if we're defining a forward declared interface.
                */
               if (d->node_type() == AST_Decl::NT_value) {
                  /*
                   * Narrow to an interface
                   */
                  fd = AST_Value::narrow_from_decl(d);
                  /*
                   * If it is a forward declared interface..
                   */
                  if (fd && !fd->is_defined()) {
                     /*
                      * Check if redefining in same scope
                      */
                     if (fd->defined_in() != s) {
                        idl_global->err()
                        ->error3(UTL_Error::EIDL_SCOPE_CONFLICT,
                                 i,
                                 fd,
                                 ScopeAsDecl(s));
                     }
                     /*
                      * All OK, do the redefinition
                      */
                     else {
                        fd->set_value_inherits((yyvsp[(3) - (3)].visval)->inherits());
                        fd->set_n_value_inherits((yyvsp[(3) - (3)].visval)->n_inherits());
                        fd->set_inherits((yyvsp[(3) - (3)].visval)->supports());
                        fd->set_n_inherits((yyvsp[(3) - (3)].visval)->n_supports());
                        /*
                         * Update place of definition
                         */
                        fd->set_imported(idl_global->imported());
                        fd->set_in_main_file(idl_global->in_main_file());
                        fd->set_line(idl_global->lineno());
                        fd->set_file_name(idl_global->filename());
                        /*
                         * Use full definition node
                         */
                        delete i;
                        i = fd;
                     }
                  }
               }
            }
            /*
             * Add the interface to its definition scope
             */
            (void) s->fe_add_valuetype(i);
         }
         /*
          * Push it on the scope stack
          */
         idl_global->scopes()->push(i);
         g_feScopeStack.Push(be_CppEnclosingScope(*n,
                                                  be_CppEnclosingScope::NameIsScope()));
      }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 853 "idl.yy"
    {
            idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceQsSeen);
            /*
             * Done with this interface - pop it off the scopes stack
             */
            g_feScopeStack.Pop();
            idl_global->scopes()->pop();
        }
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 869 "idl.yy"
    {
         UTL_Scope     *s = idl_global->scopes()->top_non_null();
         AST_Value     *i = NULL;
         AST_Decl      *d = NULL;
         AST_Value     *fd = NULL;

         /*
          * Make a new interface node and add it to its enclosing scope
          */
         if (s != NULL && (yyvsp[(1) - (1)].vhval) != NULL) {
            i = idl_global->gen()->create_valuetype(false,
                                                    (yyvsp[(1) - (1)].vhval)->custom(),
                                                    (yyvsp[(1) - (1)].vhval)->truncatable(),
                                                    (yyvsp[(1) - (1)].vhval)->value_name(),
                                                    (yyvsp[(1) - (1)].vhval)->inherits(),
                                                    (yyvsp[(1) - (1)].vhval)->n_inherits(),
                                                    (yyvsp[(1) - (1)].vhval)->supports(),
                                                    (yyvsp[(1) - (1)].vhval)->n_supports(),
                                                    s->get_pragmas());
            if (i != NULL &&
                (d = s->lookup_by_name(i->name(), false)) != NULL) {
               /*
                * See if we're defining a forward declared interface.
                */
               if (d->node_type() == AST_Decl::NT_value) {
                  /*
                   * Narrow to an interface
                   */
                  fd = AST_Value::narrow_from_decl(d);
                  /*
                   * If it is a forward declared interface..
                   */
                  if (fd && !fd->is_defined()) {
                     /*
                      * Check if redefining in same scope
                      */
                     if (fd->defined_in() != s)
                     {
                        idl_global->err()
                        ->error3(UTL_Error::EIDL_SCOPE_CONFLICT,
                                 i,
                                 fd,
                                 ScopeAsDecl(s));
                     }
                     /*
                      * All OK, do the redefinition
                      */
                     else 
                     {
                        fd->set_custom((yyvsp[(1) - (1)].vhval)->custom());
                        fd->set_truncatable((yyvsp[(1) - (1)].vhval)->truncatable());
                        fd->set_value_inherits((yyvsp[(1) - (1)].vhval)->inherits());
                        fd->set_n_value_inherits((yyvsp[(1) - (1)].vhval)->n_inherits());
                        fd->set_inherits((yyvsp[(1) - (1)].vhval)->supports());
                        fd->set_n_inherits((yyvsp[(1) - (1)].vhval)->n_supports());
                        /*
                         * Update place of definition
                         */
                        fd->set_imported(idl_global->imported());
                        fd->set_in_main_file(idl_global->in_main_file());
                        fd->set_line(idl_global->lineno());
                        fd->set_file_name(idl_global->filename());
                        /*
                         * Use full definition node
                         */
                        delete i;
                        i = fd;
                     }
                  }
               }
            }
            /*
             * Add the interface to its definition scope
             */
            (void) s->fe_add_valuetype(i);
         }
         /*
          * Push it on the scope stack
          */
         idl_global->scopes()->push(i);
         g_feScopeStack.Push(be_CppEnclosingScope(*(i->name()),
                                                  be_CppEnclosingScope::NameIsScope()));
      }
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 953 "idl.yy"
    {
           idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceQsSeen);
            /*
             * Done with this interface - pop it off the scopes stack
             */
           g_feScopeStack.Pop();
           idl_global->scopes()->pop();
        }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 965 "idl.yy"
    {
           idl_global->set_parse_state(IDL_GlobalData::PS_ValueInheritSpecSeen);
           (yyval.vhval) = new FE_ValueHeader (false, new UTL_ScopedName((yyvsp[(2) - (3)].idval), NULL), (yyvsp[(3) - (3)].visval));
        }
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 970 "idl.yy"
    {
           idl_global->set_parse_state(IDL_GlobalData::PS_ValueInheritSpecSeen);
           (yyval.vhval) = new FE_ValueHeader (true, new UTL_ScopedName((yyvsp[(3) - (4)].idval), NULL), (yyvsp[(4) - (4)].visval));
        }
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 978 "idl.yy"
    {
           (yyval.visval) = new FE_ValueInheritanceSpec (false, (yyvsp[(2) - (3)].nlval), (yyvsp[(3) - (3)].nlval));
        }
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 982 "idl.yy"
    {
           (yyval.visval) = new FE_ValueInheritanceSpec (true, (yyvsp[(3) - (4)].nlval), (yyvsp[(4) - (4)].nlval));
        }
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 986 "idl.yy"
    {
           (yyval.visval) = new FE_ValueInheritanceSpec (true, NULL, (yyvsp[(1) - (1)].nlval));
        }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 993 "idl.yy"
    {
          (yyval.nlval) = new UTL_NameList((yyvsp[(1) - (2)].idlist), (yyvsp[(2) - (2)].nlval));
        }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 1000 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_ValueNameSeen);

          if ((yyvsp[(1) - (3)].nlval) == NULL)
          {
            (yyval.nlval) = new UTL_NameList((yyvsp[(3) - (3)].idlist), NULL);
          }
          else
          {
            (yyvsp[(1) - (3)].nlval)->nconc(new UTL_NameList((yyvsp[(3) - (3)].idlist), NULL));
            (yyval.nlval) = (yyvsp[(1) - (3)].nlval);
          }
        }
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 1014 "idl.yy"
    {
          (yyval.nlval) = NULL;
        }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 1021 "idl.yy"
    {
          (yyval.nlval) = (yyvsp[(2) - (2)].nlval);
        }
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 1025 "idl.yy"
    {
          (yyval.nlval) = NULL;
        }
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 1047 "idl.yy"
    {
           UTL_Scope            *s = idl_global->scopes()->top_non_null();
           UTL_DecllistActiveIterator *l = NULL;
           AST_StateMember              *m = NULL;
           FE_Declarator                *d = NULL;

           /* idl_global->set_parse_state(IDL_GlobalData::PS_AttrCompleted); */
           /*
            * Create nodes representing attributes and add them to the
            * enclosing scope
            */
           if (s != NULL && (yyvsp[(2) - (4)].dcval) != NULL && (yyvsp[(3) - (4)].dlval) != NULL)
           {
              l = new UTL_DecllistActiveIterator((yyvsp[(3) - (4)].dlval));
              for (;!(l->is_done()); l->next())
              {
                 d = l->item();
                 if (d == NULL)
                    continue;
                 AST_Type *tp = d->compose((yyvsp[(2) - (4)].dcval));
                 if (tp == NULL)
                    continue;

                 // Check for anonymous type
                 if ((tp->node_type () == AST_Decl::NT_array)
                     || (tp->node_type () == AST_Decl::NT_sequence))
                 {
                    Identifier * id = d->name ()->head ();
                    const char *postfix =
                       (tp->node_type () == AST_Decl::NT_array)
                       ? "" : "_seq";
                    // first underscore removed by Identifier constructor
                    DDS_StdString anon_type_name =
                       DDS_StdString ("__") + DDS_StdString (id->get_string ())
                          + DDS_StdString (postfix);
                    UTL_ScopedName *anon_scoped_name =
                    new UTL_ScopedName
                    (
                       new Identifier (os_strdup(anon_type_name)),
                       NULL
                    );

                    (void) s->fe_add_typedef
                    (
                       idl_global->gen()->create_typedef
                       (
                          tp,
                          anon_scoped_name,
                          s->get_pragmas ()
                       )
                    );
                 }

                 m = idl_global->gen()->create_state_member((yyvsp[(1) - (4)].bval), tp, d->name(), s->get_pragmas());
                 /*
                  * Add one attribute to the enclosing scope
                  */
                 (void) s->fe_add_state_member(m);
              }
              delete l;
           }
        }
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 1113 "idl.yy"
    {
          (yyval.bval) = true;
        }
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 1117 "idl.yy"
    {
          (yyval.bval) = false;
        }
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 1124 "idl.yy"
    {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_ScopedName        *n = new UTL_ScopedName((yyvsp[(2) - (2)].idval), NULL);
          AST_Initializer       *i = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_InitializerIDSeen);
          /*
           * Create a node representing an operation on an interface
           * and add it to its enclosing scope
           */
          if (s != NULL)
          {
             i = idl_global->gen()->create_initializer(n, s->get_pragmas());
             (void) s->fe_add_initializer(i);
          }
          /*
           * Push the initilializer scope onto the scopes stack
           */
          idl_global->scopes()->push(i);
        }
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 1145 "idl.yy"
    {
          idl_global->scopes()->pop();
        }
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 1164 "idl.yy"
    {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          AST_Argument          *a = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_InitParDeclSeen);
          /*
           * Create a node representing an argument to an initializer
           * Add it to the enclosing scope (the initializer scope)
           */
          if ((yyvsp[(2) - (3)].dcval) != NULL && (yyvsp[(3) - (3)].deval) != NULL && s != NULL) {
            AST_Type *tp = (yyvsp[(3) - (3)].deval)->compose((yyvsp[(2) - (3)].dcval));
            if (tp != NULL) {
              a = idl_global->gen()->create_argument(AST_Argument::dir_IN, tp, (yyvsp[(3) - (3)].deval)->name(), s->get_pragmas());
              (void) s->fe_add_argument(a);
            }
          }
        }
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 1190 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_TypeDeclSeen);
        }
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 1194 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        }
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 1198 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_ConstDeclSeen);
        }
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 1202 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        }
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 1206 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptDeclSeen);
        }
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 1210 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        }
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 1214 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_AttrDeclSeen);
        }
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 1218 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        }
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 1222 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpDeclSeen);
        }
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 1226 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        }
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 1230 "idl.yy"
    {
          idl_global->err()->syntax_error(idl_global->parse_state());
        }
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 1234 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
          yyerrok;
        }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 1242 "idl.yy"
    {
          (yyval.nlval) = new UTL_NameList((yyvsp[(1) - (2)].idlist), (yyvsp[(2) - (2)].nlval));
        }
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 1250 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_SNListCommaSeen);
        }
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 1254 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_ScopedNameSeen);

          if ((yyvsp[(1) - (4)].nlval) == NULL)
            (yyval.nlval) = new UTL_NameList((yyvsp[(4) - (4)].idlist), NULL);
          else {
            (yyvsp[(1) - (4)].nlval)->nconc(new UTL_NameList((yyvsp[(4) - (4)].idlist), NULL));
            (yyval.nlval) = (yyvsp[(1) - (4)].nlval);
          }
        }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 1265 "idl.yy"
    {
          (yyval.nlval) = NULL;
        }
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 1272 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_SN_IDSeen);

          (yyval.idlist) = new UTL_IdList ((yyvsp[(1) - (1)].idval), NULL);
        }
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 1278 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_ScopeDelimSeen);
        }
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 1282 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_SN_IDSeen);

          (yyval.idlist) = new UTL_IdList (new Identifier ((yyvsp[(1) - (3)].strval)), new UTL_IdList((yyvsp[(3) - (3)].idval), NULL));
        }
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 1288 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_ScopeDelimSeen);
        }
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 1292 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_SN_IDSeen);

          (yyvsp[(1) - (4)].idlist)->nconc (new UTL_IdList ((yyvsp[(4) - (4)].idval), NULL));
          (yyval.idlist) = (yyvsp[(1) - (4)].idlist);
        }
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 1301 "idl.yy"
    {
           (void) idl_global->valid_identifier ((yyvsp[(1) - (1)].strval));
           (yyval.idval) = new Identifier ((yyvsp[(1) - (1)].strval));
        }
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 1309 "idl.yy"
    {
           UTL_Scope * s = idl_global->scopes()->top_non_null ();
           UTL_ScopedName * n = new UTL_ScopedName ((yyvsp[(2) - (2)].idval), NULL);
           AST_InterfaceFwd * f = NULL;

           bool local = ((yyvsp[(1) - (2)].cval) == 'L');
           bool abstract = ((yyvsp[(1) - (2)].cval) == 'A');

           idl_global->set_parse_state (IDL_GlobalData::PS_ForwardDeclSeen);
           /*
           * Create a node representing a forward declaration of an
           * interface. Store it in the enclosing scope
           */
           if (s != NULL) 
           {
              f = idl_global->gen()->create_interface_fwd
                 (local, abstract, n, s->get_pragmas ());
              (void) s->fe_add_interface_fwd (f);
           }
        }
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 1333 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_ConstSeen);
        }
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 1337 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_ConstTypeSeen);
        }
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 1341 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_ConstIDSeen);
        }
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 1345 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_ConstAssignSeen);
        }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 1349 "idl.yy"
    {
          UTL_ScopedName *n = new UTL_ScopedName ((yyvsp[(5) - (9)].idval), NULL);
          UTL_Scope      *s = idl_global->scopes()->top_non_null ();
          AST_Constant   *c = NULL;

          idl_global->set_parse_state (IDL_GlobalData::PS_ConstExprSeen);

          /*
           * Create a node representing a constant declaration. Store
           * it in the enclosing scope
           */

          if ((yyvsp[(9) - (9)].exval) != NULL && s != NULL)
          {
            if ((yyvsp[(9) - (9)].exval)->coerce ((yyvsp[(3) - (9)].etval)) == NULL)
            {
              idl_global->err()->coercion_error ((yyvsp[(9) - (9)].exval), (yyvsp[(3) - (9)].etval));
            }
            else
            {
              c = idl_global->gen()->create_constant
                 ((yyvsp[(3) - (9)].etval), (yyvsp[(9) - (9)].exval), n, s->get_pragmas ());
              (void) s->fe_add_constant (c);
            }
          }
        }
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 1384 "idl.yy"
    {
          (yyval.etval) = AST_Expression::EV_string;
        }
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 1388 "idl.yy"
    {
          (yyval.etval) = AST_Expression::EV_wstring;
        }
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 1392 "idl.yy"
    {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          AST_Decl              *d = NULL;
          AST_PredefinedType    *c = NULL;
          AST_Typedef           *t = NULL;

          /*
           * If the constant's type is a scoped name, it must resolve
           * to a scalar constant type
           */
          if (s != NULL && (d = s->lookup_by_name((yyvsp[(1) - (1)].idlist), true)) != NULL) {
            /*
             * Look through typedefs
             */
            while (d->node_type() == AST_Decl::NT_typedef) {
              t = AST_Typedef::narrow_from_decl(d);
              if (t == NULL)
                break;
              d = t->base_type();
            }
            if (d == NULL)
              (yyval.etval) = AST_Expression::EV_any;
            else if (d->node_type() == AST_Decl::NT_pre_defined) {
               c = AST_PredefinedType::narrow_from_decl(d);
               if (c != NULL) {
                  (yyval.etval) = idl_global->PredefinedTypeToExprType(c->pt());
               } else {
                  (yyval.etval) = AST_Expression::EV_any;
               }
            } else if (d->node_type() == AST_Decl::NT_string) {
               (yyval.etval) = AST_Expression::EV_string;
            } else
               (yyval.etval) = AST_Expression::EV_any;
          } else
             (yyval.etval) = AST_Expression::EV_any;
        }
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 1436 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr(AST_Expression::EC_or, (yyvsp[(1) - (3)].exval), (yyvsp[(3) - (3)].exval));
        }
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 1444 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr(AST_Expression::EC_xor, (yyvsp[(1) - (3)].exval), (yyvsp[(3) - (3)].exval));
        }
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 1452 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr(AST_Expression::EC_and, (yyvsp[(1) - (3)].exval), (yyvsp[(3) - (3)].exval));
        }
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 1460 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr(AST_Expression::EC_left,(yyvsp[(1) - (3)].exval),(yyvsp[(3) - (3)].exval));
        }
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 1464 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr(AST_Expression::EC_right,(yyvsp[(1) - (3)].exval),(yyvsp[(3) - (3)].exval));
        }
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 1472 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr(AST_Expression::EC_add, (yyvsp[(1) - (3)].exval), (yyvsp[(3) - (3)].exval));
        }
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 1476 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr(AST_Expression::EC_minus,(yyvsp[(1) - (3)].exval),(yyvsp[(3) - (3)].exval));
        }
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 1484 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr(AST_Expression::EC_mul, (yyvsp[(1) - (3)].exval), (yyvsp[(3) - (3)].exval));
        }
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 1488 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr(AST_Expression::EC_div, (yyvsp[(1) - (3)].exval), (yyvsp[(3) - (3)].exval));
        }
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 1492 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr(AST_Expression::EC_mod, (yyvsp[(1) - (3)].exval), (yyvsp[(3) - (3)].exval));
        }
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 1500 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr(AST_Expression::EC_u_plus,
                                              (yyvsp[(2) - (2)].exval),
                                              NULL);
        }
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 1506 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr(AST_Expression::EC_u_minus,
                                              (yyvsp[(2) - (2)].exval),
                                              NULL);
        }
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 1512 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr(AST_Expression::EC_bit_neg,
                                              (yyvsp[(2) - (2)].exval),
                                              NULL);
        }
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 1521 "idl.yy"
    {
          /*
           * An expression which is a scoped name is not resolved now,
           * but only when it is evaluated (such as when it is assigned
           * as a constant value)
           */
          (yyval.exval) = idl_global->gen()->create_expr((yyvsp[(1) - (1)].idlist));
        }
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 1531 "idl.yy"
    {
          (yyval.exval) = (yyvsp[(2) - (3)].exval);
        }
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 1538 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr ((yyvsp[(1) - (1)].ival));
        }
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 1542 "idl.yy"
    {
          (yyval.exval) = (yyvsp[(1) - (1)].exval);
        }
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 1546 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr ((yyvsp[(1) - (1)].cval));
        }
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 1550 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr (atof ((yyvsp[(1) - (1)].strval)), (yyvsp[(1) - (1)].strval));
        }
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 1554 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr((long) 1,
                                            AST_Expression::EV_bool);
        }
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 1559 "idl.yy"
    {
          (yyval.exval) = idl_global->gen()->create_expr((long) 0,
                                            AST_Expression::EV_bool);
        }
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 1567 "idl.yy"
    {
           int len = strlen((yyvsp[(1) - (2)].exval)->ev()->u.strval->get_string());
           len += strlen((yyvsp[(2) - (2)].sval)->get_string());
           char *combined = new char[len+1];
           combined[0] = '\0';
           strcat(combined, (yyvsp[(1) - (2)].exval)->ev()->u.strval->get_string());
           strcat(combined, (yyvsp[(2) - (2)].sval)->get_string());
           UTL_String *str = new UTL_String(combined);
           delete (yyvsp[(1) - (2)].exval)->ev()->u.strval;
           (yyvsp[(1) - (2)].exval)->ev()->u.strval = str;
           (yyval.exval) = (yyvsp[(1) - (2)].exval);
        }
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 1580 "idl.yy"
    {
           (yyval.exval) = idl_global->gen()->create_expr ((yyvsp[(1) - (1)].sval));
        }
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 1587 "idl.yy"
    {
            (yyvsp[(1) - (1)].exval)->evaluate (AST_Expression::EK_const);
            (yyval.exval) = idl_global->gen()->create_expr ((yyvsp[(1) - (1)].exval), AST_Expression::EV_ulong);
        }
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 1595 "idl.yy"
    {
            idl_global->set_parse_state (IDL_GlobalData::PS_TypedefSeen);
          }
    break;

  case 165:

/* Line 1806 of yacc.c  */
#line 1607 "idl.yy"
    {
           UTL_Scope * s = idl_global->scopes()->top_non_null ();
           UTL_ScopedName * n = new UTL_ScopedName ((yyvsp[(1) - (1)].idval), NULL);
           AST_Structure * str;

           idl_global->set_parse_state (IDL_GlobalData::PS_ForwardDeclSeen);

           if (s != NULL) 
           {
              str = idl_global->gen()->create_structure (n, s->get_pragmas ());
              (void) s->fe_add_structure (str);
           }
        }
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 1622 "idl.yy"
    {
           UTL_Scope * s = idl_global->scopes()->top_non_null ();
           UTL_ScopedName * n = new UTL_ScopedName ((yyvsp[(1) - (1)].idval), NULL);
           AST_Union * u;

           idl_global->set_parse_state (IDL_GlobalData::PS_ForwardDeclSeen);

           if (s != NULL) 
           {
              u = idl_global->gen()->create_union (n, s->get_pragmas ());
              (void) s->fe_add_union (u);
           }
        }
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 1639 "idl.yy"
    {
           idl_global->set_parse_state (IDL_GlobalData::PS_StructSeen);
        }
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 1643 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_StructIDSeen);
          (yyval.idval) = (yyvsp[(3) - (3)].idval);
        }
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 1651 "idl.yy"
    {
           idl_global->set_parse_state (IDL_GlobalData::PS_UnionSeen);
        }
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 1655 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_UnionIDSeen);
          (yyval.idval) = (yyvsp[(3) - (3)].idval);
        }
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 1663 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_TypeSpecSeen);
        }
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 1667 "idl.yy"
    {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_DecllistActiveIterator *l;
          FE_Declarator         *d = NULL;
          AST_Typedef           *t = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_DeclaratorsSeen);
          /*
           * Create a list of type renamings. Add them to the
           * enclosing scope
           */
          if (s != NULL && (yyvsp[(1) - (3)].dcval) != NULL && (yyvsp[(3) - (3)].dlval) != NULL) {
            l = new UTL_DecllistActiveIterator((yyvsp[(3) - (3)].dlval));
            for (;!(l->is_done()); l->next()) {
              d = l->item();
              if (d == NULL)
                continue;
              AST_Type * tp = d->compose((yyvsp[(1) - (3)].dcval));
              if (tp == NULL)
                continue;
              t = idl_global->gen()->create_typedef(tp, d->name(), s->get_pragmas());
              (void) s->fe_add_typedef(t);
            }
            delete l;
          }
        }
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 1702 "idl.yy"
    {
          (yyval.dcval) = idl_global->scopes()->bottom()->lookup_primitive_type((yyvsp[(1) - (1)].etval));
        }
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 1707 "idl.yy"
    {
          UTL_Scope     *s = idl_global->scopes()->top_non_null();
          AST_Decl      *d = NULL;

          if (s != NULL)
            d = s->lookup_by_name((yyvsp[(1) - (1)].idlist), true);
          if (d == NULL || d->node_type() == AST_Decl::NT_field)
            idl_global->err()->lookup_error((yyvsp[(1) - (1)].idlist));
          (yyval.dcval) = d;
        }
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 1742 "idl.yy"
    {
          (yyval.dlval) = new UTL_DeclList((yyvsp[(1) - (2)].deval), (yyvsp[(2) - (2)].dlval));
        }
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 1749 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_DeclsCommaSeen);
        }
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 1753 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_DeclsDeclSeen);

          if ((yyvsp[(1) - (4)].dlval) == NULL)
            (yyval.dlval) = new UTL_DeclList((yyvsp[(4) - (4)].deval), NULL);
          else {
            (yyvsp[(1) - (4)].dlval)->nconc(new UTL_DeclList((yyvsp[(4) - (4)].deval), NULL));
            (yyval.dlval) = (yyvsp[(1) - (4)].dlval);
          }
        }
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 1764 "idl.yy"
    {
          (yyval.dlval) = NULL;
        }
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 1776 "idl.yy"
    {
          (yyval.deval) = new FE_Declarator(new UTL_ScopedName((yyvsp[(1) - (1)].idval), NULL),
                                 FE_Declarator::FD_simple, NULL);
        }
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 1784 "idl.yy"
    {
          (yyval.deval) = new FE_Declarator(new UTL_ScopedName((yyvsp[(1) - (1)].dcval)->local_name(), NULL),
                                 FE_Declarator::FD_complex,
                                 (yyvsp[(1) - (1)].dcval));
        }
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 1798 "idl.yy"
    {
          (yyval.etval) = AST_Expression::EV_long;
        }
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 1802 "idl.yy"
    {
          (yyval.etval) = AST_Expression::EV_longlong;
        }
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 1806 "idl.yy"
    {
          (yyval.etval) = AST_Expression::EV_short;
        }
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 1813 "idl.yy"
    {
          (yyval.etval) = AST_Expression::EV_ulong;
        }
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 1817 "idl.yy"
    {
          (yyval.etval) = AST_Expression::EV_ulonglong;
        }
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 1821 "idl.yy"
    {
          (yyval.etval) = AST_Expression::EV_ushort;
        }
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 1828 "idl.yy"
    {
          (yyval.etval) = AST_Expression::EV_double;
        }
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 1832 "idl.yy"
    {
          (yyval.etval) = AST_Expression::EV_float;
        }
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 1836 "idl.yy"
    {
          (yyval.etval) = AST_Expression::EV_longdouble;
        }
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 1843 "idl.yy"
    {
          (yyval.etval) = AST_Expression::EV_char;
        }
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 1847 "idl.yy"
    {
          (yyval.etval) = AST_Expression::EV_wchar;
        }
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 1854 "idl.yy"
    { 
          (yyval.etval) = AST_Expression::EV_octet;
        }
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 1861 "idl.yy"
    { 
          (yyval.etval) = AST_Expression::EV_bool;
        }
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 1868 "idl.yy"
    {
          (yyval.etval) = AST_Expression::EV_any;
        }
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 1875 "idl.yy"
    {
           UTL_Scope * s = idl_global->scopes()->top_non_null ();
           UTL_ScopedName * n = new UTL_ScopedName ((yyvsp[(1) - (2)].idval), NULL);
           AST_Structure * st = NULL;

           idl_global->set_parse_state (IDL_GlobalData::PS_StructSqSeen);
           if (s != NULL) 
           {
              st = idl_global->gen()->create_structure (n, s->get_pragmas ());
              st->set_defined (true);
              st = s->fe_add_structure (st);
              st->set_defined (true);

              /* Update the declaration */

              st->set_imported (idl_global->imported ());
              st->set_in_main_file (idl_global->in_main_file ());
              st->set_line (idl_global->lineno ());
              st->set_file_name (idl_global->filename ());
           }
           idl_global->scopes()->push (st);
           g_feScopeStack.Push
           (
              be_CppEnclosingScope
              (
                 *n,
                 be_CppEnclosingScope::NameIsScope ()
              )
           );
        }
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 1906 "idl.yy"
    {
           idl_global->set_parse_state (IDL_GlobalData::PS_StructBodySeen);
        }
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 1910 "idl.yy"
    {
           idl_global->set_parse_state (IDL_GlobalData::PS_StructQsSeen);
           /*
            * Done with this struct. Pop its scope off the scopes stack
            */
           if (idl_global->scopes()->top() == NULL)
           {
              (yyval.dcval) = NULL;
           }
           else 
           {
              (yyval.dcval) = AST_Structure::narrow_from_scope
                 (idl_global->scopes()->top_non_null ());
              idl_global->scopes()->pop();
              g_feScopeStack.Pop ();
           }
        }
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 1938 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_MemberTypeSeen);
        }
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 1942 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_MemberDeclsSeen);
        }
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 1946 "idl.yy"
    {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_DecllistActiveIterator *l = NULL;
          FE_Declarator         *d = NULL;
          AST_Field             *f = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_MemberDeclsCompleted);
          /*
           * Check for illegal recursive use of type
           */
          if ((yyvsp[(1) - (5)].dcval) != NULL && AST_illegal_recursive_type((yyvsp[(1) - (5)].dcval)))
            idl_global->err()->error1(UTL_Error::EIDL_RECURSIVE_TYPE, (yyvsp[(1) - (5)].dcval));
          /*
           * Create a node representing a struct or exception member
           * Add it to the enclosing scope
           */
          else if (s != NULL && (yyvsp[(1) - (5)].dcval) != NULL && (yyvsp[(3) - (5)].dlval) != NULL) {
            l = new UTL_DecllistActiveIterator((yyvsp[(3) - (5)].dlval));
            for (;!(l->is_done()); l->next()) {
              d = l->item();
              if (d == NULL)
                continue;
              AST_Type *tp = d->compose((yyvsp[(1) - (5)].dcval));
              if (tp == NULL)
                continue;

              // Check for anonymous type
              if ((tp->node_type () == AST_Decl::NT_array)
                  || (tp->node_type () == AST_Decl::NT_sequence))
              {
                 Identifier * id = d->name ()->head ();
                 const char *postfix =
                    (tp->node_type () == AST_Decl::NT_array)
                    ? "" : "_seq";
                 // first underscore removed by Identifier constructor
                 DDS_StdString anon_type_name =
                    DDS_StdString ("__") + DDS_StdString (id->get_string ())
                       + DDS_StdString (postfix);
                 UTL_ScopedName *anon_scoped_name =
                 new UTL_ScopedName
                 (
                    new Identifier (os_strdup(anon_type_name)),
                    NULL
                 );

                 (void) s->fe_add_typedef
                 (
                    idl_global->gen()->create_typedef
                    (
                       tp,
                       anon_scoped_name,
                       s->get_pragmas ()
                    )
                 );
              }

              f = idl_global->gen()->create_field(tp, d->name(), s->get_pragmas());
              (void) s->fe_add_field(f);
            }
            delete l;
          }
        }
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 2009 "idl.yy"
    {
          idl_global->err()->syntax_error(idl_global->parse_state());
        }
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 2013 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
          yyerrok;
        }
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 2021 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_SwitchSeen);
        }
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 2025 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_SwitchOpenParSeen);
        }
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 2029 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_SwitchTypeSeen);
        }
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 2033 "idl.yy"
    {
          UTL_Scope * s = idl_global->scopes()->top_non_null ();
          UTL_ScopedName * n = new UTL_ScopedName ((yyvsp[(1) - (8)].idval), NULL);
          AST_Union * u = NULL;

          idl_global->set_parse_state (IDL_GlobalData::PS_SwitchCloseParSeen);
          /*
           * Create a node representing a union. Add it to its enclosing
           * scope
           */

          if ((yyvsp[(6) - (8)].dcval) != NULL && s != NULL)
          {
             AST_ConcreteType * tp = AST_ConcreteType::narrow_from_decl ((yyvsp[(6) - (8)].dcval));
             if (tp == NULL)
             {
                idl_global->err()->not_a_type ((yyvsp[(6) - (8)].dcval));
             }
             else
             {
                u = idl_global->gen()->create_union (n, s->get_pragmas ());
                u->set_defined (true);
                u = s->fe_add_union (u);
                u->set_defined (true);

                u->set_disc_type (tp);

                u->set_imported (idl_global->imported ());
                u->set_in_main_file (idl_global->in_main_file ());
                u->set_line (idl_global->lineno ());
                u->set_file_name (idl_global->filename ());
             }
          }
          idl_global->scopes()->push (u);
          g_feScopeStack.Push
          (
             be_CppEnclosingScope
             (
                *n,
                be_CppEnclosingScope::NameIsScope ()
             )
          );
        }
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 2077 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_UnionSqSeen);
        }
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 2081 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_UnionBodySeen);
        }
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 2085 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_UnionQsSeen);
          /*
           * Done with this union. Pop its scope from the scopes stack
           */
          if (idl_global->scopes()->top() == NULL)
            (yyval.dcval) = NULL;
          else {
            (yyval.dcval) =
              AST_Union::narrow_from_scope(
                                idl_global->scopes()->top_non_null());
            idl_global->scopes()->pop();
            g_feScopeStack.Pop();
          }
        }
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 2104 "idl.yy"
    {
          (yyval.dcval) = idl_global->scopes()->bottom()->lookup_primitive_type((yyvsp[(1) - (1)].etval));
        }
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 2108 "idl.yy"
    {
          (yyval.dcval) = idl_global->scopes()->bottom()->lookup_primitive_type((yyvsp[(1) - (1)].etval));
        }
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 2112 "idl.yy"
    {
          (yyval.dcval) = idl_global->scopes()->bottom()->lookup_primitive_type((yyvsp[(1) - (1)].etval));
        }
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 2116 "idl.yy"
    {
          (yyval.dcval) = idl_global->scopes()->bottom()->lookup_primitive_type((yyvsp[(1) - (1)].etval));
        }
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 2121 "idl.yy"
    {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          AST_Decl              *d = NULL;
          AST_PredefinedType    *p = NULL;
          AST_Typedef           *t = NULL;
          bool found               = false;

          /*
           * The discriminator is a scoped name. Try to resolve to
           * one of the scalar types or to an enum. Thread through
           * typedef's to arrive at the base type at the end of the
           * chain
           */
          if (s != NULL && (d = s->lookup_by_name((yyvsp[(1) - (1)].idlist), true)) != NULL) {
            while (!found) {
              switch (d->node_type()) {
              case AST_Decl::NT_enum:
                (yyval.dcval) = d;
                found = true;
                break;
              case AST_Decl::NT_pre_defined:
                p = AST_PredefinedType::narrow_from_decl(d);
                if (p != NULL) {
                  switch (p->pt()) {
                  case AST_PredefinedType::PT_long:
                  case AST_PredefinedType::PT_ulong:
                  case AST_PredefinedType::PT_longlong:
                  case AST_PredefinedType::PT_ulonglong:
                  case AST_PredefinedType::PT_short:
                  case AST_PredefinedType::PT_ushort:
                  case AST_PredefinedType::PT_char:
                  case AST_PredefinedType::PT_wchar:
                  case AST_PredefinedType::PT_octet:
                  case AST_PredefinedType::PT_boolean:
                    (yyval.dcval) = p;
                    found = true;
                    break;
                  default:
                    (yyval.dcval) = NULL;
                    found = true;
                    break;
                  }
                }
                break;
              case AST_Decl::NT_typedef:
                t = AST_Typedef::narrow_from_decl(d);
                if (t != NULL) d = t->base_type();
                break;
              default:
                (yyval.dcval) = NULL;
                found = true;
                break;
              }
            }
          } else
            (yyval.dcval) = NULL;

          if ((yyval.dcval) == NULL)
            idl_global->err()->lookup_error((yyvsp[(1) - (1)].idlist));
        }
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 2192 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionLabelSeen);
        }
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 2196 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemSeen);
        }
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 2200 "idl.yy"
    {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_LabellistActiveIterator *l = NULL;
          AST_UnionLabel        *d = NULL;
          AST_UnionBranch       *b = NULL;
          AST_Field             *f = (yyvsp[(3) - (5)].ffval);

          idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemCompleted);
          /*
           * Create several nodes representing branches of a union.
           * Add them to the enclosing scope (the union scope)
           */
          if (s != NULL && (yyvsp[(1) - (5)].llval) != NULL && (yyvsp[(3) - (5)].ffval) != NULL) {
            l = new UTL_LabellistActiveIterator((yyvsp[(1) - (5)].llval));
            for (;!(l->is_done()); l->next()) {
              d = l->item();
              if (d == NULL)
                continue;
              b = idl_global->gen()->create_union_branch(d,
                                                      f->field_type(),
                                                      f->name(),
                                                      f->get_decl_pragmas());
              (void) s->fe_add_union_branch(b);
            }
            delete l;
          }
        }
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 2228 "idl.yy"
    {
          idl_global->err()->syntax_error(idl_global->parse_state());
        }
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 2232 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
          yyerrok;
        }
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 2240 "idl.yy"
    {
          (yyval.llval) = new UTL_LabelList((yyvsp[(1) - (2)].ulval), (yyvsp[(2) - (2)].llval));
        }
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 2247 "idl.yy"
    {
          if ((yyvsp[(1) - (2)].llval) == NULL)
            (yyval.llval) = new UTL_LabelList((yyvsp[(2) - (2)].ulval), NULL);
          else {
            (yyvsp[(1) - (2)].llval)->nconc(new UTL_LabelList((yyvsp[(2) - (2)].ulval), NULL));
            (yyval.llval) = (yyvsp[(1) - (2)].llval);
          }
        }
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 2256 "idl.yy"
    {
          (yyval.llval) = NULL;
        }
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 2263 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_DefaultSeen);
        }
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 2267 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_LabelColonSeen);

          (yyval.ulval) = idl_global->gen()->
                    create_union_label(AST_UnionLabel::UL_default,
                                       NULL);
        }
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 2275 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_CaseSeen);
        }
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 2279 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_LabelExprSeen);
        }
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 2283 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_LabelColonSeen);

          (yyval.ulval) = idl_global->gen()->create_union_label(AST_UnionLabel::UL_label,
                                                     (yyvsp[(3) - (5)].exval));
        }
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 2293 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemTypeSeen);
        }
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 2297 "idl.yy"
    {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemDeclSeen);
          /*
           * Check for illegal recursive use of type
           */
          if ((yyvsp[(1) - (3)].dcval) != NULL && AST_illegal_recursive_type((yyvsp[(1) - (3)].dcval)))
            idl_global->err()->error1(UTL_Error::EIDL_RECURSIVE_TYPE, (yyvsp[(1) - (3)].dcval));
          /*
           * Create a field in a union branch
           */
          else if ((yyvsp[(1) - (3)].dcval) == NULL || (yyvsp[(3) - (3)].deval) == NULL)
            (yyval.ffval) = NULL;
          else {
            AST_Type *tp = (yyvsp[(3) - (3)].deval)->compose((yyvsp[(1) - (3)].dcval));
            if (tp == NULL)
            {
              (yyval.ffval) = NULL;
            }
            else
            {
              // Check for anonymous type
              if ((tp->node_type () == AST_Decl::NT_array)
                  || (tp->node_type () == AST_Decl::NT_sequence))
              {
                 Identifier * id = (yyvsp[(3) - (3)].deval)->name ()->head ();
                 const char *postfix =
                    (tp->node_type () == AST_Decl::NT_array)
                    ? "" : "_seq";
                 // first underscore removed by Identifier constructor
                 DDS_StdString anon_type_name =
                    DDS_StdString ("__") + DDS_StdString (id->get_string ())
                       + DDS_StdString (postfix);
                 UTL_ScopedName *anon_scoped_name =
                 new UTL_ScopedName
                 (
                    new Identifier (os_strdup(anon_type_name)),
                    NULL
                 );

                 (void) s->fe_add_typedef
                 (
                    idl_global->gen()->create_typedef
                    (
                       tp,
                       anon_scoped_name,
                       s->get_pragmas ()
                    )
                 );
              }

              (yyval.ffval) = idl_global->gen()->create_field(tp,
                                                   (yyvsp[(3) - (3)].deval)->name(),
                                                   s->get_pragmas());
            }
          }
        }
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 2358 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_EnumSeen);
        }
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 2362 "idl.yy"
    {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_ScopedName        *n = new UTL_ScopedName((yyvsp[(3) - (3)].idval), NULL);
          AST_Enum              *e = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_EnumIDSeen);
          /*
           * Create a node representing an enum and add it to its
           * enclosing scope
           */
          if (s != NULL) {
            e = idl_global->gen()->create_enum(n, s->get_pragmas());
            /*
             * Add it to its defining scope
             */
            (void) s->fe_add_enum(e);
          }
          /*p
           * Push the enum scope on the scopes stack
           */
          idl_global->scopes()->push(e);
        }
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 2385 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_EnumSqSeen);
        }
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 2389 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_EnumBodySeen);
        }
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 2393 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_EnumQsSeen);
          /*
           * Done with this enum. Pop its scope from the scopes stack
           */
          if (idl_global->scopes()->top() == NULL)
            (yyval.dcval) = NULL;
          else {
            (yyval.dcval) = AST_Enum::narrow_from_scope(idl_global->scopes()->top_non_null());
            idl_global->scopes()->pop();
          }
        }
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 2412 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_EnumCommaSeen);
        }
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 2421 "idl.yy"
    {
          if (idl_global->valid_identifier((yyvsp[(1) - (1)].strval)))
          {
            UTL_Scope             *s = idl_global->scopes()->top_non_null();
            UTL_ScopedName        *n =
                  new UTL_ScopedName (new Identifier ((yyvsp[(1) - (1)].strval)), NULL);
            AST_EnumVal           *e = NULL;
            AST_Enum              *c = NULL;

            /*
             * Create a node representing one enumerator in an enum
             * Add it to the enclosing scope (the enum scope)
             */
            if (s != NULL && s->scope_node_type() == AST_Decl::NT_enum) {
              c = AST_Enum::narrow_from_scope(s);
              if (c != NULL)
                e = idl_global->gen()->create_enum_val(c->next_enum_val(), n, s->get_pragmas());
              (void) s->fe_add_enum_val(e);
            }
          }
        }
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 2447 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceCommaSeen);
        }
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 2451 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceExprSeen);
        }
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 2455 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceQsSeen);
          /*
           * Remove sequence marker from scopes stack
           */
          if (idl_global->scopes()->top() == NULL)
            idl_global->scopes()->pop();
          /*
           * Create a node representing a sequence
           */
          if ((yyvsp[(4) - (6)].exval) == NULL || (yyvsp[(4) - (6)].exval)->coerce(AST_Expression::EV_ulong) == NULL) {
            idl_global->err()->coercion_error((yyvsp[(4) - (6)].exval), AST_Expression::EV_ulong);
            (yyval.dcval) = NULL;
          } else if ((yyvsp[(1) - (6)].dcval) == NULL) {
            (yyval.dcval) = NULL;
          } else {
            AST_Type *tp = AST_Type::narrow_from_decl((yyvsp[(1) - (6)].dcval));
            if (tp == NULL)
              (yyval.dcval) = NULL;
            else {
              (yyval.dcval) = idl_global->gen()->create_sequence((yyvsp[(4) - (6)].exval), tp);
              /*
               * Add this AST_Sequence to the types defined in the global scope
               */
              (void) idl_global->root()
                        ->fe_add_sequence(AST_Sequence::narrow_from_decl((yyval.dcval)));
            }
          }
        }
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 2486 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceQsSeen);
          /*
           * Remove sequence marker from scopes stack
           */
          if (idl_global->scopes()->top() == NULL)
            idl_global->scopes()->pop();
          /*
           * Create a node representing a sequence
           */
          if ((yyvsp[(1) - (2)].dcval) == NULL)
            (yyval.dcval) = NULL;
          else {
            AST_Type *tp = AST_Type::narrow_from_decl((yyvsp[(1) - (2)].dcval));
            if (tp == NULL)
              (yyval.dcval) = NULL;
            else {
              (yyval.dcval) =
                idl_global->gen()->create_sequence(
                             idl_global->gen()->create_expr((unsigned long) 0),
                             tp);
              /*
               * Add this AST_Sequence to the types defined in the global scope
               */
              (void) idl_global->root()
                        ->fe_add_sequence(AST_Sequence::narrow_from_decl((yyval.dcval)));
            }
          }
        }
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 2519 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceSeen);
          /*
           * Push a sequence marker on scopes stack
           */
          idl_global->scopes()->push(NULL);
        }
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 2527 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceSqSeen);
        }
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 2531 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceTypeSeen);
          (yyval.dcval) = (yyvsp[(5) - (5)].dcval);
        }
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 2540 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringSqSeen);
        }
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 2544 "idl.yy"
    {
           idl_global->set_parse_state(IDL_GlobalData::PS_StringExprSeen);
        }
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 2548 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringQsSeen);
          /*
           * Create a node representing a string
           */
          if ((yyvsp[(4) - (6)].exval) == NULL || (yyvsp[(4) - (6)].exval)->coerce(AST_Expression::EV_ulong) == NULL) {
            idl_global->err()->coercion_error((yyvsp[(4) - (6)].exval), AST_Expression::EV_ulong);
            (yyval.dcval) = NULL;
          } else {
            (yyval.dcval) = idl_global->gen()->create_string((yyvsp[(4) - (6)].exval));
            /*
             * Add this AST_String to the types defined in the global scope
             */
            (void) idl_global->root()
                      ->fe_add_string(AST_String::narrow_from_decl((yyval.dcval)));
          }
        }
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 2566 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringCompleted);
          /*
           * Create a node representing a string
           */
          (yyval.dcval) =
            idl_global->gen()->create_string(
                         idl_global->gen()->create_expr((unsigned long) 0));
          /*
           * Add this AST_String to the types defined in the global scope
           */
          (void) idl_global->root()
                    ->fe_add_string(AST_String::narrow_from_decl((yyval.dcval)));
        }
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 2584 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringSeen);
        }
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 2592 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringSqSeen);
        }
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 2596 "idl.yy"
    {
           idl_global->set_parse_state(IDL_GlobalData::PS_StringExprSeen);
        }
    break;

  case 280:

/* Line 1806 of yacc.c  */
#line 2600 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringQsSeen);
          /*
           * Create a node representing a string
           */
          if ((yyvsp[(4) - (6)].exval) == NULL || (yyvsp[(4) - (6)].exval)->coerce(AST_Expression::EV_ulong) == NULL) {
            idl_global->err()->coercion_error((yyvsp[(4) - (6)].exval), AST_Expression::EV_ulong);
            (yyval.dcval) = NULL;
          } else {
            (yyval.dcval) = idl_global->gen()->create_wstring((yyvsp[(4) - (6)].exval));
            /*
             * Add this AST_String to the types defined in the global scope
             */
            (void) idl_global->root()
                      ->fe_add_string(AST_String::narrow_from_decl((yyval.dcval)));
          }
        }
    break;

  case 281:

/* Line 1806 of yacc.c  */
#line 2618 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringCompleted);
          /*
           * Create a node representing a string
           */
          (yyval.dcval) =
            idl_global->gen()->create_wstring(
                         idl_global->gen()->create_expr((unsigned long) 0));
          /*
           * Add this AST_String to the types defined in the global scope
           */
          (void) idl_global->root()
                    ->fe_add_string(AST_String::narrow_from_decl((yyval.dcval)));
        }
    break;

  case 282:

/* Line 1806 of yacc.c  */
#line 2636 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringSeen);
        }
    break;

  case 283:

/* Line 1806 of yacc.c  */
#line 2643 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_ArrayIDSeen);
        }
    break;

  case 284:

/* Line 1806 of yacc.c  */
#line 2647 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_ArrayCompleted);
          /*
           * Create a node representing an array
           */
          if ((yyvsp[(3) - (3)].elval) != NULL) {
             (yyval.dcval) = idl_global->gen()->create_array(new UTL_ScopedName((yyvsp[(1) - (3)].idval), NULL),
                                                  (yyvsp[(3) - (3)].elval)->length(), (yyvsp[(3) - (3)].elval));
          }
        }
    break;

  case 285:

/* Line 1806 of yacc.c  */
#line 2661 "idl.yy"
    {
          (yyval.elval) = new UTL_ExprList((yyvsp[(1) - (2)].exval), (yyvsp[(2) - (2)].elval));
        }
    break;

  case 286:

/* Line 1806 of yacc.c  */
#line 2668 "idl.yy"
    {
          if ((yyvsp[(1) - (2)].elval) == NULL)
            (yyval.elval) = new UTL_ExprList((yyvsp[(2) - (2)].exval), NULL);
          else {
            (yyvsp[(1) - (2)].elval)->nconc(new UTL_ExprList((yyvsp[(2) - (2)].exval), NULL));
            (yyval.elval) = (yyvsp[(1) - (2)].elval);
          }
        }
    break;

  case 287:

/* Line 1806 of yacc.c  */
#line 2677 "idl.yy"
    {
          (yyval.elval) = NULL;
        }
    break;

  case 288:

/* Line 1806 of yacc.c  */
#line 2684 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_DimSqSeen);
        }
    break;

  case 289:

/* Line 1806 of yacc.c  */
#line 2688 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_DimExprSeen);
        }
    break;

  case 290:

/* Line 1806 of yacc.c  */
#line 2692 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_DimQsSeen);
          /*
           * Array dimensions are expressions which must be coerced to
           * positive integers
           */
          if ((yyvsp[(3) - (5)].exval) == NULL || (yyvsp[(3) - (5)].exval)->coerce(AST_Expression::EV_ulong) == NULL) {
            idl_global->err()->coercion_error((yyvsp[(3) - (5)].exval), AST_Expression::EV_ulong);
            (yyval.exval) = NULL;
          } else
            (yyval.exval) = (yyvsp[(3) - (5)].exval);
        }
    break;

  case 291:

/* Line 1806 of yacc.c  */
#line 2709 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_AttrSeen);
        }
    break;

  case 292:

/* Line 1806 of yacc.c  */
#line 2713 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_AttrTypeSeen);
        }
    break;

  case 293:

/* Line 1806 of yacc.c  */
#line 2717 "idl.yy"
    {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_DecllistActiveIterator *l = NULL;
          AST_Attribute         *a = NULL;
          FE_Declarator         *d = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_AttrCompleted);
          /*
           * Create nodes representing attributes and add them to the
           * enclosing scope
           */
          if (s != NULL && (yyvsp[(4) - (6)].dcval) != NULL && (yyvsp[(6) - (6)].dlval) != NULL) {
            l = new UTL_DecllistActiveIterator((yyvsp[(6) - (6)].dlval));
            for (;!(l->is_done()); l->next()) {
              d = l->item();
              if (d == NULL)
                continue;
              AST_Type *tp = d->compose((yyvsp[(4) - (6)].dcval));
              if (tp == NULL)
                continue;
              a = idl_global->gen()->create_attribute((yyvsp[(1) - (6)].bval), tp, d->name(), s->get_pragmas());
              /*
               * Add one attribute to the enclosing scope
               */
              (void) s->fe_add_attribute(a);
            }
            delete l;
          }
        }
    break;

  case 294:

/* Line 1806 of yacc.c  */
#line 2750 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_AttrROSeen);
          (yyval.bval) = true;
        }
    break;

  case 295:

/* Line 1806 of yacc.c  */
#line 2755 "idl.yy"
    {
          (yyval.bval) = false;
        }
    break;

  case 296:

/* Line 1806 of yacc.c  */
#line 2770 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_LocalSeen);
          (yyval.cval) = 'L';
        }
    break;

  case 297:

/* Line 1806 of yacc.c  */
#line 2775 "idl.yy"
    {
          idl_global->set_parse_state (IDL_GlobalData::PS_AbstractSeen);
          (yyval.cval) = 'A';
        }
    break;

  case 298:

/* Line 1806 of yacc.c  */
#line 2780 "idl.yy"
    {
          (yyval.cval) = 'I';
        }
    break;

  case 299:

/* Line 1806 of yacc.c  */
#line 2787 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptSeen);
        }
    break;

  case 300:

/* Line 1806 of yacc.c  */
#line 2791 "idl.yy"
    {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_ScopedName        *n = new UTL_ScopedName((yyvsp[(3) - (3)].idval), NULL);
          AST_Exception         *e = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptIDSeen);
          /*
           * Create a node representing an exception and add it to
           * the enclosing scope
           */
          if (s != NULL) {
            e = idl_global->gen()->create_exception(n, s->get_pragmas());
            (void) s->fe_add_exception(e);
          }
          /*
           * Push the exception scope on the scope stack
           */
          idl_global->scopes()->push(e);
        }
    break;

  case 301:

/* Line 1806 of yacc.c  */
#line 2811 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptSqSeen);
        }
    break;

  case 302:

/* Line 1806 of yacc.c  */
#line 2815 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptBodySeen);
        }
    break;

  case 303:

/* Line 1806 of yacc.c  */
#line 2819 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptQsSeen);
          /*
           * Done with this exception. Pop its scope from the scope stack
           */
          idl_global->scopes()->pop();
        }
    break;

  case 304:

/* Line 1806 of yacc.c  */
#line 2831 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpTypeSeen);
        }
    break;

  case 305:

/* Line 1806 of yacc.c  */
#line 2835 "idl.yy"
    {
          if (idl_global->valid_identifier((yyvsp[(4) - (4)].strval)))
          {
            UTL_Scope             *s = idl_global->scopes()->top_non_null();
            UTL_ScopedName        *n =
                  new UTL_ScopedName (new Identifier ((yyvsp[(4) - (4)].strval)), NULL);
            AST_Operation         *o = NULL;

            idl_global->set_parse_state(IDL_GlobalData::PS_OpIDSeen);
            /*
             * Create a node representing an operation on an interface
             * and add it to its enclosing scope
             */
            if (s != NULL && (yyvsp[(2) - (4)].dcval) != NULL) {
              AST_Type *tp = AST_Type::narrow_from_decl((yyvsp[(2) - (4)].dcval));
              if (tp == NULL) {
                idl_global->err()->not_a_type((yyvsp[(2) - (4)].dcval));
              } else if (tp->node_type() == AST_Decl::NT_except) {
                idl_global->err()->not_a_type((yyvsp[(2) - (4)].dcval));
              } else {
                o = idl_global->gen()->create_operation(tp, (yyvsp[(1) - (4)].ofval), n, s->get_pragmas());
                (void) s->fe_add_operation(o);
              }
            }
            /*
             * Push the operation scope onto the scopes stack
             */
            idl_global->scopes()->push(o);
          }
        }
    break;

  case 306:

/* Line 1806 of yacc.c  */
#line 2866 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpParsCompleted);
        }
    break;

  case 307:

/* Line 1806 of yacc.c  */
#line 2870 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseCompleted);
        }
    break;

  case 308:

/* Line 1806 of yacc.c  */
#line 2874 "idl.yy"
    {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          AST_Operation         *o = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_OpCompleted);
          /*
           * Add exceptions and context to the operation
           */
          if (s != NULL && s->scope_node_type() == AST_Decl::NT_op) {
            o = AST_Operation::narrow_from_scope(s);

            if ((yyvsp[(8) - (10)].nlval) != NULL && o != NULL)
              (void) o->fe_add_exceptions((yyvsp[(8) - (10)].nlval));
            if ((yyvsp[(10) - (10)].slval) != NULL)
              (void) o->fe_add_context((yyvsp[(10) - (10)].slval));
          }
          /*
           * Done with this operation. Pop its scope from the scopes stack
           */
          idl_global->scopes()->pop();
        }
    break;

  case 309:

/* Line 1806 of yacc.c  */
#line 2899 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpAttrSeen);
          (yyval.ofval) = AST_Operation::OP_oneway;
        }
    break;

  case 310:

/* Line 1806 of yacc.c  */
#line 2904 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpAttrSeen);
          (yyval.ofval) = AST_Operation::OP_idempotent;
        }
    break;

  case 311:

/* Line 1806 of yacc.c  */
#line 2909 "idl.yy"
    {
          (yyval.ofval) = AST_Operation::OP_noflags;
        }
    break;

  case 313:

/* Line 1806 of yacc.c  */
#line 2917 "idl.yy"
    {
          (yyval.dcval) =
            idl_global->scopes()->bottom()
               ->lookup_primitive_type(AST_Expression::EV_void);
        }
    break;

  case 314:

/* Line 1806 of yacc.c  */
#line 2926 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpSqSeen);
        }
    break;

  case 315:

/* Line 1806 of yacc.c  */
#line 2930 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpQsSeen);
        }
    break;

  case 316:

/* Line 1806 of yacc.c  */
#line 2934 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpSqSeen);
        }
    break;

  case 317:

/* Line 1806 of yacc.c  */
#line 2939 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpQsSeen);
        }
    break;

  case 319:

/* Line 1806 of yacc.c  */
#line 2949 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpParCommaSeen);
        }
    break;

  case 322:

/* Line 1806 of yacc.c  */
#line 2958 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpParDirSeen);
        }
    break;

  case 323:

/* Line 1806 of yacc.c  */
#line 2962 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpParTypeSeen);
        }
    break;

  case 324:

/* Line 1806 of yacc.c  */
#line 2966 "idl.yy"
    {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          AST_Argument          *a = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_OpParDeclSeen);
          /*
           * Create a node representing an argument to an operation
           * Add it to the enclosing scope (the operation scope)
           */
          if ((yyvsp[(3) - (5)].dcval) != NULL && (yyvsp[(5) - (5)].deval) != NULL && s != NULL) {
            AST_Type *tp = (yyvsp[(5) - (5)].deval)->compose((yyvsp[(3) - (5)].dcval));
            if (tp != NULL) {
              a = idl_global->gen()->create_argument((yyvsp[(1) - (5)].dival), tp, (yyvsp[(5) - (5)].deval)->name(), s->get_pragmas());
              (void) s->fe_add_argument(a);
            }
          }
        }
    break;

  case 325:

/* Line 1806 of yacc.c  */
#line 2987 "idl.yy"
    {
          (yyval.dival) = AST_Argument::dir_IN;
        }
    break;

  case 326:

/* Line 1806 of yacc.c  */
#line 2991 "idl.yy"
    {
          (yyval.dival) = AST_Argument::dir_OUT;
        }
    break;

  case 327:

/* Line 1806 of yacc.c  */
#line 2995 "idl.yy"
    {
          (yyval.dival) = AST_Argument::dir_INOUT;
        }
    break;

  case 328:

/* Line 1806 of yacc.c  */
#line 3002 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseSeen);
        }
    break;

  case 329:

/* Line 1806 of yacc.c  */
#line 3006 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseSqSeen);
        }
    break;

  case 330:

/* Line 1806 of yacc.c  */
#line 3011 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseQsSeen);
          (yyval.nlval) = (yyvsp[(5) - (6)].nlval);
        }
    break;

  case 331:

/* Line 1806 of yacc.c  */
#line 3016 "idl.yy"
    {
          (yyval.nlval) = NULL;
        }
    break;

  case 332:

/* Line 1806 of yacc.c  */
#line 3023 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpContextSeen);
        }
    break;

  case 333:

/* Line 1806 of yacc.c  */
#line 3027 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpContextSqSeen);
        }
    break;

  case 334:

/* Line 1806 of yacc.c  */
#line 3032 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpContextQsSeen);
          (yyval.slval) = (yyvsp[(5) - (6)].slval);
        }
    break;

  case 335:

/* Line 1806 of yacc.c  */
#line 3037 "idl.yy"
    {
          (yyval.slval) = NULL;
        }
    break;

  case 336:

/* Line 1806 of yacc.c  */
#line 3044 "idl.yy"
    {
          (yyval.slval) = new UTL_StrList((yyvsp[(1) - (2)].sval), (yyvsp[(2) - (2)].slval));
        }
    break;

  case 337:

/* Line 1806 of yacc.c  */
#line 3052 "idl.yy"
    {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpContextCommaSeen);
        }
    break;

  case 338:

/* Line 1806 of yacc.c  */
#line 3056 "idl.yy"
    {
          if ((yyvsp[(1) - (4)].slval) == NULL)
            (yyval.slval) = new UTL_StrList((yyvsp[(4) - (4)].sval), NULL);
          else {
            (yyvsp[(1) - (4)].slval)->nconc(new UTL_StrList((yyvsp[(4) - (4)].sval), NULL));
            (yyval.slval) = (yyvsp[(1) - (4)].slval);
          }
        }
    break;

  case 339:

/* Line 1806 of yacc.c  */
#line 3065 "idl.yy"
    {
          (yyval.slval) = NULL;
        }
    break;



/* Line 1806 of yacc.c  */
#line 5822 "y.tab.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 3070 "idl.yy"

/* programs */

/*
 * ???
 */
int yywrap ()
{
  return 1;
}

/*
 * Report an error situation discovered in a production
 *
 * This does not do anything since we report all error situations through
 * idl_global->err() operations
 */
void yyerror (const char *)
{
}

