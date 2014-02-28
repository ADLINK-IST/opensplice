
# line 72 "idl.yy"
#include "os_stdlib.h"
#include "os_heap.h"
#include "idl.h"
#include "idl_extern.h"

#include "fe_private.h"
#include "utl_incl.h"
#include "xbe_scopestack.h"

#include <stdio.h>

#ifdef VERSION
#undef VERSION
#endif

#ifdef WIN32
#undef CONST
#undef VOID
#undef IN
#undef OUT
#undef OPAQUE
#endif

void yyunput(int c);

#if (defined(apollo) || defined(HPUX) || defined(SGI)) && defined(__cplusplus)
#ifdef _EXTERN_C_
        extern  "C" int yywrap();
#else
        int yywrap(void);
#endif
#endif  // (defined(apollo) || defined(HPUX)) && defined(__cplusplus)


# line 97 "idl.yy"
typedef union
#ifdef __cplusplus
        YYSTYPE
#endif
 {
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
  idl_bool              bval;           /* Boolean value        */
  long                  ival;           /* Long value           */
  double                dval;           /* Double value         */
  float                 fval;           /* Float value          */
  char                  cval;           /* Char value           */

  UTL_String            *sval;          /* String value         */
  char                  *strval;        /* char * value         */
  Identifier            *idval;         /* Identifier           */
  UTL_IdList            *idlist;        /* Identifier list      */
} YYSTYPE;
# define IDENTIFIER 257
# define CONST 258
# define MODULE 259
# define LOCAL 260
# define INTERFACE 261
# define TYPEDEF 262
# define LONG 263
# define SHORT 264
# define UNSIGNED 265
# define DOUBLE 266
# define FLOAT 267
# define CHAR 268
# define WCHAR 269
# define OCTET 270
# define BOOLEAN 271
# define ANY 272
# define STRUCT 273
# define UNION 274
# define SWITCH 275
# define ENUM 276
# define SEQUENCE 277
# define STRING 278
# define WSTRING 279
# define EXCEPTION 280
# define CASE 281
# define DEFAULT 282
# define READONLY 283
# define ATTRIBUTE 284
# define ONEWAY 285
# define IDEMPOTENT 286
# define VOID 287
# define IN 288
# define OUT 289
# define INOUT 290
# define RAISES 291
# define CUSTOM 292
# define VALUETYPE 293
# define TRUNCATABLE 294
# define SUPPORTS 295
# define PUBLIC 296
# define PRIVATE 297
# define FACTORY 298
# define ABSTRACT 299
# define CONTEXT 300
# define OPAQUE 301
# define VERSION 302
# define INTEGER_LITERAL 303
# define STRING_LITERAL 304
# define CHARACTER_LITERAL 305
# define FLOATING_PT_LITERAL 306
# define TRUETOK 307
# define FALSETOK 308
# define SCOPE_DELIMITOR 309
# define LEFT_SHIFT 310
# define RIGHT_SHIFT 311
# define PRAGMA 312
# define PRAGMA_INCLUDE 313
# define PFROM 314
# define PRAGMA_ASYNC_CLIENT 315
# define PRAGMA_ASYNC_SERVER 316
# define PRAGMA_ID 317
# define PRAGMA_PREFIX 318
# define PRAGMA_VERSION 319
# define PRAGMA_END 320

#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#define YYCONST const
#else
#include <malloc.h>
#include <memory.h>
#define YYCONST
#endif

#if !defined(WIN32) && !defined(__Lynx__) && !defined(__QNXNTO__) && !defined(DDS_NETBSD) && !defined(_DARWIN_C_SOURCE)
#include <values.h>
#endif

#if defined(__cplusplus) || defined(__STDC__)

#if defined(__cplusplus) && defined(__EXTERN_C__)
extern "C" {
#endif
#ifndef yyerror
#if defined(__cplusplus)
        void yyerror(YYCONST char *);
#endif
#endif
#ifndef yylex
        int yylex(void);
#endif
        int yyparse(void);
#if defined(__cplusplus) && defined(__EXTERN_C__)
}
#endif

#endif

#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
YYSTYPE yylval;
YYSTYPE yyval;
typedef int yytabelem;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#if YYMAXDEPTH > 0
int yy_yys[YYMAXDEPTH], *yys = yy_yys;
YYSTYPE yy_yyv[YYMAXDEPTH], *yyv = yy_yyv;
#else   /* user does initial allocation */
int *yys;
YYSTYPE *yyv;
#endif
static int yymaxdepth = YYMAXDEPTH;
# define YYERRCODE 256

# line 2970 "idl.yy"

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
static YYCONST yytabelem yyexca[] ={
-1, 0,
        0, 3,
        261, 293,
        -2, 0,
-1, 1,
        0, -1,
        -2, 0,
-1, 3,
        0, 3,
        261, 293,
        125, 3,
        -2, 0,
-1, 71,
        123, 44,
        -2, 110,
-1, 74,
        123, 66,
        -2, 50,
-1, 75,
        123, 66,
        -2, 49,
-1, 179,
        257, 306,
        263, 306,
        264, 306,
        265, 306,
        266, 306,
        267, 306,
        268, 306,
        269, 306,
        270, 306,
        271, 306,
        272, 306,
        277, 306,
        278, 306,
        279, 306,
        284, 291,
        287, 306,
        309, 306,
        -2, 0,
-1, 192,
        91, 279,
        -2, 188,
-1, 207,
        257, 306,
        263, 306,
        264, 306,
        265, 306,
        266, 306,
        267, 306,
        268, 306,
        269, 306,
        270, 306,
        271, 306,
        272, 306,
        277, 306,
        278, 306,
        279, 306,
        284, 291,
        287, 306,
        309, 306,
        125, 37,
        -2, 0,
-1, 265,
        261, 293,
        125, 3,
        -2, 0,
-1, 283,
        257, 306,
        263, 306,
        264, 306,
        265, 306,
        266, 306,
        267, 306,
        268, 306,
        269, 306,
        270, 306,
        271, 306,
        272, 306,
        277, 306,
        278, 306,
        279, 306,
        284, 291,
        287, 306,
        309, 306,
        -2, 0,
-1, 311,
        125, 297,
        -2, 0,
-1, 357,
        125, 211,
        -2, 0,
-1, 402,
        41, 309,
        -2, 311,
-1, 454,
        125, 234,
        -2, 0,
        };
# define YYNPROD 335
# define YYLAST 695
static YYCONST yytabelem yyact[]={

    58,   238,   446,   250,   189,   185,   422,   315,    88,   443,
   190,   319,   389,   188,    95,    16,   100,    99,    16,    98,
    96,   289,   237,   135,   203,   135,   311,   138,   295,   296,
    57,    61,   135,    63,   137,    65,   134,    60,   140,    60,
    74,    75,   486,   139,   478,   135,   306,   332,   135,   172,
    64,    62,   438,   244,   152,   150,   132,    92,   131,    77,
    76,    87,   424,   425,   426,   245,   243,   419,   141,   142,
   143,   126,   127,   242,   125,   124,   182,   155,   240,    92,
   241,   408,    11,   153,    17,    21,    41,   390,    13,    59,
   183,    59,   279,   448,   447,   445,   205,    60,   158,    33,
    34,   159,    35,   385,   118,   119,    18,   158,   110,   111,
   113,   112,   353,   197,   209,    72,    35,    12,    42,    40,
   448,   447,    60,     6,     2,    43,   320,    10,    44,    94,
    15,   164,   165,    15,     5,   366,   133,   171,    32,    28,
     4,    26,    27,    29,    30,    31,    51,    66,   178,    59,
   270,   151,   181,   464,   184,   149,   186,   386,   192,   375,
   207,   374,   373,   321,   292,   427,   246,   230,   199,   206,
   204,   202,   201,   200,   148,   144,   293,   403,   219,   290,
    17,   186,   160,   208,    13,   351,   184,   350,   338,    93,
    14,   310,   198,    14,    16,    33,    34,   163,    35,   162,
   161,   479,    18,   468,   428,   227,   404,   228,   229,   377,
   363,   327,   326,   325,   324,   323,   322,   156,   260,   261,
    85,    83,    16,   278,    82,    92,    81,    80,    92,   277,
    79,   154,    78,   281,    32,    28,   481,    26,    27,    29,
    30,    31,   470,   146,    97,   219,   397,    17,   301,   398,
   297,    13,   298,   299,   305,   104,   484,   103,   300,   440,
   387,   287,    33,    34,   286,    35,    92,   285,   482,    18,
    92,   309,   227,   473,   228,   229,   150,   436,   192,   432,
    16,   420,   405,   349,   460,   225,   226,   222,   439,   184,
   417,   328,   152,   402,   364,   317,   294,   220,    16,   128,
   485,    32,    28,   216,    26,    27,    29,    30,    31,    15,
   130,   187,   129,   472,   215,    92,   449,   450,   431,   452,
   214,   434,   451,   433,   421,   220,   219,   414,    17,   413,
   211,   216,    13,   231,    92,   232,   192,    15,   367,   365,
   368,   357,   215,    33,    34,   430,    35,   412,   214,   401,
    18,   344,   345,   227,   372,   228,   229,   252,   369,   192,
   370,    92,   342,   343,   384,   346,   347,   348,   341,    14,
   392,   339,   376,   331,   340,   383,   333,   381,   382,   314,
   380,   379,    32,    28,   284,    26,    27,    29,    30,    31,
   312,   283,   192,   252,    92,    15,   247,    14,   248,   399,
   411,   220,   354,   264,   168,   400,    55,   216,   406,   391,
   410,   409,   330,    15,   302,   303,   304,   393,   215,   316,
    60,   186,   337,   429,   214,   236,   109,   118,   119,   107,
   108,   110,   111,   113,   112,   114,    33,    34,    92,    35,
   120,   121,   122,   441,   308,   196,   117,   307,   195,   116,
   262,   166,   291,   192,   186,    14,   461,   463,   462,    92,
   469,   471,   194,   467,   465,   396,   362,   361,   318,   268,
   175,    69,    59,    14,   475,    60,   192,   476,   459,   249,
   480,   109,   118,   119,   107,   108,   110,   111,   113,   112,
   114,    33,    34,   458,    35,   120,   121,   122,   456,   474,
   455,   454,   453,   442,   435,   415,   395,   360,   267,    60,
   174,    68,   359,   152,   394,   109,   118,   119,   107,   108,
   110,   111,   113,   112,   114,    33,    34,    59,    35,   120,
   121,   122,    60,   358,   356,   313,   266,   173,   109,   118,
   119,   107,   108,   110,   111,   113,   112,   114,    67,   335,
   157,    86,   120,   121,   122,    53,   352,    60,   263,   167,
    54,    59,   282,   109,   118,   119,   107,   108,   110,   111,
   113,   112,   114,   170,    60,   136,   334,   120,   121,   122,
    60,   276,   275,   218,    59,   274,   109,   118,   119,   107,
   108,   110,   111,   113,   112,   217,   273,   272,   271,   416,
   407,   121,   122,   388,   329,   213,   212,   210,   179,    59,
    60,    73,   180,    39,    25,    24,    23,    22,   177,   147,
   253,   259,   255,   256,   257,   258,    59,   269,   176,    70,
    20,    19,    59,   355,   265,   169,    56,    52,    84,    50,
     9,    49,     8,    48,     7,    47,    46,    45,     3,     1,
   221,    38,    71,    37,   223,   191,   253,   259,   255,   256,
   257,   258,    59,   224,   423,    90,   106,   105,   101,   123,
   466,   254,   251,   239,   371,    36,   235,   457,   444,   336,
   288,   418,   233,   145,   234,   483,   477,   437,   115,   280,
   193,   378,   102,    91,    89 };
static YYCONST yytabelem yypact[]={

  -174,-10000000,-10000000,  -174,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
  -111,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -220,  -220,  -253,  -220,
  -254,  -220,  -109,-10000000,-10000000,-10000000,-10000000,  -146,-10000000,  -135,
  -135,-10000000,  -233,  -234,-10000000,   173,   171,   168,   167,   165,
   162,-10000000,   161,   252,   323,  -135,  -121,  -284,-10000000,-10000000,
-10000000,  -286,  -287,  -261,-10000000,  -264,-10000000,  -135,  -135,  -135,
    52,   185,-10000000,    51,    -3,   218,  -135,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,   158,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,  -277,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -165,
-10000000,-10000000,-10000000,-10000000,-10000000,   138,   139,   137,-10000000,  -132,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,  -277,-10000000,-10000000,-10000000,-10000000,  -135,-10000000,  -255,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -135,-10000000,-10000000,
  -218,-10000000,  -220,-10000000,-10000000,    -3,-10000000,  -135,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,  -150,-10000000,   132,  -135,    50,    49,
  -135,-10000000,  -296,    47,  -179,    46,-10000000,  -220,-10000000,   -11,
    44,  -241,  -220,-10000000,  -277,-10000000,  -277,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,   353,   353,   353,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,    70,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,   252,  -135,  -192,   275,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,  -241,   223,   220,   217,    88,-10000000,-10000000,    40,
    82,   258,  -282,   207,   211,-10000000,-10000000,   317,   317,   317,
  -277,-10000000,   353,-10000000,  -258,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,   300,   130,-10000000,  -174,   163,   255,  -131,    38,
-10000000,   157,   156,   155,   154,   153,   152,  -135,-10000000,-10000000,
-10000000,-10000000,-10000000,   -78,-10000000,  -220,-10000000,-10000000,-10000000,-10000000,
-10000000,   126,   353,   353,   353,   353,   353,   353,   353,   353,
   353,   353,-10000000,-10000000,-10000000,   242,-10000000,   125,   123,-10000000,
-10000000,   163,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,   151,   254,
   300,  -122,-10000000,-10000000,  -220,  -135,    88,   353,-10000000,    82,
   258,  -282,   207,   207,   211,   211,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,   353,-10000000,    37,    36,    34,   163,  -135,   150,
  -160,    32,   216,-10000000,  -201,-10000000,-10000000,  -277,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,  -277,  -156,-10000000,-10000000,   205,-10000000,
   300,  -135,   253,    84,   147,   241,  -131,  -210,  -201,  -135,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,   250,-10000000,
-10000000,-10000000,  -224,   240,  -226,    42,   145,  -220,-10000000,-10000000,
-10000000,   238,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,   236,
  -248,   248,-10000000,   215,   300,  -161,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,   244,
  -220,  -226,  -135,    28,  -161,   252,   144,  -188,   184,   353,
-10000000,   232,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,  -260,-10000000,   142,  -135,   178,   227,-10000000,-10000000,
-10000000,-10000000,-10000000,   212,-10000000,  -262,-10000000 };
static YYCONST yytabelem yypgo[]={

     0,     7,     8,   694,   693,   692,   257,   189,    14,   691,
   129,   690,   689,   688,   255,     3,    90,   687,   686,   685,
     5,   684,   683,   682,   681,   680,   679,   678,   677,    13,
   676,   675,   674,     1,   673,    78,    80,    73,    66,    53,
    65,   166,   672,   671,    22,    21,     2,   670,   669,    20,
    19,    17,   244,   668,   667,   666,   665,    16,   664,   663,
     4,    10,   655,   654,   653,   652,     0,   651,   155,   152,
   151,   650,   649,   124,   648,   140,   647,   134,   646,   123,
   645,   644,   643,   642,   641,   640,   639,   638,   637,   117,
   636,   635,   634,   633,   631,   630,   629,   628,   160,   627,
   619,   618,   617,   616,   615,   614,   613,   612,   611,   608,
   607,   150,   606,   605,   604,   603,   600,   599,    12,   598,
   597,   596,   595,   585,   583,   582,   581,   576,   575,   573,
   560,   559,   558,   556,   555,   551,   550,   549,   548,   537,
   536,   535,   534,   112,    26,   533,   514,   512,   511,   510,
   508,   507,   506,   505,   504,   503,   502,     9,   501,   500,
   499,   498,   493,   478,   477,   474,   471,   470,   469,   468,
   467,    11,   466,   465,   462,   452,   451,   450,   449,   448,
   447,   446,   445,   444,   425,   422,   417,   412,   409,   406,
   404,   403,   402,   373,   370,   349,   347,   345,   329,   327,
   324,     6,   323,   322,   321,   319,   318,   317,   316,   313,
   300 };
