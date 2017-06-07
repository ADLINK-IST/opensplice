%{
#include "c_typebase.h"
#include "c_metabase.h"
#include "c_iterator.h"
#include "c_collection.h"
#include "c_metafactory.h"
#include "c_typenames.h"
#include "c_module.h"

#ifdef _WIN32
#define YY_NO_UNISTD_H
#define YYMALLOC malloc
#define YYFREE free
#endif

#define yyin                 c_odlbase_yyin
#define yyout                c_odlbase_yyout
#define yyparse              c_odlbase_yyparse
#define yychar               c_odlbase_yychar
#define yyerror              c_odlbase_yyerror
#define yylex                c_odlbase_yylex
#define yylval               c_odlbase_yylval
#define yyrestart            c_odlbase_yyrestart
/*#define yywrap               c_odlbase_yywrap*/
#define yytext               c_odlbase_yytext
#define yynerrs              c_odlbase_yynerrs
#define yyleng               c_odlbase_yyleng
#define yy_scan_string       c_odlbase_yy_scan_string
#define yy_scan_buffer       c_odlbase_yy_scan_buffer
#define yy_init_buffer       c_odlbase_yy_init_buffer
#define yy_flush_buffer      c_odlbase_yy_flush_buffer
#define yy_switch_to_buffer  c_odlbase_yy_switch_to_buffer
#define yy_delete_buffer     c_odlbase_yy_delete_buffer
#define yy_create_buffer     c_odlbase_yy_create_buffer
#define yy_load_buffer_state c_odlbase_yy_load_buffer_state
#define yy_scan_bytes        c_odlbase_yy_scan_bytes

void *object;
void *topLevel;
c_metaObject scope;

#define P_OBJECT  (c_type)c_metaResolve(scope, "c_object")
#define P_INTEGER (c_type)c_metaResolve(scope, "c_longlong")
#define P_FLOAT   (c_type)c_metaResolve(scope, "c_double")
#define P_CHAR    (c_type)c_metaResolve(scope, "c_char")
#define P_BOOL    (c_type)c_metaResolve(scope, "c_bool")
#define P_STRING  (c_type)c_metaResolve(scope, "c_string")
#define P_FIXED   (c_type)c_metaResolve(scope, "c_fixed")
#define P_VOID    (c_type)c_metaResolve(scope, "c_void")

#define MAX_FILENAME_LENGTH 256
char file_name[MAX_FILENAME_LENGTH];

int parser_line = 1;
int parser_column = 0;
int parser_state = 0;
char typeName[256];

int yylex();
int yyerror( char* text );

static void support_warning(char *text);
c_metaObject getCollectionScope(c_metaObject current, c_metaObject subType);
c_collectionType c_metaDeclareCollection(c_metaObject scope, c_collectionType c);
void c_metaDeclareMembers(c_metaObject scope, c_iter members);
void c_metaDeclareTypeDefs(c_metaObject scope, c_iter typeDefs);
void c_metaDeclareAttributes(c_metaObject scope, c_iter attributes);
void c_metaDeclareType(c_metaObject scope, c_type type);

extern FILE *yyin;

%}

%union {
    c_longlong   Integer;
    c_string     String;
    c_char       Char;
    c_string     Fixed;
    c_double     Float;
    c_direction  Mode;
    c_literal    Value;
    c_iter       List;
    void        *Object;
};

%token <Integer>
    IntegerLiteral

%token <Float>
    FloatingPtLiteral

%token <Fixed>
    FixedPtLiteral

%token <String>
    Identifier
    StringLiteral

%token <Char>
    CharacterLiteral

