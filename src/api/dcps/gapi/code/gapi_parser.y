%{

#include "q_expr.h"
#include "gapi.h"
#include "ut_stack.h"
#include "os_stdlib.h"

#ifdef _WIN32
#define YY_NO_UNISTD_H
#endif

#define YY_NEVER_INTERACTIVE 1
    
#define yyin                 gapi_parser_yyin
#define yyout                gapi_parser_yyout
#define yyparse              gapi_parser_yyparse
#define yychar               gapi_parser_yychar
#define yyerror              gapi_parser_yyerror
#define yylex                gapi_parser_yylex
#define yylval               gapi_parser_yylval
#define yyrestart            gapi_parser_yyrestart
#define yywrap               gapi_parser_yywrap
#define yytext               gapi_parser_yytext
#define yynerrs              gapi_parser_yynerrs
#define yyleng               gapi_parser_yyleng
#define yy_scan_string       gapi_parser_yy_scan_string
#define yy_scan_buffer       gapi_parser_yy_scan_buffer
#define yy_init_buffer       gapi_parser_yy_init_buffer
#define yy_flush_buffer      gapi_parser_yy_flush_buffer 
#define yy_switch_to_buffer  gapi_parser_yy_switch_to_buffer
#define yy_delete_buffer     gapi_parser_yy_delete_buffer
#define yy_create_buffer     gapi_parser_yy_create_buffer
#define yy_load_buffer_state gapi_parser_yy_load_buffer_state
#define yy_scan_bytes        gapi_parser_yy_scan_bytes

static q_expr expr;

static ut_stack exprStack;
static q_list   lastList;
     
int gapi_parser_line = 1;
int gapi_parser_column = 0;
int gapi_parser_state = 0;

int yylex();
int yyerror ( char* text );

static q_list
splitFieldname ( char *name );

static void
clearExpressionStack ( ut_stack stack );

static void
clearExpressionList ( q_list list );


/***********************************************************************
 *
 * Static function prototypes
 *
 ***********************************************************************/

%}

%union {
    gapi_char      *String;
    gapi_char       Char;
    gapi_long_long  Integer;
    gapi_double     Float;
    q_list          List;
    q_expr          Expr;
    q_tag           Tag;
}

/*
%union {
    c_char    *String;
    c_char     Char;
    c_longlong Integer;
    c_double   Float;
    c_type     Type;
    q_list     List;
    q_expr     Expr;
    q_tag      Tag;
}
*/

%start program

%token AS_KEYWORD NIL TRUET FALSET LRPAR RRPAR
%token ALL LIKE BETWEEN
%token EQUAL NOTEQUAL GREATER LESS GREATEREQUAL LESSEQUAL
%token ORDER BY
%token NOT AND OR 
%token SELECT 
%token FROM WHERE
%token COMMA
%token NATURAL INNER JOIN
%token UNDEFINEDTOKEN

%token <String>  IDENTIFIER
%token <String>  FIELDNAME
%token <Integer> PARAM

%token <String>  STRINGVALUE
%token <Char>    CHARVALUE
%token <Integer> INTEGERVALUE
%token <Float>   FLOATVALUE
%token <String>  ENUMERATEDVALUE

%type  <List>    fieldList subjectFieldList
%type  <Expr>    query topicExpression queryExpression
                 aggregation fromClause whereClause
                 joinExpr orderBy topicName subjectField
                 condition condition1 condition2 condition3
                 predicate comparisonPredicate betweenPredicate
                 comparisonPar field parameter
%type  <Tag>     relOp
%type  <String>  ident

/*
%type  <List>    propertyList fieldList scopedName 
%type  <Expr>    ID query projectionAttributes
                 fromClause relationalExpr
                 whereClauseOpt expr selectExpr 
                 orExpr andExpr equalityExpr literal 
                 postfixExpr joinExpr notExpr
                 objectConstruction field
%type  <Tag>     equalityOper relationalOper
*/

%%

/***********************************************************************
 *
 * Query Program
 *
 ***********************************************************************/

program:
    | query
        { expr = F1(Q_EXPR_PROGRAM,$1);
          ut_stackPop(exprStack);
        }
    ;

query:
      topicExpression
        { $$ = $1; }
    | queryExpression
        { $$ = $1; }
    ;

/***********************************************************************
 *
 * Query Select expression
 *
 ***********************************************************************/

topicExpression:
      SELECT aggregation fromClause whereClause
        { $$ = F3(Q_EXPR_SELECT,$2,$3,$4);
          ut_stackPop(exprStack);
          ut_stackPop(exprStack);
          ut_stackPop(exprStack);
          ut_stackPush(exprStack, $$);
        }      
    ;

queryExpression:
      condition
        { $$ = $1; }
    | condition orderBy
        { $$ = $1; }
    ;
   
fromClause:
      FROM joinExpr
        { $$ = F1(Q_EXPR_FROM,$2);
          ut_stackPop(exprStack);
          ut_stackPush(exprStack, $$);
        }
    ;
      