static YYCONST yytabelem yyr1[]={

     0,    72,    73,    73,    76,    74,    78,    74,    80,    74,
    82,    74,    84,    74,    86,    74,    87,    74,    88,    74,
    74,    89,    89,    89,    89,    89,    89,    89,    90,    91,
    92,    93,    83,    81,    81,    96,    97,    99,    94,   100,
    65,    31,   101,    22,    22,    85,    85,    85,    85,   105,
   105,   104,   107,   103,   106,   108,   102,    67,    67,    68,
    68,    68,    69,    23,    23,    70,    70,    16,   110,   110,
   110,   109,   109,   112,    71,    71,   114,   117,   113,   115,
   115,   115,   116,   116,   118,    98,    98,   119,   111,   120,
   111,   121,   111,   123,   111,   125,   111,   126,   111,   111,
    20,   127,    21,    21,    15,   128,    15,   129,    15,    66,
    95,   130,   131,   132,   133,    77,    48,    48,    48,    48,
    48,    48,    48,    48,    32,    33,    34,    34,    35,    35,
    36,    36,    37,    37,    37,    38,    38,    38,    39,    39,
    39,    39,    40,    40,    40,    40,    41,    41,    41,    42,
    42,    42,    42,    42,    42,    43,    43,    44,   134,    75,
    75,    75,    75,   136,   135,     1,     1,     2,     2,     2,
    56,    56,    56,    56,    56,    56,     4,     4,     4,     3,
     3,     3,    29,   137,    30,    30,    60,    60,    61,    62,
    49,    49,    54,    54,    54,    55,    55,    55,    52,    52,
    52,    50,    50,    57,    51,    53,   138,   139,   140,   142,
     7,   141,   144,   144,   145,   146,   143,   147,   143,   148,
   149,   150,   151,   152,   153,   154,   156,    10,     9,     9,
     9,     9,     9,     9,   155,   158,   158,   159,   160,   157,
   161,   157,    27,    28,    28,   162,    46,   163,   164,    46,
   165,    47,   166,   167,   168,   170,     8,   169,   173,   172,
   172,   171,   174,   175,     5,     5,   176,   177,    13,   179,
   180,     6,     6,   178,   182,   183,    14,    14,   181,   184,
    11,    25,    26,    26,   185,   186,    45,   187,   188,   122,
    63,    63,    64,    64,   189,   190,   191,   192,    79,   193,
   194,   196,   197,   124,    59,    59,    59,    12,    12,   198,
   195,   199,   195,   200,   203,   202,   202,   204,   205,   201,
    58,    58,    58,   206,   207,    24,    24,   208,   209,    17,
    17,    18,   210,    19,    19 };
static YYCONST yytabelem yyr2[]={

     0,     2,     4,     0,     1,     7,     1,     7,     1,     7,
     1,     7,     1,     6,     1,     7,     1,     9,     1,     7,
     2,     7,     7,    11,     7,     5,     7,     5,     1,     1,
     1,     1,    19,     2,     2,     1,     1,     1,    15,     1,
     7,     7,     1,     7,     1,     2,     2,     2,     2,     5,
     5,     7,     1,    15,     4,     1,    11,     7,     9,     7,
     9,     3,     5,     7,     1,     5,     1,     2,     2,     2,
     2,     4,     0,     9,     3,     3,     1,     1,    18,     6,
     2,     0,     8,     0,     7,     4,     0,     1,     7,     1,
     7,     1,     7,     1,     7,     1,     7,     1,     7,     2,
     5,     1,     9,     1,     3,     1,     7,     1,     9,     3,
     5,     1,     1,     1,     1,    19,     2,     2,     2,     2,
     2,     3,     3,     3,     2,     2,     2,     7,     2,     7,
     2,     7,     2,     7,     7,     2,     7,     7,     2,     7,
     7,     7,     2,     5,     5,     5,     3,     2,     7,     3,
     3,     3,     3,     3,     3,     5,     3,     3,     1,     6,
     2,     2,     2,     1,     7,     2,     2,     3,     2,     3,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     5,     1,     9,     1,     2,     2,     3,     3,
     2,     2,     3,     5,     3,     5,     7,     5,     3,     3,
     5,     3,     3,     3,     3,     3,     1,     1,     1,     1,
    19,     4,     4,     0,     1,     1,    11,     1,     7,     1,
     1,     1,     1,     1,     1,     1,     1,    35,     3,     3,
     3,     3,     2,     3,     4,     4,     0,     1,     1,    11,
     1,     7,     5,     5,     1,     1,     7,     1,     1,    11,
     1,     7,     1,     1,     1,     1,    19,     4,     1,     8,
     0,     3,     1,     1,    13,     5,     1,     1,    11,     1,
     1,    13,     3,     3,     1,     1,    13,     3,     3,     1,
     7,     5,     5,     1,     1,     1,    11,     1,     1,    13,
     3,     1,     3,     1,     1,     1,     1,     1,    19,     1,
     1,     1,     1,    21,     3,     3,     1,     2,     3,     1,
     7,     1,     9,     4,     1,     8,     0,     1,     1,    11,
     3,     3,     3,     1,     1,    13,     1,     1,     1,    13,
     1,     5,     1,     9,     1 };
static YYCONST yytabelem yychk[]={

-10000000,   -72,   -73,   -74,   -75,   -77,   -79,   -81,   -83,   -85,
   301,   256,   -89,   262,    -7,   -10,    -8,   258,   280,   -94,
   -95,   259,  -102,  -103,  -104,  -105,   315,   316,   313,   317,
   318,   319,   312,   273,   274,   276,   -31,   -64,   -67,  -106,
   293,   260,   292,   299,   -73,   -76,   -78,   -80,   -82,   -84,
   -86,   257,   -88,  -134,  -130,  -189,   -90,   -15,   -66,   309,
   257,   -15,   304,   -15,   304,   -15,   256,  -138,  -148,  -166,
   -96,   -65,   261,  -108,   -66,   -66,   293,   293,    59,    59,
    59,    59,    59,    59,   -87,    59,  -135,    -1,    -2,    -3,
   -56,    -4,   -15,    -7,   -10,    -8,   -49,   -52,   -50,   -51,
   -57,   -53,    -5,    -6,   -14,   -54,   -55,   266,   267,   263,
   268,   269,   271,   270,   272,   -13,  -178,  -181,   264,   265,
   277,   278,   279,   -48,   -49,   -50,   -57,   -51,   -52,    -6,
   -14,   -15,   -66,   257,   320,   309,  -128,   320,   314,   304,
   302,   -66,   -66,   -66,   123,   -22,    58,  -100,   123,   -68,
    58,   -70,   295,    -1,   -68,   -66,    59,  -136,   263,   266,
    44,    62,    60,    60,   263,   264,  -176,  -131,  -190,   -91,
  -129,   -66,   304,  -139,  -149,  -167,   -97,  -101,   -66,  -109,
  -107,   -69,   294,   -16,   -15,   -20,   -15,   -68,   -29,   -60,
   -61,   -62,   -66,   -11,  -174,  -179,  -182,   263,    60,   -66,
   123,   123,   -66,   320,   123,   275,   123,   -98,   -20,   125,
  -110,  -111,  -112,  -113,   -75,   -77,   -79,  -122,  -124,   256,
   -89,   -71,   298,   -63,   -59,   296,   297,   283,   285,   286,
   123,   -70,   -69,   -23,   -21,   -30,  -184,   -44,   -33,   -34,
   -35,   -36,   -37,   -38,   -39,   -40,   -41,    43,    45,   126,
   -15,   -42,    40,   303,   -43,   305,   306,   307,   308,   304,
   -44,   -44,  -177,  -132,  -191,   -92,  -140,  -150,  -168,   -99,
  -111,  -119,  -120,  -121,  -123,  -125,  -126,    -1,   -66,   284,
   -12,    -2,   287,   -98,   -70,    44,    44,    44,   -25,   -45,
    91,  -175,   124,    94,    38,   310,   311,    43,    45,    42,
    47,    37,   -41,   -41,   -41,   -33,   304,  -180,  -183,    -2,
    61,  -144,   -73,  -141,  -143,    -1,   256,    40,  -169,  -171,
   257,   125,    59,    59,    59,    59,    59,    59,   -29,  -114,
  -187,  -193,   125,   -16,  -127,  -137,   -26,  -185,    62,   -35,
   -36,   -37,   -38,   -38,   -39,   -39,   -40,   -40,   -40,    41,
    62,    62,  -133,  -143,  -192,   -93,  -142,  -144,  -145,  -147,
  -151,  -170,  -172,    59,    40,    -2,   257,   -15,   -60,   -45,
   -44,   -32,   -33,   125,   125,   125,   -29,    59,    -9,   -49,
   -50,   -57,   -51,    -8,   -15,   263,   125,    44,  -115,  -118,
   288,  -188,  -194,  -186,  -146,  -152,  -173,    41,    44,    -2,
   -29,  -195,    40,    93,    59,    41,  -171,  -116,   291,  -118,
   -61,   -66,  -196,  -198,  -199,  -153,  -117,    40,   -24,   291,
    41,  -200,  -201,   -58,   288,   289,   290,   123,    59,   -20,
  -197,  -206,    41,  -202,  -204,  -154,    41,   -17,   300,    40,
    44,    -2,  -155,  -157,   -27,   256,   -46,   282,   281,  -208,
  -207,  -203,  -205,  -156,  -158,  -159,  -161,   -28,  -162,  -163,
    40,   -20,  -201,   -60,   125,  -157,   -47,    -1,    59,   -46,
    58,   -33,  -209,    41,  -160,  -165,  -164,   -18,   304,    59,
   -60,    58,    41,   -19,    44,  -210,   304 };
static YYCONST yytabelem yydef[]={

    -2,    -2,     1,    -2,     4,     6,     8,    10,    12,    14,
     0,    18,    20,   158,   160,   161,   162,   111,   294,    33,
    34,    28,    45,    46,    47,    48,     0,     0,     0,     0,
     0,     0,     0,   206,   219,   252,    35,     0,    55,     0,
     0,   292,     0,     0,     2,     0,     0,     0,     0,     0,
     0,    16,     0,     0,     0,     0,     0,     0,   104,   105,
   109,     0,     0,     0,    25,     0,    27,     0,     0,     0,
     0,    -2,    39,     0,    -2,    -2,     0,    54,     5,     7,
     9,    11,    13,    15,     0,    19,   159,   163,   165,   166,
   167,   168,   169,   179,   180,   181,   170,   171,   172,   173,
   174,   175,   176,   177,   178,   190,   191,   198,   199,   192,
   201,   202,   204,   203,   205,     0,   272,   277,   194,     0,
   266,   273,   278,   112,   116,   117,   118,   119,   120,   121,
   122,   123,   295,    29,    21,   107,     0,    22,     0,    24,
    26,   207,   220,   253,    36,    41,    42,     0,    72,    52,
     0,    61,     0,    51,    57,    66,    17,     0,   193,   200,
   262,   265,   269,   274,   195,   197,     0,     0,     0,     0,
     0,   106,     0,     0,     0,     0,    86,     0,    40,    -2,
     0,    66,     0,    64,    67,    65,   103,    58,   164,   185,
   186,   187,    -2,   189,     0,     0,     0,   196,   267,   113,
   296,    30,   108,    23,   208,   221,   254,    -2,    43,    56,
    71,    68,    69,    70,    87,    89,    91,    93,    95,    97,
    99,     0,     0,     0,     0,    74,    75,   290,   304,   305,
    86,    59,    66,    62,   100,   182,     0,   263,   157,   125,
   126,   128,   130,   132,   135,   138,   142,     0,     0,     0,
   146,   147,     0,   149,   150,   151,   152,   153,   154,   156,
   270,   275,     0,     0,   213,    -2,     0,     0,     0,     0,
    85,     0,     0,     0,     0,     0,     0,     0,    76,   287,
   299,   307,   308,    -2,    60,     0,   101,   183,   280,   283,
   284,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,   143,   144,   145,     0,   155,     0,     0,   268,
   114,    -2,    31,   209,   213,   214,   217,   222,   255,   260,
   261,    38,    88,    90,    92,    94,    96,    98,     0,     0,
     0,     0,    53,    63,     0,     0,   281,     0,   264,   127,
   129,   131,   133,   134,   136,   137,   139,   140,   141,   148,
   271,   276,     0,   212,     0,     0,     0,    -2,     0,     0,
     0,     0,   257,    73,    81,   288,   300,   102,   184,   282,
   285,   115,   124,   298,    32,   210,   215,   218,   223,   228,
   229,   230,   231,   232,   233,   192,   256,   258,     0,    80,
     0,     0,     0,     0,     0,     0,     0,    83,     0,     0,
   289,   301,    -2,   286,   216,   224,   259,    77,     0,    79,
    84,   188,   326,     0,     0,     0,     0,     0,   302,   323,
   310,     0,   316,   317,   320,   321,   322,   225,    78,     0,
   330,     0,   312,   313,     0,     0,    82,   303,   327,   324,
   314,   318,   226,   236,   237,   240,   244,   245,   247,     0,
     0,     0,     0,     0,    -2,     0,     0,   242,     0,     0,
   328,     0,   315,   319,   227,   235,   238,   250,   241,   243,
   246,   248,     0,   325,     0,     0,     0,     0,   334,   239,
   251,   249,   329,   331,   332,     0,   333 };
