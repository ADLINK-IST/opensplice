%{
#include <string.h>
#include <stdio.h>

#include <os.h>
#include <os_report.h>

#include <ut_stack.h>

#include "mcl.h"

/* include configuration framework */
#include "cf_element.h"
#include "cf_attribute.h"
#include "cf_data.h"

#ifdef _WIN32
#define YY_NO_UNISTD_H
#endif

#define YY_NEVER_INTERACTIVE 1

#define yyin                 cfg_scanner_yyin
#define yyout                cfg_scanner_yyout
#define yyparse              cfg_scanner_yyparse
#define yychar               cfg_scanner_yychar
#define yyerror              cfg_scanner_yyerror
#define yylex                cfg_scanner_yylex
#define yylval               cfg_scanner_yylval
#define yyrestart            cfg_scanner_yyrestart
#define yywrap               cfg_scanner_yywrap
#define yytext               cfg_scanner_yytext
#define yynerrs              cfg_scanner_yynerrs
#define yyleng               cfg_scanner_yyleng
#define yy_scan_string       cfg_scanner_yy_scan_string
#define yy_scan_buffer       cfg_scanner_yy_scan_buffer
#define yy_init_buffer       cfg_scanner_yy_init_buffer
#define yy_flush_buffer      cfg_scanner_yy_flush_buffer
#define yy_switch_to_buffer  cfg_scanner_yy_switch_to_buffer
#define yy_delete_buffer     cfg_scanner_yy_delete_buffer
#define yy_create_buffer     cfg_scanner_yy_create_buffer
#define yy_load_buffer_state cfg_scanner_yy_load_buffer_state
#define yy_scan_bytes        cfg_scanner_yy_scan_bytes

#define CFG_MAX_MSG 128

#ifdef CONF_PARSER_NOFILESYS
const char *ospl_xml_data_ptr = NULL;
unsigned int ospl_xml_data_ptr_size = 0;
extern const char ospl_xml_data[];
extern const unsigned int ospl_xml_data_size;
const char *xmlptr;
size_t xmlRemain;

extern const char ospl_xml_data[];
extern const unsigned int ospl_xml_data_size;

#define YY_INPUT(buf, result, max_size) \
   { \
      result = (max_size < xmlRemain) ? max_size : xmlRemain; \
      if ( result > 0 ) \
      { \
         memcpy( buf, xmlptr, result ); \
	 xmlRemain -= result; \
	 xmlptr += result; \
      } \
   }

#endif


C_CLASS(cfg_uri);

/* function prototypes */
int yylex();
int yyerror (char *text);
static void cfg_processInstruction(c_char *name, cf_attribute attr);

extern FILE *yyin;

static cf_element cfg_topLevel;
static cfg_memoryClaimList cfg_mcl;
static ut_stack cfg_includeStack;

static c_char cfg_errMsg[CFG_MAX_MSG];
static cfg_uri cfg_curUri;


%}

%union {
    char         *str;
    cf_element   el;
    cf_attribute attr;
    c_iter       list;
};

%token TOK_ENDDEF TOK_EQUAL TOK_EMPTYCLOSE TOK_CLOSE TOK_PIEND
%token <str> TOK_NAME TOK_VALUE TOK_DATA TOK_COMMENT TOK_START TOK_END TOK_PISTART TOK_PIDATA

%type <attr> Attribute
%type <list> AttrListOpt Content EmptyOrContent
%type <el> Element

%%

Document:
   Prolog Element MiscListOpt {}
 ;

Prolog:
   MiscListOpt {} /* No versioning and encoding supported (yet) */
 ;

MiscListOpt:
   MiscListOpt Misc {}
 | /*empty*/
 ;

Misc:
   TOK_COMMENT {
                 cfg_memoryClaimListRemove(cfg_mcl, $1);
                 os_free($1);
               }
 | Pi
 ;

