
%option 8bit stack


%{
#include <stdio.h>
#include <string.h>

#include "xml_tmplExp.h"

#include <os.h>
#include <os_report.h>
#include <os_stdlib.h>

#ifdef WINCE
#undef errno
static int errno=0;
#endif

/* cfg_curState is initialized when parser is called:
 * see cfg_parse_ospl() in cfg_parser.y
 */
static int cfg_curState = -1;
static int cfg_lineno = 1;
static int cfg_column = 1;

#define plc() do {               \
        c_char *t = yytext;      \
        while (*t != 0) {        \
            if (*t == '\n') {    \
                cfg_lineno++;    \
                cfg_column = 1;  \
            }                    \
            else {               \
                cfg_column++;    \
            }                    \
            t++;                 \
        }                        \
    } while (0)

#define CFG_ENV_TOKEN_START '$'
#define CFG_ENV_TOKEN_OPEN  '{'
#define CFG_ENV_TOKEN_CLOSE '}'

static c_char *
expandEnvironment(
    const c_char *value)
{
    xml_macroAttrib macroAttrib;
    xml_streamIn    inStream;
    xml_streamOut   outStream;
    xml_macroSet    macroSet;
    xml_tmplExp     te;
    c_char          *result;
    int             expandResult;

    macroAttrib = xml_macroAttribNew (CFG_ENV_TOKEN_START,
                      CFG_ENV_TOKEN_OPEN, CFG_ENV_TOKEN_CLOSE);
    macroSet = xml_macroSetNew ();
    inStream = xml_streamInNew (value, macroAttrib);
    outStream = xml_streamOutNew(0);
    te = xml_tmplExpNew (macroSet);
    expandResult = xml_tmplExpProcessTmplToStream (te, inStream, outStream);
    xml_streamInFree (inStream);
    xml_macroSetFree(macroSet);
    xml_tmplExpFree (te);
    xml_macroAttribFree(macroAttrib);
    if (expandResult == 0) {
        result = xml_streamOutGetAndClear(outStream);
    } else {
        OS_REPORT_1(OS_ERROR, "configuration parser",
            0,
            "Failed to expand environment variable in %s",
            value);
        result = NULL;
    }
    xml_streamOutFree(outStream);
    (void)cfg_memoryClaimListAdd(cfg_mcl, result, os_free); /* ignore return value */

    return result;
}

static c_char *
extractAttributeValue(
    c_char *value)
{
    c_char *result;
    c_char *attrValue;
    int i, length;

    length = strlen(value) - 2; /* Subtract start and end quotes */
    result = cfg_memoryClaimListAdd(cfg_mcl,
        os_malloc(length + 1), os_free); /* malloc length + 1 for '\0' */
    os_strncpy(result, value + 1, length);
    result[length] = '\0';

    attrValue = expandEnvironment(result);
    cfg_memoryClaimListRemove(cfg_mcl, result);
    os_free(result);

    return attrValue;
}

static c_char *
extractEndTag(
    c_char *endTag)
{
    c_char *result;
    int i;

    /* skip '</' */
    for (i = 2; i < yyleng && endTag[i] != '>'; i++);
    if (i < yyleng) {
        result = cfg_memoryClaimListAdd(cfg_mcl,
            os_malloc(i), os_free); /* no +1 already covered by '</' */
        os_strncpy(result, endTag + 2, i - 2);
        result[i - 2] = 0;
    } else {
        result = cfg_memoryClaimListAdd(cfg_mcl, os_malloc(1), os_free);
        os_strcpy(result, "");
    }
    return result;
}

static c_char *
extractData(
    c_char *data)
{
    c_char *result;

    result = expandEnvironment(data);
    return result;
}

%}
%pointer
/*%option noyywrap*/

whitespace      [ \t\r\n]+
/** \todo Add escaping "&" to allow for example '<' in text nodes
 *        reference has following pattern:              "&#"[0-9]+";"|"&#x"[0-9a-fA-F]+";"
 */
Char            [A-Za-z\200-\377_0-9.-]
name            [A-Za-z\200-\377_]([:A-Za-z\200-\377_0-9.-]*)
data            [^<]+
comment         "<!--"([^-]|"-"[^-])*"-->"
attributevalue  \"[^"]*\"|\'[^']*\'
pidata          [^?]*

