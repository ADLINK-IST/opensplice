%{
#include <ctype.h>

#include "os_stdlib.h"
#include "os_heap.h"
#include "os_errno.h"

#define RETURN(t) context->column += (int)strlen(yytext);return t

static long
getParameterValue (
    char *str);

static char *
removeDoubleQuotes(
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

%option noyywrap
%option nounistd
%option never-interactive

%option prefix="v__parser_"

%option reentrant bison-bridge

INTEGER             -[0-9]+
UINTEGER            \+?[0-9]+
FLOAT               [+-]?({UINTEGER}"."{UINTEGER})(e[+-]?{UINTEGER})?
CHAR                \'[^\']\'
STR                 ([^']|(\'\'))*
STRSTART            (\'|\`)
STREND              (\')


IDENT               [a-zA-Z_/][a-zA-Z0-9_/]*
FIELDNAME           {IDENT}(\.{IDENT})*
PARAM               [$%][0-9][0-9]?

BLANK               [ \t]+
NEWLINE             \n
%x STRING
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
                        context->column += (int)strlen(yytext);
                    }
{COMMENT2}          {
                        context->column = 0;
                    }
{NEWLINE}           {
                        context->line++;
                        context->column = 0;
                    }
{PARAM}             {
                        context->column += (int)strlen(yytext);
                        yylval->Integer = getParameterValue(yytext);
                        return (PARAM);
                    }
{IDENT}             {
                        context->column += (int)strlen(yytext);
                        /* ensure parser frees memory */
                        yylval->String = os_strdup(yytext);
                        return (IDENTIFIER);
                    }
{FIELDNAME}         {
                        context->column += (int)strlen(yytext);
                        yylval->String = os_strdup(yytext);
                        return (FIELDNAME);
                    }
{INTEGER}           {
                        context->column += (int)strlen(yytext);
                        yylval->Integer = os_strtoll(yytext, NULL, 0);
                        return (INTEGERVALUE);
                    }
{UINTEGER}          {
                        context->column += (int)strlen(yytext);
                        yylval->UInteger = os_strtoull(yytext, NULL, 0);
                        return (UINTEGERVALUE);
                    }
{FLOAT}             {
                        context->column += (int)strlen(yytext);
                        yylval->Float = (double)atof(yytext);
                        return (FLOATVALUE);
                    }
{CHAR}              {
                        context->column += (int)strlen(yytext);
                        /* ensure parser frees memory */
                        yylval->String = os_strndup(&yytext[1], 1);
                        return (STRINGVALUE);
                    }
\'|\`               {
                        context->column++;
                        BEGIN(STRING);
                    }
<STRING>{STR}       {
                        context->column += (int)strlen(yytext);
                        yylval->String = removeDoubleQuotes(yytext);
                        return (STRINGVALUE);
                    }
<STRING>\'          {
                        context->column++;
                        BEGIN(INITIAL);
                    }
.                   {
                        context->column += (int)strlen(yytext);
                        return (UNDEFINEDTOKEN);
                    }
#endif
"/*"                BEGIN(COMMENT1);
<COMMENT1>[^*\n]*
<COMMENT1>"*"+[^*/\n]*
<COMMENT1>\n        {
                        context->line++;
                        context->column = 0;
                    }
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

static char *
removeDoubleQuotes(
    char *str)
{
    os_size_t i, j;
    os_size_t len;
    char *s = NULL;

    if (str) {
        len = strlen(str);

        s = os_malloc(len+1);

        for (i = 0, j = 0; i < len; i++, j++) {
            if (str[i] == '\'' && str[i+1] == '\'') {
                i++;
            }
            s[j] = str[i];
        }
        s[j] = '\0';
    }

    return s;
}