Element:
   TOK_START
   AttrListOpt
   EmptyOrContent              {
                                   cf_attribute a;
                                   cf_node n;
                                   c_char *closeTag;

                                   if (c_iterLength($3) > 0) {
                                       closeTag = (c_char *)c_iterTakeFirst($3);
                                       if (strcmp($1, closeTag) != 0) {
                                           snprintf(cfg_errMsg, CFG_MAX_MSG,
                                                "Close tag '%s' does not match start tag '%s'",
                                                closeTag, $1);
                                           cfg_memoryClaimListRemove(cfg_mcl, closeTag);
                                           os_free(closeTag);
                                           yyerror(cfg_errMsg);
                                           YYERROR;
                                       } else {
                                           cfg_memoryClaimListRemove(cfg_mcl, closeTag);
                                           os_free(closeTag);
                                       }
                                   } /* else empty element */

                                   $$ = cfg_memoryClaimListAdd(cfg_mcl,
                                       cf_elementNew($1),
                                       (cfg_memoryClaimListFreeFunc)cf_nodeFree);
                                   a = c_iterTakeFirst($2);
                                   while (a != NULL) {
                                       cfg_memoryClaimListRemove(cfg_mcl, a);
                                       cf_elementAddAttribute($$, a);
                                       a = c_iterTakeFirst($2);
                                   }
                                   cfg_memoryClaimListRemove(cfg_mcl, $2);
                                   c_iterFree($2);

                                   n = c_iterTakeFirst($3);
                                   while (n != NULL) {
                                       cfg_memoryClaimListRemove(cfg_mcl, n);
                                       cf_elementAddChild($$, n);
                                       n = c_iterTakeFirst($3);
                                   }
                                   cfg_memoryClaimListRemove(cfg_mcl, $3);
                                   c_iterFree($3);

                                   cfg_topLevel = $$;
                                   cfg_memoryClaimListRemove(cfg_mcl, $1);
                                   os_free($1);
                               }
 ;

EmptyOrContent:
   TOK_EMPTYCLOSE              {
                                 $$ = cfg_memoryClaimListAdd(cfg_mcl,
                                     c_iterNew(NULL),
                                     (cfg_memoryClaimListFreeFunc)c_iterFree);
                               }
 | TOK_CLOSE
   Content TOK_END             {
                                 cfg_memoryClaimListRemove(cfg_mcl, $2);
                                 $$ = cfg_memoryClaimListAdd(cfg_mcl,
                                     c_iterInsert($2, $3),
                                     (cfg_memoryClaimListFreeFunc)c_iterFree);
                               }
 ;

Content:
   Content TOK_DATA             { cf_data t;
                                  c_value value;

                                  value = c_stringValue($2);
                                  t = cfg_memoryClaimListAdd(cfg_mcl,
                                      cf_dataNew(value),
                                      (cfg_memoryClaimListFreeFunc)cf_nodeFree);
                                  $$ = c_iterAppend($1, t);
                                  cfg_memoryClaimListRemove(cfg_mcl, value.is.String);
                                  os_free(value.is.String);
                                }
 | Content Misc                 {}
 | Content Element              { $$ = c_iterAppend($1, $2); }
 | Content Pi
 | /*empty*/                    {
                                  $$ = cfg_memoryClaimListAdd(cfg_mcl,
                                      c_iterNew(NULL),
                                      (cfg_memoryClaimListFreeFunc)c_iterFree);
                                }
 ;

Pi:
   TOK_PISTART TOK_PIDATA TOK_PIEND   {
					 /* skip processing instruction */
			                 cfg_memoryClaimListRemove(cfg_mcl, $1);
			                 cfg_memoryClaimListRemove(cfg_mcl, $2);
			                 os_free($1);
			                 os_free($2);
				      }
 | TOK_PISTART TOK_PIEND              {
					 /* skip processing instruction */
			                 cfg_memoryClaimListRemove(cfg_mcl, $1);
			                 os_free($1);
				      }
 | TOK_PISTART Attribute TOK_PIEND    {
                                        cfg_processInstruction($1, $2);
                                        cfg_memoryClaimListRemove(cfg_mcl, $1);
                                        cfg_memoryClaimListRemove(cfg_mcl, $2);
                                        os_free($1);
                                        cf_nodeFree((cf_node)$2);
                                      }
 ;

AttrListOpt:
   AttrListOpt Attribute        { $$ = c_iterAppend($1, $2); }
 | /*empty*/                    {
                                  $$ = cfg_memoryClaimListAdd(cfg_mcl,
                                      c_iterNew(NULL),
                                      (cfg_memoryClaimListFreeFunc)c_iterFree);
                                }
 ;

Attribute:
   TOK_NAME                         {
                                      $$ = cfg_memoryClaimListAdd(cfg_mcl,
                                          cf_attributeNew($1, c_undefinedValue()),
                                          (cfg_memoryClaimListFreeFunc)cf_nodeFree);
                                      cfg_memoryClaimListRemove(cfg_mcl, $1);
                                      os_free($1);
                                    }
 | TOK_NAME TOK_EQUAL TOK_VALUE     {
                                      $$ = cfg_memoryClaimListAdd(cfg_mcl,
                                          cf_attributeNew($1, c_stringValue($3)),
                                          (cfg_memoryClaimListFreeFunc)cf_nodeFree);
                                      cfg_memoryClaimListRemove(cfg_mcl, $1);
                                      os_free($1);
                                      cfg_memoryClaimListRemove(cfg_mcl, $3);
                                      os_free($3);
                                    }
 ;

