%{
#include "os_stdlib.h"
#include "c_base.h"
#include "c_collection.h"
#include "q_expr.h"
#include "q_helper.h"

#ifdef WIN32
#define YY_NO_UNISTD_H
#endif

/* This define is needed for Win32, but also applies to other platforms */
#define YY_NEVER_INTERACTIVE 1

#define yyin                 q_parser_yyin
#define yyout                q_parser_yyout
#define yyparse              q_parser_yyparse
#define yychar               q_parser_yychar
#define yyerror              q_parser_yyerror
#define yylex                q_parser_yylex
#define yylval               q_parser_yylval
#define yyrestart            q_parser_yyrestart
#define yywrap               q_parser_yywrap
#define yytext               q_parser_yytext
#define yynerrs              q_parser_yynerrs
#define yyleng               q_parser_yyleng
#define yy_scan_string       q_parser_yy_scan_string
#define yy_scan_buffer       q_parser_yy_scan_buffer
#define yy_init_buffer       q_parser_yy_init_buffer
#define yy_flush_buffer      q_parser_yy_flush_buffer 
#define yy_switch_to_buffer  q_parser_yy_switch_to_buffer
#define yy_delete_buffer     q_parser_yy_delete_buffer
#define yy_create_buffer     q_parser_yy_create_buffer
#define yy_load_buffer_state q_parser_yy_load_buffer_state
#define yy_scan_bytes        q_parser_yy_scan_bytes

#define ResolveType(s,t) c_type(c_metaResolve(c_metaObject(s),#t))

#define ToBeImplemented NULL

#define c_dateNew(a,b,c,d) ToBeImplemented
#define c_timeNew(a,b,c,d) ToBeImplemented
#define c_timestampNew(a,b,c,d,e,f,g) ToBeImplemented

q_expr expr, exprError;

int q_parser_line = 1;
int q_parser_column = 0;
int q_parser_state = 0;

int yylex();
int yyerror ( char* text );
void dollar_warning();

/***********************************************************************
 *
 * Static function prototypes
 *
 ***********************************************************************/

%}

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

%start program

/* Removed tokens because these are currently unused

%token PLUS MINUS DIV MOD ABS CONCAT QUERY
%token STRUCT SET BAG LIST ARRAY 
%token REPAR LEPAR FIRST LAST IN EXISTS
%token UNIQUE SOME ANY COUNT SUM SUB MIN MAX AVG 
%token DISTINCT FLATTEN SEMI DOUBLEDOT IMPORT 
%token ORELSE FOR ANDTHEN QUOTE
%token IS_UNDEFINED IS_DEFINED UNSIGNED UNDEFINE
%token DATE ENUM TIME INTERVAL TIMESTAMP DICTIONARY
%token GROUP BY HAVING ORDER
%token ASC DESC INTERSECT UNION EXCEPT LISTTOSET ELEMENT
%token CTOKEN 
*/

%token DEFINE AS_KEYWORD NIL TRUET FALSET LRPAR RRPAR
%token MUL LIKE BETWEEN
%token EQUAL NOTEQUAL GREATER LESS GREATEREQUAL LESSEQUAL
%token NOT AND OR REF
%token DOT ALL
%token SELECT UNDEFINED 
%token FROM WHERE
%token COLON COMMA DOUBLECOLON
%token DOLLAR PERCENT 
%token SELECT_DISTINCT JOIN

%token <String>  identifier stringLiteral queryId
%token <Char>    charLiteral
%token <Integer> longLiteral
%token <Float>   doubleLiteral

%type  <List>    propertyList fieldList scopedName 
%type  <Expr>    ID query projectionAttributes
                 fromClause relationalExpr
                 whereClauseOpt expr selectExpr 
                 orExpr andExpr equalityExpr literal 
                 postfixExpr joinExpr notExpr
                 objectConstruction field
%type  <Tag>     equalityOper relationalOper

%%

/***********************************************************************
 *
 * Query Program
 *
 ***********************************************************************/

program:
    | query
        { expr = F1(Q_EXPR_PROGRAM,$1); }
    ;

query:
      selectExpr
        { $$ = $1; }
    | expr
        { $$ = $1; }
    ;

/***********************************************************************
 *
 * Query Select expression
 *
 ***********************************************************************/

