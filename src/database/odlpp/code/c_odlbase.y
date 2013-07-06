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
    SEMI LPAR RPAR MODULE DOUBLE_COLON COLON PERSISTENT TRANSIENT
    INTERFACE LRPAR RRPAR KEY KEYS COMMA _CONST EQUAL VERT HAT
    AMPER DOUBLE_RIGHT DOUBLE_LEFT PLUS MINUS TIMES SLASH PERCENT TILDE
    TOK_TRUE TOK_FALSE TYPEDEF FLOAT DOUBLE LONG SHORT UNSIGNED CHAR BOOLEAN ANY
    OCTET STRUCT UNION SWITCH CASE DEFAULT ENUM ARRAY SEQUENCE LEFT RIGHT
    ODL_STRING LEPAR REPAR READONLY ATTRIBUTE SET LIST BAG INVERSE IMPORT
    RELATIONSHIP ORDER_BY EXCEPTION ONEWAY ODL_VOID IN OUT INOUT RAISES
    CONTEXT CLASS DATE TIME EXTENDS INTERVAL TIMESTAMP DICTIONARY

%type <Value>
    FixedArraySize PositiveIntConst

%type <Object>
    PropertyName AttributeName
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
    PropertyList AttributeNameList InheritanceSpec
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
      TypeDcl SEMI
    | ConstDcl SEMI
    | ExceptDcl SEMI
    | Interface SEMI
    | Module SEMI
    | Class SEMI
    | IMPORT Identifier COLON Identifier SEMI
    ;

Class :
      ClassForward
    | ClassDcl
    ;

ClassForward :
      CLASS Identifier
        { c_metaDeclare(scope,$2,M_CLASS); }
    ;

ClassDcl:
      ClassHeader LPAR InterfaceBody RPAR
        { c_metaFinalize(c_object(scope));
          scope = scope->definedIn; }
    ;

ClassHeader:
      CLASS Identifier OptTypePropertyList
        { scope = c_metaObject(c_metaDeclare(scope,$2,M_CLASS));
          c_class(scope)->extends = NULL;
          c_class(scope)->keys = c_metaArray(scope,$3,M_UNDEFINED);
        }
    | CLASS Identifier EXTENDS ScopedName OptTypePropertyList
        { scope = c_metaObject(c_metaDeclare(scope,$2,M_CLASS));
          c_class(scope)->extends = c_class($4);
          c_class(scope)->keys = c_metaArray(scope,$5,M_UNDEFINED);
        }
    | CLASS Identifier COLON InheritanceSpec OptTypePropertyList
        { scope = c_metaObject(c_metaDeclare(scope,$2,M_CLASS));
          c_class(scope)->extends = NULL;
          c_class(scope)->keys = c_metaArray(scope,$5,M_UNDEFINED);
        }
    | CLASS Identifier EXTENDS ScopedName COLON InheritanceSpec OptTypePropertyList
        { scope = c_metaObject(c_metaDeclare(scope,$2,M_CLASS));
          c_class(scope)->extends = (c_class)c_metaResolve(scope,$4);
          c_class(scope)->keys = c_metaArray(scope,$7,M_UNDEFINED);
        }
    ;

Module:
      ModuleHeader Specification RPAR
        { scope = scope->definedIn; }
    ;

ModuleHeader:
      MODULE Identifier LPAR
        { scope = c_metaObject(c_metaDeclare(scope,$2,M_MODULE)); }
    ;

Interface:
      InterfaceDcl
    | ForwardDcl
    ;

InterfaceDcl:
      InterfaceHeader LPAR OptInterfaceBody RPAR
        { scope = scope->definedIn; }
    ;

InterfaceHeader:
      INTERFACE Identifier
        { scope = c_metaObject(c_metaDeclare(scope,$2,M_INTERFACE)); }
    | INTERFACE Identifier COLON InheritanceSpec
        { scope = c_metaObject(c_metaDeclare(scope,$2,M_INTERFACE)); }

    ;

ForwardDcl:
      INTERFACE Identifier
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
      LRPAR OptKeySpec RRPAR
        { $$ = $2; }
    ;

OptKeySpec:
      /* No key specifier */
        { $$ = NULL; }
    | KeySpec
        { $$ = $1; }
    ;

