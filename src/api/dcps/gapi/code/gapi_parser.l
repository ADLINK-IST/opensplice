
%{
/**********************************************************************
 *
 * include-statements
 *
 **********************************************************************/

#include "gapi_common.h"
#include "os_stdlib.h"

#ifdef WINCE
#undef errno
static int errno=0;
#endif


/**********************************************************************
 *
 * local macro definitions
 *
 **********************************************************************/

#define pc(x) gapi_parser_column+=x

#define RETURN(t) gapi_parser_column+=strlen(yytext);return t

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

INTEGER             [+-]?[0-9]+
FLOAT               [+-]?({INTEGER}"."{INTEGER})(e[+-]?{INTEGER})?
TRUE                [\'\`](true|TRUE)\'
FALSE               [\'\`](false|FALSE)\'
CHAR                \'[^\']\'
STR                 \"([^"]*\"\")*[^"]*\"
STR2                \'([^']*\'\')*[^']*\'
STR3                \`([^']*\'\')*[^']*\'

IDENT               [a-zA-Z][a-zA-Z0-9_]*
FIELDNAME           {IDENT}(\.{IDENT})*
ENUMVALUE           {IDENT}::{IDENT}
PARAM               %[0-9][0-9]?

BLANK               [ \t]+
NEWLINE             \n
%x COMMENT1
COMMENT2            "//"[^\n]*

%%
"("                 { RETURN (LRPAR); }
")"                 { RETURN (RRPAR); }
"="                 { RETURN (EQUAL); }
"<>"                { RETURN (NOTEQUAL); }
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

{TRUE}              { RETURN (TRUET); }
{FALSE}             { RETURN (FALSET); }

{BLANK}             {
                        gapi_parser_column+=strlen(yytext);
                    }
{COMMENT2}          {
                        gapi_parser_column=0;
                    }
{NEWLINE}           {
                        gapi_parser_line++;
                        gapi_parser_column=0;
                    }
{PARAM}             {
                        yylval.Integer = getParameterValue(yytext);
                        gapi_parser_column+=strlen(yytext);
                        return (PARAM);
                    }
{ENUMVALUE}         {
#if 0
    
                        yylval.String = (char*)malloc((unsigned)strlen(yytext)+1);
                        gapi_parser_column+=strlen(yytext);
                        os_strcpy(yylval.String,yytext);
                        return (ENUMERATEDVALUE);
#else
                        return (UNDEFINEDTOKEN);
#endif
                    }

{IDENT}             {
                        /* variable length string values are allocated dynamically and
                           must be freed by the parser calling this scanner.
                        */
                        yylval.String = (char*)malloc((unsigned)strlen(yytext)+1);
                        gapi_parser_column+=strlen(yytext);
                        os_strcpy(yylval.String,yytext);
                        return (IDENTIFIER);
                    }
{FIELDNAME}         {
                        yylval.String = (char*)malloc((unsigned)strlen(yytext)+1);
                        gapi_parser_column+=strlen(yytext);
                        os_strcpy(yylval.String,yytext);
                        return (FIELDNAME);
                    }   
{INTEGER}           {
                        if ( gapi_stringToLongLong(yytext, &yylval.Integer) ) {
                             return (INTEGERVALUE);
                        } else {
                             yylval.String = (char*)malloc((unsigned)strlen(yytext)+1);
                             gapi_parser_column+=strlen(yytext);
                             os_strcpy(yylval.String,yytext);
                             return (IDENTIFIER);
                        }
#if 0
                        yylval.Integer = atol(yytext);
                        gapi_parser_column+=strlen(yytext);
                        return (INTEGERVALUE);
#endif
                    }
{FLOAT}             {
                        yylval.Float = (double)atof(yytext);
                        gapi_parser_column+=strlen(yytext);
                        return (FLOATVALUE);
                    }
{CHAR}              { 
/*
                        yylval.Char = yytext[1];
                        gapi_parser_column+=strlen(yytext);
                        return (CHARVALUE);
*/
                        int length = strlen(yytext);
                        gapi_parser_column+=length;
                        /* variable length string values are allocated dynamically and
                           must be freed by the parser calling this scanner.
                        */
                        yylval.String = (char*)malloc(length-1);
                        os_strncpy(yylval.String, (char *)&yytext[1], length-2);
                        yylval.String[length-2]=0;
                        return (STRINGVALUE); 
                    }
{STR2}|{STR3}       {
                        int length = strlen(yytext);
                        gapi_parser_column+=length;
                        /* variable length string values are allocated dynamically and
                           must be freed by the parser calling this scanner.
                        */
                        yylval.String = (char*)malloc(length-1);
                        os_strncpy(yylval.String, (char *)&yytext[1], length-2);
                        yylval.String[length-2]=0;
                        return (STRINGVALUE); 
                    }
.                   {   gapi_parser_column+=strlen(yytext);
                        return (UNDEFINEDTOKEN);
                    }
#endif
"/*"                BEGIN(COMMENT1);
<COMMENT1>[^*\n]*
<COMMENT1>"*"+[^*/\n]*
<COMMENT1>\n             { ++gapi_parser_line; gapi_parser_column=0; }
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
    

