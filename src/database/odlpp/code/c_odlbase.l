/***********************************************************************
 *
 * Object name   : Odl grammar scanner
 * Component     : ODL Parser
 * Implementation: ODL grammar scanner (conformant to ODMG 2.0 ODL)
 *                 This file is based on the lex file provided by the
 *                 ODMG for ODMG-93 ODL and analyzes the lexical
 *                 correctness of an ODL source file.
 *
 * Author        : J.Venema
 * Date          : 2 March 2001
 * History       :
 *
 ***********************************************************************/

%x COMMENT
%{
/**********************************************************************
 *
 * include-statements
 *
 **********************************************************************/

#include "os_if.h"
#include "os_heap.h"
#include "os_errno.h"
#ifndef WIN32
#include "os_stdlib.h"
#else
extern int os_strcasecmp(const char *s1, const char *s2);
extern char *os_strdup(const char *s1);
extern char *os_strncpy(char *s1, const char *s2, size_t num);
#endif
#include "ctype.h"

/**********************************************************************
 *
 * local macro definitions
 *
 **********************************************************************/

#define pc() parser_column+=(int)strlen(yytext)
#define pl() parser_line++; parser_column=0

/**********************************************************************
 *
 * static function prototypes
 *
 **********************************************************************/

static int compareStrings(const char *String1, const char *String2);
static int extractToken(const char *_String);
static os_size_t extractIdent(char *str);
static os_size_t extractString(char *str);
static void preprocessor_directive(char *str);

/* TODO: Check if this static var is necessary.
         Copied this from idlpp (RT) */
static unsigned long long intValue;

%}

%option noyywrap

/*--------------------------------------------------------------------------*/