aggregation:
      ALL
        { $$ = L0(Q_EXPR_PROJECTION);
          ut_stackPush(exprStack, $$);
        }
    | subjectFieldList
        { $$ = L1(Q_EXPR_PROJECTION,$1);
          lastList = NULL;
          ut_stackPush(exprStack, $$);
        }
    ;

joinExpr:
      topicName
        { $$ = $1; }
    | topicName naturalJoin joinExpr
        { $$ = F2(Q_EXPR_JOIN,$1,$3);
          ut_stackPop(exprStack);
          ut_stackPush(exprStack, $$);
        }
    ;

naturalJoin:
      INNER NATURAL JOIN
    | NATURAL JOIN
    | NATURAL INNER JOIN
    ;

whereClause:
        { $$ = L0(Q_EXPR_WHERE);
          ut_stackPush(exprStack, $$);
        }
    | WHERE condition
        { $$ = F1(Q_EXPR_WHERE,$2);
          ut_stackPop(exprStack);
          ut_stackPush(exprStack, $$);
        }
    ;

orderBy:
      ORDER BY fieldList
        { $$ = L1(Q_EXPR_ORDER,$3);
          lastList = NULL;
          ut_stackPush(exprStack, $$);
        }
    ;
    
topicName:
      ident
        { $$ = q_newId($1);
          ut_stackPush(exprStack, $$);
          free($1);
        }
    ;
    
subjectFieldList:
      subjectField
        { $$ = List1($1); 
          lastList = $$;
        }
    | subjectFieldList COMMA subjectField
        { $$ = q_append($1,$3);
          lastList = $$;
        }
    ;

subjectField:
      field
        { $$ = $1; }
    | field AS_KEYWORD field
        { $$ = F2(Q_EXPR_BIND,$3,$1);
          ut_stackPop(exprStack);
          ut_stackPop(exprStack);
          ut_stackPush(exprStack, $$);
        }
    | field field
        { $$ = F2(Q_EXPR_BIND,$2,$1);
          ut_stackPop(exprStack);
          ut_stackPop(exprStack);
          ut_stackPush(exprStack, $$);
        }
    ;

fieldList:
      field
        { $$ = List1($1);
          ut_stackPop(exprStack);
          lastList = $$;
        }
    | fieldList COMMA field
        { $$ = q_append($1,$3); 
          lastList = $$;
        }
    ;

condition:
      condition2
        { $$ = $1; }
    | condition2 OR condition
        { $$ = F2(Q_EXPR_OR,$1,$3);
          ut_stackPop(exprStack);
          ut_stackPop(exprStack);
          ut_stackPush(exprStack, $$);
        }
    ;

condition1:
      predicate
        { $$ = $1; }
    | LRPAR condition RRPAR
        { $$ = $2; }
    ;
    
condition2:
      condition3
        { $$ = $1; }
    | condition3 AND condition
        { $$ = F2(Q_EXPR_AND,$1,$3);
          ut_stackPop(exprStack);
          ut_stackPop(exprStack);
          ut_stackPush(exprStack, $$);
        }
    ;

condition3:
      condition1
        { $$ = $1; }
    | NOT condition1
        { $$ = F1(Q_EXPR_NOT,$2);
          ut_stackPop(exprStack);
          ut_stackPush(exprStack, $$);
        }
    ;
    
predicate:
      comparisonPredicate
        { $$ = $1; }
    | betweenPredicate
        { $$ = $1; }
    ;

comparisonPredicate:
      comparisonPar relOp comparisonPar
        { $$ = F2($2,$1,$3);
          ut_stackPop(exprStack);
          ut_stackPop(exprStack);
          ut_stackPush(exprStack, $$);
        }
    ;

comparisonPar:
      field
        { $$ = $1; }
    | parameter
        { $$ = $1; }
    ;

betweenPredicate:
      field BETWEEN parameter AND parameter
        { $$ = F2(Q_EXPR_AND,F2(Q_EXPR_GE,$1,$3),F2(Q_EXPR_LE,q_newId(q_getId($1)),$5));
          ut_stackPop(exprStack);
          ut_stackPop(exprStack);
          ut_stackPop(exprStack);
          ut_stackPush(exprStack, $$);
        }
    | field NOT BETWEEN parameter AND parameter
        { $$ = F2(Q_EXPR_OR,F2(Q_EXPR_LT,$1,$4),F2(Q_EXPR_GT,q_newId(q_getId($1)),$6));
          ut_stackPop(exprStack);
          ut_stackPop(exprStack);
          ut_stackPop(exprStack);
          ut_stackPush(exprStack, $$);
        }
    ;

field:
      IDENTIFIER
        { $$ =  q_newId($1);
          ut_stackPush(exprStack, $$);
          free($1);
        }
    | FIELDNAME
        { 
          q_list list = splitFieldname($1);
          if ( list == NULL ) {
              assert(0);
          }
          $$ = L1(Q_EXPR_PROPERTY,list); ;
          ut_stackPush(exprStack, $$);
          free($1);
        }
    ;