selectExpr:
      SELECT projectionAttributes fromClause whereClauseOpt
        { $$ = F3(Q_EXPR_SELECT,$2,$3,$4); }
    | SELECT_DISTINCT projectionAttributes fromClause whereClauseOpt
        { $$ = F3(Q_EXPR_SELECTDISTINCT,$2,$3,$4); }
    ;

projectionAttributes:
      MUL
        { $$ = L0(Q_EXPR_PROJECTION); }
    | postfixExpr
        { $$ = F1(Q_EXPR_PROJECTION,$1); }
    | fieldList
        { $$ = L1(Q_EXPR_PROJECTION,$1); }
    | objectConstruction
        { $$ = F1(Q_EXPR_PROJECTION,$1); }
    ;

objectConstruction:
      scopedName LRPAR fieldList RRPAR
        { $$ = L2(Q_EXPR_CLASS,q_newFnc(Q_EXPR_SCOPEDNAME,$1),$3); }
    ;

fieldList:
      field
        { $$ = List1($1); }
    | fieldList COMMA field
        { $$ = q_append($1,$3); }
    ;

field:
      postfixExpr COLON postfixExpr
        { $$ = F2(Q_EXPR_BIND,$3,$1); }
    | postfixExpr AS_KEYWORD postfixExpr
        { $$ = F2(Q_EXPR_BIND,$1,$3); }
    | postfixExpr
        { $$ = $1; }
    ;

/***********************************************************************
 *
 * Query Select From clause
 *
 ***********************************************************************/

fromClause:
      FROM joinExpr
        { $$ = F1(Q_EXPR_FROM,$2); }
    ;

joinExpr:
      ID
        { $$ = $1; }
    | ID JOIN joinExpr
        { $$ = F2(Q_EXPR_JOIN,$1,$3); }
    ;

/***********************************************************************
 *
 * Query Select Where clause
 *
 ***********************************************************************/

whereClauseOpt:
        { $$ = L0(Q_EXPR_WHERE); }
    | WHERE expr
        { $$ = F1(Q_EXPR_WHERE,$2); }
    ;

/***********************************************************************
 *
 * Query q_newFnc
 *
 ***********************************************************************/

expr:
      orExpr
        { $$ = $1; }
/*
    | NOT orExpr
        { $$ = F1(Q_EXPR_NOT,$2); }
*/
    ;

/***********************************************************************
 *
 * Query Boolean q_newFnc
 *
 ***********************************************************************/

orExpr:
      andExpr
        { $$ = $1; }
    | andExpr OR orExpr
        { $$ = F2(Q_EXPR_OR,$1,$3); }
    ;
/*
andExpr:
      equalityExpr
        { $$ = $1; }
    | equalityExpr AND andExpr
        { $$ = F2(Q_EXPR_AND,$1,$3); }
    ;
*/

andExpr:
      notExpr
        { $$ = $1; }
    | notExpr AND andExpr
        { $$ = F2(Q_EXPR_AND,$1,$3); }
    ;

notExpr:
      equalityExpr
        { $$ = $1; }
    | NOT notExpr
        { $$ = F1(Q_EXPR_NOT,$2); }
    ;

/***********************************************************************
 *
 * Query Comparisson q_newFnc
 *
 ***********************************************************************/

equalityExpr:
      relationalExpr
        { $$ = $1; }
    | postfixExpr equalityOper equalityExpr
        { $$ = F2($2,$1,$3); }
    | postfixExpr LIKE equalityExpr
        { $$ = F2(Q_EXPR_LIKE,$1,$3); }
    | LRPAR expr RRPAR
        { $$ = $2; }
    ;

relationalExpr:
      postfixExpr
        { $$ = $1; }
    | postfixExpr relationalOper relationalExpr
        { $$ = F2($2,$1,$3); }
    | ID BETWEEN postfixExpr AND postfixExpr
        { $$ = F2(Q_EXPR_AND,F2(Q_EXPR_GE,$1,$3),F2(Q_EXPR_LE,q_newId(q_getId($1)),$5)); }
    | ID NOT BETWEEN postfixExpr AND postfixExpr
        { $$ = F2(Q_EXPR_OR,F2(Q_EXPR_LT,$1,$4),F2(Q_EXPR_GT,q_newId(q_getId($1)),$6)); }
    ;

postfixExpr:
      ID
        { $$ = $1; }
    | ID propertyList
        { $$ = L2(Q_EXPR_PROPERTY,$1,$2); }
    | literal
        { $$ = $1; }
    | scopedName
        { $$ = q_newFnc(Q_EXPR_SCOPEDNAME,$1); }
    ;