Digits                  [0-9]+
Oct_Digit               [0-7]
Hex_Digit               [a-fA-F0-9]
Int_Literal             [1-9][0-9]*
Oct_Literal             0{Oct_Digit}*
Hex_Literal             (0x|0X){Hex_Digit}*
Esc_Sequence1           "\\"[ntvbrfa\\\?\'\"]
Esc_Sequence2           "\\"{Oct_Digit}{1,3}
Esc_Sequence3           "\\"(x|X){Hex_Digit}{1,2}
Esc_Sequence            ({Esc_Sequence1}|{Esc_Sequence2}|{Esc_Sequence3})
Char                    ([^\n\t\"\'\\]|{Esc_Sequence})
Char_Literal            "'"({Char}|\")"'"
String_Literal          \"({Char}|"'")*\"
Float_Literal1          {Digits}"."{Digits}?(e|E)("+"|"-")?{Digits}
Float_Literal2          {Digits}(e|E)("+"|"-")?{Digits}
Float_Literal3          {Digits}"."{Digits}
Float_Literal4          {Digits}"."
Float_Literal5          "."{Digits}
Float_Literal6          "."{Digits}(e|E)("+"|"-")?{Digits}
Fixed_Literal1          {Digits}(d|D)
Fixed_Literal2          {Digits}"."(d|D)
Fixed_Literal3          "."{Digits}(d|D)
Fixed_Literal4          {Digits}"."{Digits}(d|D)

/*--------------------------------------------------------------------------*/

ODL_Identifier          [a-zA-Z_][a-zA-Z0-9_]*

/*--------------------------------------------------------------------------*/

%%

"/*"                    BEGIN(COMMENT);

<COMMENT>[^*\n]*
<COMMENT>"*"+[^*/\n]*
<COMMENT>\n             { pl(); OS_UNUSED_ARG(yyunput);}
<COMMENT>"*"+"/"        BEGIN(INITIAL);

[ \t\r]                 { pc(); }
[\n]                    { pl(); }
"//"[^\n]*\n            { pl(); }
"#"[^\n]*\n             { pl();
                          preprocessor_directive( yytext );
                        }

";"                     { pc();return ODL_SEMI; }
"}"                     { pc();return ODL_RPAR; }
"{"                     { pc();return ODL_LPAR; }
"::"                    { pc();return ODL_DOUBLE_COLON; }
":"                     { pc();return ODL_COLON; }
"("                     { pc();return ODL_LRPAR; }
")"                     { pc();return ODL_RRPAR; }
","                     { pc();return ODL_COMMA; }
"="                     { pc();return ODL_EQUAL; }
"|"                     { pc();return ODL_VERT; }
"^"                     { pc(); return ODL_HAT; }
"&"                     { pc(); return ODL_AMPER; }
">>"                    { pc(); return ODL_DOUBLE_RIGHT; }
"<<"                    { pc(); return ODL_DOUBLE_LEFT; }
"+"                     { pc(); return ODL_PLUS; }
"-"                     { pc(); return ODL_MINUS; }
"*"                     { pc(); return ODL_TIMES; }
"/"                     { pc(); return ODL_SLASH; }
"%"                     { pc(); return ODL_PERCENT; }
"~"                     { pc(); return ODL_TILDE; }
"<"                     { pc(); return ODL_LEFT; }
">"                     { pc(); return ODL_RIGHT; }
"["                     { pc(); return ODL_LEPAR; }
"]"                     { pc(); return ODL_REPAR; }

{Float_Literal1}        |
{Float_Literal2}        |
{Float_Literal3}        |
{Float_Literal4}        |
{Float_Literal5}        |
{Float_Literal6}        { yylval.Float = (double)atof(yytext);
                          pc();return FloatingPtLiteral;
                        }
{Fixed_Literal1}        |
{Fixed_Literal2}        |
{Fixed_Literal3}        |
{Fixed_Literal4}        { pc();return FixedPtLiteral;
                        }
{Int_Literal}           { sscanf(yytext,"%llu",&intValue);
                          yylval.Integer = (c_longlong) intValue;
                          pc();return IntegerLiteral;
                        }
{Oct_Literal}           { sscanf(yytext,"%llo",&intValue);
                          yylval.Integer = (c_longlong) intValue;
                          pc();return IntegerLiteral;
                        }
{Hex_Literal}           { sscanf(yytext,"%llx",&intValue);
                          yylval.Integer = (c_longlong) intValue;
                          pc();return IntegerLiteral;
                        }

{Char_Literal}         {
                         char *str = &yytext[1];
                         if (*str == '\\') {
                           switch (*++str) {
                            case 'n': yylval.Char = '\n'; break;
                            case 't': yylval.Char = '\t'; break;
                            case 'v': yylval.Char = '\v'; break;
                            case 'b': yylval.Char = '\b'; break;
                            case 'r': yylval.Char = '\r'; break;
                            case 'f': yylval.Char = '\f'; break;
                            case 'a': yylval.Char = '\a'; break;
                            case '\\': yylval.Char = '\\'; break;
                            case '?': yylval.Char = '\?'; break;
                            case '\'': yylval.Char = '\''; break;
                            case '"': yylval.Char = '"'; break;
                            case '0': case '1': case '2': case '3':
                            case '4': case '5': case '6': case '7':
                             yylval.Char = 0;
                              while (*str && *str != '\'') {
                                yylval.Char = (c_char) (yylval.Char * 8 + (*str++ - '0'));
                              }
                              break;
                            case 'x':
                            case 'X':
                             yylval.Char = 0;
                              str++;
                              while (*str && *str != '\'') {
                                if (*str >= '0' && *str <= '9')
                                  yylval.Char = (c_char) (yylval.Char * 16 + (*str - '0'));
                                else if (*str >= 'a' && *str <= 'f')
                                  yylval.Char = (c_char) (yylval.Char * 16 + (*str - 'a') + 10);
                                else if (*str >= 'A' && *str <= 'F')
                                  yylval.Char = (c_char) (yylval.Char * 16 + (*str - 'f') + 10);
                                str++;
                              }
                              break;
                            }
                          }
                          else {
                             yylval.Char = *str;
                          }
                          pc();return CharacterLiteral;
                        }
{String_Literal}        { extractString(yytext);
                          pc();return StringLiteral;
                        }
{ODL_Identifier}        { extractIdent(yytext);
                          pc();return extractToken(yytext);
                        }

.                       {
                            pc();
                            return yytext[0];
                        }
%%

/**********************************************************************
 *
 * static functions
 *
 **********************************************************************/

static int
compareStrings(const char *String1, const char *String2)
{
   return os_strcasecmp(String1, String2);
}

static int
extractToken(const char *_String)
{
   if (!compareStrings(_String, "module"      ) ) return ODL_MODULE;
   if (!compareStrings(_String, "persistent"  ) ) return ODL_PERSISTENT;
   if (!compareStrings(_String, "transient"   ) ) return ODL_TRANSIENT;
   if (!compareStrings(_String, "interface"   ) ) return ODL_INTERFACE;
   if (!compareStrings(_String, "key"         ) ) return ODL_KEY;
   if (!compareStrings(_String, "keys"        ) ) return ODL_KEYS;
   if (!compareStrings(_String, "const"       ) ) return ODL_CONST;
   if (!compareStrings(_String, "true"        ) ) return ODL_TRUE;
   if (!compareStrings(_String, "false"       ) ) return ODL_FALSE;
   if (!compareStrings(_String, "typedef"     ) ) return ODL_TYPEDEF;
   if (!compareStrings(_String, "float"       ) ) return ODL_FLOAT;
   if (!compareStrings(_String, "double"      ) ) return ODL_DOUBLE;
   if (!compareStrings(_String, "long"        ) ) return ODL_LONG;
   if (!compareStrings(_String, "short"       ) ) return ODL_SHORT;
   if (!compareStrings(_String, "unsigned"    ) ) return ODL_UNSIGNED;
   if (!compareStrings(_String, "char"        ) ) return ODL_CHAR;
   if (!compareStrings(_String, "boolean"     ) ) return ODL_BOOLEAN;
   if (!compareStrings(_String, "any"         ) ) return ODL_ANY;
   if (!compareStrings(_String, "octet"       ) ) return ODL_OCTET;
   if (!compareStrings(_String, "struct"      ) ) return ODL_STRUCT;
   if (!compareStrings(_String, "union"       ) ) return ODL_UNION;
   if (!compareStrings(_String, "switch"      ) ) return ODL_SWITCH;
   if (!compareStrings(_String, "case"        ) ) return ODL_CASE;
   if (!compareStrings(_String, "default"     ) ) return ODL_DEFAULT;
   if (!compareStrings(_String, "enum"        ) ) return ODL_ENUM;
   if (!compareStrings(_String, "array"       ) ) return ODL_ARRAY;
   if (!compareStrings(_String, "sequence"    ) ) return ODL_SEQUENCE;
   if (!compareStrings(_String, "String"      ) ) return ODL_STRING;
   if (!compareStrings(_String, "readonly"    ) ) return ODL_READONLY;
   if (!compareStrings(_String, "attribute"   ) ) return ODL_ATTRIBUTE;
   if (!compareStrings(_String, "set"         ) ) return ODL_SET;
   if (!compareStrings(_String, "list"        ) ) return ODL_LIST;
   if (!compareStrings(_String, "bag"         ) ) return ODL_BAG;
   if (!compareStrings(_String, "inverse"     ) ) return ODL_INVERSE;
   if (!compareStrings(_String, "relationship") ) return ODL_RELATIONSHIP;
   if (!compareStrings(_String, "order_by"    ) ) return ODL_ORDER_BY;
   if (!compareStrings(_String, "exception"   ) ) return ODL_EXCEPTION;
   if (!compareStrings(_String, "oneway"      ) ) return ODL_ONEWAY;
   if (!compareStrings(_String, "void"        ) ) return ODL_VOID;
   if (!compareStrings(_String, "in"          ) ) return ODL_IN;
   if (!compareStrings(_String, "out"         ) ) return ODL_OUT;
   if (!compareStrings(_String, "inout"       ) ) return ODL_INOUT;
   if (!compareStrings(_String, "raises"      ) ) return ODL_RAISES;
   if (!compareStrings(_String, "context"     ) ) return ODL_CONTEXT;
   if (!compareStrings(_String, "class"       ) ) return ODL_CLASS;
   if (!compareStrings(_String, "date"        ) ) return ODL_DATE;
   if (!compareStrings(_String, "time"        ) ) return ODL_TIME;
   if (!compareStrings(_String, "extends"     ) ) return ODL_EXTENDS;
   if (!compareStrings(_String, "timestamp"   ) ) return ODL_TIMESTAMP;
   if (!compareStrings(_String, "interval"    ) ) return ODL_INTERVAL;
   if (!compareStrings(_String, "dictionary"  ) ) return ODL_DICTIONARY;
   if (!compareStrings(_String, "import"      ) ) return ODL_IMPORT;
   if (!compareStrings(_String, "atomic"      ) ) return ODL_ATOMIC;
   if (!compareStrings(_String, "weak"        ) ) return ODL_WEAK;

/* If _String starts with '_' the identifier is an escaped keyword, like
   '_key'. The ODL parser must treat the identifier as 'key'.
*/
   if (_String[0] == '_') {
       /* adapt yylval.String */
       char *tmp = os_strdup(yylval.String);
       assert(compareStrings(_String, yylval.String) == 0);
       os_strncpy(yylval.String, &tmp[1], strlen(tmp) - 1);
       yylval.String[strlen(tmp) - 1] = '\0';
                 os_free(tmp);
   }

   return Identifier;
}

static os_size_t
extractIdent(char *str)
{
   os_size_t length = strlen(str);
   yylval.String = (char*)malloc(length+1);
   os_strncpy(yylval.String, str, length);
   yylval.String[length]=0;
   return length;
}

static os_size_t
extractString(char *str)
{
   os_size_t length = strlen(str) - 1;
   yylval.String = (char*)malloc(length);
   os_strncpy(yylval.String, ++str, length-1);
   yylval.String[length-1]=0;
   return length;
}

static void
preprocessor_directive(char *str)
{
   long lineNumber;
   size_t length;
   char *fileName, *ptr;

   str++; /* skip # */
   lineNumber = strtol(str,&ptr,10);
   while ((*ptr != 0) && ((*ptr == ' ') || (*ptr == '\t'))) ptr++;
   if (*ptr == '"') {
       ptr++;
       fileName = ptr;
   } else {
       return;
   }
   while (isgraph((int)*ptr) && (*ptr != '"')) ptr++;
   length = (size_t) (ptr - fileName);
   if (length >= MAX_FILENAME_LENGTH) {
      fprintf(stderr,"filename <%s> exceeds max length of %d\n",
                      fileName,MAX_FILENAME_LENGTH);
      exit(-1);
   }
   while ((*ptr != 0) && ((*ptr == ' ') || (*ptr == '\t'))) ptr++;
   if (*ptr != '"') return;
   os_strncpy(file_name,fileName,length);
   file_name[length] = 0;
   parser_line = (int) lineNumber;
   parser_column = 0;
}
