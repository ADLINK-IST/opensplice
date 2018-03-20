
%option never-interactive

%{
/**********************************************************************
 *
 * include-statements
 *
 **********************************************************************/
#include <ctype.h>

#include "os_stdlib.h"
#include "os_heap.h"
#include "os_errno.h"


/**********************************************************************
 *
 * global macro definitions
 *
 **********************************************************************/
#define YY_NEVER_INTERACTIVE 1 /* stops flex from calling isatty() */

#define isatty thisisnotisatty

int isatty (int x) { OS_UNUSED_ARG(x); return 0; }

/**********************************************************************
 *
 * local macro definitions
 *
 **********************************************************************/

#define pc(x) v_parser_column+=x

#define RETURN(t) v_parser_column+=(int)strlen(yytext);return t

/**********************************************************************
 *
 * static function prototypes
 *
 **********************************************************************/

static long
getParameterValue (
    char *str);

/* Removed tokens because these are currently unused

"+"                { RETURN (PLUS); }
"-"                { RETURN (SUB); }
"/"                { RETURN (DIV); }
mod                { RETURN (MOD); }
abs                { RETURN (ABS); }
"||"               { RETURN (CONCAT); }
struct             { RETURN (STRUCT); }
set                { RETURN (SET); }
bag                { RETURN (BAG); }
list               { RETURN (LIST); }
array              { RETURN (ARRAY); }
"]"                { RETURN (REPAR); }
"["                { RETURN (LEPAR); }
".."               { RETURN (DOUBLEDOT); }
";"                { RETURN (SEMI); }
first              { RETURN (FIRST); }
last               { RETURN (LAST); }
for                { RETURN (FOR); }
in                 { RETURN (IN); }
exists             { RETURN (EXISTS); }
unique             { RETURN (UNIQUE); }
some               { RETURN (SOME); }
any                { RETURN (ANY); }
count              { RETURN (COUNT); }
sum                { RETURN (SUM); }
min                { RETURN (MIN); }
max                { RETURN (MAX); }
avg                { RETURN (AVG); }
distinct           { RETURN (DISTINCT); }
group              { RETURN (GROUP); }
by                 { RETURN (BY); }
having             { RETURN (HAVING); }
order              { RETURN (ORDER); }
asc                { RETURN (ASC); }
desc               { RETURN (DESC); }
intersect          { RETURN (INTERSECT); }
union              { RETURN (UNION); }
except             { RETURN (EXCEPT); }
listtoset          { RETURN (LISTTOSET); }
element            { RETURN (ELEMENT); }
flatten            { RETURN (FLATTEN); }
*/

%}

/* %option noyywrap */