propertyList:
      propertyOper ID
        { $$ = List1($2); }
    | propertyList propertyOper ID
        { $$ = q_append($1,$3); }
    ;

propertyOper:
      DOT
    | REF
    ;

ID:
      identifier
        { $$ = q_newId($1);
          /* frees dynamically malloced variable length string value
             allocated by the lexical scanner.
          */
          free($1);
        }
    ;

scopedName:
      identifier
        { $$ = List1(q_newId($1));
          /* frees dynamically malloced variable length string value
             allocated by the lexical scanner.
          */
          free($1);
        }
    | DOUBLECOLON identifier
        { $$ = List1(q_newId($2));
          /* frees dynamically malloced variable length string value
             allocated by the lexical scanner.
          */
          free($2);
        }
    | scopedName DOUBLECOLON identifier
        { $$ = q_append($1,q_newId($3));
          /* frees dynamically malloced variable length string value
             allocated by the lexical scanner.
          */
          free($3);
        }
    ;

equalityOper:
      EQUAL
        { $$ = Q_EXPR_EQ; }
    | NOTEQUAL
        { $$ = Q_EXPR_NE; }
    ;

relationalOper:
      LESS
        { $$ = Q_EXPR_LT; }
    | LESSEQUAL
        { $$ = Q_EXPR_LE; }
    | GREATER
        { $$ = Q_EXPR_GT; }
    | GREATEREQUAL
        { $$ = Q_EXPR_GE; }
    ;

literal:
      longLiteral
        { $$ = q_newInt($1); }
    | doubleLiteral
        { $$ = q_newDbl($1); }
    | charLiteral
        { $$ = q_newChr($1); }
    | stringLiteral
        { $$ = q_newStr($1);
          /* frees dynamically malloced variable length string value
             allocated by the lexical scanner.
          */
          free($1);
        }
    | DOLLAR longLiteral
        { $$ = q_newVar($2);
          dollar_warning();
        }
    | PERCENT longLiteral
        { $$ = q_newVar($2); }
    | TRUET
        { $$ = q_newInt(TRUE); }
    | FALSET
        { $$ = q_newInt(FALSE); }
    | NIL
        { $$ = NULL; }
    | UNDEFINED
        { $$ = NULL; }
    | ID
        { $$ = $1; }
    ;

%%

#include "q_parser.h"
#include "q__parser.h"
#include "os_report.h"
#include "os.h"

static os_mutex q_mtx;
static os_int32 initialise = 1; /* reset by q_parserInit */

int
yyerror (
    char* text )
{
    q_list params = NULL;

    OS_REPORT_4(OS_ERROR,"SQL parse failed",0,"%s near %s at line: %d, column: %d",
                text, yytext, q_parser_line, q_parser_column);
    yyclearin;

    q_dispose(expr);
    expr = NULL;
    params = q_insert(params,q_newStr(text));
    params = q_insert(params,q_newStr(yytext));
    params = q_insert(params,q_newInt(q_parser_line));
    params = q_insert(params,q_newInt(q_parser_column));
    exprError = q_newFnc(Q_EXPR_ERROR,params);

    return 0;
}

void
dollar_warning()
{
    OS_REPORT(OS_WARNING,"SQL parser",0,"The use of '$' is deprecated, use '%%' instead");
}

int
yywrap()
{
    return 1;
}

q_expr
q_parse (
    const c_char *expression)
{
    q_expr e;
    assert(expression != NULL);

    os_mutexLock(&q_mtx);
    q_parser_line = 1;
    q_parser_column = 0;
    q_parser_state = 0;
    expr = NULL;
    yy_scan_string((char *) expression);
    yyparse();
    e = expr;
    yy_delete_buffer(YY_CURRENT_BUFFER);
    q_exprSetText(e, expression);
    os_mutexUnlock(&q_mtx);
    return e;
}

void
q_parserInit()
{
    os_mutexAttr attr;

    if (initialise) {
        initialise = 0;

        os_mutexAttrInit(&attr);
	attr.scopeAttr = OS_SCOPE_PRIVATE;
	if ( os_mutexInit(&q_mtx, &attr) != os_resultSuccess )
	{
           OS_REPORT(OS_ERROR, "SQL parser", 0, "mutex init failed");
        }
    }
}