typedef struct
#ifdef __cplusplus
        yytoktype
#endif
{ char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#       define YYDEBUG  0       /* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
        "IDENTIFIER",   257,
        "CONST",        258,
        "MODULE",       259,
        "LOCAL",        260,
        "INTERFACE",    261,
        "TYPEDEF",      262,
        "LONG", 263,
        "SHORT",        264,
        "UNSIGNED",     265,
        "DOUBLE",       266,
        "FLOAT",        267,
        "CHAR", 268,
        "WCHAR",        269,
        "OCTET",        270,
        "BOOLEAN",      271,
        "ANY",  272,
        "STRUCT",       273,
        "UNION",        274,
        "SWITCH",       275,
        "ENUM", 276,
        "SEQUENCE",     277,
        "STRING",       278,
        "WSTRING",      279,
        "EXCEPTION",    280,
        "CASE", 281,
        "DEFAULT",      282,
        "READONLY",     283,
        "ATTRIBUTE",    284,
        "ONEWAY",       285,
        "IDEMPOTENT",   286,
        "VOID", 287,
        "IN",   288,
        "OUT",  289,
        "INOUT",        290,
        "RAISES",       291,
        "CUSTOM",       292,
        "VALUETYPE",    293,
        "TRUNCATABLE",  294,
        "SUPPORTS",     295,
        "PUBLIC",       296,
        "PRIVATE",      297,
        "FACTORY",      298,
        "ABSTRACT",     299,
        "CONTEXT",      300,
        "OPAQUE",       301,
        "VERSION",      302,
        "INTEGER_LITERAL",      303,
        "STRING_LITERAL",       304,
        "CHARACTER_LITERAL",    305,
        "FLOATING_PT_LITERAL",  306,
        "TRUETOK",      307,
        "FALSETOK",     308,
        "SCOPE_DELIMITOR",      309,
        "LEFT_SHIFT",   310,
        "RIGHT_SHIFT",  311,
        "PRAGMA",       312,
        "PRAGMA_INCLUDE",       313,
        "PFROM",        314,
        "PRAGMA_ASYNC_CLIENT",  315,
        "PRAGMA_ASYNC_SERVER",  316,
        "PRAGMA_ID",    317,
        "PRAGMA_PREFIX",        318,
        "PRAGMA_VERSION",       319,
        "PRAGMA_END",   320,
        "-unknown-",    -1      /* ends search */
};

char * yyreds[] =
{
        "-no such reduction-",
        "start : definitions",
        "definitions : definition definitions",
        "definitions : /* empty */",
        "definition : type_dcl",
        "definition : type_dcl ';'",
        "definition : const_dcl",
        "definition : const_dcl ';'",
        "definition : exception",
        "definition : exception ';'",
        "definition : interface_def",
        "definition : interface_def ';'",
        "definition : module",
        "definition : module ';'",
        "definition : value",
        "definition : value ';'",
        "definition : OPAQUE IDENTIFIER",
        "definition : OPAQUE IDENTIFIER ';'",
        "definition : error",
        "definition : error ';'",
        "definition : pragma",
        "pragma : PRAGMA_ASYNC_CLIENT scoped_name PRAGMA_END",
        "pragma : PRAGMA_ASYNC_SERVER scoped_name PRAGMA_END",
        "pragma : PRAGMA_INCLUDE STRING_LITERAL PFROM STRING_LITERAL PRAGMA_END",
        "pragma : PRAGMA_ID scoped_name STRING_LITERAL",
        "pragma : PRAGMA_PREFIX STRING_LITERAL",
        "pragma : PRAGMA_VERSION scoped_name VERSION",
        "pragma : PRAGMA error",
        "module : MODULE",
        "module : MODULE IDENTIFIER",
        "module : MODULE IDENTIFIER '{'",
        "module : MODULE IDENTIFIER '{' definitions",
        "module : MODULE IDENTIFIER '{' definitions '}'",
        "interface_def : interface",
        "interface_def : forward",
        "interface : interface_header",
        "interface : interface_header '{'",
        "interface : interface_header '{' exports",
        "interface : interface_header '{' exports '}'",
        "interface_decl : INTERFACE",
        "interface_decl : INTERFACE id",
        "interface_header : opt_local interface_decl inheritance_spec",
        "inheritance_spec : ':'",
        "inheritance_spec : ':' at_least_one_scoped_name",
        "inheritance_spec : /* empty */",
        "value : value_dcl",
        "value : value_abs_dcl",
        "value : value_box_dcl",
        "value : value_forward_dcl",
        "value_forward_dcl : VALUETYPE id",
        "value_forward_dcl : abstract_valuetype id",
        "value_box_dcl : VALUETYPE id type_spec",
        "value_abs_dcl : abstract_valuetype id value_inheritance_spec",
        "value_abs_dcl : abstract_valuetype id value_inheritance_spec '{' exports '}'",
        "abstract_valuetype : ABSTRACT VALUETYPE",
        "value_dcl : value_header",
        "value_dcl : value_header '{' value_elements '}'",
        "value_header : VALUETYPE id value_inheritance_spec",
        "value_header : CUSTOM VALUETYPE id value_inheritance_spec",
        "value_inheritance_spec : ':' at_least_one_value_name support_dcl",
        "value_inheritance_spec : ':' TRUNCATABLE at_least_one_value_name support_dcl",
        "value_inheritance_spec : support_dcl",
        "at_least_one_value_name : value_name value_names",
        "value_names : value_names ',' value_name",
        "value_names : /* empty */",
        "support_dcl : SUPPORTS at_least_one_scoped_name",
        "support_dcl : /* empty */",
        "value_name : scoped_name",
        "value_element : export",
        "value_element : state_member",
        "value_element : init_dcl",
        "value_elements : value_elements value_element",
        "value_elements : /* empty */",
        "state_member : access_decl type_spec at_least_one_declarator ';'",
        "access_decl : PUBLIC",
        "access_decl : PRIVATE",
        "init_dcl : FACTORY id",
        "init_dcl : FACTORY id '(' init_param_dcls ')' raises_expr",
        "init_dcl : FACTORY id '(' init_param_dcls ')' raises_expr ';'",
        "init_param_dcls : init_param_dcls ',' init_param_dcl",
        "init_param_dcls : init_param_dcl",
        "init_param_dcls : /* empty */",
        "raises_expr : RAISES '(' at_least_one_scoped_name ')'",
        "raises_expr : /* empty */",
        "init_param_dcl : IN simple_type_spec simple_declarator",
        "exports : exports export",
        "exports : /* empty */",
        "export : type_dcl",
        "export : type_dcl ';'",
        "export : const_dcl",
        "export : const_dcl ';'",
        "export : exception",
        "export : exception ';'",
        "export : attribute",
        "export : attribute ';'",
        "export : operation",
        "export : operation ';'",
        "export : error",
        "export : error ';'",
        "export : pragma",
        "at_least_one_scoped_name : scoped_name scoped_names",
        "scoped_names : scoped_names ','",
        "scoped_names : scoped_names ',' scoped_name",
        "scoped_names : /* empty */",
        "scoped_name : id",
        "scoped_name : SCOPE_DELIMITOR",
        "scoped_name : SCOPE_DELIMITOR id",
        "scoped_name : scoped_name SCOPE_DELIMITOR",
        "scoped_name : scoped_name SCOPE_DELIMITOR id",
        "id : IDENTIFIER",
        "forward : opt_local interface_decl",
        "const_dcl : CONST",
        "const_dcl : CONST const_type",
        "const_dcl : CONST const_type id",
        "const_dcl : CONST const_type id '='",
        "const_dcl : CONST const_type id '=' expression",
        "const_type : integer_type",
        "const_type : char_type",
        "const_type : octet_type",
        "const_type : boolean_type",
        "const_type : floating_pt_type",
        "const_type : string_type_spec",
        "const_type : wstring_type_spec",
        "const_type : scoped_name",
        "expression : const_expr",
        "const_expr : or_expr",
        "or_expr : xor_expr",
        "or_expr : or_expr '|' xor_expr",
        "xor_expr : and_expr",
        "xor_expr : xor_expr '^' and_expr",
        "and_expr : shift_expr",
        "and_expr : and_expr '&' shift_expr",
        "shift_expr : add_expr",
        "shift_expr : shift_expr LEFT_SHIFT add_expr",
        "shift_expr : shift_expr RIGHT_SHIFT add_expr",
        "add_expr : mult_expr",
        "add_expr : add_expr '+' mult_expr",
        "add_expr : add_expr '-' mult_expr",
        "mult_expr : unary_expr",
        "mult_expr : mult_expr '*' unary_expr",
        "mult_expr : mult_expr '/' unary_expr",
        "mult_expr : mult_expr '%' unary_expr",
        "unary_expr : primary_expr",
        "unary_expr : '+' primary_expr",
        "unary_expr : '-' primary_expr",
        "unary_expr : '~' primary_expr",
        "primary_expr : scoped_name",
        "primary_expr : literal",
        "primary_expr : '(' const_expr ')'",
        "literal : INTEGER_LITERAL",
        "literal : string_literal",
        "literal : CHARACTER_LITERAL",
        "literal : FLOATING_PT_LITERAL",
        "literal : TRUETOK",
        "literal : FALSETOK",
        "string_literal : string_literal STRING_LITERAL",
        "string_literal : STRING_LITERAL",
        "positive_int_expr : const_expr",
        "type_dcl : TYPEDEF",
        "type_dcl : TYPEDEF type_declarator",
        "type_dcl : struct_type",
        "type_dcl : union_type",
        "type_dcl : enum_type",
        "type_declarator : type_spec",
        "type_declarator : type_spec at_least_one_declarator",
        "type_spec : simple_type_spec",
        "type_spec : constructed_type_spec",
        "simple_type_spec : base_type_spec",
        "simple_type_spec : template_type_spec",
        "simple_type_spec : scoped_name",
        "base_type_spec : integer_type",
        "base_type_spec : floating_pt_type",
        "base_type_spec : char_type",
        "base_type_spec : boolean_type",
        "base_type_spec : octet_type",
        "base_type_spec : any_type",
        "template_type_spec : sequence_type_spec",
        "template_type_spec : string_type_spec",
        "template_type_spec : wstring_type_spec",
        "constructed_type_spec : struct_type",
        "constructed_type_spec : union_type",
        "constructed_type_spec : enum_type",
        "at_least_one_declarator : declarator declarators",
        "declarators : declarators ','",
        "declarators : declarators ',' declarator",
        "declarators : /* empty */",
        "declarator : simple_declarator",
        "declarator : complex_declarator",
        "simple_declarator : id",
        "complex_declarator : array_declarator",
        "integer_type : signed_int",
        "integer_type : unsigned_int",
        "signed_int : LONG",
        "signed_int : LONG LONG",
        "signed_int : SHORT",
        "unsigned_int : UNSIGNED LONG",
        "unsigned_int : UNSIGNED LONG LONG",
        "unsigned_int : UNSIGNED SHORT",
        "floating_pt_type : DOUBLE",
        "floating_pt_type : FLOAT",
        "floating_pt_type : LONG DOUBLE",
        "char_type : CHAR",
        "char_type : WCHAR",
        "octet_type : OCTET",
        "boolean_type : BOOLEAN",
        "any_type : ANY",
        "struct_type : STRUCT",
        "struct_type : STRUCT id",
        "struct_type : STRUCT id '{'",
        "struct_type : STRUCT id '{' at_least_one_member",
        "struct_type : STRUCT id '{' at_least_one_member '}'",
        "at_least_one_member : member members",
        "members : members member",
        "members : /* empty */",
        "member : type_spec",
        "member : type_spec at_least_one_declarator",
        "member : type_spec at_least_one_declarator ';'",
        "member : error",
        "member : error ';'",
        "union_type : UNION",
        "union_type : UNION id",
        "union_type : UNION id SWITCH",
        "union_type : UNION id SWITCH '('",
        "union_type : UNION id SWITCH '(' switch_type_spec",
        "union_type : UNION id SWITCH '(' switch_type_spec ')'",
        "union_type : UNION id SWITCH '(' switch_type_spec ')' '{'",
        "union_type : UNION id SWITCH '(' switch_type_spec ')' '{' at_least_one_case_branch",
        "union_type : UNION id SWITCH '(' switch_type_spec ')' '{' at_least_one_case_branch '}'",
        "switch_type_spec : integer_type",
        "switch_type_spec : char_type",
        "switch_type_spec : octet_type",
        "switch_type_spec : boolean_type",
        "switch_type_spec : enum_type",
        "switch_type_spec : scoped_name",
        "at_least_one_case_branch : case_branch case_branches",
        "case_branches : case_branches case_branch",
        "case_branches : /* empty */",
        "case_branch : at_least_one_case_label",
        "case_branch : at_least_one_case_label element_spec",
        "case_branch : at_least_one_case_label element_spec ';'",
        "case_branch : error",
        "case_branch : error ';'",
        "at_least_one_case_label : case_label case_labels",
        "case_labels : case_labels case_label",
        "case_labels : /* empty */",
        "case_label : DEFAULT",
        "case_label : DEFAULT ':'",
        "case_label : CASE",
        "case_label : CASE const_expr",
        "case_label : CASE const_expr ':'",
        "element_spec : type_spec",
        "element_spec : type_spec declarator",
        "enum_type : ENUM",
        "enum_type : ENUM id",
        "enum_type : ENUM id '{'",
        "enum_type : ENUM id '{' at_least_one_enumerator",
        "enum_type : ENUM id '{' at_least_one_enumerator '}'",
        "at_least_one_enumerator : enumerator enumerators",
        "enumerators : enumerators ','",
        "enumerators : enumerators ',' enumerator",
        "enumerators : /* empty */",
        "enumerator : IDENTIFIER",
        "sequence_type_spec : seq_head ','",
        "sequence_type_spec : seq_head ',' positive_int_expr",
        "sequence_type_spec : seq_head ',' positive_int_expr '>'",
        "sequence_type_spec : seq_head '>'",
        "seq_head : SEQUENCE",
        "seq_head : SEQUENCE '<'",
        "seq_head : SEQUENCE '<' simple_type_spec",
        "string_type_spec : string_head '<'",
        "string_type_spec : string_head '<' positive_int_expr",
        "string_type_spec : string_head '<' positive_int_expr '>'",
        "string_type_spec : string_head",
        "string_head : STRING",
        "wstring_type_spec : wstring_head '<'",
        "wstring_type_spec : wstring_head '<' positive_int_expr",
        "wstring_type_spec : wstring_head '<' positive_int_expr '>'",
        "wstring_type_spec : wstring_head",
        "wstring_head : WSTRING",
        "array_declarator : id",
        "array_declarator : id at_least_one_array_dim",
        "at_least_one_array_dim : array_dim array_dims",
        "array_dims : array_dims array_dim",
        "array_dims : /* empty */",
        "array_dim : '['",
        "array_dim : '[' positive_int_expr",
        "array_dim : '[' positive_int_expr ']'",
        "attribute : opt_readonly ATTRIBUTE",
        "attribute : opt_readonly ATTRIBUTE simple_type_spec",
        "attribute : opt_readonly ATTRIBUTE simple_type_spec at_least_one_declarator",
        "opt_readonly : READONLY",
        "opt_readonly : /* empty */",
        "opt_local : LOCAL",
        "opt_local : /* empty */",
        "exception : EXCEPTION",
        "exception : EXCEPTION id",
        "exception : EXCEPTION id '{'",
        "exception : EXCEPTION id '{' members",
        "exception : EXCEPTION id '{' members '}'",
        "operation : opt_op_attribute op_type_spec",
        "operation : opt_op_attribute op_type_spec IDENTIFIER",
        "operation : opt_op_attribute op_type_spec IDENTIFIER parameter_list",
        "operation : opt_op_attribute op_type_spec IDENTIFIER parameter_list opt_raises",
        "operation : opt_op_attribute op_type_spec IDENTIFIER parameter_list opt_raises opt_context",
        "opt_op_attribute : ONEWAY",
        "opt_op_attribute : IDEMPOTENT",
        "opt_op_attribute : /* empty */",
        "op_type_spec : simple_type_spec",
        "op_type_spec : VOID",
        "parameter_list : '('",
        "parameter_list : '(' ')'",
        "parameter_list : '('",
        "parameter_list : '(' at_least_one_parameter ')'",
        "at_least_one_parameter : parameter parameters",
        "parameters : parameters ','",
        "parameters : parameters ',' parameter",
        "parameters : /* empty */",
        "parameter : direction",
        "parameter : direction simple_type_spec",
        "parameter : direction simple_type_spec declarator",
        "direction : IN",
        "direction : OUT",
        "direction : INOUT",
        "opt_raises : RAISES",
        "opt_raises : RAISES '('",
        "opt_raises : RAISES '(' at_least_one_scoped_name ')'",
        "opt_raises : /* empty */",
        "opt_context : CONTEXT",
        "opt_context : CONTEXT '('",
        "opt_context : CONTEXT '(' at_least_one_string_literal ')'",
        "opt_context : /* empty */",
        "at_least_one_string_literal : STRING_LITERAL string_literals",
        "string_literals : string_literals ','",
        "string_literals : string_literals ',' STRING_LITERAL",
        "string_literals : /* empty */",
};
#endif /* YYDEBUG */
# line  1 "/usr/ccs/bin/yaccpar"
/*
 * Copyright (c) 1993 by Sun Microsystems, Inc.
 */

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR         goto yyerrlab
#define YYACCEPT        return(0)
#define YYABORT         return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
        if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
        {\
                yyerror( "syntax error - cannot backup" );\
                goto yyerrlab;\
        }\
        yychar = newtoken;\
        yystate = *yyps;\
        yylval = newvalue;\
        goto yynewstate;\
}
#define YYRECOVERING()  (!!yyerrflag)
#define YYNEW(type)     os_malloc(sizeof(type) * yynewmax)
#define YYCOPY(to, from, type) \
        (type *) memcpy(to, (char *) from, yymaxdepth * sizeof (type))