KeySpec:
      KEY KeyList
        { $$ = $2; }
    | KEYS KeyList
        { $$ = $2; }
    ;

KeyList:
       Key
        { $$ = $1; }
     | Key COMMA KeyList
       { $$ = c_iterConcat($1,$3); }
     ;

Key:
       PropertyName
        { $$ = c_iterNew($1); }
     | LRPAR PropertyList RRPAR
        { $$ = $2; }
     ;

PropertyList:
       PropertyName
        { $$ = c_iterNew($1); }
     | PropertyName COMMA PropertyList
        { $$ = c_iterInsert($3,$1); }
     ;

PropertyName:
       Identifier
        { $$ = c_stringNew(c_getBase(scope),$1); }
     ;

AttributeNameList:
       AttributeName
        { $$ = c_iterNew($1); }
     | AttributeName COMMA AttributeNameList
        { $$ = c_iterInsert($3,$1); }
     ;

AttributeName:
       Identifier
        { $$ = (void *)c_metaResolve(scope,$1); }
     ;

InterfaceBody:
      Export
    | Export InterfaceBody
    ;

Export:
      TypeDcl SEMI
    | ConstDcl SEMI
    | ExceptDcl SEMI
    | AttrDcl SEMI
    | RelDcl SEMI
    | OpDcl SEMI
    ;

InheritanceSpec:
      ScopedName
        { $$ = c_iterNew($1); }
    | ScopedName COMMA InheritanceSpec
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
    | DOUBLE_COLON Identifier
        { $$ = (void *)c_metaResolve(topLevel,$2);
          if ($$ == NULL) {
              yyerror("Undefined identifier");
              YYABORT;
          }
        }
    | ScopedName DOUBLE_COLON Identifier
        { $$ = (void *)c_metaResolve($1,$3);
          if ($$ == NULL) {
              yyerror("Undefined identifier");
              YYABORT;
          }
        }
    ;

ConstDcl:
      _CONST ConstType Identifier EQUAL ConstExp
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
    | OrExpr VERT XOrExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_OR;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    ;

XOrExpr:
      AndExpr
    | XOrExpr HAT AndExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_XOR;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    ;

AndExpr:
      ShiftExpr
    | AndExpr AMPER ShiftExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_AND;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    ;

ShiftExpr:
      AddExpr
    | ShiftExpr DOUBLE_RIGHT AddExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_SHIFTRIGHT;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    | ShiftExpr DOUBLE_LEFT AddExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_SHIFTLEFT;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    ;

AddExpr:
      MultExpr
    | AddExpr PLUS MultExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_PLUS;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    | AddExpr MINUS MultExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_MINUS;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    ;

MultExpr:
      UnaryExpr
    | MultExpr TIMES UnaryExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_MUL;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    | MultExpr SLASH UnaryExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_DIV;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    | MultExpr PERCENT UnaryExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_MOD;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
        }
    ;

UnaryExpr:
      MINUS PrimaryExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_MINUS;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,1);
          c_expression($$)->operands[0] = $2;
        }
    | PLUS PrimaryExpr
        { $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_PLUS;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,1);
          c_expression($$)->operands[0] = $2;
        }
    | TILDE PrimaryExpr
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
    | LRPAR ConstExp RRPAR
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
    | TOK_TRUE
        { $$ = c_metaDefine(scope,M_LITERAL);
          c_literal($$)->value = c_boolValue(TRUE); }
    | TOK_FALSE
        { $$ = c_metaDefine(scope,M_LITERAL);
          c_literal($$)->value = c_boolValue(FALSE); }
    ;

PositiveIntConst:
      ConstExp
        { $$ = c_operandValue($1); }
    ;

TypeDcl:
      TYPEDEF TypeSpec Declarators
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
    | DATE
        { support_warning("date type"); $$ = NULL; }
    | TIME
        { support_warning("time type"); $$ = NULL; }
    | INTERVAL
        { support_warning("interval type"); $$ = NULL;}
    | TIMESTAMP
        { support_warning("timestamp"); $$ = NULL; }
    ;

TemplateTypeSpec:
      ArrayType
    | SequenceType
    | StringType
    | CollType
    ;