%token
    ODL_SEMI ODL_LPAR ODL_RPAR ODL_MODULE ODL_DOUBLE_COLON ODL_COLON
    ODL_PERSISTENT ODL_TRANSIENT ODL_INTERFACE ODL_LRPAR ODL_RRPAR ODL_KEY
    ODL_KEYS ODL_COMMA ODL_CONST ODL_EQUAL ODL_VERT ODL_HAT ODL_AMPER
    ODL_DOUBLE_RIGHT ODL_DOUBLE_LEFT ODL_PLUS ODL_MINUS ODL_TIMES ODL_SLASH
    ODL_PERCENT ODL_TILDE ODL_TRUE ODL_FALSE ODL_TYPEDEF ODL_FLOAT ODL_DOUBLE
    ODL_LONG ODL_SHORT ODL_UNSIGNED ODL_CHAR ODL_BOOLEAN ODL_ANY ODL_OCTET
    ODL_STRUCT ODL_UNION ODL_SWITCH ODL_CASE ODL_DEFAULT ODL_ENUM ODL_ARRAY
    ODL_SEQUENCE ODL_LEFT ODL_RIGHT ODL_STRING ODL_LEPAR ODL_REPAR ODL_READONLY
    ODL_ATTRIBUTE ODL_SET ODL_LIST ODL_BAG ODL_INVERSE ODL_IMPORT
    ODL_RELATIONSHIP ODL_ORDER_BY ODL_EXCEPTION ODL_ONEWAY ODL_VOID ODL_IN
    ODL_OUT ODL_INOUT ODL_RAISES ODL_CONTEXT ODL_CLASS ODL_DATE ODL_TIME
    ODL_EXTENDS ODL_INTERVAL ODL_TIMESTAMP ODL_DICTIONARY ODL_ATOMIC ODL_WEAK

%type <Value>
    FixedArraySize PositiveIntConst

%type <Object>
    PropertyName
    ConstType TypeSpec SimpleTypeSpec BaseTypeSpec
    TemplateTypeSpec CollType ConstrTypeSpec
    IntegerType SignedInt SignedLongLongInt SignedLongInt
    SignedShortInt UnsignedInt UnsignedLongLongInt
    UnsignedLongInt UnsignedShortInt StructType UnionType
    FloatingPtType BooleanType OctetType CharType AnyType
    SwitchTypeSpec Case CaseLabel EnumType Enumerator SimpleDeclarator
    ArrayType SequenceType StringType ArrayDeclarator DomainType ComplexDeclarator
    OpTypeSpec ParameterDcls ParamDcl
    ScopedName /* ElementSpec */
    ConstExp OrExpr XOrExpr AndExpr ShiftExpr AddExpr MultExpr UnaryExpr
    PrimaryExpr Literal Declarator
    SL

%type <List>
    OptKeySpec KeySpec Key KeyList
    PropertyList InheritanceSpec
    Declarators Member MemberList SwitchBody CaseLabelList EnumeratorList
    ArraySizeList ParamDclList ScopedNameList
    OptMemberList TypePropertyList
    OptTypePropertyList ClassHeader StringLiteralList
    StructHeader UnionHeader

%type <Mode>
    ParamAttribute

%%

start:
      Specification
    ;

Specification:
      Definition
    | Definition Specification
    ;

Definition:
      TypeDcl ODL_SEMI
    | ConstDcl ODL_SEMI
    | ExceptDcl ODL_SEMI
    | Interface ODL_SEMI
    | Module ODL_SEMI
    | Class ODL_SEMI
    | ODL_IMPORT Identifier ODL_COLON Identifier ODL_SEMI
    ;

Class :
      ClassForward
    | ClassDcl
    ;

ClassForward :
      ODL_CLASS Identifier
        { c_metaDeclare(scope,$2,M_CLASS); }
    ;

ClassDcl:
      ClassHeader ODL_LPAR InterfaceBody ODL_RPAR
        { c_metaFinalize(c_object(scope));
          scope = scope->definedIn; }
    ;

ClassHeader:
      ODL_CLASS Identifier OptTypePropertyList
        { scope = c_metaObject(c_metaDeclare(scope,$2,M_CLASS));
          c_class(scope)->extends = NULL;
          c_class(scope)->keys = c_metaArray(scope,$3,M_UNDEFINED);
        }
    | ODL_CLASS Identifier ODL_EXTENDS ScopedName OptTypePropertyList
        { scope = c_metaObject(c_metaDeclare(scope,$2,M_CLASS));
          c_class(scope)->extends = c_class($4);
          c_class(scope)->keys = c_metaArray(scope,$5,M_UNDEFINED);
        }
    | ODL_CLASS Identifier ODL_COLON InheritanceSpec OptTypePropertyList
        { scope = c_metaObject(c_metaDeclare(scope,$2,M_CLASS));
          c_class(scope)->extends = NULL;
          c_class(scope)->keys = c_metaArray(scope,$5,M_UNDEFINED);
        }
    | ODL_CLASS Identifier ODL_EXTENDS ScopedName ODL_COLON InheritanceSpec OptTypePropertyList
        { scope = c_metaObject(c_metaDeclare(scope,$2,M_CLASS));
          c_class(scope)->extends = (c_class)c_metaResolve(scope,$4);
          c_class(scope)->keys = c_metaArray(scope,$7,M_UNDEFINED);
        }
    ;