#define YYENLARGE( from, type) \
        (type *) realloc((char *) from, yynewmax * sizeof(type))
#ifndef YYDEBUG
#       define YYDEBUG  1       /* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;                    /* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG          (-10000000)

/*
** global variables used by the parser
*/
YYSTYPE *yypv;                  /* top of value stack */
int *yyps;                      /* top of state stack */

int yystate;                    /* current state */
int yytmp;                      /* extra var (lasts between blocks) */

int yynerrs;                    /* number of errors */
int yyerrflag;                  /* error recovery flag */
int yychar;                     /* current input token number */



#ifdef YYNMBCHARS
#define YYLEX()         yycvtok(yylex())
/*
** yycvtok - return a token if i is a wchar_t value that exceeds 255.
**      If i<255, i itself is the token.  If i>255 but the neither
**      of the 30th or 31st bit is on, i is already a token.
*/
#if defined(__STDC__) || defined(__cplusplus)
int yycvtok(int i)
#else
int yycvtok(i) int i;
#endif
{
        int first = 0;
        int last = YYNMBCHARS - 1;
        int mid;
        wchar_t j;

        if(i&0x60000000){/*Must convert to a token. */
                if( yymbchars[last].character < i ){
                        return i;/*Giving up*/
                }
                while ((last>=first)&&(first>=0)) {/*Binary search loop*/
                        mid = (first+last)/2;
                        j = yymbchars[mid].character;
                        if( j==i ){/*Found*/
                                return yymbchars[mid].tvalue;
                        }else if( j<i ){
                                first = mid + 1;
                        }else{
                                last = mid -1;
                        }
                }
                /*No entry in the table.*/
                return i;/* Giving up.*/
        }else{/* i is already a token. */
                return i;
        }
}
#else/*!YYNMBCHARS*/
#define YYLEX()         yylex()
#endif/*!YYNMBCHARS*/

/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
#if defined(__STDC__) || defined(__cplusplus)
int yyparse(void)
#else
int yyparse()
#endif
{
        register YYSTYPE *yypvt = 0;    /* top of value stack for $vars */

#if defined(__cplusplus) || defined(lint)
/*
        hacks to please C++ and lint - goto's inside
        switch should never be executed
*/
        static int __yaccpar_lint_hack__ = 0;
        switch (__yaccpar_lint_hack__)
        {
                case 1: goto yyerrlab;
                case 2: goto yynewstate;
        }
#endif

        /*
        ** Initialize externals - yyparse may be called more than once
        */
        yypv = &yyv[-1];
        yyps = &yys[-1];
        yystate = 0;
        yytmp = 0;
        yynerrs = 0;
        yyerrflag = 0;
        yychar = -1;

#if YYMAXDEPTH <= 0
        if (yymaxdepth <= 0)
        {
                if ((yymaxdepth = YYEXPAND(0)) <= 0)
                {
                        yyerror("yacc initialization error");
                        YYABORT;
                }
        }
#endif

        {
                register YYSTYPE *yy_pv;        /* top of value stack */
                register int *yy_ps;            /* top of state stack */
                register int yy_state;          /* current state */
                register int  yy_n;             /* internal state number info */
        goto yystack;   /* moved from 6 lines above to here to please C++ */

                /*
                ** get globals into registers.
                ** branch to here only if YYBACKUP was called.
                */
        yynewstate:
                yy_pv = yypv;
                yy_ps = yyps;
                yy_state = yystate;
                goto yy_newstate;

                /*
                ** get globals into registers.
                ** either we just started, or we just finished a reduction
                */
        yystack:
                yy_pv = yypv;
                yy_ps = yyps;
                yy_state = yystate;

                /*
                ** top of for (;;) loop while no reductions done
                */
        yy_stack:
                /*
                ** put a state and value onto the stacks
                */
#if YYDEBUG
                /*
                ** if debugging, look up token value in list of value vs.
                ** name pairs.  0 and negative (-1) are special values.
                ** Note: linear search is used since time is not a real
                ** consideration while debugging.
                */
                if ( yydebug )
                {
                        register int yy_i;

                        printf( "State %d, token ", yy_state );
                        if ( yychar == 0 )
                                printf( "end-of-file\n" );
                        else if ( yychar < 0 )
                                printf( "-none-\n" );
                        else
                        {
                                for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
                                        yy_i++ )
                                {
                                        if ( yytoks[yy_i].t_val == yychar )
                                                break;
                                }
                                printf( "%s\n", yytoks[yy_i].t_name );
                        }
                }
#endif /* YYDEBUG */
                if ( ++yy_ps >= &yys[ yymaxdepth ] )    /* room on stack? */
                {
                        /*
                        ** reallocate and recover.  Note that pointers
                        ** have to be reset, or bad things will happen
                        */
                        long yyps_index = (yy_ps - yys);
                        long yypv_index = (yy_pv - yyv);
                        long yypvt_index = (yypvt - yyv);
                        int yynewmax;
#ifdef YYEXPAND
                        yynewmax = YYEXPAND(yymaxdepth);
#else
                        yynewmax = 2 * yymaxdepth;      /* double table size */
                        if (yymaxdepth == YYMAXDEPTH)   /* first time growth */
                        {
                                char *newyys = (char *)YYNEW(int);
                                char *newyyv = (char *)YYNEW(YYSTYPE);
                                if (newyys != 0 && newyyv != 0)
                                {
                                        yys = YYCOPY(newyys, yys, int);
                                        yyv = YYCOPY(newyyv, yyv, YYSTYPE);
                                }
                                else
                                        yynewmax = 0;   /* failed */
                        }
                        else                            /* not first time */
                        {
                                yys = YYENLARGE(yys, int);
                                yyv = YYENLARGE(yyv, YYSTYPE);
                                if (yys == 0 || yyv == 0)
                                        yynewmax = 0;   /* failed */
                        }
#endif
                        if (yynewmax <= yymaxdepth)     /* tables not expanded */
                        {
                                yyerror( "yacc stack overflow" );
                                YYABORT;
                        }
                        yymaxdepth = yynewmax;

                        yy_ps = yys + yyps_index;
                        yy_pv = yyv + yypv_index;
                        yypvt = yyv + yypvt_index;
                }
                *yy_ps = yy_state;
                *++yy_pv = yyval;

                /*
                ** we have a new state - find out what to do
                */
        yy_newstate:
                if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
                        goto yydefault;         /* simple state */
#if YYDEBUG
                /*
                ** if debugging, need to mark whether new token grabbed
                */
                yytmp = yychar < 0;
#endif
                if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
                        yychar = 0;             /* reached EOF */
#if YYDEBUG
                if ( yydebug && yytmp )
                {
                        register int yy_i;

                        printf( "Received token " );
                        if ( yychar == 0 )
                                printf( "end-of-file\n" );
                        else if ( yychar < 0 )
                                printf( "-none-\n" );
                        else
                        {
                                for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
                                        yy_i++ )
                                {
                                        if ( yytoks[yy_i].t_val == yychar )
                                                break;
                                }
                                printf( "%s\n", yytoks[yy_i].t_name );
                        }
                }
#endif /* YYDEBUG */
                if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
                        goto yydefault;
                if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )  /*valid shift*/
                {
                        yychar = -1;
                        yyval = yylval;
                        yy_state = yy_n;
                        if ( yyerrflag > 0 )
                                yyerrflag--;
                        goto yy_stack;
                }

        yydefault:
                if ( ( yy_n = yydef[ yy_state ] ) == -2 )
                {
#if YYDEBUG
                        yytmp = yychar < 0;
#endif
                        if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
                                yychar = 0;             /* reached EOF */
#if YYDEBUG
                        if ( yydebug && yytmp )
                        {
                                register int yy_i;

                                printf( "Received token " );
                                if ( yychar == 0 )
                                        printf( "end-of-file\n" );
                                else if ( yychar < 0 )
                                        printf( "-none-\n" );
                                else
                                {
                                        for ( yy_i = 0;
                                                yytoks[yy_i].t_val >= 0;
                                                yy_i++ )
                                        {
                                                if ( yytoks[yy_i].t_val
                                                        == yychar )
                                                {
                                                        break;
                                                }
                                        }
                                        printf( "%s\n", yytoks[yy_i].t_name );
                                }
                        }
#endif /* YYDEBUG */
                        /*
                        ** look through exception table
                        */
                        {
                                register YYCONST int *yyxi = yyexca;

                                while ( ( *yyxi != -1 ) ||
                                        ( yyxi[1] != yy_state ) )
                                {
                                        yyxi += 2;
                                }
                                while ( ( *(yyxi += 2) >= 0 ) &&
                                        ( *yyxi != yychar ) )
                                        ;
                                if ( ( yy_n = yyxi[1] ) < 0 )
                                        YYACCEPT;
                        }
                }

                /*
                ** check for syntax error
                */
                if ( yy_n == 0 )        /* have an error */
                {
                        /* no worry about speed here! */
                        switch ( yyerrflag )
                        {
                        case 0:         /* new error */
                                yyerror( "syntax error" );
                                goto skip_init;
                        yyerrlab:
                                /*
                                ** get globals into registers.
                                ** we have a user generated syntax type error
                                */
                                yy_pv = yypv;
                                yy_ps = yyps;
                                yy_state = yystate;
                        skip_init:
                                yynerrs++;
                                /* FALLTHRU */
                        case 1:
                        case 2:         /* incompletely recovered error */
                                        /* try again... */
                                yyerrflag = 3;
                                /*
                                ** find state where "error" is a legal
                                ** shift action
                                */
                                while ( yy_ps >= yys )
                                {
                                        yy_n = yypact[ *yy_ps ] + YYERRCODE;
                                        if ( yy_n >= 0 && yy_n < YYLAST &&
                                                yychk[yyact[yy_n]] == YYERRCODE)                                        {
                                                /*
                                                ** simulate shift of "error"
                                                */
                                                yy_state = yyact[ yy_n ];
                                                goto yy_stack;
                                        }
                                        /*
                                        ** current state has no shift on
                                        ** "error", pop stack
                                        */
#if YYDEBUG
#       define _POP_ "Error recovery pops state %d, uncovers state %d\n"
                                        if ( yydebug )
                                                printf( _POP_, *yy_ps,
                                                        yy_ps[-1] );
#       undef _POP_
#endif
                                        yy_ps--;
                                        yy_pv--;
                                }
                                /*
                                ** there is no state on stack with "error" as
                                ** a valid shift.  give up.
                                */
                                YYABORT;
                        case 3:         /* no shift yet; eat a token */
#if YYDEBUG
                                /*
                                ** if debugging, look up token in list of
                                ** pairs.  0 and negative shouldn't occur,
                                ** but since timing doesn't matter when
                                ** debugging, it doesn't hurt to leave the
                                ** tests here.
                                */
                                if ( yydebug )
                                {
                                        register int yy_i;

                                        printf( "Error recovery discards " );
                                        if ( yychar == 0 )
                                                printf( "token end-of-file\n" );
                                        else if ( yychar < 0 )
                                                printf( "token -none-\n" );
                                        else
                                        {
                                                for ( yy_i = 0;
                                                        yytoks[yy_i].t_val >= 0;
                                                        yy_i++ )
                                                {
                                                        if ( yytoks[yy_i].t_val
                                                                == yychar )
                                                        {
                                                                break;
                                                        }
                                                }
                                                printf( "token %s\n",
                                                        yytoks[yy_i].t_name );
                                        }
                                }
#endif /* YYDEBUG */
                                if ( yychar == 0 )      /* reached EOF. quit */
                                        YYABORT;
                                yychar = -1;
                                goto yy_newstate;
                        }
                }/* end if ( yy_n == 0 ) */
                /*
                ** reduction by production yy_n
                ** put stack tops, etc. so things right after switch
                */
