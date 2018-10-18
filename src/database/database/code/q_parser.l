%{
/**********************************************************************
 *
 * include-statements
 *
 **********************************************************************/
#include "os_heap.h"

/**********************************************************************
 *
 * local macro definitions
 *
 **********************************************************************/

#define RETURN(t) context->column += (int)strlen(yytext); return t

/**********************************************************************
 *
 * static function prototypes
 *
 **********************************************************************/

static char *
removeDoubleQuotes(
    char *str,
    char ch);

/* Removed tokens because these are currently unused

"+"             { RETURN (PLUS); }
"-"             { RETURN (SUB); }
"/"             { RETURN (DIV); }
mod             { RETURN (MOD); }
abs             { RETURN (ABS); }
"||"            { RETURN (CONCAT); }
struct          { RETURN (STRUCT); }
set             { RETURN (SET); }
bag             { RETURN (BAG); }
list            { RETURN (LIST); }
array           { RETURN (ARRAY); }
"]"             { RETURN (REPAR); }
"["             { RETURN (LEPAR); }
".."            { RETURN (DOUBLEDOT); }
";"             { RETURN (SEMI); }
first           { RETURN (FIRST); }
last            { RETURN (LAST); }
for             { RETURN (FOR); }
in              { RETURN (IN); }
exists          { RETURN (EXISTS); }
unique          { RETURN (UNIQUE); }
some            { RETURN (SOME); }
any             { RETURN (ANY); }
count           { RETURN (COUNT); }
sum             { RETURN (SUM); }
min             { RETURN (MIN); }
max             { RETURN (MAX); }
avg             { RETURN (AVG); }
distinct        { RETURN (DISTINCT); }
group           { RETURN (GROUP); }
by              { RETURN (BY); }
having          { RETURN (HAVING); }
order           { RETURN (ORDER); }
asc             { RETURN (ASC); }
desc            { RETURN (DESC); }
intersect       { RETURN (INTERSECT); }
union           { RETURN (UNION); }
except          { RETURN (EXCEPT); }
listtoset       { RETURN (LISTTOSET); }
element         { RETURN (ELEMENT); }
flatten         { RETURN (FLATTEN); }
*/

%}

%option noyywrap
%option nounistd
%option never-interactive

%option prefix="q_parser_"

%option reentrant bison-bridge

INTEGER         -[0-9]+
UINTEGER        \+?[0-9]+
FLOAT           [+-]?({UINTEGER}"."{UINTEGER})(e[+-]?{UINTEGER})?
CHAR            \'[^\']\'

%x STRING1
STR1            (([^"]*\"\")*[^"]*)
%x STRING2
STR2            (([^']*\'\')*[^']*)

IDENT           [a-zA-Z][a-zA-Z0-9_]*

BLANK           [ \t]+
NEWLINE         \n
%x COMMENT1
COMMENT2        "//"[^\n]*
SD              ("select"{BLANK}"distict")

%%
"$"             { RETURN (DOLLAR); }
"%"             { RETURN (PERCENT); }
"("             { RETURN (LRPAR); }
")"             { RETURN (RRPAR); }
"="             { RETURN (EQUAL); }
"!="            { RETURN (NOTEQUAL); }
"<>"            { RETURN (NOTEQUAL); }
">"             { RETURN (GREATER); }
"<"             { RETURN (LESS); }
">="            { RETURN (GREATEREQUAL); }
"<="            { RETURN (LESSEQUAL); }
"*"             { RETURN (MUL); }
"."             { RETURN (DOT); }
"->"            { RETURN (REF); }
":"             { RETURN (COLON); }
"::"            { RETURN (DOUBLECOLON); }
","             { RETURN (COMMA); }
define          { RETURN (DEFINE); }
as              { RETURN (AS_KEYWORD); }
nil             { RETURN (NIL); }
true            { RETURN (TRUET); }
false           { RETURN (FALSET); }
like            { RETURN (LIKE); }
not             { RETURN (NOT); }
and             { RETURN (AND); }
or              { RETURN (OR); }
all             { RETURN (ALL); }
select          { RETURN (SELECT); }
from            { RETURN (FROM); }
where           { RETURN (WHERE); }
join            { RETURN (JOIN); }
between	        { RETURN (BETWEEN); }
DEFINE          { RETURN (DEFINE); }
AS              { RETURN (AS_KEYWORD); }
NIL             { RETURN (NIL); }
TRUE            { RETURN (TRUET); }
FALSE           { RETURN (FALSET); }
LIKE            { RETURN (LIKE); }
NOT             { RETURN (NOT); }
AND             { RETURN (AND); }
OR              { RETURN (OR); }
ALL             { RETURN (ALL); }
SELECT          { RETURN (SELECT); }
FROM            { RETURN (FROM); }
WHERE           { RETURN (WHERE); }
JOIN            { RETURN (JOIN); }
BETWEEN         { RETURN (BETWEEN); }

{BLANK}         {
                    context->column += (int)strlen(yytext);
                }
{COMMENT2}      {
                    context->column = 0;
                }
{NEWLINE}       {
                    context->line++;
                    context->column = 0;
                }
{IDENT}         {
                    context->column += (int)strlen(yytext);
                    /* variable length string values are allocated dynamically and
                       must be freed by the parser calling this scanner.
                    */
                    yylval->String = os_strdup(yytext);
                    return (identifier);
                }
{INTEGER}       {
                    context->column += (int)strlen(yytext);
                    yylval->Integer = os_atoll(yytext);
                    return (longlongLiteral);
                }
{UINTEGER}      {
                    context->column += (int)strlen(yytext);
                    yylval->UInteger = os_atoull(yytext);
                    return (ulonglongLiteral);
                }
{FLOAT}         {
                    context->column += (int)strlen(yytext);
                    yylval->Float = (double)atof(yytext);
                    return (doubleLiteral);
                }
{CHAR}          {
                    context->column += (int)strlen(yytext);
                    yylval->Char = yytext[1];
                    return (charLiteral);
                }
\"              {
                    context->column++;
                    BEGIN(STRING1);
                }
<STRING1>{STR1} {
                    context->column += (int)strlen(yytext);
                    yylval->String = removeDoubleQuotes(yytext, '\"');
                    return (stringLiteral);
                }
<STRING1>\"     {
                    context->column++;
                    BEGIN(INITIAL);
                }
\'|\`           {
                    context->column++;
                    BEGIN(STRING2);
                }
<STRING2>{STR2} {
                    context->column += (int)strlen(yytext);
                    yylval->String = removeDoubleQuotes(yytext, '\'');
                    return (stringLiteral);
                }
<STRING2>\'     {
                    context->column++;
                    BEGIN(INITIAL);
                }
{SD}            {   return (SELECT_DISTINCT); }
.               {   context->column += (int)strlen(yytext); }

"/*"                    BEGIN(COMMENT1);
<COMMENT1>[^*\n]*
<COMMENT1>"*"+[^*/\n]*
<COMMENT1>\n            { context->line++; context->column = 0; }
<COMMENT1>"*"+"/"       BEGIN(INITIAL);

%%


static char *
removeDoubleQuotes(
    char *str,
    char ch)
{
    os_size_t i, j;
    os_size_t len;
    char *s = NULL;

    if (str) {
        len = strlen(str);

        s = os_malloc(len+1);

        for (i = 0, j = 0; i < len; i++, j++) {
            if ((str[i] == ch) && (str[i+1] == ch)) {
                i++;
            }
            s[j] = str[i];
        }
        s[j] = '\0';
    }

    return s;
}
