%{
/**********************************************************************
 *
 * include-statements
 *
 **********************************************************************/

/**********************************************************************
 *
 * local macro definitions
 *
 **********************************************************************/

#ifdef WINCE
#undef errno
static int errno=0;
#endif

#define pc(x) q_parser_column+=x

#define RETURN(t) q_parser_column+=strlen(yytext);return t

/**********************************************************************
 *
 * static function prototypes
 *
 **********************************************************************/

/* Removed tokens because these are currently unused

"+"		{ RETURN (PLUS); }
"-"		{ RETURN (SUB); }
"/"		{ RETURN (DIV); }
mod		{ RETURN (MOD); }
abs		{ RETURN (ABS); }
"||"		{ RETURN (CONCAT); }
struct          { RETURN (STRUCT); }
set		{ RETURN (SET); }
bag		{ RETURN (BAG); }
list		{ RETURN (LIST); }
array		{ RETURN (ARRAY); }
"]"		{ RETURN (REPAR); }
"["		{ RETURN (LEPAR); }
".."		{ RETURN (DOUBLEDOT); }
";"		{ RETURN (SEMI); }
first		{ RETURN (FIRST); }
last		{ RETURN (LAST); }
for		{ RETURN (FOR); }
in		{ RETURN (IN); }
exists	        { RETURN (EXISTS); }
unique	        { RETURN (UNIQUE); }
some		{ RETURN (SOME); }
any		{ RETURN (ANY); }
count		{ RETURN (COUNT); }
sum		{ RETURN (SUM); }
min		{ RETURN (MIN); }
max		{ RETURN (MAX); }
avg		{ RETURN (AVG); }
distinct	{ RETURN (DISTINCT); }
group		{ RETURN (GROUP); }
by		{ RETURN (BY); }
having	        { RETURN (HAVING); }
order		{ RETURN (ORDER); }
asc		{ RETURN (ASC); }
desc		{ RETURN (DESC); }
intersect	{ RETURN (INTERSECT); }
union		{ RETURN (UNION); }
except	        { RETURN (EXCEPT); }
listtoset	{ RETURN (LISTTOSET); }
element	        { RETURN (ELEMENT); }
flatten	        { RETURN (FLATTEN); }
*/

%}

/* %option noyywrap */

INTEGER        	[+-]?[0-9]+
FLOAT          	[+-]?({INTEGER}"."{INTEGER})(e[+-]?{INTEGER})?
CHAR  	       	\'[^\']\'
STR		\"([^"]*\"\")*[^"]*\"
STR2		\'([^']*\'\')*[^']*\'
STR3		\`([^']*\'\')*[^']*\'

IDENT		[a-zA-Z][a-zA-Z0-9_]*

BLANK		[ \t]+
NEWLINE	        \n
%x COMMENT1
COMMENT2        "//"[^\n]*
SD		("select"{BLANK}"distict")

%%
"$"		{  OS_UNUSED_ARG(yyunput); RETURN (DOLLAR); }
"%"		{ RETURN (PERCENT); }
"("		{ RETURN (LRPAR); }
")"             { RETURN (RRPAR); }
"="		{ RETURN (EQUAL); }
"!="		{ RETURN (NOTEQUAL); }
"<>"		{ RETURN (NOTEQUAL); }
">"             { RETURN (GREATER); }
"<"		{ RETURN (LESS); }
">="		{ RETURN (GREATEREQUAL); }
"<="		{ RETURN (LESSEQUAL); }
"*"		{ RETURN (MUL); }
"."  		{ RETURN (DOT); }
"->"	        { RETURN (REF); }
":"		{ RETURN (COLON); } 
"::"		{ RETURN (DOUBLECOLON); } 
","		{ RETURN (COMMA); }
define		{ RETURN (DEFINE); }
as		{ RETURN (AS_KEYWORD); }
nil		{ RETURN (NIL); }
true		{ RETURN (TRUET); }
false		{ RETURN (FALSET); }
like		{ RETURN (LIKE); }
not		{ RETURN (NOT); }
and		{ RETURN (AND); }
or		{ RETURN (OR); }
all		{ RETURN (ALL); }
select	        { RETURN (SELECT); }
from		{ RETURN (FROM); }
where		{ RETURN (WHERE); }
join	        { RETURN (JOIN); }
between	        { RETURN (BETWEEN); }
DEFINE		{ RETURN (DEFINE); }
AS		{ RETURN (AS_KEYWORD); }
NIL		{ RETURN (NIL); }
TRUE		{ RETURN (TRUET); }
FALSE		{ RETURN (FALSET); }
LIKE		{ RETURN (LIKE); }
NOT		{ RETURN (NOT); }
AND		{ RETURN (AND); }
OR		{ RETURN (OR); }
ALL		{ RETURN (ALL); }
SELECT	        { RETURN (SELECT); }
FROM		{ RETURN (FROM); }
WHERE		{ RETURN (WHERE); }
JOIN	        { RETURN (JOIN); }
BETWEEN	        { RETURN (BETWEEN); }

{BLANK}         {
                    q_parser_column+=strlen(yytext);
                }
{COMMENT2}	{
                    q_parser_column=0;
                }
{NEWLINE}	{
                    q_parser_line++;
                    q_parser_column=0;
                }
{IDENT}         {
                    /* variable length string values are allocated dynamically and
                       must be freed by the parser calling this scanner.
                    */
                    yylval.String = (char*)malloc((unsigned)strlen(yytext)+1);
                    q_parser_column+=strlen(yytext);
                    os_strcpy(yylval.String,yytext);
                    return (identifier);
		}
{INTEGER}       {
                    yylval.Integer = atol(yytext);
                    q_parser_column+=strlen(yytext);
                    return (longLiteral);
                }
{FLOAT}         {
                    yylval.Float = (double)atof(yytext);
                    q_parser_column+=strlen(yytext);
                    return (doubleLiteral);
                }
{CHAR}          { 
                    yylval.Char = yytext[1];
                    q_parser_column+=strlen(yytext);
                    return (charLiteral);
                }
{STR}|{STR2}|{STR3}    {
                    int length = strlen(yytext);
                    q_parser_column+=length;
                    /* variable length string values are allocated dynamically and
                       must be freed by the parser calling this scanner.
                    */
                    yylval.String = (char*)malloc(length-1);
                    os_strncpy(yylval.String, (char *)&yytext[1], length-2);
                    yylval.String[length-2]=0;
                    return (stringLiteral); 
		}
{SD}    {   return (SELECT_DISTINCT); }
.       {   q_parser_column+=strlen(yytext); }

"/*"			 BEGIN(COMMENT1);
<COMMENT1>[^*\n]*
<COMMENT1>"*"+[^*/\n]*
<COMMENT1>\n             { ++q_parser_line; q_parser_column=0; }
<COMMENT1>"*"+"/"        BEGIN(INITIAL);

%%