#if YYDEBUG
                /*
                ** if debugging, print the string that is the user's
                ** specification of the reduction which is just about
                ** to be done.
                */
                if ( yydebug )
                        printf( "Reduce by (%d) \"%s\"\n",
                                yy_n, yyreds[ yy_n ] );
#endif
                yytmp = yy_n;                   /* value to switch over */
                yypvt = yy_pv;                  /* $vars top of value stack */
                /*
                ** Look in goto table for next state
                ** Sorry about using yy_state here as temporary
                ** register variable, but why not, if it works...
                ** If yyr2[ yy_n ] doesn't have the low order bit
                ** set, then there is no action to be done for
                ** this reduction.  So, no saving & unsaving of
                ** registers done.  The only difference between the
                ** code just after the if and the body of the if is
                ** the goto yy_stack in the body.  This way the test
                ** can be made before the choice of what to do is needed.
                */
                {
                        /* length of production doubled with extra bit */
                        register int yy_len = yyr2[ yy_n ];

                        if ( !( yy_len & 01 ) )
                        {
                                yy_len >>= 1;
                                yyval = ( yy_pv -= yy_len )[1]; /* $$ = $1 */
                                yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
                                        *( yy_ps -= yy_len ) + 1;
                                if ( yy_state >= YYLAST ||
                                        yychk[ yy_state =
                                        yyact[ yy_state ] ] != -yy_n )
                                {
                                        yy_state = yyact[ yypgo[ yy_n ] ];
                                }
                                goto yy_stack;
                        }
                        yy_len >>= 1;
                        yyval = ( yy_pv -= yy_len )[1]; /* $$ = $1 */
                        yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
                                *( yy_ps -= yy_len ) + 1;
                        if ( yy_state >= YYLAST ||
                                yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
                        {
                                yy_state = yyact[ yypgo[ yy_n ] ];
                        }
                }
                                        /* save until reenter driver code */
                yystate = yy_state;
                yyps = yy_ps;
                yypv = yy_pv;
        }
        /*
        ** code supplied by user is placed in this switch
        */
        switch( yytmp )
        {

case 4:
# line 270 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_TypeDeclSeen);
        } break;
case 5:
# line 274 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 6:
# line 278 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ConstDeclSeen);
        } break;
case 7:
# line 282 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 8:
# line 286 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptDeclSeen);
        } break;
case 9:
# line 290 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 10:
# line 294 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceDeclSeen);
        } break;
case 11:
# line 298 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 12:
# line 302 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ModuleDeclSeen);
        } break;
case 14:
# line 307 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ValueDeclSeen);
        } break;
case 15:
# line 311 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 16:
# line 315 "idl.yy"
{
                idl_global->set_parse_state(IDL_GlobalData::PS_OpaqueDeclSeen);
                UTL_Scope *s = idl_global->scopes()->top_non_null();
                UTL_ScopedName *n = new UTL_ScopedName(new
                                                Identifier(yypvt[-0].strval,1,0,I_FALSE),NULL);
                AST_Opaque *o = NULL;

                if(s != NULL)
                {
                        o = idl_global->gen()->create_opaque(n,s->get_pragmas());
                        (void) s->fe_add_opaque(o);
                }
        } break;
case 17:
# line 329 "idl.yy"
{
                idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 18:
# line 333 "idl.yy"
{
          idl_global->err()->syntax_error(idl_global->parse_state());
        } break;
case 19:
# line 337 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
          yyerrok;
        } break;
case 21:
# line 346 "idl.yy"
{
                UTL_Scope *s = idl_global->scopes()->top_non_null();
                AST_Decl *d = s->lookup_by_name(yypvt[-1].idlist,I_FALSE);
                if(d)
                {
                        if((AST_Interface *)d->narrow((long)&AST_Interface::type_id) ||
                                (AST_Operation *)d->narrow((long)&AST_Operation::type_id))
                        {
                                d->get_decl_pragmas().set_pragma_client_synchronicity(I_TRUE);
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
        } break;
case 22:
# line 367 "idl.yy"
{
                UTL_Scope *s = idl_global->scopes()->top_non_null();
                AST_Decl *d = s->lookup_by_name(yypvt[-1].idlist,I_FALSE);
                if(d)
                {
                        if((AST_Interface *)d->narrow((long)&AST_Interface::type_id) ||
                                (AST_Operation *)d->narrow((long)&AST_Operation::type_id))
                        {
                                d->get_decl_pragmas().set_pragma_server_synchronicity(I_TRUE);
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
        } break;
case 23:
# line 388 "idl.yy"
{
      UTL_IncludeFiles::AddIncludeFile(yypvt[-3].sval->get_string(),yypvt[-1].sval->get_string());
        } break;
case 24:
# line 392 "idl.yy"
{
                UTL_Scope     *s = idl_global->scopes()->top_non_null();
                AST_Decl *d = s->lookup_by_name(yypvt[-1].idlist,I_FALSE);
                if(d)
                {
                        if(!d->get_decl_pragmas().get_pragma_ID())
                        {
                                d->get_decl_pragmas().set_pragma_ID(new UTL_String(yypvt[-0].sval->get_string()));
                        }
                        else
                        {
                                idl_global->err()->warning_msg("Identifier already has ID.  Pragma ignored.");
                        }
                }
                else
                {
                                idl_global->err()->warning_msg("Identifier not found.");
                }
        } break;
case 25:
# line 412 "idl.yy"
{
                UTL_Scope     *s = idl_global->scopes()->top_non_null();
                s->get_pragmas().set_pragma_prefix(yypvt[-0].sval);
        } break;
case 26:
# line 417 "idl.yy"
{
                UTL_Scope     *s = idl_global->scopes()->top_non_null();
                AST_Decl *d = s->lookup_by_name(yypvt[-1].idlist,I_FALSE);
                if(d)
                {
                        if(!d->get_decl_pragmas().get_pragma_version())
                        {
                                d->get_decl_pragmas().set_pragma_version(new UTL_String(yypvt[-0].strval));
                        }
                        else
                        {
                                idl_global->err()->warning_msg("Identifier already has version.  Pragma ignored.");
                        }
                }
                else
                {
                        idl_global->err()->error_msg("Identifier not found.");
                }
        } break;
case 27:
# line 437 "idl.yy"
{
          idl_global->err()->warning_msg("Unrecognized pragma ignored.");
        } break;
case 28:
# line 443 "idl.yy"
{
            idl_global->set_parse_state(IDL_GlobalData::PS_ModuleSeen);
          } break;
case 29:
# line 447 "idl.yy"
{
            if (idl_global->valid_identifier(yypvt[-0].strval))
            {
              Identifier * id = new Identifier (yypvt[-0].strval, 1, 0, I_FALSE);
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

                    if(inMainFile)
                    {
                       m->set_imported(FALSE);
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
          } break;
case 30:
# line 488 "idl.yy"
{
            idl_global->set_parse_state(IDL_GlobalData::PS_ModuleSqSeen);
          } break;
case 31:
# line 492 "idl.yy"
{
            idl_global->set_parse_state(IDL_GlobalData::PS_ModuleBodySeen);
          } break;
case 32:
# line 496 "idl.yy"
{
            idl_global->set_parse_state(IDL_GlobalData::PS_ModuleQsSeen);
            /*
             * Finished with this module - pop it from the scope stack
             */
            idl_global->scopes()->pop();
            g_feScopeStack.Pop();
          } break;
case 35:
# line 513 "idl.yy"
{
          UTL_Scope     *s = idl_global->scopes()->top_non_null();
          AST_Interface *i = NULL;
          AST_Decl      *d = NULL;
          AST_Interface *fd = NULL;

          /*
           * Make a new interface node and add it to its enclosing scope
           */
          if (s != NULL && yypvt[-0].ihval != NULL) {
            i = idl_global->gen()->create_interface(yypvt[-0].ihval->local(),
                                                    yypvt[-0].ihval->interface_name(),
                                                    yypvt[-0].ihval->inherits(),
                                                    yypvt[-0].ihval->n_inherits(),
                                                    s->get_pragmas());
            if (i != NULL &&
                (d = s->lookup_by_name(i->name(), I_FALSE)) != NULL) {
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
                  else {
                    fd->set_local(yypvt[-0].ihval->local());
                    fd->set_inherits(yypvt[-0].ihval->inherits());
                    fd->set_n_inherits(yypvt[-0].ihval->n_inherits());
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
            (void) s->fe_add_interface(i);
          }
          /*
           * Push it on the scope stack
           */
          idl_global->scopes()->push(i);
          g_feScopeStack.Push(be_CppEnclosingScope(*(yypvt[-0].ihval->interface_name()),
            be_CppEnclosingScope::NameIsScope()));
      } break;
case 36:
# line 608 "idl.yy"
{
            idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceSqSeen);
        } break;
case 37:
# line 612 "idl.yy"
{
            idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceBodySeen);
        } break;
case 38:
# line 616 "idl.yy"
{
            idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceQsSeen);
            /*
             * Done with this interface - pop it off the scopes stack
             */
            g_feScopeStack.Pop();
            idl_global->scopes()->pop();
        } break;
case 39:
# line 628 "idl.yy"
{
             idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceSeen);
         } break;
case 40:
# line 632 "idl.yy"
{
             idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceIDSeen);
             yyval.idval = yypvt[-0].idval;
         } break;
case 41:
# line 641 "idl.yy"
{
            idl_global->set_parse_state(IDL_GlobalData::PS_InheritSpecSeen);
            /*
             * Create an AST representation of the information in the header
             * part of an interface - this representation contains a computed
             * list of all interfaces which this interface inherits from,
             * recursively
             */
            yyval.ihval = new FE_InterfaceHeader(yypvt[-2].bval, new UTL_ScopedName(yypvt[-1].idval, NULL), yypvt[-0].nlval);
        } break;
case 42:
# line 655 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_InheritColonSeen);
        } break;
case 43:
# line 659 "idl.yy"
{
          yyval.nlval = yypvt[-0].nlval;
        } break;
case 44:
# line 663 "idl.yy"
{
          yyval.nlval = NULL;
        } break;
case 49:
# line 678 "idl.yy"
{
              UTL_Scope         *s = idl_global->scopes()->top_non_null();
              UTL_ScopedName    *n = new UTL_ScopedName(yypvt[-0].idval, NULL);
              AST_ValueFwd      *f = NULL;

              idl_global->set_parse_state(IDL_GlobalData::PS_ValueForwardDeclSeen);
              /*
               * Create a node representing a forward declaration of a
               * value type. Store it in the enclosing scope
               */
              if (s != NULL) {
                f = idl_global->gen()->create_valuetype_fwd(I_FALSE, n, s->get_pragmas());
                (void) s->fe_add_valuetype_fwd(f);
              }
        } break;
case 50:
# line 694 "idl.yy"
{
              UTL_Scope         *s = idl_global->scopes()->top_non_null();
              UTL_ScopedName    *n = new UTL_ScopedName(yypvt[-0].idval, NULL);
              AST_ValueFwd      *f = NULL;

              idl_global->set_parse_state(IDL_GlobalData::PS_ValueForwardDeclSeen);
              /*
               * Create a node representing a forward declaration of a
               * value type. Store it in the enclosing scope
               */
              if (s != NULL) {
                f = idl_global->gen()->create_valuetype_fwd(I_TRUE, n, s->get_pragmas());
                (void) s->fe_add_valuetype_fwd(f);
              }
        } break;
case 51:
# line 713 "idl.yy"
{
              UTL_Scope         *s = idl_global->scopes()->top_non_null();
              UTL_ScopedName    *n = new UTL_ScopedName(yypvt[-1].idval, NULL);
              AST_BoxedValue    *b = NULL;
              AST_Type          *tp = AST_Type::narrow_from_decl(yypvt[-0].dcval);

              idl_global->set_parse_state(IDL_GlobalData::PS_BoxedValueDeclSeen);
              if (tp == NULL)
              {
                 idl_global->err()->not_a_type(yypvt[-0].dcval);
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
        } break;
case 52:
# line 739 "idl.yy"
{
         UTL_Scope      *s = idl_global->scopes()->top_non_null();
         AST_Value      *i = NULL;
         AST_Decl       *d = NULL;
         AST_Value     *fd = NULL;
         UTL_ScopedName *n = new UTL_ScopedName(yypvt[-1].idval, NULL);

         /*
          * Make a new interface node and add it to its enclosing scope
          */
         if (s != NULL)
         {
            i = idl_global->gen()->create_valuetype
                (
                   I_TRUE,
                   I_FALSE,
                   yypvt[-0].visval->truncatable(),
                   n,
                   yypvt[-0].visval->inherits(),
                   yypvt[-0].visval->n_inherits(),
                   yypvt[-0].visval->supports(),
                   yypvt[-0].visval->n_supports(),
                   s->get_pragmas()
                );

            if (i != NULL &&
                (d = s->lookup_by_name(i->name(), I_FALSE)) != NULL) {
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
                        fd->set_value_inherits(yypvt[-0].visval->inherits());
                        fd->set_n_value_inherits(yypvt[-0].visval->n_inherits());
                        fd->set_inherits(yypvt[-0].visval->supports());
                        fd->set_n_inherits(yypvt[-0].visval->n_supports());
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
      } break;
case 53:
# line 825 "idl.yy"
{
            idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceQsSeen);
            /*
             * Done with this interface - pop it off the scopes stack
             */
            g_feScopeStack.Pop();
            idl_global->scopes()->pop();
        } break;
case 55:
# line 841 "idl.yy"
{
         UTL_Scope     *s = idl_global->scopes()->top_non_null();
         AST_Value     *i = NULL;
         AST_Decl      *d = NULL;
         AST_Value     *fd = NULL;

         /*
          * Make a new interface node and add it to its enclosing scope
          */
         if (s != NULL && yypvt[-0].vhval != NULL) {
            i = idl_global->gen()->create_valuetype(I_FALSE,
                                                    yypvt[-0].vhval->custom(),
                                                    yypvt[-0].vhval->truncatable(),
                                                    yypvt[-0].vhval->value_name(),
                                                    yypvt[-0].vhval->inherits(),
                                                    yypvt[-0].vhval->n_inherits(),
                                                    yypvt[-0].vhval->supports(),
                                                    yypvt[-0].vhval->n_supports(),
                                                    s->get_pragmas());
            if (i != NULL &&
                (d = s->lookup_by_name(i->name(), I_FALSE)) != NULL) {
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
                        fd->set_custom(yypvt[-0].vhval->custom());
                        fd->set_truncatable(yypvt[-0].vhval->truncatable());
                        fd->set_value_inherits(yypvt[-0].vhval->inherits());
                        fd->set_n_value_inherits(yypvt[-0].vhval->n_inherits());
                        fd->set_inherits(yypvt[-0].vhval->supports());
                        fd->set_n_inherits(yypvt[-0].vhval->n_supports());
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
      } break;
case 56:
# line 923 "idl.yy"
{
            idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceQsSeen);
            /*
             * Done with this interface - pop it off the scopes stack
             */
            g_feScopeStack.Pop();
            idl_global->scopes()->pop();
        } break;
case 57:
# line 935 "idl.yy"
{
            idl_global->set_parse_state(IDL_GlobalData::PS_ValueInheritSpecSeen);
            yyval.vhval = new FE_ValueHeader(I_FALSE, new UTL_ScopedName(yypvt[-1].idval, NULL), yypvt[-0].visval);
        } break;
case 58:
# line 940 "idl.yy"
{
            idl_global->set_parse_state(IDL_GlobalData::PS_ValueInheritSpecSeen);
            yyval.vhval = new FE_ValueHeader(I_TRUE, new UTL_ScopedName(yypvt[-1].idval, NULL), yypvt[-0].visval);
        } break;
case 59:
# line 948 "idl.yy"
{
           yyval.visval = new FE_ValueInheritanceSpec (I_FALSE, yypvt[-1].nlval, yypvt[-0].nlval);
        } break;
case 60:
# line 952 "idl.yy"
{
           yyval.visval = new FE_ValueInheritanceSpec (I_TRUE, yypvt[-1].nlval, yypvt[-0].nlval);
        } break;
case 61:
# line 956 "idl.yy"
{
           yyval.visval = new FE_ValueInheritanceSpec (I_TRUE, NULL, yypvt[-0].nlval);
        } break;
case 62:
# line 963 "idl.yy"
{
          yyval.nlval = new UTL_NameList(yypvt[-1].idlist, yypvt[-0].nlval);
        } break;
case 63:
# line 970 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ValueNameSeen);

          if (yypvt[-2].nlval == NULL)
          {
            yyval.nlval = new UTL_NameList(yypvt[-0].idlist, NULL);
          }
          else
          {
            yypvt[-2].nlval->nconc(new UTL_NameList(yypvt[-0].idlist, NULL));
            yyval.nlval = yypvt[-2].nlval;
          }
        } break;
case 64:
# line 984 "idl.yy"
{
          yyval.nlval = NULL;
        } break;
case 65:
# line 991 "idl.yy"
{
          yyval.nlval = yypvt[-0].nlval;
        } break;
case 66:
# line 995 "idl.yy"
{
          yyval.nlval = NULL;
        } break;
case 73:
# line 1017 "idl.yy"
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
           if (s != NULL && yypvt[-2].dcval != NULL && yypvt[-1].dlval != NULL)
           {
              l = new UTL_DecllistActiveIterator(yypvt[-1].dlval);
              for (;!(l->is_done()); l->next())
              {
                 d = l->item();
                 if (d == NULL)
                    continue;
                 AST_Type *tp = d->compose(yypvt[-2].dcval);
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
                    DDSString anon_type_name =
                       DDSString ("__") + DDSString (id->get_string ())
                          + DDSString (postfix);
                    UTL_ScopedName *anon_scoped_name =
                    new UTL_ScopedName
                    (
                       new Identifier (anon_type_name),
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

                 m = idl_global->gen()->create_state_member(yypvt[-3].bval, tp, d->name(), s->get_pragmas());
                 /*
                  * Add one attribute to the enclosing scope
                  */
                 (void) s->fe_add_state_member(m);
              }
              delete l;
           }
        } break;
case 74:
# line 1083 "idl.yy"
{
          yyval.bval = I_TRUE;
        } break;
case 75:
# line 1087 "idl.yy"
{
          yyval.bval = I_FALSE;
        } break;
case 76:
# line 1094 "idl.yy"
{
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_ScopedName        *n = new UTL_ScopedName(yypvt[-0].idval, NULL);
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
        } break;
case 77:
# line 1115 "idl.yy"
{
          idl_global->scopes()->pop();
        } break;
case 84:
# line 1134 "idl.yy"
{
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          AST_Argument          *a = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_InitParDeclSeen);
          /*
           * Create a node representing an argument to an initializer
           * Add it to the enclosing scope (the initializer scope)
           */
          if (yypvt[-1].dcval != NULL && yypvt[-0].deval != NULL && s != NULL) {
            AST_Type *tp = yypvt[-0].deval->compose(yypvt[-1].dcval);
            if (tp != NULL) {
              a = idl_global->gen()->create_argument(AST_Argument::dir_IN, tp, yypvt[-0].deval->name(), s->get_pragmas());
              (void) s->fe_add_argument(a);
            }
          }
        } break;
case 87:
# line 1160 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_TypeDeclSeen);
        } break;
case 88:
# line 1164 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 89:
# line 1168 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ConstDeclSeen);
        } break;
case 90:
# line 1172 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 91:
# line 1176 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptDeclSeen);
        } break;