%%
#include "cfg_scanner.h"
#include "cfg_parser.h"

#define CFG_PI_NAME    "OpenSplice-PI"
#define CFG_WHITESPACE " \t\r\n"

#define CFG_PI_TOKEN_EQUAL "="
#define CFG_PI_TOKEN_INCLUDE "include"

/********** CFG_URI should become a seperate module *****************/
#define URI_FILESCHEMA "file://"

#define cfg_uri(o) ((cfg_uri)(o))
#define cfg_uri_file(o) ((cfg_uri_file)(o))

C_CLASS(cfg_uri_file);

enum cfg_uri_kind {
    CFG_URI_KIND_UNDEFINED,
    CFG_URI_KIND_FILE
};

C_STRUCT(cfg_uri) {
    enum cfg_uri_kind kind;
    YY_BUFFER_STATE buf;
};

C_STRUCT(cfg_uri_file) {
    C_EXTENDS(cfg_uri);
    FILE *f;
};

static cfg_uri
cfg_uriOpen(
    const c_char *uriStr)
{
    cfg_uri uri;
#ifdef CONF_PARSER_NOFILESYS
    /* ospl_xml_data and ospl_xml_data_size are generated by processing ospl.xml
       with gbin2c */
    if (ospl_xml_data_ptr == NULL)
    {
        xmlptr = ospl_xml_data;
        xmlRemain = ospl_xml_data_size;
    }
    else
    {
        xmlptr = ospl_xml_data_ptr;
        xmlRemain = ospl_xml_data_ptr_size;
    }

    uri = os_malloc(C_SIZEOF(cfg_uri_file));
    cfg_uri_file(uri)->f=0;
    uri->kind = CFG_URI_KIND_FILE;
    uri->buf = yy_create_buffer(cfg_uri_file(uri)->f, YY_BUF_SIZE);
#else
    char *filename;

    uri = NULL;
    if ((uriStr != NULL) && (*uriStr == '\"')) {
        uriStr++;
    }
    if ((uriStr != NULL) &&
        (strncmp(uriStr, URI_FILESCHEMA, strlen(URI_FILESCHEMA)) == 0)) {
        uri = os_malloc(C_SIZEOF(cfg_uri_file));
        if (uri != NULL) {
            uri->kind = CFG_URI_KIND_FILE;
            uri->buf = ((YY_BUFFER_STATE)0);
            filename = os_fileNormalize((char *)((os_size_t)uriStr + strlen(URI_FILESCHEMA)));
            cfg_uri_file(uri)->f = fopen(filename, "r");
            if (cfg_uri_file(uri)->f) {
                uri->buf = yy_create_buffer(cfg_uri_file(uri)->f, YY_BUF_SIZE);
            } else {
                OS_REPORT_1(OS_ERROR, "configuration parser", 0, "Could not open %s",uriStr);
                os_free(uri);
                uri = NULL;
            }
            os_free(filename);
        }
    }
#endif
    return uri;
}

static void
cfg_uriClose(
    cfg_uri uri)
{
#ifdef CONF_PARSER_NOFILESYS
    yy_delete_buffer(uri->buf);
    os_free(uri);
#else
    if (uri != NULL) {
        switch(uri->kind) {
        case CFG_URI_KIND_FILE:
            fclose(cfg_uri_file(uri)->f);
        break;
        case CFG_URI_KIND_UNDEFINED:
        default:
        break;
        }
        yy_delete_buffer(uri->buf);
        os_free(uri);
    }
#endif
}

static YY_BUFFER_STATE
cfg_uriGetParseBuffer(
    cfg_uri uri)
{
    YY_BUFFER_STATE buf;

    buf = ((YY_BUFFER_STATE)0);
    if (uri != NULL) {
        buf = uri->buf;
    }
    return buf;
}

/********** CFG_URI should become a seperate module *****************/


int
yyerror ( char *text )
{
   OS_REPORT_4(OS_ERROR, "configuration parser", 0,
               "%s near the token %s (line: %d, column: %d)",
               text, yytext, cfg_lineno, cfg_column);

    return -1;
}

int
yywrap()
{
    int cont;

    /* Check whether to process more buffers */
    if (ut_stackIsEmpty(cfg_includeStack)) {
        cont = 1;
    } else {
        cont = 0;
        /* close current buffer and continue with previous */
        cfg_uriClose(cfg_curUri);
        cfg_curUri = ut_stackPop(cfg_includeStack);
        yy_switch_to_buffer(cfg_uriGetParseBuffer(cfg_curUri));
    }

    return cont;
}