Module:
      ModuleHeader Specification ODL_RPAR
        { scope = scope->definedIn; }
    ;

ModuleHeader:
      ODL_MODULE Identifier ODL_LPAR
        { scope = c_metaObject(c_metaDeclare(scope,$2,M_MODULE)); }
    ;

Interface:
      InterfaceDcl
    | ForwardDcl
    ;

InterfaceDcl:
      InterfaceHeader ODL_LPAR OptInterfaceBody ODL_RPAR
        { scope = scope->definedIn; }
    ;

InterfaceHeader:
      ODL_INTERFACE Identifier
        { scope = c_metaObject(c_metaDeclare(scope,$2,M_INTERFACE)); }
    | ODL_INTERFACE Identifier ODL_COLON InheritanceSpec
        { scope = c_metaObject(c_metaDeclare(scope,$2,M_INTERFACE)); }

    ;

ForwardDcl:
      ODL_INTERFACE Identifier
        { c_metaDeclare(scope,$2,M_INTERFACE); }
    ;

OptInterfaceBody:
    | InterfaceBody
    ;

OptTypePropertyList:
        { $$ = NULL; }
    | TypePropertyList
        { $$ = $1; }
    ;

TypePropertyList:
      ODL_LRPAR OptKeySpec ODL_RRPAR
        { $$ = $2; }
    ;

OptKeySpec:
      /* No key specifier */
        { $$ = NULL; }
    | KeySpec
        { $$ = $1; }
    ;

KeySpec:
      ODL_KEY KeyList
        { $$ = $2; }
    | ODL_KEYS KeyList
        { $$ = $2; }
    ;

KeyList:
       Key
        { $$ = $1; }
     | Key ODL_COMMA KeyList
       { $$ = c_iterConcat($1,$3); }
     ;

Key:
       PropertyName
        { $$ = c_iterNew($1); }
     | ODL_LRPAR PropertyList ODL_RRPAR
        { $$ = $2; }
     ;

PropertyList:
       PropertyName
        { $$ = c_iterNew($1); }
     | PropertyName ODL_COMMA PropertyList
        { $$ = c_iterInsert($3,$1); }
     ;

PropertyName:
       Identifier
        { $$ = c_stringNew(c_getBase(scope),$1); }
     ;

InterfaceBody:
      Export
    | Export InterfaceBody
    ;

Export:
      TypeDcl ODL_SEMI
    | ConstDcl ODL_SEMI
    | ExceptDcl ODL_SEMI
    | AttrDcl ODL_SEMI
    | RelDcl ODL_SEMI
    | OpDcl ODL_SEMI
    ;

InheritanceSpec:
      ScopedName
        { $$ = c_iterNew($1); }
    | ScopedName ODL_COMMA InheritanceSpec
        { $$ = c_iterInsert($3,$1); }
    ;

ScopedName:
      Identifier
        { $$ = (void *)c_metaResolve(scope,$1);
          if ($$ == NULL) {
              yyerror("Undefined identifier");
              YYABORT;
          }
        }
    | ODL_DOUBLE_COLON Identifier
        { $$ = (void *)c_metaResolve(topLevel,$2);
          if ($$ == NULL) {
              yyerror("Undefined identifier");
              YYABORT;
          }
        }
    | ScopedName ODL_DOUBLE_COLON Identifier
        { $$ = (void *)c_metaResolve($1,$3);
          if ($$ == NULL) {
              yyerror("Undefined identifier");
              YYABORT;
          }
        }
    ;

ConstDcl:
      ODL_CONST ConstType Identifier ODL_EQUAL ConstExp
        { object = c_metaDeclare(scope,$3,M_CONSTANT);
          c_constant(object)->type = $2;
          c_constant(object)->operand = $5;
          c_metaFinalize(object);
        }
    ;