case 92:
# line 1180 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 93:
# line 1184 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_AttrDeclSeen);
        } break;
case 94:
# line 1188 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 95:
# line 1192 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpDeclSeen);
        } break;
case 96:
# line 1196 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 97:
# line 1200 "idl.yy"
{
          idl_global->err()->syntax_error(idl_global->parse_state());
        } break;
case 98:
# line 1204 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
          yyerrok;
        } break;
case 100:
# line 1213 "idl.yy"
{
          yyval.nlval = new UTL_NameList(yypvt[-1].idlist, yypvt[-0].nlval);
        } break;
case 101:
# line 1221 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_SNListCommaSeen);
        } break;
case 102:
# line 1225 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ScopedNameSeen);

          if (yypvt[-3].nlval == NULL)
            yyval.nlval = new UTL_NameList(yypvt[-0].idlist, NULL);
          else {
            yypvt[-3].nlval->nconc(new UTL_NameList(yypvt[-0].idlist, NULL));
            yyval.nlval = yypvt[-3].nlval;
          }
        } break;
case 103:
# line 1236 "idl.yy"
{
          yyval.nlval = NULL;
        } break;
case 104:
# line 1243 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_SN_IDSeen);

          yyval.idlist = new UTL_IdList(yypvt[-0].idval, NULL);
        } break;
case 105:
# line 1249 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ScopeDelimSeen);
        } break;
case 106:
# line 1253 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_SN_IDSeen);

          yyval.idlist = new UTL_IdList(new Identifier(yypvt[-2].strval, 1, 0, I_FALSE),
                              new UTL_IdList(yypvt[-0].idval, NULL));
        } break;
case 107:
# line 1261 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ScopeDelimSeen);
        } break;
case 108:
# line 1265 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_SN_IDSeen);

          yypvt[-3].idlist->nconc(new UTL_IdList(yypvt[-0].idval, NULL));
          yyval.idlist = yypvt[-3].idlist;
        } break;
case 109:
# line 1274 "idl.yy"
{
            (void) idl_global->valid_identifier(yypvt[-0].strval);
            yyval.idval = new Identifier(yypvt[-0].strval, 1, 0, I_FALSE);
        } break;
case 110:
# line 1283 "idl.yy"
{
              UTL_Scope         *s = idl_global->scopes()->top_non_null();
              UTL_ScopedName    *n = new UTL_ScopedName(yypvt[-0].idval, NULL);
              AST_InterfaceFwd  *f = NULL;

              idl_global->set_parse_state(IDL_GlobalData::PS_ForwardDeclSeen);
              /*
               * Create a node representing a forward declaration of an
               * interface. Store it in the enclosing scope
               */
              if (s != NULL) {
                f = idl_global->gen()->create_interface_fwd(yypvt[-1].bval, n, s->get_pragmas());
                (void) s->fe_add_interface_fwd(f);
              }
        } break;
case 111:
# line 1302 "idl.yy"
{
          idl_global->set_parse_state (IDL_GlobalData::PS_ConstSeen);
        } break;
case 112:
# line 1306 "idl.yy"
{
          idl_global->set_parse_state (IDL_GlobalData::PS_ConstTypeSeen);
        } break;
case 113:
# line 1310 "idl.yy"
{
          idl_global->set_parse_state (IDL_GlobalData::PS_ConstIDSeen);
        } break;
case 114:
# line 1314 "idl.yy"
{
          idl_global->set_parse_state (IDL_GlobalData::PS_ConstAssignSeen);
        } break;
case 115:
# line 1318 "idl.yy"
{
          UTL_ScopedName *n = new UTL_ScopedName (yypvt[-4].idval, NULL);
          UTL_Scope      *s = idl_global->scopes()->top_non_null ();
          AST_Constant   *c = NULL;

          idl_global->set_parse_state (IDL_GlobalData::PS_ConstExprSeen);

          /*
           * Create a node representing a constant declaration. Store
           * it in the enclosing scope
           */

          if (yypvt[-0].exval != NULL && s != NULL)
          {
            if (yypvt[-0].exval->coerce (yypvt[-6].etval) == NULL)
            {
              idl_global->err()->coercion_error (yypvt[-0].exval, yypvt[-6].etval);
            }
            else
            {
              c = idl_global->gen()->create_constant
                 (yypvt[-6].etval, yypvt[-0].exval, n, s->get_pragmas ());
              (void) s->fe_add_constant (c);
            }
          }
        } break;
case 121:
# line 1353 "idl.yy"
{
          yyval.etval = AST_Expression::EV_string;
        } break;
case 122:
# line 1357 "idl.yy"
{
          yyval.etval = AST_Expression::EV_wstring;
        } break;
case 123:
# line 1361 "idl.yy"
{
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          AST_Decl              *d = NULL;
          AST_PredefinedType    *c = NULL;
          AST_Typedef           *t = NULL;

          /*
           * If the constant's type is a scoped name, it must resolve
           * to a scalar constant type
           */
          if (s != NULL && (d = s->lookup_by_name(yypvt[-0].idlist, I_TRUE)) != NULL) {
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
              yyval.etval = AST_Expression::EV_any;
            else if (d->node_type() == AST_Decl::NT_pre_defined) {
               c = AST_PredefinedType::narrow_from_decl(d);
               if (c != NULL) {
                  yyval.etval = idl_global->PredefinedTypeToExprType(c->pt());
               } else {
                  yyval.etval = AST_Expression::EV_any;
               }
            } else if (d->node_type() == AST_Decl::NT_string) {
               yyval.etval = AST_Expression::EV_string;
            } else
               yyval.etval = AST_Expression::EV_any;
          } else
             yyval.etval = AST_Expression::EV_any;
        } break;
case 127:
# line 1405 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_or, yypvt[-2].exval, yypvt[-0].exval);
        } break;
case 129:
# line 1413 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_xor, yypvt[-2].exval, yypvt[-0].exval);
        } break;
case 131:
# line 1421 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_and, yypvt[-2].exval, yypvt[-0].exval);
        } break;
case 133:
# line 1429 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_left,yypvt[-2].exval,yypvt[-0].exval);
        } break;
case 134:
# line 1433 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_right,yypvt[-2].exval,yypvt[-0].exval);
        } break;
case 136:
# line 1441 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_add, yypvt[-2].exval, yypvt[-0].exval);
        } break;
case 137:
# line 1445 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_minus,yypvt[-2].exval,yypvt[-0].exval);
        } break;
case 139:
# line 1453 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_mul, yypvt[-2].exval, yypvt[-0].exval);
        } break;
case 140:
# line 1457 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_div, yypvt[-2].exval, yypvt[-0].exval);
        } break;
case 141:
# line 1461 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_mod, yypvt[-2].exval, yypvt[-0].exval);
        } break;
case 143:
# line 1469 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_u_plus,
                                              yypvt[-0].exval,
                                              NULL);
        } break;
case 144:
# line 1475 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_u_minus,
                                              yypvt[-0].exval,
                                              NULL);
        } break;
case 145:
# line 1481 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_bit_neg,
                                              yypvt[-0].exval,
                                              NULL);
        } break;
case 146:
# line 1490 "idl.yy"
{
          /*
           * An expression which is a scoped name is not resolved now,
           * but only when it is evaluated (such as when it is assigned
           * as a constant value)
           */
          yyval.exval = idl_global->gen()->create_expr(yypvt[-0].idlist);
        } break;
case 148:
# line 1500 "idl.yy"
{
          yyval.exval = yypvt[-1].exval;
        } break;
case 149:
# line 1507 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr (yypvt[-0].ival);
        } break;
case 150:
# line 1511 "idl.yy"
{
          yyval.exval = yypvt[-0].exval;
        } break;
case 151:
# line 1515 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr (yypvt[-0].cval);
        } break;
case 152:
# line 1519 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr (atof (yypvt[-0].strval), yypvt[-0].strval);
        } break;
case 153:
# line 1523 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr((idl_bool) I_TRUE,
                                            AST_Expression::EV_bool);
        } break;
case 154:
# line 1528 "idl.yy"
{
          yyval.exval = idl_global->gen()->create_expr((idl_bool) I_FALSE,
                                            AST_Expression::EV_bool);
        } break;
case 155:
# line 1536 "idl.yy"
{
           int len = strlen(yypvt[-1].exval->ev()->u.strval->get_string());
           len += strlen(yypvt[-0].sval->get_string());
           char *combined = new char[len+1];
           combined[0] = '\0';
           os_strcat(combined, yypvt[-1].exval->ev()->u.strval->get_string());
           os_strcat(combined, yypvt[-0].sval->get_string());
           UTL_String *str = new UTL_String(combined);
           delete yypvt[-1].exval->ev()->u.strval;
           yypvt[-1].exval->ev()->u.strval = str;
           yyval.exval = yypvt[-1].exval;
        } break;
case 156:
# line 1549 "idl.yy"
{
           yyval.exval = idl_global->gen()->create_expr(yypvt[-0].sval);
        } break;
case 157:
# line 1556 "idl.yy"
{
            yypvt[-0].exval->evaluate(AST_Expression::EK_const);
            yyval.exval = idl_global->gen()->create_expr(yypvt[-0].exval, AST_Expression::EV_ulong);
        } break;
case 158:
# line 1564 "idl.yy"
{
            idl_global->set_parse_state(IDL_GlobalData::PS_TypedefSeen);
          } break;
case 163:
# line 1575 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_TypeSpecSeen);
        } break;