static void
cfg_processInstruction(
    c_char *name,
    cf_attribute attr)
{
    c_value value;
    int error = 1;

    value = cf_attributeValue(attr);
    if (value.kind == V_STRING) {
        /* include configuration from uri */
        ut_stackPush(cfg_includeStack, cfg_curUri);
        cfg_curUri = cfg_uriOpen(value.is.String);
        if (cfg_curUri != NULL) {
            error = 0;
            yy_switch_to_buffer(cfg_uriGetParseBuffer(cfg_curUri));
        } else {
            cfg_curUri = ut_stackPop(cfg_includeStack);
        }
    }

    if (error) {
        OS_REPORT(OS_WARNING, "configuration parser", 0,
                    "Unknown processing instruction");
    }
}

cfgprs_status
cfg_parse_str (
    const char *str,
    cf_element *spliceElement)
{
    cfgprs_status s;
    YY_BUFFER_STATE str_buffer;

    /* Scanner initialisation */
    cfg_curState = INITIAL;
    cfg_lineno = 1;
    cfg_column = 1;

    cfg_topLevel = NULL;
    cfg_mcl = cfg_memoryClaimListNew();
    cfg_includeStack = ut_stackNew(UT_STACK_DEFAULT_INC);

    if (str == NULL) {
        s = CFGPRS_NO_INPUT;
    } else {
        str_buffer = yy_scan_string(str);
        yy_switch_to_buffer(str_buffer);
        if (yyparse() == 0) {
            s = CFGPRS_OK;
        } else {
            s = CFGPRS_ERROR;
        }
        yy_delete_buffer(str_buffer);
    }

    if (s == CFGPRS_OK) {
        cfg_memoryClaimListRemove(cfg_mcl, cfg_topLevel);
        *spliceElement = cfg_topLevel;
    } else {
        cfg_memoryClaimListReleaseAll(cfg_mcl);
    }

    assert(cfg_memoryClaimListClaimCount(cfg_mcl) == 0);
    cfg_memoryClaimListFree(cfg_mcl);
    cfg_mcl = NULL;

    assert(ut_stackIsEmpty(cfg_includeStack));
    ut_stackFree(cfg_includeStack);
    cfg_includeStack = NULL;

    return s;
}

cfgprs_status
cfg_parse_ospl (
    const char *uri,
    cf_element *spliceElement)
{
    cfgprs_status s;
    cfg_uri u;

    /* Scanner initialisation */
    cfg_curState = INITIAL;
    cfg_lineno = 1;
    cfg_column = 1;

    s = CFGPRS_OK;
    cfg_topLevel = NULL;
    cfg_mcl = cfg_memoryClaimListNew();
    cfg_includeStack = ut_stackNew(UT_STACK_DEFAULT_INC);

#ifndef CONF_PARSER_NOFILESYS
    if (uri == NULL || strlen(uri) == 0 ) {
        s = CFGPRS_NO_INPUT;
    }
    else
#endif 
    {
        /* Check if uri contains file:// schema! */
        cfg_curUri = cfg_uriOpen(uri);
        if (cfg_curUri != NULL) {
            BEGIN(INITIAL);
            yy_switch_to_buffer(cfg_uriGetParseBuffer(cfg_curUri));
            if (yyparse() != 0) {
                s = CFGPRS_ERROR;
                yy_flush_buffer(YY_CURRENT_BUFFER);
                /* empty stack */
                if (!ut_stackIsEmpty(cfg_includeStack)) {
                    do {
                        u = ut_stackPop(cfg_includeStack);
                        cfg_uriClose(u);
                    } while (!ut_stackIsEmpty(cfg_includeStack));
                }
            }
            cfg_uriClose(cfg_curUri);
        } else {
            s = CFGPRS_NO_INPUT;
        }
    }
    cfg_curUri = NULL;

    if (s == CFGPRS_OK) {
        cfg_memoryClaimListRemove(cfg_mcl, cfg_topLevel);
        *spliceElement = cfg_topLevel;
    } else {
        cfg_memoryClaimListReleaseAll(cfg_mcl);
    }

    assert(cfg_memoryClaimListClaimCount(cfg_mcl) == 0);
    cfg_memoryClaimListFree(cfg_mcl);
    cfg_mcl = NULL;

    assert(ut_stackIsEmpty(cfg_includeStack));
    ut_stackFree(cfg_includeStack);
    cfg_includeStack = NULL;

    return s;
}