%x ST_ELEMENT
%x ST_ELCONTENT
%x ST_PI
%x ST_PISPLICE

/**
 * ST_ELEMENT: start of an element
 * ST_ELCONTENT: non-empty element
 * ST_PI: state for generic processing instructions.
 * ST_PISPLICE: state for SPLICE processing instructions.
 */

%%
<INITIAL,ST_ELEMENT,ST_PI,ST_PISPLICE>{whitespace}      { plc(); /* skip */ }

<ST_ELEMENT>"/>"{whitespace}?               {
                                              plc();
                                              cfg_curState = yy_top_state();
                                              yy_pop_state();
                                              return TOK_EMPTYCLOSE;
                                            }
<ST_ELEMENT,ST_PI,ST_PISPLICE>"="           {
                                              plc();
                                              return TOK_EQUAL;
                                            }
<ST_ELEMENT>">"{whitespace}?                {
                                              plc();
                                              yy_push_state(cfg_curState);
                                              cfg_curState = ST_ELCONTENT;
                                              BEGIN(ST_ELCONTENT);
                                              return TOK_CLOSE;
                                            }
<ST_ELEMENT,ST_PI,ST_PISPLICE>{name}        {
                                              plc();
                                              yylval.str = cfg_memoryClaimListAdd(cfg_mcl,
                                                  os_malloc(strlen(yytext) + 1), os_free);
                                              os_strcpy(yylval.str, yytext);
                                              return TOK_NAME;
                                            }
<ST_ELEMENT,ST_PISPLICE>{attributevalue}    {
                                              plc();
                                              yylval.str = extractAttributeValue(yytext);
                                              return TOK_VALUE;
                                            }

<INITIAL,ST_ELCONTENT>"<"{name}             {
                                              plc();
                                              yy_push_state(cfg_curState);
                                              cfg_curState = ST_ELEMENT;
                                              BEGIN(ST_ELEMENT);
                                              yylval.str = cfg_memoryClaimListAdd(cfg_mcl,
                                                  os_malloc(strlen(yytext + 1) + 1), os_free);
                                              os_strcpy(yylval.str, yytext + 1);
                                              return TOK_START;
                                            }
<ST_ELCONTENT>"</"{name}">"{whitespace}?    {
                                              plc();
                                              yy_pop_state(); /* ST_ELEMENT, since element
                                                                 has finished */
                                              cfg_curState = yy_top_state();
                                              yy_pop_state();
                                              yylval.str = extractEndTag(yytext);
                                              return TOK_END;
                                            }
<INITIAL,ST_ELCONTENT>{comment}             {
                                              plc();
                                              yylval.str = cfg_memoryClaimListAdd(cfg_mcl,
                                                  os_malloc(strlen(yytext) + 1), os_free);
                                              os_strcpy(yylval.str, yytext);
                                              return TOK_COMMENT;
                                            }

<ST_ELCONTENT>{data}                        {
                                              plc();
                                              yylval.str = extractData(yytext);
                                              return TOK_DATA;
                                            }

<INITIAL,ST_ELCONTENT>"<?"{name}            {
                                              plc();
                                              yylval.str = cfg_memoryClaimListAdd(cfg_mcl,
                                                  os_malloc(strlen(yytext + 2) + 1), os_free);
                                              os_strcpy(yylval.str, yytext + 2);
                                              yy_push_state(cfg_curState);
                                              if (strcmp(yylval.str, "OpenSplice-PI") == 0) {
                                                  cfg_curState = ST_PISPLICE;
                                              } else {
                                                  cfg_curState = ST_PI;
                                              }
                                              BEGIN(cfg_curState);
                                              return TOK_PISTART;
                                            }

<ST_PI>{pidata}                             {
                                              plc();
                                              yylval.str = cfg_memoryClaimListAdd(cfg_mcl,
                                                  os_malloc(strlen(yytext) + 1), os_free);
                                              os_strcpy(yylval.str, yytext);
                                              return TOK_PIDATA;
                                            }

<ST_PI,ST_PISPLICE>"?>"                     {
                                              plc();
                                              cfg_curState = yy_top_state();
                                              yy_pop_state();
                                              return TOK_PIEND;
                                            }

.                                           {
                                            }