CollType:
      SET LEFT SimpleTypeSpec RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = C_SET;
          c_collectionType($$)->subType = $3;
          c_collectionType($$)->maxSize = 0;
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    | LIST LEFT SimpleTypeSpec RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = C_LIST;
          c_collectionType($$)->subType = $3;
          c_collectionType($$)->maxSize = 0;
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    | BAG LEFT SimpleTypeSpec RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = C_BAG;
          c_collectionType($$)->subType = $3;
          c_collectionType($$)->maxSize = 0;
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    | DICTIONARY LEFT SimpleTypeSpec COMMA SimpleTypeSpec RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = C_DICTIONARY;
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
    | Declarator COMMA Declarators
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
      FLOAT
        { $$ = c_metaResolve(scope, "c_float"); }
    | DOUBLE
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
      SHORT
{ $$ = c_metaResolve(scope, "c_short"); }
    ;

SignedLongInt:
      LONG
        { $$ = c_metaResolve(scope, "c_long"); }
    ;

SignedLongLongInt:
      LONG LONG
        { $$ = c_metaResolve(scope, "c_longlong"); }
    ;

UnsignedInt:
      UnsignedShortInt
    | UnsignedLongInt
    | UnsignedLongLongInt
    ;

UnsignedShortInt:
      UNSIGNED SHORT
        { $$ = c_metaResolve(scope, "c_ushort"); }
    ;

UnsignedLongInt:
      UNSIGNED LONG
        { $$ = c_metaResolve(scope, "c_ulong"); }
    ;

UnsignedLongLongInt:
      UNSIGNED LONG LONG
        { $$ = c_metaResolve(scope, "c_ulonglong"); }
    ;

CharType:
      CHAR
        { $$ = c_metaResolve(scope, "c_char"); }
    ;

BooleanType:
      BOOLEAN
        { $$ = c_metaResolve(scope, "c_bool"); }
    ;

OctetType:
      OCTET
        { $$ = c_metaResolve(scope, "c_octet"); }
    ;

AnyType:
      ANY
        { $$ = c_metaResolve(scope, "c_any"); }
    ;

StructType:
      StructHeader LPAR MemberList RPAR
        { c_structure(scope)->members = c_metaArray(scope,$3,M_MEMBER);
          c_metaFinalize(scope);
          $$ = scope;
          scope = scope->definedIn;
        }
    ;

StructHeader:
    STRUCT Identifier
        { scope = c_metaDeclare(scope,$2,M_STRUCTURE); }
    ;

MemberList:
      Member
    | Member MemberList
        { $$ = c_iterConcat($1,$2); }
    ;

Member:
      TypeSpec Declarators SEMI
        { $$ = c_bindMembers(scope,$2,$1);
          c_metaDeclareMembers(scope, $$);
        }
    ;

UnionType:
       UnionHeader SWITCH LRPAR SwitchTypeSpec RRPAR LPAR SwitchBody RPAR
        { c_union(scope)->switchType = $4;
          c_union(scope)->cases = c_metaArray(scope,$7,M_UNIONCASE);
          c_metaFinalize(scope);
          $$ = scope;
          scope = scope->definedIn;
        }
    ;

UnionHeader:
    UNION Identifier
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
      CaseLabelList ElementSpec SEMI
        { $$ = $2;
          c_unionCase($$)->labels = c_metaArray(scope,$1,M_LITERAL);
        }
    ;
*/
Case:
      CaseLabelList TypeSpec Declarator SEMI
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
      CASE ConstExp COLON
        { $$ = c_operandValue($2); }
    | DEFAULT COLON
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
      ENUM Identifier LPAR EnumeratorList RPAR
        { $$ = c_metaDeclare(scope,$2,M_ENUMERATION);
          c_enumeration($$)->elements = c_metaArray(scope,$4,M_CONSTANT);
          c_metaFinalize($$);
        }
    ;

EnumeratorList:
      Enumerator
        { $$ = c_iterNew($1); }
    | Enumerator COMMA EnumeratorList
        { $$ = c_iterInsert($3,$1); }
    ;

Enumerator:
      Identifier
        { $$ = (void *)c_metaDeclare(scope,$1,M_CONSTANT); }
    ;