ConstType:
      IntegerType
    | CharType
    | OctetType
    | BooleanType
    | FloatingPtType
    | StringType
    | ScopedName
    ;

ConstExp:
      OrExpr
    ;

OrExpr:
      XOrExpr
    | OrExpr ODL_VERT XOrExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_OR;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    ;

XOrExpr:
      AndExpr
    | XOrExpr ODL_HAT AndExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_XOR;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    ;

AndExpr:
      ShiftExpr
    | AndExpr ODL_AMPER ShiftExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_AND;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    ;

ShiftExpr:
      AddExpr
    | ShiftExpr ODL_DOUBLE_RIGHT AddExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_SHIFTRIGHT;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    | ShiftExpr ODL_DOUBLE_LEFT AddExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_SHIFTLEFT;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    ;

AddExpr:
      MultExpr
    | AddExpr ODL_PLUS MultExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_PLUS;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    | AddExpr ODL_MINUS MultExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_MINUS;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    ;

MultExpr:
      UnaryExpr
    | MultExpr ODL_TIMES UnaryExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_MUL;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    | MultExpr ODL_SLASH UnaryExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_DIV;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    | MultExpr ODL_PERCENT UnaryExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_MOD;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    ;

UnaryExpr:
      ODL_MINUS PrimaryExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_MINUS;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,1);
          c_expression($$)->operands[0] = $2;
        }
    | ODL_PLUS PrimaryExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_PLUS;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,1);
          c_expression($$)->operands[0] = $2;
        }
    | ODL_TILDE PrimaryExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_NOT;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,1);
          c_expression($$)->operands[0] = $2;
        }
    | PrimaryExpr
    ;

PrimaryExpr:
      ScopedName
    | Literal
    | ODL_LRPAR ConstExp ODL_RRPAR
        { $$ = $2; }
    ;

Literal:
      IntegerLiteral
        { $$ = c_metaDefine(scope,M_LITERAL);
          c_literal($$)->value = c_longlongValue($1); }
    | StringLiteral
        { $$ = c_metaDefine(scope,M_LITERAL);
          c_literal($$)->value = c_stringValue($1); }
    | CharacterLiteral
        { $$ = c_metaDefine(scope,M_LITERAL);
          c_literal($$)->value = c_charValue($1); }
    | FixedPtLiteral
        { $$ = c_metaDefine(scope,M_LITERAL);
          /* Fixed point not yet supported, translate to string */
          c_literal($$)->value = c_stringValue($1); }
    | FloatingPtLiteral
        { $$ = c_metaDefine(scope,M_LITERAL);
          c_literal($$)->value = c_doubleValue($1); }
    | ODL_TRUE
        { $$ = c_metaDefine(scope,M_LITERAL);
          c_literal($$)->value = c_boolValue(TRUE); }
    | ODL_FALSE
        { $$ = c_metaDefine(scope,M_LITERAL);
          c_literal($$)->value = c_boolValue(FALSE); }
    ;

PositiveIntConst:
      ConstExp
        { $$ = c_operandValue($1); }
    ;

TypeDcl:
      ODL_TYPEDEF TypeSpec Declarators
        { c_iter typeDefs;
          typeDefs = c_bindTypes(scope,$3,$2);
          c_metaDeclareTypeDefs(scope, typeDefs);
          c_iterFree(typeDefs);
        }
    | StructType { }
    | UnionType { }
    | EnumType { }
    ;

TypeSpec:
      SimpleTypeSpec
    | ConstrTypeSpec
    ;

SimpleTypeSpec:
      BaseTypeSpec
    | TemplateTypeSpec
    | ScopedName
    ;

BaseTypeSpec:
      FloatingPtType
    | IntegerType
    | CharType
    | BooleanType
    | OctetType
    | AnyType
    | ODL_DATE
        { support_warning("date type"); $$ = NULL; }
    | ODL_TIME
        { support_warning("time type"); $$ = NULL; }
    | ODL_INTERVAL
        { support_warning("interval type"); $$ = NULL;}
    | ODL_TIMESTAMP
        { support_warning("timestamp"); $$ = NULL; }
    ;

TemplateTypeSpec:
      ArrayType
    | SequenceType
    | StringType
    | CollType
    ;