case 164:
# line 1579 "idl.yy"
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
          if (s != NULL && yypvt[-2].dcval != NULL && yypvt[-0].dlval != NULL) {
            l = new UTL_DecllistActiveIterator(yypvt[-0].dlval);
            for (;!(l->is_done()); l->next()) {
              d = l->item();
              if (d == NULL)
                continue;
              AST_Type * tp = d->compose(yypvt[-2].dcval);
              if (tp == NULL)
                continue;
              t = idl_global->gen()->create_typedef(tp, d->name(), s->get_pragmas());
              (void) s->fe_add_typedef(t);
            }
            delete l;
          }
        } break;
case 167:
# line 1614 "idl.yy"
{
          yyval.dcval = idl_global->scopes()->bottom()->lookup_primitive_type(yypvt[-0].etval);
        } break;
case 169:
# line 1619 "idl.yy"
{
          UTL_Scope     *s = idl_global->scopes()->top_non_null();
          AST_Decl      *d = NULL;

          if (s != NULL)
            d = s->lookup_by_name(yypvt[-0].idlist, I_TRUE);
          if (d == NULL || d->node_type() == AST_Decl::NT_field)
            idl_global->err()->lookup_error(yypvt[-0].idlist);
          yyval.dcval = d;
        } break;
case 182:
# line 1654 "idl.yy"
{
          yyval.dlval = new UTL_DeclList(yypvt[-1].deval, yypvt[-0].dlval);
        } break;
case 183:
# line 1661 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_DeclsCommaSeen);
        } break;
case 184:
# line 1665 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_DeclsDeclSeen);

          if (yypvt[-3].dlval == NULL)
            yyval.dlval = new UTL_DeclList(yypvt[-0].deval, NULL);
          else {
            yypvt[-3].dlval->nconc(new UTL_DeclList(yypvt[-0].deval, NULL));
            yyval.dlval = yypvt[-3].dlval;
          }
        } break;
case 185:
# line 1676 "idl.yy"
{
          yyval.dlval = NULL;
        } break;
case 188:
# line 1688 "idl.yy"
{
          yyval.deval = new FE_Declarator(new UTL_ScopedName(yypvt[-0].idval, NULL),
                                 FE_Declarator::FD_simple, NULL);
        } break;
case 189:
# line 1696 "idl.yy"
{
          yyval.deval = new FE_Declarator(new UTL_ScopedName(yypvt[-0].dcval->local_name(), NULL),
                                 FE_Declarator::FD_complex,
                                 yypvt[-0].dcval);
        } break;
case 192:
# line 1710 "idl.yy"
{
          yyval.etval = AST_Expression::EV_long;
        } break;
case 193:
# line 1714 "idl.yy"
{
          yyval.etval = AST_Expression::EV_longlong;
        } break;
case 194:
# line 1718 "idl.yy"
{
          yyval.etval = AST_Expression::EV_short;
        } break;
case 195:
# line 1725 "idl.yy"
{
          yyval.etval = AST_Expression::EV_ulong;
        } break;
case 196:
# line 1729 "idl.yy"
{
          yyval.etval = AST_Expression::EV_ulonglong;
        } break;
case 197:
# line 1733 "idl.yy"
{
          yyval.etval = AST_Expression::EV_ushort;
        } break;
case 198:
# line 1740 "idl.yy"
{
          yyval.etval = AST_Expression::EV_double;
        } break;
case 199:
# line 1744 "idl.yy"
{
          yyval.etval = AST_Expression::EV_float;
        } break;
case 200:
# line 1748 "idl.yy"
{
          yyval.etval = AST_Expression::EV_longdouble;
        } break;
case 201:
# line 1755 "idl.yy"
{
          yyval.etval = AST_Expression::EV_char;
        } break;
case 202:
# line 1759 "idl.yy"
{
          yyval.etval = AST_Expression::EV_wchar;
        } break;
case 203:
# line 1766 "idl.yy"
{
          yyval.etval = AST_Expression::EV_octet;
        } break;
case 204:
# line 1773 "idl.yy"
{
          yyval.etval = AST_Expression::EV_bool;
        } break;
case 205:
# line 1780 "idl.yy"
{
          yyval.etval = AST_Expression::EV_any;
        } break;
case 206:
# line 1787 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_StructSeen);
        } break;
case 207:
# line 1791 "idl.yy"
{
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_ScopedName        *n = new UTL_ScopedName(yypvt[-0].idval, NULL);
          AST_Structure         *d = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_StructIDSeen);
          /*
           * Create a node representing a struct declaration. Add it
           * to the enclosing scope
           */
          if (s != NULL) {
            d = idl_global->gen()->create_structure(n, s->get_pragmas());
            (void) s->fe_add_structure(d);
          }
          /*
           * Push the scope of the struct on the scopes stack
           */
          idl_global->scopes()->push(d);
          g_feScopeStack.Push
          (
             be_CppEnclosingScope
             (
                *n,
                be_CppEnclosingScope::NameIsScope()
             )
          );

        } break;
case 208:
# line 1820 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_StructSqSeen);
        } break;
case 209:
# line 1824 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_StructBodySeen);
        } break;
case 210:
# line 1828 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_StructQsSeen);
          /*
           * Done with this struct. Pop its scope off the scopes stack
           */
          if (idl_global->scopes()->top() == NULL)
            yyval.dcval = NULL;
          else {
            yyval.dcval =
              AST_Structure::narrow_from_scope(
                                   idl_global->scopes()->top_non_null());
            idl_global->scopes()->pop();
            g_feScopeStack.Pop();
          }
        } break;
case 214:
# line 1854 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_MemberTypeSeen);
        } break;
case 215:
# line 1858 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_MemberDeclsSeen);
        } break;
case 216:
# line 1862 "idl.yy"
{
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_DecllistActiveIterator *l = NULL;
          FE_Declarator         *d = NULL;
          AST_Field             *f = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_MemberDeclsCompleted);
          /*
           * Check for illegal recursive use of type
           */
          if (yypvt[-4].dcval != NULL && AST_illegal_recursive_type(yypvt[-4].dcval))
            idl_global->err()->error1(UTL_Error::EIDL_RECURSIVE_TYPE, yypvt[-4].dcval);
          /*
           * Create a node representing a struct or exception member
           * Add it to the enclosing scope
           */
          else if (s != NULL && yypvt[-4].dcval != NULL && yypvt[-2].dlval != NULL) {
            l = new UTL_DecllistActiveIterator(yypvt[-2].dlval);
            for (;!(l->is_done()); l->next()) {
              d = l->item();
              if (d == NULL)
                continue;
              AST_Type *tp = d->compose(yypvt[-4].dcval);
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
                 DDSString anon_type_name =
                    DDSString ("__") + DDSString (id->get_string ())
                       + DDSString (postfix);
                 UTL_ScopedName *anon_scoped_name =
                 new UTL_ScopedName
                 (
                    new Identifier (anon_type_name),
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
        } break;
case 217:
# line 1925 "idl.yy"
{
          idl_global->err()->syntax_error(idl_global->parse_state());
        } break;
case 218:
# line 1929 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
          yyerrok;
        } break;
case 219:
# line 1937 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionSeen);
        } break;
case 220:
# line 1941 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionIDSeen);
        } break;
case 221:
# line 1945 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_SwitchSeen);
        } break;
case 222:
# line 1949 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_SwitchOpenParSeen);
        } break;
case 223:
# line 1953 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_SwitchTypeSeen);
        } break;
case 224:
# line 1957 "idl.yy"
{
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_ScopedName        *n = new UTL_ScopedName(yypvt[-8].idval, NULL);
          AST_Union             *u = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_SwitchCloseParSeen);
          /*
           * Create a node representing a union. Add it to its enclosing
           * scope
           */
          if (yypvt[-2].dcval != NULL && s != NULL) {
            AST_ConcreteType    *tp = AST_ConcreteType::narrow_from_decl(yypvt[-2].dcval);
            if (tp == NULL) {
              idl_global->err()->not_a_type(yypvt[-2].dcval);
            } else {
              u = idl_global->gen()->create_union(tp, n, s->get_pragmas());
              (void) s->fe_add_union(u);
            }
          }
          /*
           * Push the scope of the union on the scopes stack
           */
          idl_global->scopes()->push(u);
          g_feScopeStack.Push
          (
             be_CppEnclosingScope
             (
                *n,
                be_CppEnclosingScope::NameIsScope()
             )
          );
        } break;
case 225:
# line 1990 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionSqSeen);
        } break;
case 226:
# line 1994 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionBodySeen);
        } break;
case 227:
# line 1998 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionQsSeen);
          /*
           * Done with this union. Pop its scope from the scopes stack
           */
          if (idl_global->scopes()->top() == NULL)
            yyval.dcval = NULL;
          else {
            yyval.dcval =
              AST_Union::narrow_from_scope(
                                idl_global->scopes()->top_non_null());
            idl_global->scopes()->pop();
            g_feScopeStack.Pop();
          }
        } break;
case 228:
# line 2017 "idl.yy"
{
          yyval.dcval = idl_global->scopes()->bottom()->lookup_primitive_type(yypvt[-0].etval);
        } break;
case 229:
# line 2021 "idl.yy"
{
          yyval.dcval = idl_global->scopes()->bottom()->lookup_primitive_type(yypvt[-0].etval);
        } break;
case 230:
# line 2025 "idl.yy"
{
          yyval.dcval = idl_global->scopes()->bottom()->lookup_primitive_type(yypvt[-0].etval);
        } break;
case 231:
# line 2029 "idl.yy"
{
          yyval.dcval = idl_global->scopes()->bottom()->lookup_primitive_type(yypvt[-0].etval);
        } break;
case 233:
# line 2034 "idl.yy"
{
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          AST_Decl              *d = NULL;
          AST_PredefinedType    *p = NULL;
          AST_Typedef           *t = NULL;
          long                  found = I_FALSE;

          /*
           * The discriminator is a scoped name. Try to resolve to
           * one of the scalar types or to an enum. Thread through
           * typedef's to arrive at the base type at the end of the
           * chain
           */
          if (s != NULL && (d = s->lookup_by_name(yypvt[-0].idlist, I_TRUE)) != NULL) {
            while (!found) {
              switch (d->node_type()) {
              case AST_Decl::NT_enum:
                yyval.dcval = d;
                found = I_TRUE;
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
                    yyval.dcval = p;
                    found = I_TRUE;
                    break;
                  default:
                    yyval.dcval = NULL;
                    found = I_TRUE;
                    break;
                  }
                }
                break;
              case AST_Decl::NT_typedef:
                t = AST_Typedef::narrow_from_decl(d);
                if (t != NULL) d = t->base_type();
                break;
              default:
                yyval.dcval = NULL;
                found = I_TRUE;
                break;
              }
            }
          } else
            yyval.dcval = NULL;

          if (yyval.dcval == NULL)
            idl_global->err()->lookup_error(yypvt[-0].idlist);
        } break;
case 237:
# line 2105 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionLabelSeen);
        } break;
case 238:
# line 2109 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemSeen);
        } break;
case 239:
# line 2113 "idl.yy"
{
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_LabellistActiveIterator *l = NULL;
          AST_UnionLabel        *d = NULL;
          AST_UnionBranch       *b = NULL;
          AST_Field             *f = yypvt[-2].ffval;

          idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemCompleted);
          /*
           * Create several nodes representing branches of a union.
           * Add them to the enclosing scope (the union scope)
           */
          if (s != NULL && yypvt[-4].llval != NULL && yypvt[-2].ffval != NULL) {
            l = new UTL_LabellistActiveIterator(yypvt[-4].llval);
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
        } break;
case 240:
# line 2141 "idl.yy"
{
          idl_global->err()->syntax_error(idl_global->parse_state());
        } break;
case 241:
# line 2145 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
          yyerrok;
        } break;
case 242:
# line 2153 "idl.yy"
{
          yyval.llval = new UTL_LabelList(yypvt[-1].ulval, yypvt[-0].llval);
        } break;
case 243:
# line 2160 "idl.yy"
{
          if (yypvt[-1].llval == NULL)
            yyval.llval = new UTL_LabelList(yypvt[-0].ulval, NULL);
          else {
            yypvt[-1].llval->nconc(new UTL_LabelList(yypvt[-0].ulval, NULL));
            yyval.llval = yypvt[-1].llval;
          }
        } break;
case 244:
# line 2169 "idl.yy"
{
          yyval.llval = NULL;
        } break;
case 245:
# line 2176 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_DefaultSeen);
        } break;
case 246:
# line 2180 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_LabelColonSeen);

          yyval.ulval = idl_global->gen()->
                    create_union_label(AST_UnionLabel::UL_default,
                                       NULL);
        } break;
case 247:
# line 2188 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_CaseSeen);
        } break;
case 248:
# line 2192 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_LabelExprSeen);
        } break;
case 249:
# line 2196 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_LabelColonSeen);

          yyval.ulval = idl_global->gen()->create_union_label(AST_UnionLabel::UL_label,
                                                     yypvt[-2].exval);
        } break;
case 250:
# line 2206 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemTypeSeen);
        } break;
case 251:
# line 2210 "idl.yy"
{
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemDeclSeen);
          /*
           * Check for illegal recursive use of type
           */
          if (yypvt[-2].dcval != NULL && AST_illegal_recursive_type(yypvt[-2].dcval))
            idl_global->err()->error1(UTL_Error::EIDL_RECURSIVE_TYPE, yypvt[-2].dcval);
          /*
           * Create a field in a union branch
           */
          else if (yypvt[-2].dcval == NULL || yypvt[-0].deval == NULL)
            yyval.ffval = NULL;
          else {
            AST_Type *tp = yypvt[-0].deval->compose(yypvt[-2].dcval);
            if (tp == NULL)
            {
              yyval.ffval = NULL;
            }
            else
            {
              // Check for anonymous type
              if ((tp->node_type () == AST_Decl::NT_array)
                  || (tp->node_type () == AST_Decl::NT_sequence))
              {
                 Identifier * id = yypvt[-0].deval->name ()->head ();
                 const char *postfix =
                    (tp->node_type () == AST_Decl::NT_array)
                    ? "" : "_seq";
                 // first underscore removed by Identifier constructor
                 DDSString anon_type_name =
                    DDSString ("__") + DDSString (id->get_string ())
                       + DDSString (postfix);
                 UTL_ScopedName *anon_scoped_name =
                 new UTL_ScopedName
                 (
                    new Identifier (anon_type_name),
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

              yyval.ffval = idl_global->gen()->create_field(tp,
                                                   yypvt[-0].deval->name(),
                                                   s->get_pragmas());
            }
          }
        } break;
case 252:
# line 2271 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_EnumSeen);
        } break;
case 253:
# line 2275 "idl.yy"
{
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_ScopedName        *n = new UTL_ScopedName(yypvt[-0].idval, NULL);
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
        } break;
case 254:
# line 2298 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_EnumSqSeen);
        } break;
case 255:
# line 2302 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_EnumBodySeen);
        } break;
