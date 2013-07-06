%x comment
%{
/**********************************************************************
 *
 * include-statements
 *
 **********************************************************************/
#include <os_heap.h>
#include <os_stdlib.h>
#include <ctype.h>
#include <c_stringSupport.h>

/**********************************************************************
 *
 * local macro definitions
 *
 **********************************************************************/

#define pc() parser_column+=strlen(yytext)
#define pl() parser_line++; parser_column=0

/**********************************************************************
 *
 * static function prototypes
 *
 **********************************************************************/

static int compareStrings(const char *String1, const char *String2);
static int extractToken(const char *_String);
static int extractIdent(char *str);
static int extractString(char *str);
static void preprocessor_directive(char *str);

/* TODO: Check if this static var is necessary.
         Copied this from idlpp (RT) */
static long long intValue;
static unsigned long long uintValue;

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

"/*"                    BEGIN(comment);

<comment>[^*\n]*
<comment>"*"+[^*/\n]*
<comment>\n             { pl(); }
<comment>"*"+"/"        BEGIN(INITIAL);

[ \t\r]                 { pc(); }
[\n]                    { pl(); }
"//"[^\n]*\n            { pl(); }
"#"[^\n]*\n             { pl();
                          preprocessor_directive( yytext );
                        }

";"                     { pc();return IDLTOK_SEMI; }
"}"                     { pc();return IDLTOK_RPAR; }
"{"                     { pc();return IDLTOK_LPAR; }
"::"                    { pc();return IDLTOK_DOUBLE_COLON; }
":"                     { pc();return IDLTOK_COLON; }
"("                     { pc();return IDLTOK_LRPAR; }
")"                     { pc();return IDLTOK_RRPAR; }
","                     { pc();return IDLTOK_COMMA; }
"="                     { pc();return IDLTOK_EQUAL; }
"|"                     { pc();return IDLTOK_VERT; }
"^"                     { pc(); return IDLTOK_HAT; }
"&"                     { pc(); return IDLTOK_AMPER; }
">>"                    { pc(); return IDLTOK_DOUBLE_RIGHT; }
"<<"                    { pc(); return IDLTOK_DOUBLE_LEFT; }
"+"                     { pc(); return IDLTOK_PLUS; }
"-"                     { pc(); return IDLTOK_MINUS; }
"*"                     { pc(); return IDLTOK_TIMES; }
"/"                     { pc(); return IDLTOK_SLASH; }
"%"                     { pc(); return IDLTOK_PERCENT; }
"~"                     { pc(); return IDLTOK_TILDE; }
"<"                     { pc(); return IDLTOK_LEFT; } 
">"                     { pc(); return IDLTOK_RIGHT; }
"["                     { pc(); return IDLTOK_LEPAR; }
"]"                     { pc(); return IDLTOK_REPAR; }

"@annotation"			{ pc(); return IDLTOK_ANNOTATION; }
"@"						{ pc(); return IDLTOK_AT; }
"//@"					{ pc(); return IDLTOK_AT_POST; }

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
{Int_Literal}           { sscanf (yytext, "%llu", (unsigned long long *)&intValue);
                          yylval.Integer = intValue;
                          pc();return IntegerLiteral;
                        }
{Oct_Literal}           { sscanf(yytext,"%llo",&uintValue);
                          yylval.Integer = uintValue;
                          pc();return IntegerLiteral;
                        }
{Hex_Literal}           { sscanf(yytext,"%llx",&uintValue);
                          yylval.Integer = uintValue;
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
                                yylval.Char = yylval.Char * 8 + (*str++ - '0');
                              }
                              break;
                            case 'x':
                            case 'X':
                             yylval.Char = 0;
                              str++;
                              while (*str && *str != '\'') {
                                if (*str >= '0' && *str <= '9')
                                  yylval.Char = yylval.Char * 16 +
                                                       (*str - '0');
                                else if (*str >= 'a' && *str <= 'f')
                                  yylval.Char = yylval.Char * 16 +
                                                       (*str - 'a') + 10;
                                else if (*str >= 'A' && *str <= 'F')
                                  yylval.Char = yylval.Char * 16 +
                                                       (*str - 'f') + 10;
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
   if (!compareStrings(_String, "abstract"    ) ) return IDLTOK_ABSTRACT;
   if (!compareStrings(_String, "any"         ) ) return IDLTOK_ANY;
/* if (!compareStrings(_String, "array"       ) ) return IDLTOK_ARRAY;		*/
   if (!compareStrings(_String, "attribute"   ) ) return IDLTOK_ATTRIBUTE;
/* if (!compareStrings(_String, "bag"         ) ) return IDLTOK_BAG;		*/
   if (!compareStrings(_String, "boolean"     ) ) return IDLTOK_BOOLEAN;
   if (!compareStrings(_String, "case"        ) ) return IDLTOK_CASE;
   if (!compareStrings(_String, "char"        ) ) return IDLTOK_CHAR;
/* if (!compareStrings(_String, "class"       ) ) return IDLTOK_CLASS;		*/
   if (!compareStrings(_String, "component"   ) ) return IDLTOK_COMPONENT;
   if (!compareStrings(_String, "const"       ) ) return IDLTOK_CONST;
   if (!compareStrings(_String, "consumes"    ) ) return IDLTOK_CONSUMES;
   if (!compareStrings(_String, "context"     ) ) return IDLTOK_CONTEXT;
   if (!compareStrings(_String, "custom"      ) ) return IDLTOK_CUSTOM;
/* if (!compareStrings(_String, "date"        ) ) return IDLTOK_DATE;		*/
   if (!compareStrings(_String, "default"     ) ) return IDLTOK_DEFAULT;
/* if (!compareStrings(_String, "dictionary"  ) ) return IDLTOK_DICTIONARY;	*/
   if (!compareStrings(_String, "double"      ) ) return IDLTOK_DOUBLE;
   if (!compareStrings(_String, "emits"       ) ) return IDLTOK_EMITS;
   if (!compareStrings(_String, "enum"        ) ) return IDLTOK_ENUM;
   if (!compareStrings(_String, "eventtype"   ) ) return IDLTOK_EVENTTYPE;
   if (!compareStrings(_String, "exception"   ) ) return IDLTOK_EXCEPTION;
/* if (!compareStrings(_String, "extends"     ) ) return IDLTOK_EXTENDS;	*/
/* if (!compareStrings(_String, "extent"      ) ) return IDLTOK_EXTENT;	*/
   if (!compareStrings(_String, "factory"     ) ) return IDLTOK_FACTORY;
   if (!compareStrings(_String, "FALSE"       ) ) return IDLTOK_FALSE;
   if (!compareStrings(_String, "finder"      ) ) return IDLTOK_FINDER;
   if (!compareStrings(_String, "fixed"       ) ) return IDLTOK_FIXED;
   if (!compareStrings(_String, "float"       ) ) return IDLTOK_FLOAT;
   if (!compareStrings(_String, "getraises"   ) ) return IDLTOK_GETRAISES;
   if (!compareStrings(_String, "home"        ) ) return IDLTOK_HOME;
   if (!compareStrings(_String, "import"      ) ) return IDLTOK_IMPORT;
   if (!compareStrings(_String, "in"          ) ) return IDLTOK_IN;
   if (!compareStrings(_String, "inout"       ) ) return IDLTOK_INOUT;
   if (!compareStrings(_String, "interface"   ) ) return IDLTOK_INTERFACE;
/* if (!compareStrings(_String, "interval"    ) ) return IDLTOK_INTERVAL;	*/
/* if (!compareStrings(_String, "inverse"     ) ) return IDLTOK_INVERSE;	*/
/* if (!compareStrings(_String, "key"         ) ) return IDLTOK_KEY;		*/
/* if (!compareStrings(_String, "keys"        ) ) return IDLTOK_KEYS;		*/
/* if (!compareStrings(_String, "list"        ) ) return IDLTOK_LIST;		*/
   if (!compareStrings(_String, "local"       ) ) return IDLTOK_LOCAL;
   if (!compareStrings(_String, "long"        ) ) return IDLTOK_LONG;
   if (!compareStrings(_String, "manages"     ) ) return IDLTOK_MANAGES;
   if (!compareStrings(_String, "map"         ) ) return IDLTOK_MAP;
   if (!compareStrings(_String, "module"      ) ) return IDLTOK_MODULE;
   if (!compareStrings(_String, "multiple"    ) ) return IDLTOK_MULTIPLE;
   if (!compareStrings(_String, "native"      ) ) return IDLTOK_NATIVE;
   if (!compareStrings(_String, "Object"      ) ) return IDLTOK_OBJECT;
   if (!compareStrings(_String, "octet"       ) ) return IDLTOK_OCTET;
   if (!compareStrings(_String, "oneway"      ) ) return IDLTOK_ONEWAY;
/* if (!compareStrings(_String, "order_by"    ) ) return IDLTOK_ORDER_BY;	*/
   if (!compareStrings(_String, "out"         ) ) return IDLTOK_OUT;
/* if (!compareStrings(_String, "persistent"  ) ) return IDLTOK_PERSISTENT;	*/
   if (!compareStrings(_String, "primarykey"  ) ) return IDLTOK_PRIMARYKEY;
   if (!compareStrings(_String, "private"     ) ) return IDLTOK_PRIVATE;
   if (!compareStrings(_String, "provides"    ) ) return IDLTOK_PROVIDES;
   if (!compareStrings(_String, "public"      ) ) return IDLTOK_PUBLIC;
   if (!compareStrings(_String, "publishes"   ) ) return IDLTOK_PUBLISHES;
   if (!compareStrings(_String, "raises"      ) ) return IDLTOK_RAISES;
   if (!compareStrings(_String, "readonly"    ) ) return IDLTOK_READONLY;
/* if (!compareStrings(_String, "relationship") ) return IDLTOK_RELATIONSHIP;	*/
   if (!compareStrings(_String, "setraises"   ) ) return IDLTOK_SETRAISES;
   if (!compareStrings(_String, "sequence"    ) ) return IDLTOK_SEQUENCE;
/* if (!compareStrings(_String, "set"         ) ) return IDLTOK_SET;		*/
   if (!compareStrings(_String, "short"       ) ) return IDLTOK_SHORT;
   if (!compareStrings(_String, "string"      ) ) return IDLTOK_STRING;
   if (!compareStrings(_String, "struct"      ) ) return IDLTOK_STRUCT;
   if (!compareStrings(_String, "supports"    ) ) return IDLTOK_SUPPORTS;
   if (!compareStrings(_String, "switch"      ) ) return IDLTOK_SWITCH;
/* if (!compareStrings(_String, "time"        ) ) return IDLTOK_TIME;		*/
/* if (!compareStrings(_String, "timestamp"   ) ) return IDLTOK_TIMESTAMP;	*/
/* if (!compareStrings(_String, "transient"   ) ) return IDLTOK_TRANSIENT;	*/
   if (!compareStrings(_String, "TRUE"        ) ) return IDLTOK_TRUE;
   if (!compareStrings(_String, "truncatable" ) ) return IDLTOK_TRUNCATABLE;
   if (!compareStrings(_String, "typedef"     ) ) return IDLTOK_TYPEDEF;
   if (!compareStrings(_String, "typeid"      ) ) return IDLTOK_TYPEID;
   if (!compareStrings(_String, "typeprefix"  ) ) return IDLTOK_TYPEPREFIX;
   if (!compareStrings(_String, "union"       ) ) return IDLTOK_UNION;
   if (!compareStrings(_String, "uses"        ) ) return IDLTOK_USES;
   if (!compareStrings(_String, "unsigned"    ) ) return IDLTOK_UNSIGNED;
   if (!compareStrings(_String, "ValueBase"   ) ) return IDLTOK_VALUEBASE;
   if (!compareStrings(_String, "valuetype"   ) ) return IDLTOK_VALUETYPE;
   if (!compareStrings(_String, "void"        ) ) return IDLTOK_VOID;
   if (!compareStrings(_String, "wchar"       ) ) return IDLTOK_WCHAR;
   if (!compareStrings(_String, "wstring"     ) ) return IDLTOK_WSTRING;

   return Identifier;
}

static int
extractIdent(char *str)
{
   int length = strlen(str);
   if (str[0] == '_') {
      /* escaped identifier, remove the preceeding _ */
      length--;
      str++;
   }
   yylval.String = (char*)os_malloc(length+1);
   os_strncpy(yylval.String, str, length);
   yylval.String[length]=0;
   return length;
}

static int
extractString(char *str)
{
   int length = strlen(str) - 1;
   yylval.String = (char*)os_malloc(length);
   os_strncpy(yylval.String, ++str, length-1);
   yylval.String[length-1]=0;
   return length;
}

static void
preprocessor_directive(char *str)
{
    int lineNumber, length;
    char *fileName, *ptr;
    c_iter pragmaItems;
    char *pragmaName, *typeName, *fieldName, *newFieldName;
    char valueString[1000];
    c_metaObject mo;

    str++; /* skip # */
    if (strncmp(str, "pragma", 6) == 0) {
        str += 7;
        pragmaItems = c_splitString(str, " ,\t\r\n");
        pragmaName = c_iterTakeFirst(pragmaItems);
        if (pragmaName && strcmp(pragmaName, "keylist") == 0) {
            typeName = c_iterTakeFirst(pragmaItems);
            if (typeName) {
                if (idl_checkKeyListTypeName(scope, typeName, yyerror) == 0) {
                    mo = c_metaObject(c_metaFindByName (scope, typeName, CQ_METAOBJECTS));
                } else {
                    mo = NULL;
                }
                valueString[0] = '\0';
                if (mo) {
                    while (c_iterLength(pragmaItems)) {
                        fieldName = c_iterTakeFirst(pragmaItems);
                        if (fieldName[0] == '_') { /* escape character, which must be removed */
                            newFieldName = (char *)os_malloc(strlen(fieldName));
                            os_strcpy(newFieldName, &(fieldName[1]));
                            os_free(fieldName);
                            fieldName = newFieldName;
                        }
                        idl_checkKeyListFieldName(mo, fieldName, yyerror);
                        strcat(valueString, fieldName);
                        if (c_iterLength(pragmaItems)) {
                            strcat(valueString, ",");
                        }
                    }
                }
                idl_keyDefAdd (idl_keyDefDefGet(), scope, typeName, valueString);
            }
        } else if(pragmaName && strcmp(pragmaName, "cats") == 0) {
            typeName = c_iterTakeFirst(pragmaItems);
            if (typeName) {
                if (idl_checkKeyListTypeName(scope, typeName, yyerror) == 0) {
                    mo = c_metaObject(c_metaFindByName (scope, typeName, CQ_METAOBJECTS));
                } else {
                    mo = NULL;
                }
                valueString[0] = '\0';
                if (mo) {
                    while (c_iterLength(pragmaItems)) {
                        fieldName = c_iterTakeFirst(pragmaItems);
                        if (fieldName[0] == '_') { /* escape character, which must be removed */
                            newFieldName = (char *)os_malloc(strlen(fieldName));
                            os_strcpy(newFieldName, &(fieldName[1]));
                            os_free(fieldName);
                            fieldName = newFieldName;
                        }
                        idl_checkSimpleFieldName(mo, fieldName, yyerror);
                        strcat(valueString, fieldName);
                        if (c_iterLength(pragmaItems)) {
                            strcat(valueString, ",");
                        }
                    }
                }
                idl_catsDefAdd (idl_catsDefDefGet(), scope, typeName, valueString);
            }
        } else if(pragmaName && strcmp(pragmaName, "stac") == 0) {
            typeName = c_iterTakeFirst(pragmaItems);
            if (typeName) {
                if (idl_checkKeyListTypeName(scope, typeName, yyerror) == 0) {
                    mo = c_metaObject(c_metaFindByName (scope, typeName, CQ_METAOBJECTS));
                } else {
                    mo = NULL;
                }
                valueString[0] = '\0';
                if (mo) {
                    while (c_iterLength(pragmaItems)) {
                        fieldName = c_iterTakeFirst(pragmaItems);
                        if (fieldName[0] == '_') { /* escape character, which must be removed */
                            newFieldName = (char *)os_malloc(strlen(fieldName));
                            os_strcpy(newFieldName, &(fieldName[1]));
                            os_free(fieldName);
                            fieldName = newFieldName;
                        }
                        idl_checkSimpleFieldName(mo, fieldName, yyerror);
                        strcat(valueString, fieldName);
                        if (c_iterLength(pragmaItems)) {
                            strcat(valueString, ",");
                        }
                    }
                }
                idl_stacDefAdd (idl_stacDefDefGet(), scope, typeName, valueString);
            }
        } else if(pragmaName && strcmp(pragmaName, "stream") == 0) {
            typeName = c_iterTakeFirst(pragmaItems);
            if (typeName && (idl_checkKeyListTypeName(scope, typeName, yyerror) == 0)) {
                idl_streamsDefAdd(idl_streamsDefDefGet(), scope, typeName);
            }
        }
    }
    /* The Visual Studio compiler generates different output */
    if (strncmp (str, "line", 4) == 0) { /* Visual Studio only */
        str = &str[4]; /* Skip the line expression */
    }
    lineNumber = strtol(str,&ptr,10);
    while ((*ptr != 0) && ((*ptr == ' ') || (*ptr == '\t'))) ptr++;
    if (*ptr == '"') {
        ptr++;
        if (*ptr != '<') {
            fileName = ptr;
        } else {
            return;
        }
    } else {
        return;
    }
    while (isprint((int)*ptr) && (*ptr != '"')) ptr++;
    length = abs((size_t)ptr - (size_t)fileName);
    if (length >= MAX_FILENAME_LENGTH) {
        fprintf(stderr,"filename <%s> exceeds max length of %d\n",fileName,MAX_FILENAME_LENGTH);
        exit(-1);
    }
    while ((*ptr != 0) && ((*ptr == ' ') || (*ptr == '\t'))) ptr++;
    if (*ptr != '"') return;
    os_strncpy(file_name,fileName,length);
    file_name[length] = 0;
    idl_fileMapAdd(idl_fileMapDefGet(), file_name);
    parser_line = lineNumber;
    parser_column = 0;
}