CollType:
      ODL_SET ODL_LEFT SimpleTypeSpec ODL_RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = OSPL_C_SET;
          c_collectionType($$)->subType = $3;
          c_collectionType($$)->maxSize = 0;
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    | ODL_LIST ODL_LEFT SimpleTypeSpec ODL_RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = OSPL_C_LIST;
          c_collectionType($$)->subType = $3;
          c_collectionType($$)->maxSize = 0;
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    | ODL_BAG ODL_LEFT SimpleTypeSpec ODL_RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = OSPL_C_BAG;
          c_collectionType($$)->subType = $3;
          c_collectionType($$)->maxSize = 0;
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    | ODL_DICTIONARY ODL_LEFT SimpleTypeSpec ODL_COMMA SimpleTypeSpec ODL_RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = OSPL_C_DICTIONARY;
          c_collectionType($$)->subType = $3;
          c_collectionType($$)->maxSize = 0;
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    ;

ConstrTypeSpec:
      StructType
    | UnionType
    | EnumType
    ;

/***********************************************************************
 * The declarators is just a iter with elements of declarator          *
 ***********************************************************************/
Declarators:
      Declarator
        { $$ = c_iterNew($1); }
    | Declarator ODL_COMMA Declarators
        { $$ = c_iterInsert($3,$1); }
    ;

/***********************************************************************
 * A declarator is a iter starting with a string followed by zero or   *
 * more positive integers (spl_d_ulong).                               *
 ***********************************************************************/
Declarator:
      SimpleDeclarator
    | ComplexDeclarator
    ;

SimpleDeclarator:
      Identifier
        { $$ = c_declaratorNew($1,NULL); }
    ;

ComplexDeclarator:
      ArrayDeclarator
    ;

FloatingPtType:
      ODL_FLOAT
        { $$ = c_metaResolve(scope, "c_float"); }
    | ODL_DOUBLE
        { $$ = c_metaResolve(scope, "c_double"); }
    ;

IntegerType:
      SignedInt
    | UnsignedInt
    ;

SignedInt:
      SignedShortInt
    | SignedLongInt
    | SignedLongLongInt
    ;

SignedShortInt:
      ODL_SHORT
{ $$ = c_metaResolve(scope, "c_short"); }
    ;

SignedLongInt:
      ODL_LONG
        { $$ = c_metaResolve(scope, "c_long"); }
    ;

SignedLongLongInt:
      ODL_LONG ODL_LONG
        { $$ = c_metaResolve(scope, "c_longlong"); }
    ;

UnsignedInt:
      UnsignedShortInt
    | UnsignedLongInt
    | UnsignedLongLongInt
    ;

UnsignedShortInt:
      ODL_UNSIGNED ODL_SHORT
        { $$ = c_metaResolve(scope, "c_ushort"); }
    ;

UnsignedLongInt:
      ODL_UNSIGNED ODL_LONG
        { $$ = c_metaResolve(scope, "c_ulong"); }
    ;

UnsignedLongLongInt:
      ODL_UNSIGNED ODL_LONG ODL_LONG
        { $$ = c_metaResolve(scope, "c_ulonglong"); }
    ;

CharType:
      ODL_CHAR
        { $$ = c_metaResolve(scope, "c_char"); }
    ;

BooleanType:
      ODL_BOOLEAN
        { $$ = c_metaResolve(scope, "c_bool"); }
    ;

OctetType:
      ODL_OCTET
        { $$ = c_metaResolve(scope, "c_octet"); }
    ;

AnyType:
      ODL_ANY
        { $$ = c_metaResolve(scope, "c_any"); }
    ;

StructType:
      StructHeader ODL_LPAR MemberList ODL_RPAR
        { c_structure(scope)->members = c_metaArray(scope,$3,M_MEMBER);
          c_metaFinalize(scope);
          $$ = scope;
          scope = scope->definedIn;
        }
    ;

StructHeader:
    ODL_STRUCT Identifier
        { scope = c_metaDeclare(scope,$2,M_STRUCTURE); }
    ;

MemberList:
      Member
    | Member MemberList
        { $$ = c_iterConcat($1,$2); }
    ;

Member:
      TypeSpec Declarators ODL_SEMI
        { $$ = c_bindMembers(scope,$2,$1);
          c_metaDeclareMembers(scope, $$);
        }
    ;