case 256:
# line 2306 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_EnumQsSeen);
          /*
           * Done with this enum. Pop its scope from the scopes stack
           */
          if (idl_global->scopes()->top() == NULL)
            yyval.dcval = NULL;
          else {
            yyval.dcval = AST_Enum::narrow_from_scope(idl_global->scopes()->top_non_null());
            idl_global->scopes()->pop();
          }
        } break;
case 258:
# line 2325 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_EnumCommaSeen);
        } break;
case 261:
# line 2334 "idl.yy"
{
          if (idl_global->valid_identifier(yypvt[-0].strval))
          {
            UTL_Scope             *s = idl_global->scopes()->top_non_null();
            UTL_ScopedName        *n =
                  new UTL_ScopedName(new Identifier(yypvt[-0].strval, 1, 0, I_FALSE), NULL);
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
        } break;
case 262:
# line 2360 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceCommaSeen);
        } break;
case 263:
# line 2364 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceExprSeen);
        } break;
case 264:
# line 2368 "idl.yy"
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
          if (yypvt[-2].exval == NULL || yypvt[-2].exval->coerce(AST_Expression::EV_ulong) == NULL) {
            idl_global->err()->coercion_error(yypvt[-2].exval, AST_Expression::EV_ulong);
            yyval.dcval = NULL;
          } else if (yypvt[-5].dcval == NULL) {
            yyval.dcval = NULL;
          } else {
            AST_Type *tp = AST_Type::narrow_from_decl(yypvt[-5].dcval);
            if (tp == NULL)
              yyval.dcval = NULL;
            else {
              yyval.dcval = idl_global->gen()->create_sequence(yypvt[-2].exval, tp);
              /*
               * Add this AST_Sequence to the types defined in the global scope
               */
              (void) idl_global->root()
                        ->fe_add_sequence(AST_Sequence::narrow_from_decl(yyval.dcval));
            }
          }
        } break;
case 265:
# line 2399 "idl.yy"
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
          if (yypvt[-1].dcval == NULL)
            yyval.dcval = NULL;
          else {
            AST_Type *tp = AST_Type::narrow_from_decl(yypvt[-1].dcval);
            if (tp == NULL)
              yyval.dcval = NULL;
            else {
              yyval.dcval =
                idl_global->gen()->create_sequence(
                             idl_global->gen()->create_expr((unsigned long) 0),
                             tp);
              /*
               * Add this AST_Sequence to the types defined in the global scope
               */
              (void) idl_global->root()
                        ->fe_add_sequence(AST_Sequence::narrow_from_decl(yyval.dcval));
            }
          }
        } break;
case 266:
# line 2432 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceSeen);
          /*
           * Push a sequence marker on scopes stack
           */
          idl_global->scopes()->push(NULL);
        } break;
case 267:
# line 2440 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceSqSeen);
        } break;
case 268:
# line 2444 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceTypeSeen);
          yyval.dcval = yypvt[-0].dcval;
        } break;
case 269:
# line 2453 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_StringSqSeen);
        } break;
case 270:
# line 2457 "idl.yy"
{
           idl_global->set_parse_state(IDL_GlobalData::PS_StringExprSeen);
        } break;
case 271:
# line 2461 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_StringQsSeen);
          /*
           * Create a node representing a string
           */
          if (yypvt[-2].exval == NULL || yypvt[-2].exval->coerce(AST_Expression::EV_ulong) == NULL) {
            idl_global->err()->coercion_error(yypvt[-2].exval, AST_Expression::EV_ulong);
            yyval.dcval = NULL;
          } else {
            yyval.dcval = idl_global->gen()->create_string(yypvt[-2].exval);
            /*
             * Add this AST_String to the types defined in the global scope
             */
            (void) idl_global->root()
                      ->fe_add_string(AST_String::narrow_from_decl(yyval.dcval));
          }
        } break;
case 272:
# line 2479 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_StringCompleted);
          /*
           * Create a node representing a string
           */
          yyval.dcval =
            idl_global->gen()->create_string(
                         idl_global->gen()->create_expr((unsigned long) 0));
          /*
           * Add this AST_String to the types defined in the global scope
           */
          (void) idl_global->root()
                    ->fe_add_string(AST_String::narrow_from_decl(yyval.dcval));
        } break;
case 273:
# line 2497 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_StringSeen);
        } break;
case 274:
# line 2505 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_StringSqSeen);
        } break;
case 275:
# line 2509 "idl.yy"
{
           idl_global->set_parse_state(IDL_GlobalData::PS_StringExprSeen);
        } break;
case 276:
# line 2513 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_StringQsSeen);
          /*
           * Create a node representing a string
           */
          if (yypvt[-2].exval == NULL || yypvt[-2].exval->coerce(AST_Expression::EV_ulong) == NULL) {
            idl_global->err()->coercion_error(yypvt[-2].exval, AST_Expression::EV_ulong);
            yyval.dcval = NULL;
          } else {
            yyval.dcval = idl_global->gen()->create_wstring(yypvt[-2].exval);
            /*
             * Add this AST_String to the types defined in the global scope
             */
            (void) idl_global->root()
                      ->fe_add_string(AST_String::narrow_from_decl(yyval.dcval));
          }
        } break;
case 277:
# line 2531 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_StringCompleted);
          /*
           * Create a node representing a string
           */
          yyval.dcval =
            idl_global->gen()->create_wstring(
                         idl_global->gen()->create_expr((unsigned long) 0));
          /*
           * Add this AST_String to the types defined in the global scope
           */
          (void) idl_global->root()
                    ->fe_add_string(AST_String::narrow_from_decl(yyval.dcval));
        } break;
case 278:
# line 2549 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_StringSeen);
        } break;
case 279:
# line 2556 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ArrayIDSeen);
        } break;
case 280:
# line 2560 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ArrayCompleted);
          /*
           * Create a node representing an array
           */
          if (yypvt[-0].elval != NULL) {
             yyval.dcval = idl_global->gen()->create_array(new UTL_ScopedName(yypvt[-2].idval, NULL),
                                                  yypvt[-0].elval->length(), yypvt[-0].elval);
          }
        } break;
case 281:
# line 2574 "idl.yy"
{
          yyval.elval = new UTL_ExprList(yypvt[-1].exval, yypvt[-0].elval);
        } break;
case 282:
# line 2581 "idl.yy"
{
          if (yypvt[-1].elval == NULL)
            yyval.elval = new UTL_ExprList(yypvt[-0].exval, NULL);
          else {
            yypvt[-1].elval->nconc(new UTL_ExprList(yypvt[-0].exval, NULL));
            yyval.elval = yypvt[-1].elval;
          }
        } break;
case 283:
# line 2590 "idl.yy"
{
          yyval.elval = NULL;
        } break;
case 284:
# line 2597 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_DimSqSeen);
        } break;
case 285:
# line 2601 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_DimExprSeen);
        } break;
case 286:
# line 2605 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_DimQsSeen);
          /*
           * Array dimensions are expressions which must be coerced to
           * positive integers
           */
          if (yypvt[-2].exval == NULL || yypvt[-2].exval->coerce(AST_Expression::EV_ulong) == NULL) {
            idl_global->err()->coercion_error(yypvt[-2].exval, AST_Expression::EV_ulong);
            yyval.exval = NULL;
          } else
            yyval.exval = yypvt[-2].exval;
        } break;
case 287:
# line 2622 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_AttrSeen);
        } break;
case 288:
# line 2626 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_AttrTypeSeen);
        } break;
case 289:
# line 2630 "idl.yy"
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
          if (s != NULL && yypvt[-2].dcval != NULL && yypvt[-0].dlval != NULL) {
            l = new UTL_DecllistActiveIterator(yypvt[-0].dlval);
            for (;!(l->is_done()); l->next()) {
              d = l->item();
              if (d == NULL)
                continue;
              AST_Type *tp = d->compose(yypvt[-2].dcval);
              if (tp == NULL)
                continue;
              a = idl_global->gen()->create_attribute(yypvt[-5].bval, tp, d->name(), s->get_pragmas());
              /*
               * Add one attribute to the enclosing scope
               */
              (void) s->fe_add_attribute(a);
            }
            delete l;
          }
        } break;
case 290:
# line 2663 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_AttrROSeen);
          yyval.bval = I_TRUE;
        } break;
case 291:
# line 2668 "idl.yy"
{
          yyval.bval = I_FALSE;
        } break;
case 292:
# line 2675 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_LocalSeen);
          yyval.bval = I_TRUE;
        } break;
case 293:
# line 2680 "idl.yy"
{
          yyval.bval = I_FALSE;
        } break;
case 294:
# line 2687 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptSeen);
        } break;
case 295:
# line 2691 "idl.yy"
{
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_ScopedName        *n = new UTL_ScopedName(yypvt[-0].idval, NULL);
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
        } break;
case 296:
# line 2711 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptSqSeen);
        } break;
case 297:
# line 2715 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptBodySeen);
        } break;
case 298:
# line 2719 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptQsSeen);
          /*
           * Done with this exception. Pop its scope from the scope stack
           */
          idl_global->scopes()->pop();
        } break;
case 299:
# line 2731 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpTypeSeen);
        } break;
case 300:
# line 2735 "idl.yy"
{
          if (idl_global->valid_identifier(yypvt[-0].strval))
          {
            UTL_Scope             *s = idl_global->scopes()->top_non_null();
            UTL_ScopedName        *n =
                  new UTL_ScopedName(new Identifier(yypvt[-0].strval, 1, 0, I_FALSE), NULL);
            AST_Operation         *o = NULL;

            idl_global->set_parse_state(IDL_GlobalData::PS_OpIDSeen);
            /*
             * Create a node representing an operation on an interface
             * and add it to its enclosing scope
             */
            if (s != NULL && yypvt[-2].dcval != NULL) {
              AST_Type *tp = AST_Type::narrow_from_decl(yypvt[-2].dcval);
              if (tp == NULL) {
                idl_global->err()->not_a_type(yypvt[-2].dcval);
              } else if (tp->node_type() == AST_Decl::NT_except) {
                idl_global->err()->not_a_type(yypvt[-2].dcval);
              } else {
                o = idl_global->gen()->create_operation(tp, yypvt[-3].ofval, n, s->get_pragmas());
                (void) s->fe_add_operation(o);
              }
            }
            /*
             * Push the operation scope onto the scopes stack
             */
            idl_global->scopes()->push(o);
          }
        } break;
case 301:
# line 2766 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpParsCompleted);
        } break;
case 302:
# line 2770 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseCompleted);
        } break;
case 303:
# line 2774 "idl.yy"
{
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          AST_Operation         *o = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_OpCompleted);
          /*
           * Add exceptions and context to the operation
           */
          if (s != NULL && s->scope_node_type() == AST_Decl::NT_op) {
            o = AST_Operation::narrow_from_scope(s);

            if (yypvt[-2].nlval != NULL && o != NULL)
              (void) o->fe_add_exceptions(yypvt[-2].nlval);
            if (yypvt[-0].slval != NULL)
              (void) o->fe_add_context(yypvt[-0].slval);
          }
          /*
           * Done with this operation. Pop its scope from the scopes stack
           */
          idl_global->scopes()->pop();
        } break;
case 304:
# line 2799 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpAttrSeen);
          yyval.ofval = AST_Operation::OP_oneway;
        } break;
case 305:
# line 2804 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpAttrSeen);
          yyval.ofval = AST_Operation::OP_idempotent;
        } break;
case 306:
# line 2809 "idl.yy"
{
          yyval.ofval = AST_Operation::OP_noflags;
        } break;
case 308:
# line 2817 "idl.yy"
{
          yyval.dcval =
            idl_global->scopes()->bottom()
               ->lookup_primitive_type(AST_Expression::EV_void);
        } break;
case 309:
# line 2826 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpSqSeen);
        } break;
case 310:
# line 2830 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpQsSeen);
        } break;
case 311:
# line 2834 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpSqSeen);
        } break;
case 312:
# line 2839 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpQsSeen);
        } break;
case 314:
# line 2849 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpParCommaSeen);
        } break;
case 317:
# line 2858 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpParDirSeen);
        } break;
case 318:
# line 2862 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpParTypeSeen);
        } break;
case 319:
# line 2866 "idl.yy"
{
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          AST_Argument          *a = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_OpParDeclSeen);
          /*
           * Create a node representing an argument to an operation
           * Add it to the enclosing scope (the operation scope)
           */
          if (yypvt[-2].dcval != NULL && yypvt[-0].deval != NULL && s != NULL) {
            AST_Type *tp = yypvt[-0].deval->compose(yypvt[-2].dcval);
            if (tp != NULL) {
              a = idl_global->gen()->create_argument(yypvt[-4].dival, tp, yypvt[-0].deval->name(), s->get_pragmas());
              (void) s->fe_add_argument(a);
            }
          }
        } break;
case 320:
# line 2887 "idl.yy"
{
          yyval.dival = AST_Argument::dir_IN;
        } break;
case 321:
# line 2891 "idl.yy"
{
          yyval.dival = AST_Argument::dir_OUT;
        } break;
case 322:
# line 2895 "idl.yy"
{
          yyval.dival = AST_Argument::dir_INOUT;
        } break;
case 323:
# line 2902 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseSeen);
        } break;
case 324:
# line 2906 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseSqSeen);
        } break;
case 325:
# line 2911 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseQsSeen);
          yyval.nlval = yypvt[-1].nlval;
        } break;
case 326:
# line 2916 "idl.yy"
{
          yyval.nlval = NULL;
        } break;
case 327:
# line 2923 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpContextSeen);
        } break;
case 328:
# line 2927 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpContextSqSeen);
        } break;
case 329:
# line 2932 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpContextQsSeen);
          yyval.slval = yypvt[-1].slval;
        } break;
case 330:
# line 2937 "idl.yy"
{
          yyval.slval = NULL;
        } break;
case 331:
# line 2944 "idl.yy"
{
          yyval.slval = new UTL_StrList(yypvt[-1].sval, yypvt[-0].slval);
        } break;
case 332:
# line 2952 "idl.yy"
{
          idl_global->set_parse_state(IDL_GlobalData::PS_OpContextCommaSeen);
        } break;
case 333:
# line 2956 "idl.yy"
{
          if (yypvt[-3].slval == NULL)
            yyval.slval = new UTL_StrList(yypvt[-0].sval, NULL);
          else {
            yypvt[-3].slval->nconc(new UTL_StrList(yypvt[-0].sval, NULL));
            yyval.slval = yypvt[-3].slval;
          }
        } break;
case 334:
# line 2965 "idl.yy"
{
          yyval.slval = NULL;
        } break;
# line  531 "/usr/ccs/bin/yaccpar"
        }
        goto yystack;           /* reset registers in driver code */
}