INTEGER             -[0-9]+
UINTEGER            \+?[0-9]+
FLOAT               [+-]?({UINTEGER}"."{UINTEGER})(e[+-]?{UINTEGER})?
CHAR                \'[^\']\'
STR                 \"([^"]*\"\")*[^"]*\"
STR2                \'([^']*\'\')*[^']*\'
STR3                \`([^']*\'\')*[^']*\'

IDENT               [a-zA-Z_/][a-zA-Z0-9_/]*
FIELDNAME           {IDENT}(\.{IDENT})*
PARAM               [$%][0-9][0-9]?

BLANK               [ \t]+
NEWLINE             \n
%x COMMENT1
COMMENT2            "//"[^\n]*

%%
"("                 { RETURN (LRPAR); }
")"                 { RETURN (RRPAR); }
"="                 { RETURN (EQUAL); }
"<>"                { RETURN (NOTEQUAL); }
"!="                { RETURN (NOTEQUAL); }
">"                 { RETURN (GREATER); }
"<"                 { RETURN (LESS); }
">="                { RETURN (GREATEREQUAL); }
"<="                { RETURN (LESSEQUAL); }
"*"                 { RETURN (ALL); }
","                 { RETURN (COMMA); }
as                  { RETURN (AS_KEYWORD); }
nil                 { RETURN (NIL); }
like                { RETURN (LIKE); }
not                 { RETURN (NOT); }
and                 { RETURN (AND); }
or                  { RETURN (OR); }
select              { RETURN (SELECT); }
from                { RETURN (FROM); }
where               { RETURN (WHERE); }
join                { RETURN (JOIN); }
between             { RETURN (BETWEEN); }
AS                  { RETURN (AS_KEYWORD); }
NIL                 { RETURN (NIL); }
LIKE                { RETURN (LIKE); }
NOT                 { RETURN (NOT); }
AND                 { RETURN (AND); }
OR                  { RETURN (OR); }
SELECT              { RETURN (SELECT); }
FROM                { RETURN (FROM); }
WHERE               { RETURN (WHERE); }
JOIN                { RETURN (JOIN); }
BETWEEN             { RETURN (BETWEEN); }

{BLANK}             {
                        v_parser_column+=(int)strlen(yytext);
                    }
{COMMENT2}          {
                        v_parser_column=0;
                    }
{NEWLINE}           {
                        v_parser_line++;
                        v_parser_column=0;
                    }
{PARAM}             {
                        yylval.Integer = getParameterValue(yytext);
                        v_parser_column+=(int)strlen(yytext);
                        return (PARAM);
                    }
{IDENT}             {
                        /* variable length string values are allocated dynamically and
                           must be freed by the parser calling this scanner.
                        */
                        yylval.String = (char*)malloc(strlen(yytext)+1);
                        v_parser_column+=(int)strlen(yytext);
                        os_strcpy(yylval.String,yytext);
                        return (IDENTIFIER);
                    }
{FIELDNAME}         {
                        yylval.String = (char*)malloc(strlen(yytext)+1);
                        v_parser_column+=(int)strlen(yytext);
                        os_strcpy(yylval.String,yytext);
                        return (FIELDNAME);
                    }
{INTEGER}           {
                        c_char *ptr = NULL;
                        yylval.Integer = os_strtoll(yytext, &ptr, 0);
                        v_parser_column+=(int)strlen(yytext);
                        return (INTEGERVALUE);
                    }
{UINTEGER}          {
                        c_char *ptr = NULL;
                        yylval.UInteger = os_strtoull(yytext, &ptr, 0);
                        v_parser_column+=(int)strlen(yytext);
                        return (UINTEGERVALUE);
                    }
{FLOAT}             {
                        yylval.Float = (double)atof(yytext);
                        v_parser_column+=(int)strlen(yytext);
                        return (FLOATVALUE);
                    }
{CHAR}              {
/*
                        yylval.Char = yytext[1];
                        v_parser_column+=(int)strlen(yytext);
                        return (CHARVALUE);
*/
                        os_size_t length = strlen(yytext);
                        v_parser_column+=length;
                        /* variable length string values are allocated dynamically and
                           must be freed by the parser calling this scanner.
                        */
                        yylval.String = (char*)malloc(length-1);
                        os_strncpy(yylval.String, (char *)&yytext[1], length-2);
                        yylval.String[length-2]=0;
                        return (STRINGVALUE);
                    }
{STR2}|{STR3}       {
                        os_size_t length = strlen(yytext);
                        v_parser_column+=length;
                        /* variable length string values are allocated dynamically and
                           must be freed by the parser calling this scanner.
                        */
                        yylval.String = (char*)malloc(length-1);
                        os_strncpy(yylval.String, (char *)&yytext[1], length-2);
                        yylval.String[length-2]=0;
                        return (STRINGVALUE);
                    }
.                   {   v_parser_column+=(int)strlen(yytext);
                        return (UNDEFINEDTOKEN);
                    }
#endif
"/*"                BEGIN(COMMENT1);
<COMMENT1>[^*\n]*
<COMMENT1>"*"+[^*/\n]*
<COMMENT1>\n             { ++v_parser_line; v_parser_column=0; }
<COMMENT1>"*"+"/"        BEGIN(INITIAL);

%%

static long
getParameterValue (
    char *str)
{
    long value;


    str++;

    value = atol(str);

    return value;
}