UnionType:
       UnionHeader ODL_SWITCH ODL_LRPAR SwitchTypeSpec ODL_RRPAR ODL_LPAR SwitchBody ODL_RPAR
        { c_union(scope)->switchType = $4;
          c_union(scope)->cases = c_metaArray(scope,$7,M_UNIONCASE);
          c_metaFinalize(scope);
          $$ = scope;
          scope = scope->definedIn;
        }
    ;

UnionHeader:
    ODL_UNION Identifier
        { scope = c_metaDeclare(scope,$2,M_UNION); }
    ;

SwitchTypeSpec:
      IntegerType
    | CharType
    | BooleanType
    | EnumType
    | ScopedName
    ;

SwitchBody:
      Case
        { $$ = c_iterNew($1); }
    | SwitchBody Case
        { $$ = c_iterAppend($1,$2); }
    ;

/**
 * Case will always result a iter with the following layout:
 * - the first element is always the ElementSpec (read d_attribute)
 * - when no elements follow the first element is the default case,
 *   otherwise the remaining elements are caselabels
 *   (read d_constant)
 */
/*
Case:
      CaseLabelList ElementSpec ODL_SEMI
        { $$ = $2;
          c_unionCase($$)->labels = c_metaArray(scope,$1,M_LITERAL);
        }
    ;
*/
Case:
      CaseLabelList TypeSpec Declarator ODL_SEMI
        { $$ = c_unionCaseNew(scope, c_declaratorName($3), c_declaratorType($3,$2), $1);
          c_metaDeclareType(scope, $2);
        }
    ;

CaseLabelList:
      CaseLabel
        { $$ = c_iterNew($1); }
    | CaseLabel CaseLabelList
        { $$ = c_iterInsert($2,$1); }
    ;

CaseLabel:
      ODL_CASE ConstExp ODL_COLON
        { $$ = c_operandValue($2); }
    | ODL_DEFAULT ODL_COLON
        { $$ = NULL; }
    ;
/*
ElementSpec:
      TypeSpec Declarator
        { $$ = c_unionCaseNew(scope,$1,$2);
          c_metaDeclareType(scope, $$);
        }
    ;
*/

EnumType:
      ODL_ENUM Identifier ODL_LPAR EnumeratorList ODL_RPAR
        { $$ = c_metaDeclare(scope,$2,M_ENUMERATION);
          c_enumeration($$)->elements = c_metaArray(scope,$4,M_CONSTANT);
          c_metaFinalize($$);
        }
    ;

EnumeratorList:
      Enumerator
        { $$ = c_iterNew($1); }
    | Enumerator ODL_COMMA EnumeratorList
        { $$ = c_iterInsert($3,$1); }
    ;

Enumerator:
      Identifier
        { $$ = (void *)c_metaDeclare(scope,$1,M_CONSTANT); }
    | Identifier ODL_EQUAL ConstExp
        { $$ = (void *)c_metaDeclare(scope,$1,M_CONSTANT);
          c_constant($$)->operand = $3;
        }
    ;

ArrayType:
      ODL_ARRAY ODL_LEFT SimpleTypeSpec ODL_COMMA PositiveIntConst ODL_RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = OSPL_C_ARRAY;
          assert($5->value.is.LongLong <= C_MAX_LONG);
          c_collectionType($$)->maxSize = (c_ulong)$5->value.is.LongLong;
          c_collectionType($$)->subType = $3;
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    | ODL_ARRAY ODL_LEFT SimpleTypeSpec ODL_RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = OSPL_C_ARRAY;
          c_collectionType($$)->maxSize = 0;
          c_collectionType($$)->subType = $3;
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    ;

SequenceType:
      ODL_SEQUENCE ODL_LEFT SimpleTypeSpec ODL_COMMA PositiveIntConst ODL_RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = OSPL_C_SEQUENCE;
          assert($5->value.is.LongLong <= C_MAX_LONG);
          c_collectionType($$)->maxSize = (c_ulong)$5->value.is.LongLong;
          c_collectionType($$)->subType = $3;
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    | ODL_SEQUENCE ODL_LEFT SimpleTypeSpec ODL_RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = OSPL_C_SEQUENCE;
          c_collectionType($$)->maxSize = 0;
          c_collectionType($$)->subType = $3;
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    ;

