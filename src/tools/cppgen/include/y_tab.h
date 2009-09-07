
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

#ifdef WIN32
#undef CONST
#undef VOID
#undef IN
#undef OUT
#undef OPAQUE
#endif

extern YYSTYPE yylval;
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
