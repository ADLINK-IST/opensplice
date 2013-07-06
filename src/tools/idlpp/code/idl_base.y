%{
#include <os_stdlib.h>
#include <os_heap.h>
#include <c_typebase.h>
#include <c_metabase.h>
#include <c_metafactory.h>
#include <c_iterator.h>
#include <c_collection.h>
#include "c_module.h"
#include "idl_fileMap.h"
#include "idl_keyDef.h"
#include "idl_catsDef.h"
#include "idl_stacDef.h"
#include "idl_streamsDef.h"
#include "idl_sematicRules.h"
#include "idl_unsupported.h"
#include "dds_cpp.h"

#ifdef _WIN32
#define YY_NO_UNISTD_H
#endif

#define yyin                 idl_base_yyin
#define yyout                idl_base_yyout
#define yyparse              idl_base_yyparse
#define yychar               idl_base_yychar
#define yyerror              idl_base_yyerror
#define yylex                idl_base_yylex
#define yylval               idl_base_yylval
#define yyrestart            idl_base_yyrestart
/*#define yywrap               idl_base_yywrap*/
#define yytext               idl_base_yytext
#define yynerrs              idl_base_yynerrs
#define yyleng               idl_base_yyleng
#define yy_scan_string       idl_base_yy_scan_string
#define yy_scan_buffer       idl_base_yy_scan_buffer
#define yy_init_buffer       idl_base_yy_init_buffer
#define yy_flush_buffer      idl_base_yy_flush_buffer
#define yy_switch_to_buffer  idl_base_yy_switch_to_buffer
#define yy_delete_buffer     idl_base_yy_delete_buffer
#define yy_create_buffer     idl_base_yy_create_buffer
#define yy_load_buffer_state idl_base_yy_load_buffer_state
#define yy_scan_bytes        idl_base_yy_scan_bytes

#define YY_INPUT(buf,result,max_size)\
  {\
    int c;\
    c = preprocess_getc();\
    if (c != EOF) {\
      buf[0] = c;\
      result = 1;\
    } else {\
      result = YY_NULL;\
    }\
  }

void *object;
void *topLevel;
c_metaObject scope;
c_metaObject scopeInternal;
c_metaObject annotationScope;

#define P_OBJECT  (c_type)metaResolve(scope, "c_object")
#define P_INTEGER (c_type)metaResolve(scope, "c_longlong")
#define P_FLOAT   (c_type)metaResolve(scope, "c_double")
/*#define P_CHAR    (c_type)metaResolve(scope, "c_char")*/
#define P_BOOL    (c_type)metaResolve(scope, "c_bool")
#define P_STRING  (c_type)metaResolve(scope, "c_string")
#define P_VOID    (NULL)

#define MAX_FILENAME_LENGTH 256
char file_name[MAX_FILENAME_LENGTH];

typedef enum c_scopeWhen {
    C_SCOPE_ALWAYS,
    C_SCOPE_NEVER,
    C_SCOPE_SMART
} c_scopeWhen;

int parser_line = 1;
int parser_column = 0;
int parser_state = 0;
int parser_error = 0;
char typeName[256];

int yylex();
int yyerror(char* text);

static void
support_warning(
    char *text);

static c_metaObject
getCollectionScope(
    c_metaObject current,
    c_metaObject subType);

static c_collectionType
declareCollection(
    c_metaObject scope,
    c_collectionType c);

static void
declareMembers(
    c_metaObject scope,
    c_iter members);

static void
declareTypeDefs(
    c_metaObject scope,
    c_iter typeDefs);

static void
declareAttributes(
    c_metaObject scope,
    c_iter attributes);

static void
declareType(
    c_metaObject scope,
    c_type type);

static c_metaObject
declareMetaObject(
    c_metaObject scope,
    c_char *name,
    c_metaKind kind);

static c_metaObject
declareUnsupportedType(
    c_metaObject scope,
    char *name,
    enum idl_unsupported_type unsupportedType);

static int
checkTypeDefs(
    c_metaObject scope,
    c_iter declarations);

static c_metaObject
metaResolve(
    c_metaObject scope,
    const char *name);

static c_metaObject
metaResolveAnnotation(
	c_metaObject scope,
	const char* name);

static c_result
metaFinalize(
    c_metaObject o);

static c_char *
getScopedTypeName(
    const c_metaObject scope,
    const c_type type,
    const char *separator,
    c_scopeWhen scopeWhen);

static c_char *
getScopedConstName(
    const c_metaObject scope,
    const c_constant c,
    const char *separator,
    c_scopeWhen scopeWhen);

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
    WideStringLiteral

%token <Char>
    CharacterLiteral
    WideCharacterLiteral

%token
    // A
    IDLTOK_AMPER IDLTOK_ANY IDLTOK_ATTRIBUTE IDLTOK_ABSTRACT IDLTOK_ANNOTATION IDLTOK_AT IDLTOK_AT_POST
    // B
    IDLTOK_BOOLEAN
    // C
    IDLTOK_COLON IDLTOK_CONST IDLTOK_CONTEXT IDLTOK_CASE
    IDLTOK_COMMA IDLTOK_CHAR IDLTOK_COMPONENT
    IDLTOK_CONSUMES IDLTOK_CUSTOM
    // D
    IDLTOK_DOUBLE_COLON IDLTOK_DOUBLE_LEFT
    IDLTOK_DOUBLE_RIGHT IDLTOK_DOUBLE IDLTOK_DEFAULT
    // E
    IDLTOK_EQUAL IDLTOK_ENUM IDLTOK_EXCEPTION IDLTOK_EMITS
    IDLTOK_EVENTTYPE
    // F
    IDLTOK_FLOAT IDLTOK_FALSE IDLTOK_FACTORY IDLTOK_FINDER
    IDLTOK_FIXED
    // G
    IDLTOK_GETRAISES
    // H
    IDLTOK_HAT IDLTOK_HOME
    // I
    IDLTOK_IMPORT IDLTOK_INTERFACE IDLTOK_IN IDLTOK_INOUT    // IDLTOK_INTERVAL IDLTOK_INVERSE
    // K
    // L
    IDLTOK_LONG IDLTOK_LPAR IDLTOK_LRPAR IDLTOK_LEPAR
    IDLTOK_LEFT IDLTOK_LIST IDLTOK_LOCAL
    // M
    IDLTOK_MINUS IDLTOK_MODULE IDLTOK_MULTIPLE IDLTOK_MANAGES IDLTOK_MAP
    // N
    IDLTOK_NATIVE
    // O
    IDLTOK_OCTET IDLTOK_ONEWAY IDLTOK_OUT IDLTOK_OBJECT
    // P
    IDLTOK_PLUS IDLTOK_PERCENT IDLTOK_PRIMARYKEY
    IDLTOK_PRIVATE IDLTOK_PROVIDES IDLTOK_PUBLIC
    IDLTOK_PUBLISHES                                         // IDLTOK_PERSISTENT
    // R
    IDLTOK_REPAR IDLTOK_RRPAR IDLTOK_RPAR IDLTOK_READONLY
    IDLTOK_RIGHT IDLTOK_RAISES
    // S
    IDLTOK_SEQUENCE IDLTOK_STRUCT IDLTOK_SWITCH IDLTOK_SHORT
    IDLTOK_SEMI IDLTOK_SLASH IDLTOK_STRING IDLTOK_SETRAISES
    IDLTOK_SUPPORTS
    // T
    IDLTOK_TIME IDLTOK_TIMES IDLTOK_TILDE IDLTOK_TIMESTAMP
    IDLTOK_TYPEDEF IDLTOK_TRUE IDLTOK_TRUNCATABLE
    IDLTOK_TYPEID IDLTOK_TYPEPREFIX                          // IDLTOK_TRANSIENT
    // U
    IDLTOK_UNION IDLTOK_UNSIGNED IDLTOK_USES
    // V
    IDLTOK_VOID IDLTOK_VALUEBASE IDLTOK_VALUETYPE
    IDLTOK_VERT
    // W
    IDLTOK_WCHAR IDLTOK_WSTRING

%type <Value>
    FixedArraySize PositiveIntConst

%type <Object>
    /* PropertyName AttributeName */
    AnnAttr_noAnn
    ConstType TypeSpec SimpleTypeSpec BaseTypeSpec ElementTypeSpec ElementTypeSpec_noAnnPost
    TemplateTypeSpec /* CollType */ ConstrTypeSpec
    IntegerType SignedInt SignedLongLongInt SignedLongInt
    SignedShortInt UnsignedInt UnsignedLongLongInt
    UnsignedLongInt UnsignedShortInt StructType UnionType UnionType_noAnn
    FloatingPtType BooleanType OctetType CharType AnyType
    SwitchTypeSpec SwitchTypeSpec_noAnnPost SwitchTypeName Case Case_noAnnPost CaseLabel EnumType Enumerator Enumerator_noAnn SimpleDeclarator
    SequenceType MapType StringType ArrayDeclarator /* DomainType */ ComplexDeclarator
    OpTypeSpec ParameterDcls ParamDcl
    ImportedScope ScopedName AnnotationName AnnotationScopedName /* ElementSpec */
    ConstExp OrExpr XOrExpr AndExpr ShiftExpr AddExpr MultExpr UnaryExpr
    PrimaryExpr Literal Declarator
    SL
    InterfaceName ValueName ParamTypeSpec WideStringType ComponentInheritanceSpec InterfaceType
    WideCharType FixedPtConstType ObjectType ValueBaseType FixedPtType
    ValueForwardDcl ValueBoxDcl ValueAbsDcl ValueHeader

%type <List>
    /* PropertyList AttributeNameList */ InheritanceSpec
    Declarators Member Member_noAnnPost Member_noAnn MemberList SwitchBody CaseLabelList EnumeratorList EnumeratorList_noAnnPost
    ArraySizeList ParamDclList ScopedNameList
    OptMemberList /* TypePropertyList */
    /* OptTypePropertyList */ StringLiteralList
    StructHeader StructHeader_noAnn UnionHeader
    InterfaceNameList ValueNameList

%type <Mode>
    ParamAttribute

%type <String>
	ArrayIdentifier ArrayIdentifier_noAnnPost

%%

start:
      Specification
    ;

Specification: // (1)
      Imports Definitions
    | Definitions
    ;

Definitions: // (1)
      Definition
    | Definition Definitions
    ;

Imports: // (1)
      Import
    | Import Imports
    ;

Import: // (100)
      IDLTOK_IMPORT ImportedScope IDLTOK_SEMI
    ;

ImportedScope: // (101)
      ScopedName
    | SL
    ;

Definition: // (2)
      TypeDcl IDLTOK_SEMI
    | TypeDcl IDLTOK_SEMI AnnApplPost
    | ConstDcl IDLTOK_SEMI
    | ExceptDcl IDLTOK_SEMI
    | Interface IDLTOK_SEMI
    | Module IDLTOK_SEMI
    | Value IDLTOK_SEMI
    | TypeIdDcl IDLTOK_SEMI
    | TypePrefixDcl IDLTOK_SEMI
    | Event IDLTOK_SEMI
    | Component IDLTOK_SEMI
    | HomeDcl IDLTOK_SEMI
    | Annotation IDLTOK_SEMI
    | Annotation IDLTOK_SEMI AnnApplPost
    ;

Annotation:
	  AnnDcl
	| AnnFwdDcl
	;

AnnDcl:
	  AnnHeader IDLTOK_LPAR AnnBody IDLTOK_RPAR
{ scope = scope->definedIn;}

AnnFwdDcl:
	  IDLTOK_ANNOTATION IDLTOK_LOCAL IDLTOK_INTERFACE Identifier
{
	scope = declareMetaObject(scope,$4,M_ENUMERATION);
}
    | IDLTOK_ANNOTATION IDLTOK_LRPAR IDLTOK_RRPAR IDLTOK_LOCAL IDLTOK_INTERFACE Identifier
{
	scope = declareMetaObject(scope,$6,M_ENUMERATION);
}


AnnHeader:
	  AnnFwdDcl
	| AnnFwdDcl AnnInheritanceSpec
	;

AnnBody:
	  AnnAttr
	| AnnBody AnnAttr
	;

AnnInheritanceSpec:
	  IDLTOK_COLON AnnotationName
	;


AnnotationScopedName: // AnnotationScopedName also looks in the ::<_ospl_internal> scope for built-in annotations
      Identifier
        {
          idl_checkReferencedIdentifier(scope,$1,yyerror);
          $$ = (void *)metaResolveAnnotation(scope,$1);
          if ($$ == NULL) {
            yyerror("Undefined identifier");
            YYABORT;
          }
        }
    | IDLTOK_DOUBLE_COLON Identifier
        {
          idl_checkReferencedIdentifier(topLevel,$2,yyerror);
          $$ = (void *)metaResolveAnnotation(topLevel,$2);
          if ($$ == NULL) {
            yyerror("Undefined identifier");
            YYABORT;
          }
        }
    | ScopedName IDLTOK_DOUBLE_COLON Identifier
        {
          idl_checkReferencedIdentifier($1,$3,yyerror);
          $$ = (void *)metaResolveAnnotation($1,$3);
          if ($$ == NULL) {
            yyerror("Undefined identifier");
            YYABORT;
          }
        }
    ;


AnnotationName:
	  AnnotationScopedName
	;

AnnAttr:
	  AnnAttr_noAnnPost
	| AnnAttr_noAnnPost AnnApplPost
	;

AnnAttr_noAnnPost:
	  AnnAttr_noAnn
	| AnnAppl AnnAttr_noAnn
	;

AnnAttr_noAnn:
	  IDLTOK_ATTRIBUTE ParamTypeSpec SimpleDeclarator IDLTOK_SEMI
{	$$ = c_metaDeclare(scope, c_declaratorName($3), M_ATTRIBUTE);}
	| IDLTOK_ATTRIBUTE ParamTypeSpec SimpleDeclarator IDLTOK_DEFAULT ConstExp IDLTOK_SEMI
{	$$ = c_metaDeclare(scope, c_declaratorName($3), M_ATTRIBUTE);}

AnnAppl:
	  IDLTOK_AT AnnApplDcl
	;

AnnApplPost:
      IDLTOK_AT_POST AnnApplDcl
	;

AnnotationDclName:
	  AnnotationName
{
	annotationScope = c_metaObject($1);
}

AnnApplDcl:
	  AnnotationName
	| AnnotationDclName IDLTOK_LRPAR IDLTOK_RRPAR {annotationScope = NULL;}
	| AnnotationDclName IDLTOK_LRPAR AnnApplParams IDLTOK_RRPAR {annotationScope = NULL;}
	;

AnnApplParams:
	  ConstExp
	| AnnApplParamsList
	;

AnnApplParamsList:
	  AnnApplParam
	| AnnApplParamsList IDLTOK_COMMA AnnApplParam
	;

AnnApplParam:
	  Identifier IDLTOK_EQUAL ConstExp
{
	c_metaObject o;
	if(!(o = c_metaResolve(annotationScope, $1))) {
		yyerror("Unresolved annotation parameter");
		YYABORT;
	}

	if(o->definedIn != annotationScope) {
		yyerror("Resolved parameter identifier not in annotation scope");
		YYABORT;
	}
}

HomeDcl: // (126)
      HomeHeader HomeBody
    ;

HomeHeader: // (127)
      IDLTOK_HOME Identifier HomeInheritanceSpec SupportedInterfaceSpec IDLTOK_MANAGES ScopedName PrimaryKeySpec
    ;

HomeInheritanceSpec: // (128)
      /* empty */
    | IDLTOK_COLON ScopedName
    ;

PrimaryKeySpec: // (129)
      /* empty */
    | IDLTOK_PRIMARYKEY ScopedName
    ;

HomeBody: // (130)
      IDLTOK_LPAR HomeExports IDLTOK_RPAR
    | IDLTOK_LPAR IDLTOK_RPAR
    ;

HomeExports: // (130)
      HomeExport
    | HomeExport HomeExports
    ;

HomeExport: // (131)
      Export
    | FactoryDcl
    | FinderDcl
    ;

FactoryDcl: // (132)
      IDLTOK_FACTORY Identifier IDLTOK_LRPAR InitParamDcls IDLTOK_RRPAR RaisesExpr
    | IDLTOK_FACTORY Identifier IDLTOK_LRPAR InitParamDcls IDLTOK_RRPAR
    | IDLTOK_FACTORY Identifier IDLTOK_LRPAR IDLTOK_RRPAR RaisesExpr
    | IDLTOK_FACTORY Identifier IDLTOK_LRPAR IDLTOK_RRPAR
    ;

FinderDcl: // (133)
      IDLTOK_FINDER Identifier IDLTOK_LRPAR InitParamDcls IDLTOK_RRPAR RaisesExpr
    | IDLTOK_FINDER Identifier IDLTOK_LRPAR InitParamDcls IDLTOK_RRPAR
    | IDLTOK_FINDER Identifier IDLTOK_LRPAR IDLTOK_RRPAR RaisesExpr
    | IDLTOK_FINDER Identifier IDLTOK_LRPAR IDLTOK_RRPAR
    ;

Component: // (112)
      ComponentDcl
    | ComponentForwardDcl
    ;

ComponentForwardDcl: // (113)
      IDLTOK_COMPONENT Identifier
    ;

ComponentDcl: // (114)
      ComponentHeader IDLTOK_LPAR ComponentBody IDLTOK_RPAR
    ;

ComponentHeader: // (115)
      IDLTOK_COMPONENT Identifier ComponentInheritanceSpec SupportedInterfaceSpec
    | IDLTOK_COMPONENT Identifier ComponentInheritanceSpec
    | IDLTOK_COMPONENT Identifier SupportedInterfaceSpec
    ;

SupportedInterfaceSpec: // (116)
      IDLTOK_SUPPORTS ScopedNameList
    ;

ComponentInheritanceSpec: // (117)
      IDLTOK_COLON ScopedName
        { $$ = NULL; }
    ;

ComponentBody: // (118)
      /* empty */
    | ComponentExportList
    ;

ComponentExportList: // (118)
      ComponentExport
    | ComponentExport ComponentExportList
    ;

ComponentExport: // (119)
      ProvidesDcl IDLTOK_SEMI
    | UsesDcl IDLTOK_SEMI
    | EmitsDcl IDLTOK_SEMI
    | PublishesDcl IDLTOK_SEMI
    | ConsumesDcl IDLTOK_SEMI
    | AttrDcl IDLTOK_SEMI
    ;

ProvidesDcl: // (120)
      IDLTOK_PROVIDES InterfaceType Identifier
    ;

InterfaceType: // (121)
      ScopedName
        { $$ = NULL; }
    | IDLTOK_OBJECT
        { $$ = NULL; }
    ;

UsesDcl: // (122)
      IDLTOK_USES IDLTOK_MULTIPLE InterfaceType Identifier
    | IDLTOK_USES InterfaceType Identifier
    ;

EmitsDcl: // (123)
      IDLTOK_EMITS ScopedName Identifier
    ;

PublishesDcl: // (124)
      IDLTOK_PUBLISHES ScopedName Identifier
    ;

ConsumesDcl: // (125)
      IDLTOK_CONSUMES ScopedName Identifier
    ;

Event: // (134)
      EventDcl
    | EventAbsDcl
    | EventForwardDcl
    ;

EventForwardDcl: // (135)
      IDLTOK_ABSTRACT IDLTOK_EVENTTYPE Identifier
    | IDLTOK_EVENTTYPE Identifier
    ;

EventAbsDcl: // (136)
      IDLTOK_ABSTRACT IDLTOK_EVENTTYPE Identifier ValueInheritanceSpec IDLTOK_LPAR Exports IDLTOK_RPAR
    | IDLTOK_ABSTRACT IDLTOK_EVENTTYPE Identifier IDLTOK_LPAR Exports IDLTOK_RPAR
    ;

EventDcl: // (137)
      EventHeader IDLTOK_LPAR ValueElements IDLTOK_RPAR
    ;

EventHeader: // (138)
      IDLTOK_CUSTOM IDLTOK_EVENTTYPE Identifier ValueInheritanceSpec
    | IDLTOK_CUSTOM IDLTOK_EVENTTYPE Identifier
    | IDLTOK_EVENTTYPE Identifier ValueInheritanceSpec
    | IDLTOK_EVENTTYPE Identifier
    ;

TypePrefixDcl:
      IDLTOK_TYPEPREFIX ScopedName StringLiteral
    ;

TypeIdDcl: // (102)
      IDLTOK_TYPEID ScopedName StringLiteral
    ;

Value: // (13)
      ValueDcl
    | ValueAbsDcl
    | ValueBoxDcl
    | ValueForwardDcl
    ;

ValueForwardDcl: // (14)
      IDLTOK_ABSTRACT IDLTOK_VALUETYPE Identifier
        {
          $$ = declareUnsupportedType(scope, $3, IDL_UNSUP_VALUETYPE);
        }
    | IDLTOK_VALUETYPE Identifier
        {
          $$ = declareUnsupportedType(scope, $2, IDL_UNSUP_VALUETYPE);
        }
    ;

ValueBoxDcl: // (15)
      IDLTOK_VALUETYPE Identifier TypeSpec
        {
          $$ = declareUnsupportedType(scope, $2, IDL_UNSUP_VALUETYPE);
        }
    ;

ValueAbsDcl: // (16)
      IDLTOK_ABSTRACT IDLTOK_VALUETYPE Identifier IDLTOK_LPAR Exports IDLTOK_RPAR
        { $$ = declareUnsupportedType(scope, $3, IDL_UNSUP_VALUETYPE); }
    | IDLTOK_ABSTRACT IDLTOK_VALUETYPE Identifier ValueInheritanceSpec IDLTOK_LPAR Exports IDLTOK_RPAR
        { $$ = declareUnsupportedType(scope, $3, IDL_UNSUP_VALUETYPE); }
    ;

ValueDcl: // (17)
      ValueHeader IDLTOK_LPAR ValueElements IDLTOK_RPAR
    | ValueHeader IDLTOK_LPAR IDLTOK_RPAR
    ;

ValueElements: // (17)
      ValueElement
    | ValueElement ValueElements
    ;

ValueHeader: // (18)
      IDLTOK_CUSTOM IDLTOK_VALUETYPE Identifier ValueInheritanceSpec
      { $$ = declareUnsupportedType(scope, $3, IDL_UNSUP_VALUETYPE); }
    | IDLTOK_CUSTOM IDLTOK_VALUETYPE Identifier
      { $$ = declareUnsupportedType(scope, $3, IDL_UNSUP_VALUETYPE); }
    | IDLTOK_VALUETYPE Identifier ValueInheritanceSpec
      { $$ = declareUnsupportedType(scope, $2, IDL_UNSUP_VALUETYPE); }
    | IDLTOK_VALUETYPE Identifier
      { $$ = declareUnsupportedType(scope, $2, IDL_UNSUP_VALUETYPE); }
    ;

ValueInheritanceSpec: // (19)
      IDLTOK_COLON IDLTOK_TRUNCATABLE ValueNameList
    | IDLTOK_COLON IDLTOK_TRUNCATABLE ValueNameList ValueInheritanceSpecSupports
    | IDLTOK_COLON ValueNameList
    | IDLTOK_COLON ValueNameList ValueInheritanceSpecSupports
    | ValueInheritanceSpecSupports
    ;

ValueInheritanceSpecSupports: // (19)
      IDLTOK_SUPPORTS InterfaceNameList
    ;

InterfaceNameList: // (19)
      InterfaceName
        { $$ = NULL; }
    | InterfaceName IDLTOK_COMMA InterfaceNameList
        { $$ = NULL; }
    ;

InterfaceName: // (11)
      ScopedName
    ;

ValueNameList: // (19)
      ValueName
        { $$ = NULL; }
    | ValueName IDLTOK_COMMA ValueNameList
        { $$ = NULL; }
    ;

ValueName: // (20)
      ScopedName
    ;

ValueElement: // (21)
      Export
    | StateMember
    | InitDcl
    ;

StateMember: // (22)
      StateAttribute TypeSpec Declarators IDLTOK_SEMI
    ;

StateAttribute: // (22)
      IDLTOK_PUBLIC
    | IDLTOK_PRIVATE
    ;

InitDcl: // (23)
      IDLTOK_FACTORY Identifier IDLTOK_LRPAR InitParamDcls IDLTOK_RRPAR RaisesExpr IDLTOK_SEMI
    | IDLTOK_FACTORY Identifier IDLTOK_LRPAR IDLTOK_RRPAR RaisesExpr IDLTOK_SEMI
    | IDLTOK_FACTORY Identifier IDLTOK_LRPAR InitParamDcls IDLTOK_RRPAR IDLTOK_SEMI
    | IDLTOK_FACTORY Identifier IDLTOK_LRPAR IDLTOK_RRPAR IDLTOK_SEMI
    ;

InitParamDcls: // (24)
      InitParamDcl
    | InitParamDcl IDLTOK_COMMA InitParamDcls
    ;

InitParamDcl: // (25)
      InitParamAttribute ParamTypeSpec SimpleDeclarator
    ;

InitParamAttribute: // (26)
      IDLTOK_IN
    ;

Exports: // (16)
      /* empty */
    | Export Exports
    ;

Module: // (3)
      ModuleHeader /* Specification */ Definitions IDLTOK_RPAR
        { scope = scope->definedIn; }
    ;

ModuleHeader: // (3)
      IDLTOK_MODULE Identifier IDLTOK_LPAR
        {
          if (idl_checkModuleDefinition (scope, $2,yyerror)) {
            YYABORT;
          }
          scope = c_metaObject(declareMetaObject(scope,$2,M_MODULE));
          idl_fileMapAssociation(idl_fileMapDefGet(), c_object(scope), file_name);
        }
    ;

Interface: // (4)
      InterfaceDcl
    | ForwardDcl
    ;

InterfaceDcl: // (5)
      InterfaceHeader IDLTOK_LPAR OptInterfaceBody IDLTOK_RPAR
        { scope = scope->definedIn; }
    ;

InterfaceHeader: // (7)
      IDLTOK_INTERFACE Identifier
        {
          if (idl_checkInterfaceDefinition(scope, $2, yyerror)) {
            YYABORT;
          }
          scope = c_metaObject(declareMetaObject(scope,$2,M_INTERFACE));
          idl_fileMapAssociation(idl_fileMapDefGet(), c_object(scope), file_name);
        }
    | IDLTOK_INTERFACE Identifier IDLTOK_COLON InheritanceSpec
        {
          if (idl_checkInterfaceDefinition(scope, $2, yyerror)) {
            YYABORT;
          }
          scope = c_metaObject(declareMetaObject(scope,$2,M_INTERFACE));
          idl_fileMapAssociation(idl_fileMapDefGet(), c_object(scope), file_name);
        }
    | InterfaceAttribute IDLTOK_INTERFACE Identifier
        {
          if (idl_checkInterfaceDefinition(scope, $3, yyerror)) {
            YYABORT;
          }
          scope = c_metaObject(declareMetaObject(scope,$3,M_INTERFACE));
          idl_fileMapAssociation(idl_fileMapDefGet(), c_object(scope), file_name);
        }
    | InterfaceAttribute IDLTOK_INTERFACE Identifier IDLTOK_COLON InheritanceSpec
        {
          if (idl_checkInterfaceDefinition(scope, $3, yyerror)) {
            YYABORT;
          }
          scope = c_metaObject(declareMetaObject(scope,$3,M_INTERFACE));
          idl_fileMapAssociation(idl_fileMapDefGet(), c_object(scope), file_name);
        }
    ;

ForwardDcl: // (6)
      IDLTOK_INTERFACE Identifier
        {
          if (idl_checkInterfaceDefinition(scope, $2, yyerror)) {
              YYABORT;
          }
          idl_fileMapAssociation(idl_fileMapDefGet(),
                                 c_object(declareMetaObject(scope,$2,M_INTERFACE)),
                                 file_name);
        }
    | InterfaceAttribute IDLTOK_INTERFACE Identifier
	{
          if (idl_checkInterfaceDefinition(scope, $3, yyerror)) {
              YYABORT;
          }
          idl_fileMapAssociation(idl_fileMapDefGet(),
                                 c_object(declareMetaObject(scope,$3,M_INTERFACE)),
                                 file_name);
        }
    ;

InterfaceAttribute: // (6)(7)
      IDLTOK_ABSTRACT
    | IDLTOK_LOCAL
    ;

OptInterfaceBody: // (5)
    | InterfaceBody
    ;

InterfaceBody: // (8)
      Export
    | Export InterfaceBody
    ;

Export: // (9)
      TypeDcl IDLTOK_SEMI
    | ConstDcl IDLTOK_SEMI
    | ExceptDcl IDLTOK_SEMI
    | AttrDcl IDLTOK_SEMI
    | OpDcl IDLTOK_SEMI
    | TypeIdDcl IDLTOK_SEMI
    | TypePrefixDcl IDLTOK_SEMI
    ;

InheritanceSpec: // (10) ":" is defined by caller
      ScopedName
        { $$ = c_iterNew($1); }
    | ScopedName IDLTOK_COMMA InheritanceSpec
        { $$ = c_iterInsert($3,$1); }
    ;

ScopedName: // (12)
      Identifier
        {
          idl_checkReferencedIdentifier(scope,$1,yyerror);
          $$ = (void *)metaResolve(scope,$1);
          if ($$ == NULL) {
            yyerror("Undefined identifier");
            YYABORT;
          }
        }
    | IDLTOK_DOUBLE_COLON Identifier
        {
          idl_checkReferencedIdentifier(topLevel,$2,yyerror);
          $$ = (void *)metaResolve(topLevel,$2);
          if ($$ == NULL) {
            yyerror("Undefined identifier");
            YYABORT;
          }
        }
    | ScopedName IDLTOK_DOUBLE_COLON Identifier
        {
          idl_checkReferencedIdentifier($1,$3,yyerror);
          $$ = (void *)metaResolve($1,$3);
          if ($$ == NULL) {
            yyerror("Undefined identifier");
            YYABORT;
          }
        }
    ;

ConstDcl: // (27)
      IDLTOK_CONST ConstType Identifier IDLTOK_EQUAL ConstExp
        {
          if (idl_checkConstantDefinition(scope,$3,yyerror)) {
	        YYABORT;
          }
          object = declareMetaObject(scope,$3,M_CONSTANT);
          c_constant(object)->type = $2;
          c_constant(object)->operand = $5;
          metaFinalize(object);
          idl_checkConstantDeclaration(scope,$2,object,yyerror);
          idl_fileMapAssociation(idl_fileMapDefGet(), object, file_name);
        }
    ;

ConstType: // (28)
      IntegerType
    | CharType
    | WideCharType
    | OctetType
    | BooleanType
    | FloatingPtType
    | StringType
    | WideStringType
    | FixedPtConstType
    | ScopedName
    ;

ConstExp: // (29)
      OrExpr
    ;

OrExpr: // (30)
      XOrExpr
    | OrExpr IDLTOK_VERT XOrExpr
        {
          $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_OR;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    ;

XOrExpr: // (31)
      AndExpr
    | XOrExpr IDLTOK_HAT AndExpr
        {
          $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_XOR;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    ;

AndExpr: // (32)
      ShiftExpr
    | AndExpr IDLTOK_AMPER ShiftExpr
        {
          $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_AND;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    ;

ShiftExpr: // (33)
      AddExpr
    | ShiftExpr IDLTOK_DOUBLE_RIGHT AddExpr
      {
        $$ = c_metaDefine(scope,M_EXPRESSION);
        c_expression($$)->kind = E_SHIFTRIGHT;
        c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
        c_expression($$)->operands[0] = $1;
        c_expression($$)->operands[1] = $3;
        idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
      }
    | ShiftExpr IDLTOK_DOUBLE_LEFT AddExpr
      {
        $$ = c_metaDefine(scope,M_EXPRESSION);
        c_expression($$)->kind = E_SHIFTLEFT;
        c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
        c_expression($$)->operands[0] = $1;
        c_expression($$)->operands[1] = $3;
        idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
      }
    ;

AddExpr: // (34)
      MultExpr
    | AddExpr IDLTOK_PLUS MultExpr
      {
        $$ = c_metaDefine(scope,M_EXPRESSION);
        c_expression($$)->kind = E_PLUS;
        c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
        c_expression($$)->operands[0] = $1;
        c_expression($$)->operands[1] = $3;
        idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
      }
    | AddExpr IDLTOK_MINUS MultExpr
      {
        $$ = c_metaDefine(scope,M_EXPRESSION);
        c_expression($$)->kind = E_MINUS;
        c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
        c_expression($$)->operands[0] = $1;
        c_expression($$)->operands[1] = $3;
        idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
      }
    ;

MultExpr: // (35)
      UnaryExpr
    | MultExpr IDLTOK_TIMES UnaryExpr
        {
          $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_MUL;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    | MultExpr IDLTOK_SLASH UnaryExpr
        {
          $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_DIV;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    | MultExpr IDLTOK_PERCENT UnaryExpr
        {
          $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_MOD;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,2);
          c_expression($$)->operands[0] = $1;
          c_expression($$)->operands[1] = $3;
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    ;

UnaryExpr: // (36)(37)
      IDLTOK_MINUS PrimaryExpr
        {
          $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_MINUS;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,1);
          c_expression($$)->operands[0] = $2;
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    | IDLTOK_PLUS PrimaryExpr
        {
          $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_PLUS;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,1);
          c_expression($$)->operands[0] = $2;
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    | IDLTOK_TILDE PrimaryExpr
        {
          $$ = c_metaDefine(scope,M_EXPRESSION);
          c_expression($$)->kind = E_NOT;
          c_expression($$)->operands = c_arrayNew(P_OBJECT,1);
          c_expression($$)->operands[0] = $2;
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    | PrimaryExpr
    ;

PrimaryExpr: // (38)
      ScopedName
    | Literal
    | IDLTOK_LRPAR ConstExp IDLTOK_RRPAR
        { $$ = $2; }
    ;

Literal: // (39)(40)
      IntegerLiteral
        {
          $$ = c_metaDefine(scope,M_LITERAL);
          c_literal($$)->value = c_longlongValue($1);
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    | StringLiteral
        {
          $$ = c_metaDefine(scope,M_LITERAL);
          c_literal($$)->value = c_stringValue($1);
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    | WideStringLiteral
        { support_warning("wstring literal"); $$ = NULL; }
    | CharacterLiteral
        {
          $$ = c_metaDefine(scope,M_LITERAL);
          c_literal($$)->value = c_charValue($1);
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    | WideCharacterLiteral
        { support_warning("wchar type"); $$ = NULL; }
    | FixedPtLiteral
        {
          $$ = c_metaDefine(scope,M_LITERAL);
          /* Fixed point not yet supported, translate to string */
          c_literal($$)->value = c_stringValue($1);
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    | FloatingPtLiteral
        {
          $$ = c_metaDefine(scope,M_LITERAL);
          c_literal($$)->value = c_doubleValue($1);
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    | IDLTOK_TRUE
        {
          $$ = c_metaDefine(scope,M_LITERAL);
          c_literal($$)->value = c_boolValue(TRUE);
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    | IDLTOK_FALSE
        {
          $$ = c_metaDefine(scope,M_LITERAL);
          c_literal($$)->value = c_boolValue(FALSE);
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    ;

PositiveIntConst: // (41)
      ConstExp
        {
          if (idl_checkPositiveInt(scope,$1,yyerror)) {
            YYABORT;
          }
          $$ = c_operandValue($1);
        }
    ;

TypeDcl: // (42)(43)
      IDLTOK_TYPEDEF TypeSpec Declarators
        {
          c_iter typeDefs;
          if (idl_checkTypeReference (scope,$2,yyerror)) {
            YYABORT;
          }
          if (checkTypeDefs(scope,$3)) {
              YYABORT;
          }
          typeDefs = c_bindTypes(scope,$3,$2);
          declareTypeDefs(scope, typeDefs);
        }
    | StructType { }
    | UnionType { }
    | EnumType { }
    | IDLTOK_NATIVE SimpleDeclarator
    | ConstrForwardDcl
    ;

ConstrForwardDcl: // (99)
      IDLTOK_STRUCT Identifier
      {
      	idl_fileMapAssociation(idl_fileMapDefGet(), c_baseObject(declareMetaObject(scope, $2, M_STRUCTURE)), file_name);
      }
    | IDLTOK_UNION Identifier
      {
      	idl_fileMapAssociation(idl_fileMapDefGet(), c_baseObject(declareMetaObject(scope, $2, M_STRUCTURE)), file_name);
      }
    ;

TypeSpec: // (44)
      SimpleTypeSpec
    | ConstrTypeSpec
    ;

ParamTypeSpec: // (95)
      BaseTypeSpec
    | StringType
    | WideStringType
    | ScopedName
    ;

SimpleTypeSpec: // (45)
      BaseTypeSpec
    | TemplateTypeSpec
    | ScopedName
    ;

BaseTypeSpec: // (46)
      FloatingPtType
    | IntegerType
    | CharType
    | WideCharType
    | BooleanType
    | OctetType
    | AnyType
    | ObjectType
    | ValueBaseType
    ;

TemplateTypeSpec: // (47)
    SequenceType
    | MapType
    | StringType
    | WideStringType
    | FixedPtType
    ;

ConstrTypeSpec: // (48)
      StructType
    | UnionType
    | EnumType
    ;

/***********************************************************************
 * The declarators is just a iter with elements of declarator          *
 ***********************************************************************/
Declarators: // (49)
      Declarator
        { $$ = c_iterNew($1); }
    | Declarator IDLTOK_COMMA Declarators
        { $$ = c_iterInsert($3,$1); }
    ;

/***********************************************************************
 * A declarator is a iter starting with a string followed by zero or   *
 * more positive integers (spl_d_ulong).                               *
 ***********************************************************************/
Declarator: // (50)
      SimpleDeclarator
    | ComplexDeclarator
    ;

SimpleDeclarator: // (51)
      Identifier
        { $$ = c_declaratorNew($1,NULL); }
    ;

ComplexDeclarator: // (52)
      ArrayDeclarator
    ;

FloatingPtType: // (53)
      IDLTOK_FLOAT
        { $$ = metaResolve(scope, "c_float"); }
    | IDLTOK_DOUBLE
        { $$ = metaResolve(scope, "c_double"); }
	| IDLTOK_LONG IDLTOK_DOUBLE
        { $$ = idl_unsupportedTypeGetMetadata(IDL_UNSUP_LONGDOUBLE); }
    ;

IntegerType: // (54)
      SignedInt
    | UnsignedInt
    ;

SignedInt: // (55)
      SignedShortInt
    | SignedLongInt
    | SignedLongLongInt
    ;

SignedShortInt: // (56)
      IDLTOK_SHORT
        { $$ = metaResolve(scope, "c_short"); }
    ;

SignedLongInt: // (57)
      IDLTOK_LONG
        { $$ = metaResolve(scope, "c_long"); }
    ;

SignedLongLongInt: // (58)
      IDLTOK_LONG IDLTOK_LONG
        { $$ = metaResolve(scope, "c_longlong"); }
    ;

UnsignedInt: // (59)
      UnsignedShortInt
    | UnsignedLongInt
    | UnsignedLongLongInt
    ;

UnsignedShortInt: // (60)
      IDLTOK_UNSIGNED IDLTOK_SHORT
        { $$ = metaResolve(scope, "c_ushort"); }
    ;

UnsignedLongInt: // (61)
      IDLTOK_UNSIGNED IDLTOK_LONG
        { $$ = metaResolve(scope, "c_ulong"); }
    ;

UnsignedLongLongInt: // (62)
      IDLTOK_UNSIGNED IDLTOK_LONG IDLTOK_LONG
        { $$ = metaResolve(scope, "c_ulonglong"); }
    ;

CharType: // (63)
      IDLTOK_CHAR
        { $$ = metaResolve(scope, "c_char"); }
    ;

WideCharType: // (64)
      IDLTOK_WCHAR
        { $$ = idl_unsupportedTypeGetMetadata(IDL_UNSUP_WCHAR); }
    ;

BooleanType: // (65)
      IDLTOK_BOOLEAN
        { $$ = metaResolve(scope, "c_bool"); }
    ;

OctetType: // (66)
      IDLTOK_OCTET
        { $$ = metaResolve(scope, "c_octet"); }
    ;

AnyType: // (67)
      IDLTOK_ANY
        { $$ = idl_unsupportedTypeGetMetadata(IDL_UNSUP_ANY); }
    ;

ObjectType: // (68)
      IDLTOK_OBJECT
        { $$ = idl_unsupportedTypeGetMetadata(IDL_UNSUP_OBJECT); }
    ;

ValueBaseType: // (98)
      IDLTOK_VALUEBASE
        { $$ = idl_unsupportedTypeGetMetadata(IDL_UNSUP_VALUEBASE); }
    ;

FixedPtType: // (96)
      IDLTOK_FIXED IDLTOK_LEFT PositiveIntConst IDLTOK_COMMA PositiveIntConst IDLTOK_RIGHT
        { $$ = idl_unsupportedTypeGetMetadata(IDL_UNSUP_FIXED); }
    ;

StructType: // (69)
      StructHeader IDLTOK_LPAR MemberList IDLTOK_RPAR
        {
          if (idl_checkStructDeclaratorDefinition(scope,$3,yyerror)) {
            YYABORT;
          }
          c_structure(scope)->members = c_metaArray(scope,$3,M_MEMBER);
          if (metaFinalize(scope) == S_ILLEGALRECURSION) {
              yyerror("Illegal recursion in struct");
              YYABORT;
          }
          $$ = scope;
          scope = scope->definedIn;
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    ;

StructHeader:
	  StructHeader_noAnn
	| AnnAppl StructHeader_noAnn {$$ = $2;}
	;

StructHeader_noAnn: // (69)
      IDLTOK_STRUCT Identifier
        {
          if (idl_checkTypeDefinition(scope,$2,yyerror)) {
            YYABORT;
          }
          scope = declareMetaObject(scope,$2,M_STRUCTURE);
        }
    | IDLTOK_STRUCT Identifier IDLTOK_COLON ScopedName
        {
        	yyerror("Structure inheritance will be supported in a future version.");
        	YYABORT;
        }

MemberList: // (70)
      Member
    | Member MemberList
        { $$ = c_iterConcat($1,$2); }
    ;

Member:
	  Member_noAnnPost
	| Member_noAnnPost AnnApplPost
	;

Member_noAnnPost:
	  Member_noAnn
	| AnnAppl Member_noAnn {$$ = $2;}
	;

Member_noAnn: // (71)
      TypeSpec Declarators IDLTOK_SEMI
        {
        	if(c_isFinal($1)) {
          		$$ = c_bindMembers(scope,$2,$1);
          		declareMembers(scope, $$);
          	}else {
          		yyerror("Illegal use of incomplete type (see 3.11.2.3 in OMG IDL specification).");
          		YYABORT;
          	}
        }
    ;

UnionType:
	  UnionType_noAnn
	| AnnAppl UnionType_noAnn {$$ = $2;}
	;

UnionType_noAnn: // (72)
      UnionHeader IDLTOK_SWITCH IDLTOK_LRPAR SwitchTypeSpec IDLTOK_RRPAR IDLTOK_LPAR SwitchBody IDLTOK_RPAR
        {
          if (idl_checkUnionDeclaratorDefinition(scope,$4,$7,yyerror)) {
            YYABORT;
          }
          idl_checkUnionCaseDefinition(scope,$4,$7,yyerror);
          c_union(scope)->cases = c_metaArray(scope,$7,M_UNIONCASE);
          if (metaFinalize(scope) == S_ILLEGALRECURSION) {
             yyerror("Illegal recursion in union");
             YYABORT;
          }
          $$ = scope;
          scope = scope->definedIn;
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    ;

UnionHeader: // (72)
      IDLTOK_UNION Identifier
        {
          if (idl_checkTypeDefinition (scope,$2,yyerror)) {
            YYABORT;
          }
          scope = declareMetaObject(scope,$2,M_UNION);
        }
    ;

SwitchTypeName: // (73)
      IntegerType
        { c_union(scope)->switchType = $1; }
    | CharType
        { c_union(scope)->switchType = $1; }
    | WideCharType
        { c_union(scope)->switchType = $1; }
    | BooleanType
        { c_union(scope)->switchType = $1; }
    | EnumType
        { c_union(scope)->switchType = $1; }
    | OctetType
        { c_union(scope)->switchType = $1; }
    | ScopedName
        { c_union(scope)->switchType = $1; }
    ;

SwitchTypeSpec_noAnnPost:
	  SwitchTypeName
	| AnnAppl SwitchTypeName { $$ = $2; }
	;

SwitchTypeSpec:
	  SwitchTypeSpec_noAnnPost
	| SwitchTypeSpec_noAnnPost AnnApplPost
	;

SwitchBody: // (74)
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
      CaseLabelList ElementSpec IDLTOK_SEMI
        {
          $$ = $2;
          c_unionCase($$)->labels = c_metaArray(scope,$1,M_LITERAL);
        }
    ;
*/
Case:
	  Case_noAnnPost
	| Case_noAnnPost AnnApplPost
	;

Case_noAnnPost: // (75)(77)
      CaseLabelList TypeSpec Declarator IDLTOK_SEMI
        {
          $$ = c_unionCaseNew(scope, c_declaratorName($3), c_declaratorType($3,$2), $1);
          declareType(scope, $2);
        }
    ;

CaseLabelList: // (76)
      CaseLabel
        { $$ = c_iterNew($1); }
    | CaseLabel CaseLabelList
        { $$ = c_iterInsert($2,$1); }
    ;

CaseLabel: // (76)
      IDLTOK_CASE // Lookup expression in scope of switchType
      		{$<Object>$ = scope; scope = c_metaObject(c_union(scope)->switchType)->definedIn;} 
      		ConstExp
      		{scope = $<Object>2;} 
      		IDLTOK_COLON
        {
          idl_checkConstantOperand (scope, c_union(scope)->switchType,c_operand($3),yyerror);
          $$ = c_operandValue($3);
        }
    | IDLTOK_DEFAULT IDLTOK_COLON
        { $$ = NULL; }
    ;
/*
ElementSpec:
      TypeSpec Declarator
  	{ $$ = c_UnionCaseNew(scope,$1,$2);
          declareType(scope, $$);
        }
    ;
*/

EnumType: // (78)
      IDLTOK_ENUM Identifier IDLTOK_LPAR EnumeratorList IDLTOK_RPAR
        {
          if (idl_checkTypeDefinition (scope,$2,yyerror)) {
            YYABORT;
          }
          $$ = declareMetaObject(scope,$2,M_ENUMERATION);
          c_enumeration($$)->elements = c_metaArray(scope,$4,M_CONSTANT);
          metaFinalize($$);
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    ;

EnumeratorList:
	  EnumeratorList_noAnnPost
	| EnumeratorList_noAnnPost AnnApplPost
	;

EnumeratorList_noAnnPost: // (78)
      Enumerator
        { $$ = c_iterNew($1); }
    | Enumerator IDLTOK_COMMA EnumeratorList
        {
          $$ = c_iterInsert($3,$1);
          if (idl_checkEnumerationElementCount($$,yyerror)) {
            YYABORT;
          }
        }
    | Enumerator IDLTOK_COMMA AnnApplPost EnumeratorList
        {
          $$ = c_iterInsert($4,$1);
          if (idl_checkEnumerationElementCount($$,yyerror)) {
            YYABORT;
          }
        }
    ;

Enumerator:
	  Enumerator_noAnn
	| AnnAppl Enumerator_noAnn {$$ = $2;}
	;

Enumerator_noAnn: // (79)
      Identifier
        {
          if (idl_checkEnumerationElementDefinition (scope,$1,yyerror)) {
            YYABORT;
          }
          $$ = (void *)declareMetaObject(scope,$1,M_CONSTANT);
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    ;

SequenceType: // (80)
      IDLTOK_SEQUENCE IDLTOK_LEFT ElementTypeSpec IDLTOK_COMMA PositiveIntConst IDLTOK_RIGHT
        {
          $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = C_SEQUENCE;
          c_collectionType($$)->maxSize = (c_long)$5->value.is.LongLong;
          c_collectionType($$)->subType = $3;
          metaFinalize($$);
          $$ = declareCollection(scope, c_collectionType($$));
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    | IDLTOK_SEQUENCE IDLTOK_LEFT SimpleTypeSpec IDLTOK_RIGHT
        {
          $$ = c_metaDefine(getCollectionScope(scope,$3),M_COLLECTION);
          c_collectionType($$)->kind = C_SEQUENCE;
          c_collectionType($$)->maxSize = 0;
          c_collectionType($$)->subType = $3;
          metaFinalize($$);
          $$ = declareCollection(scope, c_collectionType($$));
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    ;

MapType:
	  IDLTOK_MAP IDLTOK_LEFT SimpleTypeSpec IDLTOK_COMMA ElementTypeSpec IDLTOK_COMMA PositiveIntConst IDLTOK_RIGHT
{
	yyerror("Maptypes will be supported in a future version.");
	YYABORT;
}
	| IDLTOK_MAP IDLTOK_LEFT SimpleTypeSpec IDLTOK_COMMA ElementTypeSpec IDLTOK_RIGHT
{
	yyerror("Maptypes will be supported in a future version.");
	YYABORT;
}

ElementTypeSpec:
	  ElementTypeSpec_noAnnPost
	| ElementTypeSpec AnnApplPost
	;

ElementTypeSpec_noAnnPost:
	  SimpleTypeSpec
	| AnnAppl SimpleTypeSpec {$$ = $2;}
	;

StringType: // (81)
      IDLTOK_STRING IDLTOK_LEFT PositiveIntConst IDLTOK_RIGHT
        {
          $$ = c_metaDefine(scope,M_COLLECTION);
          c_collectionType($$)->kind = C_STRING;
          c_collectionType($$)->maxSize = (c_long)$3->value.is.LongLong;
          c_collectionType($$)->subType = (c_type)metaResolve(scope,"c_char");
          metaFinalize($$);
          $$ = declareCollection(scope, c_collectionType($$));
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    | IDLTOK_STRING
        { $$ = P_STRING; }
    ;

WideStringType: // (82)
      IDLTOK_WSTRING IDLTOK_LEFT PositiveIntConst IDLTOK_RIGHT
        { $$ = idl_unsupportedTypeGetMetadata(IDL_UNSUP_WSTRING); }
    | IDLTOK_WSTRING
        { $$ = idl_unsupportedTypeGetMetadata(IDL_UNSUP_WSTRING); }
    ;

FixedPtConstType: // (97)
      IDLTOK_FIXED
        { $$ = NULL; }
    ;

ArrayDeclarator: // (83)
      ArrayIdentifier ArraySizeList
        { $$ = c_declaratorNew($1,$2); }
    ;

ArrayIdentifier:
	  ArrayIdentifier_noAnnPost
	| ArrayIdentifier_noAnnPost AnnApplPost
	;

ArrayIdentifier_noAnnPost:
	  Identifier
	| AnnAppl Identifier {$$ = $2;}
	;

ArraySizeList: // (83)
      FixedArraySize
        { $$ = c_iterNew($1); }
    | FixedArraySize ArraySizeList
        { $$ = c_iterAppend($2,$1); }
    ;

FixedArraySize: // (84)
      IDLTOK_LEPAR PositiveIntConst IDLTOK_REPAR
        { $$ = $2; }
    ;

/*************************************************************************
IDL-conformant correction to ODMG 2.0: ATTRIBUTE Type Name1, Name2, ...
*************************************************************************/

/*
AttrDcl:
      IDLTOK_READONLY IDLTOK_ATTRIBUTE DomainType Declarators
        {
          c_iter attributes;
          attributes = c_bindAttributes(scope,$4,$3,TRUE);
          declareAttributes(scope, attributes);
        }
    | IDLTOK_ATTRIBUTE DomainType Declarators
        {
          c_iter attributes;
          attributes = c_bindAttributes(scope,$3,$2,FALSE);
          declareAttributes(scope, attributes);
        }
    ;
*/

AttrDcl: // (85)
      ReadonlyAttrSpec
    | AttrSpec
    ;

ReadonlyAttrSpec: // (104)
      IDLTOK_READONLY IDLTOK_ATTRIBUTE ParamTypeSpec ReadonlyAttrDeclarator
    ;

ReadonlyAttrDeclarator: // (105)
      SimpleDeclarator {}
      RaisesExpr
    | SimpleDeclaratorList
    ;

SimpleDeclaratorList: // (105)
      SimpleDeclarator {}
    | SimpleDeclarator {}
      IDLTOK_COMMA SimpleDeclaratorList
    ;

AttrSpec: // (106)
      IDLTOK_ATTRIBUTE ParamTypeSpec AttrDeclarator
    ;

AttrDeclarator: // (107)
      SimpleDeclarator {}
      AttrRaisesExpr
    | SimpleDeclaratorList
    ;

AttrRaisesExpr: // (108)
      GetExcepExpr
    | GetExcepExpr SetExcepExpr
    | SetExcepExpr
    ;

GetExcepExpr: // (109)
      IDLTOK_GETRAISES IDLTOK_LRPAR ExceptionList IDLTOK_RRPAR
    ;

SetExcepExpr: // (110)
      IDLTOK_SETRAISES IDLTOK_LRPAR ExceptionList IDLTOK_RRPAR
    ;

ExceptionList: // (111) "(" and ")" are defined by caller
      ScopedName {}
    | ScopedName {}
      IDLTOK_COMMA ExceptionList
    ;

// DomainType:
//       SimpleTypeSpec
//     | StructType
//     | EnumType
//     ;

ExceptDcl: // (86)
      IDLTOK_EXCEPTION Identifier IDLTOK_LPAR OptMemberList IDLTOK_RPAR
        {
          if (idl_checkExceptionDefinition (scope, $2, yyerror)) {
            YYABORT;
          }
          object = declareMetaObject(scope,$2,M_EXCEPTION);
          c_structure(object)->members = c_metaArray(scope,$4,M_MEMBER);
          metaFinalize(object);
          idl_fileMapAssociation(idl_fileMapDefGet(), object, file_name);
        }
    ;

OptMemberList: // (86)
      /* No member iter */
        { $$ = NULL; }
    | MemberList
        { $$ = $1; }
    ;

OpDcl: // (87)
      OpAttribute OpTypeSpec Identifier ParameterDcls RaisesExpr ContextExpr
//      {
//        if (idl_checkOperationDefinition (scope, $3, yyerror)) {
//          YYABORT;
//        }
//        object = declareMetaObject(scope,$3,M_OPERATION);
//        if (object != NULL) {
//          c_operation(object)->result = $2;
//          c_operation(object)->parameters = c_metaArray(scope,$4,M_PARAMETER);
//          metaFinalize(object);
//        }
//        idl_fileMapAssociation(idl_fileMapDefGet(), object, file_name);
//      }
    ;

OpAttribute: // (88)
    | IDLTOK_ONEWAY
    ;

OpTypeSpec: // (89)
      SimpleTypeSpec
        { $$ = $1; }
    | IDLTOK_VOID
        { $$ = P_VOID; }
    ;

ParameterDcls: // (90)
      IDLTOK_LRPAR ParamDclList IDLTOK_RRPAR
        { $$ = $2; }
    | IDLTOK_LRPAR IDLTOK_RRPAR
        { $$ = NULL; }
    ;

ParamDclList: // (90)
      ParamDcl
        { $$ = c_iterNew($1); }
    | ParamDcl IDLTOK_COMMA ParamDclList
        { $$ = c_iterInsert($3,$1); }
    ;

ParamDcl: // (91)
      ParamAttribute SimpleTypeSpec Declarator
        {
          $$ = c_metaDefine(scope,M_PARAMETER);
          c_specifier($$)->name = $3;
          c_specifier($$)->type = $2;
          c_parameter($$)->mode = $1;
          declareType(scope, $2);
          idl_fileMapAssociation(idl_fileMapDefGet(), $$, file_name);
        }
    ;

ParamAttribute: // (92)
      IDLTOK_IN
        { $$ = D_IN; }
    | IDLTOK_OUT
        { $$ = D_OUT; }
    | IDLTOK_INOUT
        { $$ = D_INOUT; }
    ;

RaisesExpr: // (93)
    | IDLTOK_RAISES IDLTOK_LRPAR ScopedNameList IDLTOK_RRPAR
    ;

ScopedNameList: // (93)
      ScopedName
        { $$ = c_iterNew($1); }
    | ScopedName IDLTOK_COMMA ScopedNameList
        { $$ = c_iterInsert($3,$1); }
    ;

ContextExpr: // (94)
    | IDLTOK_CONTEXT IDLTOK_LRPAR StringLiteralList IDLTOK_RRPAR
    ;

SL:
      StringLiteral
        { $$ = $1; }
    ;

StringLiteralList:
      SL
        { $$ = c_iterNew($1); }
    | SL IDLTOK_COMMA StringLiteralList
        { $$ = c_iterInsert($3,$1); }
    ;

%%
#include "idl_base.h"
#include <os_stdlib.h>

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
    printf("*** DDS error in file %s: %s near the token %s (line: %d, column: %d)\n",
           file_name, text, yytext, parser_line, parser_column);
    parser_error = 1;
    return -1;
}

void
idl_idlinit(c_module schema)
{
    idl_fileMap fileMap;
    idl_keyDef keyDef;
    idl_catsDef catsDef;
    idl_stacDef stacDef;
    idl_streamsDef streamsDef;

    topLevel = schema;
    scope = topLevel;
    fileMap = idl_fileMapNew();
    idl_fileMapDefSet(fileMap);
    keyDef = idl_keyDefNew();
    idl_keyDefDefSet(keyDef);
    catsDef = idl_catsDefNew();
    idl_catsDefDefSet(catsDef);
    stacDef = idl_stacDefNew();
    idl_stacDefDefSet(stacDef);
    streamsDef = idl_streamsDefNew();
    idl_streamsDefDefSet(streamsDef);
}

int
idl_idlparse(
    const char *fname)
{
    char *name = NULL;

    parser_line = 1;
    parser_column = 0;
    parser_state = 0;
    parser_error = 0;

    name = os_strdup(fname);

    if (name == NULL) {
        printf("IDL parser error: out of memory:");
        parser_error = 1;
        return parser_error;
    }

    yyin = fopen(name, "r");

    if (yyin == NULL) {
        printf("IDL parser error: opening file %s\n", name);
        parser_error = 1;
        return parser_error;
    }
    preprocess(yyin, name);

    yyparse();

    fclose(yyin);

    os_free(name);

    return parser_error;
}

static void support_warning(char *text)
{
    printf("DDS warning not supported yet: %s at line %d column %d, parsing is stopped\n",
	   text, parser_line, parser_column);
    exit (1);
}

static c_metaObject
getCollectionScope(
    c_metaObject current,
    c_metaObject subType)
{
    return current;
}

static c_collectionType
declareCollection(
    c_metaObject scope,
    c_collectionType c)
{
    c_string name;
    c_char *collName;
    c_metaObject found;

    name = c_metaName(c_metaObject(c));
    if (name == NULL) {
        collName = getScopedTypeName(scope, c_type(c), "::", C_SCOPE_ALWAYS);
        found = c_metaBind(scope, collName, c_metaObject(c));
        return c_collectionType(found);
    } else {
        c_free(name);
        return NULL;
    }
}

static void
declareIfArray(
    c_metaObject scope,
    c_type type)
{
    if (c_baseObject(type)->kind == M_COLLECTION) {
        declareIfArray(scope, c_collectionType(type)->subType);
        declareCollection(scope, c_collectionType(type));
    }
}

static void
declareMemberIfArray(
    void *o,
    c_iterActionArg arg)
{
    c_member member;
    c_metaObject scope;

    member = c_member(o);
    scope = c_metaObject(arg);

    declareIfArray(scope, c_specifier(member)->type);
}

static void
declareMembers(
    c_metaObject scope,
    c_iter members)
{
    /* Members is a special case, iter has to be reused afterwards */
    /* Therefore use walk instead of take                          */
    c_iterWalk(members, declareMemberIfArray, (void *)scope);
}

static int
checkTypeDefs(
    c_metaObject scope,
    c_iter declarations)
{
    int i;
    c_declarator d;
    int errId = 0;

    for (i = 0; i < c_iterLength(declarations); i++) {
		d = c_iterObject(declarations, i);
		errId += idl_checkTypeDefinition (scope, c_declaratorName(d), yyerror);
    }
    return errId;
}

static void
declareTypeDefs(
    c_metaObject scope,
    c_iter typeDefs)
{
    c_typeDef typeDef;

    typeDef = c_iterTakeFirst(typeDefs);
    while (typeDef != NULL) {
		idl_fileMapAssociation(idl_fileMapDefGet(), c_object(typeDef), file_name);
        declareIfArray(scope, typeDef->alias);
        c_free(typeDef);
        typeDef = c_iterTakeFirst(typeDefs);
    }
}

static void
declareAttributes(
    c_metaObject scope,
    c_iter attributes)
{
    c_attribute attribute;

    attribute = c_iterTakeFirst(attributes);
    while (attribute != NULL) {
        declareIfArray(scope, c_property(attribute)->type);
        c_free(attribute);
        attribute = c_iterTakeFirst(attributes);
    }
}

static void
declareType(
    c_metaObject scope,
    c_type type)
{
    declareIfArray(scope, type);
}

static c_metaObject
declareMetaObject(
    c_metaObject scope,
    c_char *name,
    c_metaKind kind)
{
    c_metaObject found;

    found = c_metaObject(c_metaDeclare(scope,name,kind));
    if (found == NULL) {
    	found = c_metaResolve(scope, name);
    	if(c_isFinal(found)) {
        	printf("***DDS parse error %s redeclared at line: %d\n", name, parser_line);
	        exit(-1);
	    }
    }
    return found;
}

static c_metaObject
declareUnsupportedType(
    c_metaObject scope,
    char *name,
    enum idl_unsupported_type unsupportedType)
{
    c_metaObject o;
#if 0
    os_time delay = {30,0};
printf("Attach debugger\n");
fflush(stdout);
os_nanoSleep(delay);
#endif

    o = declareMetaObject(scope, name, M_TYPEDEF);
    if (o) {
        c_typeDef(o)->alias = idl_unsupportedTypeGetMetadata(unsupportedType);
        assert(c_typeDef(o)->alias != NULL);
        metaFinalize(o);
    }
    return o;
}


static c_metaObject
metaResolve(
    c_metaObject scope,
    const char *name)
{
    c_metaObject found;
    found = c_metaResolve(scope,name);
    if (found == NULL) {
        printf("***DDS parse error %s undefined at line: %d (searching from scope '%s')\n", name, parser_line, scope->name);
        exit(-1);
    }
    return found;
}

static c_metaObject
metaResolveAnnotation(
	c_metaObject scope,
	const char *name)
{
    c_metaObject found;
    found = c_metaResolve(scope,name);
    if (found == NULL) {
        /* Lookup _ospl_internal scope */
    	if(!scopeInternal) {
            scopeInternal = c_metaResolve(topLevel, "_ospl_internal");
    		if(!scopeInternal) {
                printf("***DDS parse error ::_ospl_internal undefined at line: %d\n", parser_line);
    			exit(-1);
    		}
    	}

        /* Lookup object in <_ospl_internal> scope */
    	found = c_metaResolve(scopeInternal, name);
        if(!found) {
            printf("***DDS parse error %s undefined at line: %d (searching from scope '%s')\n", name, parser_line, scope->name);
	        exit(-1);
        }
    }
    return found;
}

static c_result
metaFinalize(
    c_metaObject o)
{
    c_result result;
    c_string name;

    result = c_metaFinalize(o);
    name = c_metaName(o);
    switch(result) {
    case S_ACCEPTED:
    break;
    case S_REDECLARED:
        printf("***DDS parse error %s redeclared at line: %d\n", name, parser_line);
        exit(-1);
    break;
    case S_ILLEGALOBJECT:
        printf("***DDS parse error %s illegal object at line: %d\n", name, parser_line);
        exit(-1);
    break;
    case S_REDEFINED:
        printf("***DDS parse error %s redefined at line: %d\n", name, parser_line);
        exit(-1);
    break;
    case S_ILLEGALRECURSION:
        printf("***DDS parse error %s illegal recursion at line: %d\n", name, parser_line);
        exit(-1);
    break;
    default:
        printf("***DDS internal parser error occured at line: %d\n", parser_line);
        exit(-1);
    break;
    }
    return result;
}

static c_char *
getCollKindName(
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    const c_collectionType c)
{
    c_char *result;

#define _CASE_(k) case k: result = os_strdup(#k); break
    switch(c->kind) {
    _CASE_(C_ARRAY);
    _CASE_(C_SEQUENCE);
    _CASE_(C_SET);
    _CASE_(C_LIST);
    _CASE_(C_BAG);
    _CASE_(C_DICTIONARY);
    _CASE_(C_STRING);
    _CASE_(C_WSTRING);
    default:
        /* QAC EXPECT 3416; No side effects in this case */
        assert(FALSE);
        result = os_strdup("C_UNDEFINED");
    break;
    }
#undef _CASE_

    return result;
}

static c_char *
getCollAnonName(
    const c_metaObject scope,
    const c_collectionType c,
    const char *separator)
{
    c_char *result;
    c_char *kind;
    c_char *subTypeName;
    c_long resultSize;

    kind = getCollKindName(c);
    subTypeName = getScopedTypeName(c_metaObject(c->subType)->definedIn, c->subType,
                                      separator, C_SCOPE_ALWAYS);
    resultSize = (int)strlen(kind) + (int)strlen(subTypeName) + 32;
    /* QAC EXPECT 5007; will not use wrapper */
    result = (c_char *)os_malloc((size_t)resultSize);
    if (c->kind == C_STRING) {
        if (c->maxSize > 0) {
            snprintf(result, (size_t)resultSize, "%s<%d>", kind, c->maxSize);
        } else {
            snprintf(result, (size_t)resultSize, "%s", kind);
        }
    } else {
        if (c->maxSize > 0) {
            snprintf(result, (size_t)resultSize, "%s<%s,%d>", kind, subTypeName, c->maxSize);
        } else {
            snprintf(result, (size_t)resultSize, "%s<%s>", kind, subTypeName);
        }
    }

    return result;
}

static c_char *
getScopedTypeName(
    const c_metaObject scope,
    const c_type type,
    const char *separator,
    c_scopeWhen scopeWhen)
{
    c_char *typeName, *result, *tmp;
    c_long tmpSize;
    c_metaObject scop = scope;

    typeName = c_metaName(c_metaObject(type));
    if (!typeName && (c_baseObject(type)->kind == M_COLLECTION)) {
        typeName = getCollAnonName(scope, c_collectionType(type),
                                     separator);
        result = typeName;
    } else {
        result = os_strdup(typeName);
        /* QAC EXPECT 3416; No side effects in this case, expected behaviour */
        while ((scop != NULL) && (c_metaName(scop) != NULL)) {
            tmpSize = (int)strlen(c_metaName(scop)) + (int)strlen(separator) + (int)strlen(result) + 1;
            /* QAC EXPECT 5007; will not use wrapper */
            tmp = os_malloc ((size_t)tmpSize);
            snprintf (tmp, (size_t)tmpSize, "%s%s%s",  c_metaName(scop), separator, result);
            /* QAC EXPECT 5007; will not use wrapper */
            os_free (result);
            result = tmp;
            scop = scop->definedIn;
        }
    }
    return result;
}

static c_char *
getScopedConstName(
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    const c_metaObject scope,
    /* QAC EXPECT 5007; suppress QACtools error */
    const c_constant c,
    const char *separator,
    c_scopeWhen scopeWhen)
{
    c_char *name, *moduleName, *result = NULL;
    c_metaObject module;
    c_long resultSize;

    name = c_metaName(c_metaObject(c));
    if (name) {
        module = c_metaModule(c_metaObject(c));
        moduleName = c_metaName(module);
        if ((moduleName != NULL) &&
            ((scopeWhen == C_SCOPE_ALWAYS) ||
             ((scopeWhen == C_SCOPE_SMART) && (scope != module)))) {
            /* the const is defined within another module */
            resultSize = (int)strlen(moduleName) + (int)strlen(name) + (int)strlen(separator) + 1;
            /* QAC EXPECT 5007; will not use wrapper */
            result = (c_char *)os_malloc((size_t)resultSize);
            snprintf(result, (size_t)resultSize, "%s%s%s", moduleName, separator, name);
        } else {
            result = os_strdup(name);
        }
    }

    return result;
}