StringType:
      ODL_STRING ODL_LEFT PositiveIntConst ODL_RIGHT
        { $$ = c_metaDefine(scope,M_COLLECTION);
          c_collectionType($$)->kind = OSPL_C_STRING;
          assert($3->value.is.LongLong <= C_MAX_LONG);
          c_collectionType($$)->maxSize = (c_ulong)$3->value.is.LongLong;
          c_collectionType($$)->subType = (c_type)c_metaResolve(scope,"c_char");
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    | ODL_STRING
        { $$ = P_STRING; }
    ;

ArrayDeclarator:
      Identifier ArraySizeList
        { $$ = c_declaratorNew($1,$2); }
    ;

ArraySizeList:
      FixedArraySize
        { $$ = c_iterNew($1); }
    | FixedArraySize ArraySizeList
        { $$ = c_iterInsert($2,$1); }
    ;

FixedArraySize:
      ODL_LEPAR PositiveIntConst ODL_REPAR
        { $$ = $2; }
    ;

/*************************************************************************
IDL-conformant correction to ODMG 2.0: ATTRIBUTE Type Name1, Name2, ...
*************************************************************************/

AttrDcl:
      ODL_READONLY ODL_ATTRIBUTE DomainType Declarators
        { c_iter attributes;
          attributes = c_bindAttributes(scope,$4,$3,TRUE);
          c_metaDeclareAttributes(scope, attributes);
          c_iterFree(attributes);
        }
    | ODL_ATTRIBUTE DomainType Declarators
        { c_iter attributes;
          attributes = c_bindAttributes(scope,$3,$2,FALSE);
          c_metaDeclareAttributes(scope, attributes);
          c_iterFree(attributes);
        }
    ;

DomainType:
      SimpleTypeSpec
    | StructType
    | EnumType
    ;

RelDcl:
      ODL_RELATIONSHIP RelCollectionType ODL_LEFT Identifier ODL_RIGHT Identifier ODL_INVERSE Identifier ODL_DOUBLE_COLON Identifier
    | ODL_RELATIONSHIP Identifier Identifier ODL_INVERSE Identifier ODL_DOUBLE_COLON Identifier
    ;

RelCollectionType:
      ODL_SET
    | ODL_LIST
    | ODL_BAG
    ;

ExceptDcl:
      ODL_EXCEPTION Identifier ODL_LPAR OptMemberList ODL_RPAR
        { object = c_metaDeclare(scope,$2,M_EXCEPTION);
          c_structure(object)->members = c_metaArray(scope,$4,M_MEMBER);
          c_metaFinalize(object);
        }
    ;

OptMemberList:
      /* No member iter */
        { $$ = NULL; }
    | MemberList
        { $$ = $1; }
    ;

OpDcl:
      OpAttribute OpTypeSpec Identifier ParameterDcls RaisesExpr ContextExpr
        { object = c_metaDeclare(scope,$3,M_OPERATION);
          if (object != NULL) {
              c_operation(object)->result = $2;
              c_operation(object)->parameters = c_metaArray(scope,$4,M_PARAMETER);
              c_metaFinalize(object);
          }
        }

    ;

OpAttribute:
    | ODL_ONEWAY
    ;

OpTypeSpec:
      SimpleTypeSpec
        { $$ = $1; }
    | ODL_VOID
        { $$ = P_VOID; }
    ;

ParameterDcls:
      ODL_LRPAR ParamDclList ODL_RRPAR
        { $$ = $2; }
    | ODL_LRPAR ODL_RRPAR
        { $$ = NULL; }
    ;

ParamDclList:
      ParamDcl
        { $$ = c_iterNew($1); }
    | ParamDcl ODL_COMMA ParamDclList
        { $$ = c_iterInsert($3,$1); }
    ;

ParamDcl:
      ParamAttribute SimpleTypeSpec Declarator
        { $$ = c_metaDefine(scope,M_PARAMETER);
          c_specifier($$)->name = $3;
          c_specifier($$)->type = $2;
          c_parameter($$)->mode = $1;
          c_metaDeclareType(scope, $2);
        }
    ;

ParamAttribute:
      ODL_IN
        { $$ = D_IN; }
    | ODL_OUT
        { $$ = D_OUT; }
    | ODL_INOUT
        { $$ = D_INOUT; }
    ;

RaisesExpr:
    | ODL_RAISES ODL_LRPAR ScopedNameList ODL_RRPAR
    ;

ScopedNameList:
      ScopedName
        { $$ = c_iterNew($1); }
    | ScopedName ODL_COMMA ScopedNameList
        { $$ = c_iterInsert($3,$1); }
    ;

ContextExpr:
    | ODL_CONTEXT ODL_LRPAR StringLiteralList ODL_RRPAR
    ;

SL:
      StringLiteral
        { $$ = $1; }
    ;

StringLiteralList:
      SL
        { $$ = c_iterNew($1); }
    | SL ODL_COMMA StringLiteralList
        { $$ = c_iterInsert($3,$1); }
    ;

%%
#include "c_odlbase.h"

/* option was: noyywrap */
/*
int yywrap()
{
  return (1);
}
*/

int
yyerror ( char *text )
{
    printf("*** error in file %s: %s near the token %s (line: %d, column: %d)\n",
           file_name, text, yytext, parser_line, parser_column);
    return -1;
}

static void support_warning(char *text)
{
    printf("warning not supported yet: %s at line %d column %d\n",
           text, parser_line, parser_column);
}

void
c_odlinit(c_module schema)
{
    topLevel = schema;
    scope = topLevel;
}

void
c_odlparse (
    const char *fname)
{
    parser_line = 1;
    parser_column = 0;
    parser_state = 0;

    yyin = fopen(fname, "r");

    if (yyin == NULL) {
        fprintf(stderr, "ODL parser error: opening file '%s'\n", fname);
        return;
    }

    yyparse();

    fclose(yyin);

    return;
}

c_metaObject
getCollectionScope(
    c_metaObject current,
    c_metaObject subType)
{
    OS_UNUSED_ARG(subType);
    return current;
/*
    c_metaObject scope;

    if (current == NULL) return NULL;
    scope = current;
    while ((c_baseObject(scope)->kind != M_MODULE) &&
           (scope != scope->definedIn)) {
        scope = scope->definedIn;
    }
    return scope;
*/
}

c_collectionType
c_metaDeclareCollection(
    c_metaObject scope,
    c_collectionType c)
{
    c_string name;
    c_char *collName;
    c_metaObject found;

    name = c_metaName(c_metaObject(c));
    if (!name) {
        collName = c_getScopedTypeName(scope, c_type(c), "::", C_SCOPE_ALWAYS);
        found = c_metaBind(scope, collName, c_metaObject(c));
        return c_collectionType(found);
    } else {
        c_free(name);
        return NULL;
    }
}


void
c_metaDeclareIfArray(
    c_metaObject scope,
    c_type type)
{
    if (c_baseObject(type)->kind == M_COLLECTION) {
        c_metaDeclareIfArray(scope, c_collectionType(type)->subType);
        c_metaDeclareCollection(scope, c_collectionType(type));
    }
}


void
c_metaDeclareMemberIfArray(
    c_member member,
    c_metaObject scope)
{
    c_metaDeclareIfArray(scope, c_specifier(member)->type);
}

void
c_metaDeclareMembers(
    c_metaObject scope,
    c_iter members)
{
    /* Members is a special case, iter has to be reused afterwards */
    /* Therefore use walk instead of take                          */
    c_iterWalk(members, (c_iterWalkAction)c_metaDeclareMemberIfArray, (void *)scope);
}

void
c_metaDeclareTypeDefs(
    c_metaObject scope,
    c_iter typeDefs)
{
    c_typeDef typeDef;

    typeDef = c_iterTakeFirst(typeDefs);
    while (typeDef != NULL) {
        c_metaDeclareIfArray(scope, typeDef->alias);
        c_free(typeDef);
        typeDef = c_iterTakeFirst(typeDefs);
    }
}

void
c_metaDeclareAttributes(
    c_metaObject scope,
    c_iter attributes)
{
    c_attribute attribute;

    attribute = c_iterTakeFirst(attributes);
    while (attribute != NULL) {
        c_metaDeclareIfArray(scope, c_property(attribute)->type);
        c_free(attribute);
        attribute = c_iterTakeFirst(attributes);
    }
}

void
c_metaDeclareType(
    c_metaObject scope,
    c_type type)
{
    c_metaDeclareIfArray(scope, type);
}