relOp:
      EQUAL
        { $$ = Q_EXPR_EQ; }
    | NOTEQUAL
        { $$ = Q_EXPR_NE; }
    | LESS
        { $$ = Q_EXPR_LT; }
    | LESSEQUAL
        { $$ = Q_EXPR_LE; }
    | GREATER
        { $$ = Q_EXPR_GT; }
    | GREATEREQUAL
        { $$ = Q_EXPR_GE; }
    | LIKE
        { $$ = Q_EXPR_LIKE; }
    ;

parameter:
      INTEGERVALUE
        { $$ = q_newInt($1);
          ut_stackPush(exprStack, $$);
        }
    | FLOATVALUE
        { $$ = q_newDbl($1);
          ut_stackPush(exprStack, $$);
        }
    | CHARVALUE
        { $$ = q_newChr($1);
          ut_stackPush(exprStack, $$);
        }
    | STRINGVALUE
        { $$ = q_newStr($1);
          ut_stackPush(exprStack, $$);
          free($1);
        }
    | ENUMERATEDVALUE
        { $$ = q_newStr($1);
          ut_stackPush(exprStack, $$);
          free($1);
        }
    | PARAM
        { $$ = q_newVar($1);
          ut_stackPush(exprStack, $$);
        }
    | TRUET
        { $$ = q_newChr(TRUE);
          ut_stackPush(exprStack, $$);
        }
    | FALSET
        { $$ = q_newChr(FALSE);
          ut_stackPush(exprStack, $$);
        }
    | NIL
        { $$ = NULL; }
    ;

ident:
      IDENTIFIER
          { $$ = $1; }
    ;



/***********************************************************************
 *
 * Query Select From clause
 *
 ***********************************************************************/
     
/***********************************************************************
 *
 * Query Select Where clause
 *
 ***********************************************************************/

/***********************************************************************
 *
 * Query q_newFnc
 *
 ***********************************************************************/

/***********************************************************************
 *
 * Query Boolean q_newFnc
 *
 ***********************************************************************/

/***********************************************************************
 *
 * Query Comparisson q_newFnc
 *
 ***********************************************************************/


%%

#include "gapi_parser.h"
#include "os_report.h"
#include "os_mutex.h"
#include "gapi.h"
#include "q_helper.h"

static os_mutex      parserMutex;
static gapi_boolean  parserInitialized = FALSE;

int
yyerror (
    char* text )
{
    OS_REPORT_4(OS_ERROR,"SQL parse failed",0,"%s near %s at line: %d, column: %d",
                text, yytext, gapi_parser_line, gapi_parser_column);
     
    yyclearin;

    q_dispose(expr);
    expr = NULL;

    return 0;
}

int
yywrap()
{
    return 1;
}

q_expr
gapi_parseExpression (
    const char *queryString)
{
    q_expr e = NULL;

    assert(queryString != NULL);
    
    os_mutexLock(&parserMutex);
  
    gapi_parser_line = 1;
    gapi_parser_column = 0;
    gapi_parser_state = 0;
    expr = NULL;
    lastList = NULL;
    exprStack = ut_stackNew(32);
    if ( exprStack ) {
        yy_scan_string(queryString);
        yyparse();
        e = expr;
        yy_delete_buffer(YY_CURRENT_BUFFER);
        clearExpressionStack(exprStack);
        clearExpressionList(lastList);
        q_exprSetText(e, queryString);
    } else {
        OS_REPORT(OS_ERROR,"SQL parse failed",0,"memory allocation failed");
    }   
    
    os_mutexUnlock(&parserMutex);

    return e;
}



void
gapi_parserInit(void)
{
    os_mutexAttr attr;

    if ( !parserInitialized) {
        parserInitialized = TRUE;

        os_mutexAttrInit(&attr);
	attr.scopeAttr = OS_SCOPE_PRIVATE;
        os_mutexInit(&parserMutex, &attr);
    }
}

void
gapi_parserDeinit(void)
{
    os_mutexDestroy(&parserMutex);
}

static char *
getProperty(
    char **str)
{
    char *e;
    char *result = NULL;

    if ( *str ) {
        e = strchr(*str, '.');
        if ( e != NULL ) {
            *e = '\0';
            result = *str;
            *str = e + 1;
        } else {
            result = *str;
            *str = NULL;
        }
    }

    return result;
}
        

static q_list
splitFieldname (
    char *name)
{
    char  *str;
    char  *property;
    q_list list = NULL;

    str = name;

    property = getProperty(&str);
    while ( property ) {
        list = q_append(list, q_newId(property));
        property = getProperty(&str);
    }

    return list;
}


static void
clearExpressionStack (
    ut_stack stack)
{
    while ( !ut_stackIsEmpty(stack) ) {
        q_expr e = (q_expr) ut_stackPop(stack);
        q_dispose(e);
    }

    ut_stackFree(stack);
}

static void
clearExpressionList (
    q_list list)
{
    if ( list ) {
        q_expr q = L1(Q_EXPR_ARRAY, list);
        q_dispose(q);
    }
}
    
    