ArrayType:
      ARRAY LEFT SimpleTypeSpec COMMA PositiveIntConst RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = C_ARRAY;
          c_collectionType($$)->maxSize = $5->value.is.LongLong;
          c_collectionType($$)->subType = $3;
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    | ARRAY LEFT SimpleTypeSpec RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = C_ARRAY;
          c_collectionType($$)->maxSize = 0;
          c_collectionType($$)->subType = $3;
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    ;

SequenceType:
      SEQUENCE LEFT SimpleTypeSpec COMMA PositiveIntConst RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = C_SEQUENCE;
          c_collectionType($$)->maxSize = $5->value.is.LongLong;
          c_collectionType($$)->subType = $3;
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    | SEQUENCE LEFT SimpleTypeSpec RIGHT
        { $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = C_SEQUENCE;
          c_collectionType($$)->maxSize = 0;
          c_collectionType($$)->subType = $3;
          c_metaFinalize($$);
          $$ = c_metaDeclareCollection(scope, c_collectionType($$));
        }
    ;

StringType:
      ODL_STRING LEFT PositiveIntConst RIGHT
        { $$ = c_metaDefine(scope,M_COLLECTION);
          c_collectionType($$)->kind = C_STRING;
          c_collectionType($$)->maxSize = $3->value.is.LongLong;
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
      LEPAR PositiveIntConst REPAR
        { $$ = $2; }
    ;

/*************************************************************************
IDL-conformant correction to ODMG 2.0: ATTRIBUTE Type Name1, Name2, ...
*************************************************************************/

AttrDcl:
      READONLY ATTRIBUTE DomainType Declarators
        { c_iter attributes;
          attributes = c_bindAttributes(scope,$4,$3,TRUE);
          c_metaDeclareAttributes(scope, attributes);
          c_iterFree(attributes);
        }
    | ATTRIBUTE DomainType Declarators
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
      RELATIONSHIP RelCollectionType LEFT Identifier RIGHT Identifier INVERSE Identifier DOUBLE_COLON Identifier
    | RELATIONSHIP Identifier Identifier INVERSE Identifier DOUBLE_COLON Identifier
    ;

RelCollectionType:
      SET
    | LIST
    | BAG
    ;

ExceptDcl:
      EXCEPTION Identifier LPAR OptMemberList RPAR
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
    | ONEWAY
    ;

OpTypeSpec:
      SimpleTypeSpec
        { $$ = $1; }
    | ODL_VOID
        { $$ = P_VOID; }
    ;

ParameterDcls:
      LRPAR ParamDclList RRPAR
        { $$ = $2; }
    | LRPAR RRPAR
        { $$ = NULL; }
    ;

ParamDclList:
      ParamDcl
        { $$ = c_iterNew($1); }
    | ParamDcl COMMA ParamDclList
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
      IN
        { $$ = D_IN; }
    | OUT
        { $$ = D_OUT; }
    | INOUT
        { $$ = D_INOUT; }
    ;

RaisesExpr:
    | RAISES LRPAR ScopedNameList RRPAR
    ;

ScopedNameList:
      ScopedName
        { $$ = c_iterNew($1); }
    | ScopedName COMMA ScopedNameList
        { $$ = c_iterInsert($3,$1); }
    ;

ContextExpr:
    | CONTEXT LRPAR StringLiteralList RRPAR
    ;

SL:
      StringLiteral
        { $$ = $1; }
    ;

StringLiteralList:
      SL
        { $$ = c_iterNew($1); }
    | SL COMMA StringLiteralList
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
    long l;
    char *msg;

    parser_line = 1;
    parser_column = 0;
    parser_state = 0;

    yyin = fopen(fname, "r");

    if (yyin == NULL) {
        l = strlen(fname) + strlen("ODL parser error: opening file ") +1;
        msg = (char *)malloc(l);
        sprintf(msg, "ODL parser error: opening file %s", fname);
        free(msg);
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
    c_metaObject scope;

    OS_UNUSED_ARG(subType);
    return current;
/*
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
    c_type found;

    if (c_baseObject(type)->kind == M_COLLECTION) {
        c_metaDeclareIfArray(scope, c_collectionType(type)->subType);
        found = c_type(c_metaDeclareCollection(scope, c_collectionType(type)));
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
