/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "c_base.h"
#include "sd_list.h"
#include "sd_string.h"
#include "sd_misc.h"
#include "sd_xmlNode.h"
#include "sd_stringsXML.h"
#include "sd_xmlParser.h"
#include "sd__resultCodes.h"
#include "sd__resultCodesXML.h"
#include "sd_errorReport.h"

#include "os_heap.h"
#include "os_stdlib.h"

/* for tracing, change to #define TRACE(p) p */
#define TRACE(p)

#define IS_ALPHA(c)  ((((c>='a') && (c<='z')) || ((c>='A') && (c<='Z')))?TRUE:FALSE)
#define IS_QUOTE(c)  (((c=='\"') || (c=='\''))?TRUE:FALSE)

#define SET_XMLPARSER_ERROR(parser, errorType) \
    sd_xmlParserSetError(parser, SD_ERRNO(errorType),  SD_MESSAGE(errorType));

typedef enum {
    SD_XML_TOKEN_ERROR,
    SD_XML_TOKEN_BTAG,
    SD_XML_TOKEN_ETAG,
    SD_XML_TOKEN_STAG,
    SD_XML_TOKEN_DATA,
    SD_XML_TOKEN_EOF
} sd_xmlTokenKind;

typedef enum {
    SD_XML_STATE_INITIAL,
    SD_XML_STATE_BTAG,
    SD_XML_STATE_ETAG,
    SD_XML_STATE_DATA,
    SD_XML_STATE_READY,
    SD_XML_STATE_ERROR
} sd_xmlStateKind;

typedef struct sd_xmlParserEntry_s *sd_xmlParserEntry;
struct sd_xmlParserEntry_s {
    sd_xmlParserEntry   next;
    sd_xmlParserElement element;
};

typedef sd_xmlParserEntry *sd_xmlParserStack;


C_STRUCT(sd_xmlParser) {
    sd_xmlParserCallback  callback;
    void                 *argument;
    sd_xmlStateKind       state;
    c_char               *value;
    const c_char         *xmlString;
    const c_char         *readPtr;
    sd_xmlParserEntry     stack;
    sd_errorReport        errorInfo;
};


static sd_xmlTokenKind
sd_xmlStringGetToken (
    c_char **str,
    c_char **value);

static c_bool
sd_xmlStringGetIdent (
    c_char **str,
    c_char **ident);

static c_bool
sd_xmlStringSkipAssign (
    c_char **str);


static c_bool
sd_xmlStringGetQuotedString (
    c_char **str,
    c_char **value);

static c_bool
sd_xmlStringProcSpecial (
    c_char *str,
    c_char **retval);


static c_bool
sd_xmlStringIsEmpty (
    c_char **str);

static void
sd_xmlParserStackPush (
    sd_xmlParserStack   stack,
    sd_xmlParserElement element)
{
    sd_xmlParserEntry entry = (sd_xmlParserEntry) os_malloc(sizeof(*entry));

    assert(stack);

    if ( entry ) {
        entry->element = element;
        entry->next    = *stack;
        *stack         = entry;
    }
}

static sd_xmlParserElement
sd_xmlParserStackPop (
    sd_xmlParserStack stack)
{
    sd_xmlParserEntry   entry   = NULL;
    sd_xmlParserElement element = NULL;

    assert(stack);

    if ( *stack ) {
        entry = *stack;
        *stack = entry->next;
        element = entry->element;
        os_free(entry);
    }

    return element;
}

static sd_xmlParserElement
sd_xmlParserStackPeek (
    sd_xmlParserStack stack)
{
    sd_xmlParserEntry   entry   = NULL;
    sd_xmlParserElement element = NULL;

    assert(stack);

    if ( *stack ) {
        entry = *stack;
        element = entry->element;
    }

    return element;
}

static sd_xmlParserElement
sd_xmlParserStackPeek2 (
    sd_xmlParserStack stack)
{
    sd_xmlParserEntry   entry   = NULL;
    sd_xmlParserElement element = NULL;

    assert(stack);

    if ( *stack ) {
        entry = *stack;
        entry = entry->next;
        if ( entry ) {
            element = entry->element;
        }
    }

    return element;
}

static sd_xmlParserAttribute
sd_xmlParserAttributeNew ( void )
{
    sd_xmlParserAttribute attribute;

    attribute = (sd_xmlParserAttribute) os_malloc(C_SIZEOF(sd_xmlParserAttribute));
    if ( attribute ) {
        attribute->name  = NULL;
        attribute->value = NULL;
    }

    return attribute;
}

static void
sd_xmlParserAttributeFree (
    sd_xmlParserAttribute attribute)
{
    assert(attribute);

    if ( attribute ) {
        if ( attribute->name ) {
            os_free(attribute->name);
        }
        if ( attribute->value ) {
            os_free(attribute->value);
        }
        os_free(attribute);
    }
}

static sd_xmlParserElement
sd_xmlParserElementNew ( void )
{
    sd_xmlParserElement element;

    element = (sd_xmlParserElement) os_malloc(C_SIZEOF(sd_xmlParserElement));
    if ( element ) {
        memset(element, 0, C_SIZEOF(sd_xmlParserElement));
    }

    return element;
}

static void
sd_xmlParserElementFree (
    sd_xmlParserElement element)
{
    assert(element);

    if ( element ) {
        if ( element->name ) {
            os_free(element->name);
        }
        if ( element->data ) {
            os_free(element->data);
        }
        if ( element->attributes ) {
            sd_xmlParserAttribute attribute;

            attribute = (sd_xmlParserAttribute) sd_listTakeFirst(element->attributes);
            while ( attribute ) {
                sd_xmlParserAttributeFree(attribute);
                attribute = (sd_xmlParserAttribute) sd_listTakeFirst(element->attributes);
            }
            sd_listFree(element->attributes);
        }

        os_free(element);
    }
}


static sd_xmlParserAttribute
sd_xmlParseAttribute (
    sd_xmlParser  parser,
    c_char      **str)
{
    sd_xmlParserAttribute attribute;

    attribute = sd_xmlParserAttributeNew();
    if ( attribute ) {
        if ( sd_xmlStringGetIdent(str, &attribute->name) ) {
            if ( sd_xmlStringSkipAssign(str) ) {
                if ( !sd_xmlStringGetQuotedString(str, &attribute->value) ) {
                    sd_xmlParserAttributeFree(attribute);
                    attribute = NULL;
                    SET_XMLPARSER_ERROR(parser, INVALID_XML_FORMAT);
                }
            } else {
                sd_xmlParserAttributeFree(attribute);
                attribute = NULL;
                SET_XMLPARSER_ERROR(parser, INVALID_XML_FORMAT);
            }
        } else {
            SET_XMLPARSER_ERROR(parser, INVALID_XML_FORMAT);
        }
    }

    return attribute;
}

static sd_xmlParserElement
sd_xmlParseElement (
    sd_xmlParser  parser,
    c_char      **str)
{
    sd_xmlParserElement   element;
    sd_xmlParserAttribute attribute;
    c_bool                ready   = TRUE;
    c_bool                noError = TRUE;

    element = sd_xmlParserElementNew();

    if ( element ) {
        if ( sd_xmlStringGetIdent(str, &element->name) ) {
            if ( !sd_xmlStringIsEmpty(str) ) {
                element->attributes = sd_listNew();
                if ( element->attributes ) {
                    ready = FALSE;
                } else {
                    noError = FALSE;
                }
            } else {
                ready = TRUE;
            }
        } else {
            SET_XMLPARSER_ERROR(parser, INVALID_XML_FORMAT);
        }
    }

    while ( !ready && noError ) {
        if ( !sd_xmlStringIsEmpty(str) ) {
            attribute = sd_xmlParseAttribute(parser, str);
            if ( attribute ) {
                sd_listAppend(element->attributes, attribute);
            } else {
                noError = FALSE;
            }
        } else {
            ready = TRUE;
        }
    }

    if ( !noError ) {
        sd_xmlParserElementFree(element);
        element = NULL;
    }

    return element;
}

static c_bool
sd_xmlParseData (
    sd_xmlParser  parser,
    c_char       *str)
{
    c_bool              result = FALSE;
    sd_xmlParserElement element;

    assert(parser);

    element = sd_xmlParserStackPeek(&parser->stack);
    assert(element);

    if ( sd_xmlStringProcSpecial(str, &element->data) ) {
        result = TRUE;
    } else {
        SET_XMLPARSER_ERROR(parser, INVALID_DATA_FORMAT);
    }

    return result;
}


static sd_xmlParser
sd_xmlParserNew (
     const c_char         *xmlString,
     sd_xmlParserCallback  callback,
     void                 *argument)
{
    sd_xmlParser parser;

    parser = (sd_xmlParser)os_malloc(C_SIZEOF(sd_xmlParser));

    if ( parser ) {
        parser->callback    = callback;
        parser->argument    = argument;
        parser->state       = SD_XML_STATE_INITIAL;
        parser->value       = NULL;
        parser->xmlString   = (char *)xmlString;
        parser->readPtr     = xmlString;
        parser->errorInfo   = NULL;
        parser->stack       = NULL;
    }

    return parser;
}

static void
sd_xmlParserFree (
    sd_xmlParser parser)
{
    sd_xmlParserElement element;

    assert(parser);

    if ( parser->value ) {
        os_free(parser->value);
    }

    element = sd_xmlParserStackPop(&parser->stack);
    while ( element ) {
        sd_xmlParserElementFree(element);
        element = sd_xmlParserStackPop(&parser->stack);
    }

    os_free(parser);
}



static c_bool
sd_xmlParserMatchCurrent (
    sd_xmlParser        parser,
    sd_xmlParserElement element)
{
    c_bool              result = FALSE;
    sd_xmlParserElement current;

    assert(parser);
    assert(element);
    assert(element->name);

    current = sd_xmlParserStackPeek(&parser->stack);
    if ( current ) {
        assert(current->name);
        if ( strcmp(current->name, element->name) == 0 ) {
            result = TRUE;
        }
    }
    return result;
}

static c_bool
sd_xmlParserGetToken (
    sd_xmlParser parser)
{
    c_bool               result = TRUE;
    sd_xmlTokenKind      token;
    sd_xmlParserElement  element;
    c_char              *str;

    if ( parser->value ) {
        os_free(parser->value);
        parser->value = NULL;
    }

    token = sd_xmlStringGetToken((char**)&parser->readPtr, &parser->value);

    switch ( token ) {
        case SD_XML_TOKEN_BTAG:
            if ( parser->state != SD_XML_STATE_DATA ) {
                str = parser->value;
                TRACE(printf("<%s>\n", str));
                element = sd_xmlParseElement(parser, &str);
                if ( element ) {
                    sd_xmlParserStackPush(&parser->stack, element);
                    parser->state = SD_XML_STATE_BTAG;
                    result = parser->callback(SD_XML_PARSER_KIND_ELEMENT_START,
                                          element, parser->argument, parser);
                } else {
                    parser->state = SD_XML_STATE_ERROR;
                    token = SD_XML_TOKEN_ERROR;
                    SET_XMLPARSER_ERROR(parser, INVALID_TAG_FORMAT)
                }
            } else {
                parser->state = SD_XML_STATE_ERROR;
                token = SD_XML_TOKEN_ERROR;
                SET_XMLPARSER_ERROR(parser, UNEXPECTED_OPENING_TAG)
            }
            break;
        case SD_XML_TOKEN_ETAG:
            str = parser->value;
            TRACE(printf("</%s>\n", str));
            element = sd_xmlParseElement(parser, &str);
            if ( element ) {
                if ( sd_xmlParserMatchCurrent(parser, element) ) {
                    sd_xmlParserElementFree(sd_xmlParserStackPop(&parser->stack));
                    parser->state = SD_XML_STATE_ETAG;
                    result = parser->callback(SD_XML_PARSER_KIND_ELEMENT_END,
                                          element, parser->argument, parser);
                } else {
                    parser->state = SD_XML_STATE_ERROR;
                    token = SD_XML_TOKEN_ERROR;
                    SET_XMLPARSER_ERROR(parser, UNEXPECTED_CLOSING_TAG)
                }
                sd_xmlParserElementFree(element);
            } else {
                parser->state = SD_XML_STATE_ERROR;
                token = SD_XML_TOKEN_ERROR;
                SET_XMLPARSER_ERROR(parser, INVALID_TAG_FORMAT)
            }
            break;
        case SD_XML_TOKEN_STAG:
            if ( parser->state != SD_XML_STATE_DATA ) {
                str = parser->value;
                TRACE(printf("<%s/>\n", str));
                element = sd_xmlParseElement(parser, &str);
                if ( element ) {
                    result = parser->callback(SD_XML_PARSER_KIND_ELEMENT_START,
                                          element, parser->argument, parser);
                    result = parser->callback(SD_XML_PARSER_KIND_ELEMENT_END,
                                          element, parser->argument, parser);
                    sd_xmlParserElementFree(element);
                } else {
                    parser->state = SD_XML_STATE_ERROR;
                    token = SD_XML_TOKEN_ERROR;
                    SET_XMLPARSER_ERROR(parser, INVALID_TAG_FORMAT)
                }
            } else {
                parser->state = SD_XML_STATE_ERROR;
                token = SD_XML_TOKEN_ERROR;
                SET_XMLPARSER_ERROR(parser, UNEXPECTED_OPENING_TAG)
            }
            break;
        case SD_XML_TOKEN_DATA:
            if ( parser->state == SD_XML_STATE_BTAG ) {
                if ( sd_xmlParseData(parser, parser->value) ) {
                    parser->state = SD_XML_STATE_DATA;
                    result = parser->callback(SD_XML_PARSER_KIND_DATA,
                                          NULL, parser->argument, parser);
                } else {
                    parser->state = SD_XML_STATE_ERROR;
                    token = SD_XML_TOKEN_ERROR;
                    SET_XMLPARSER_ERROR(parser, INVALID_DATA_FORMAT)
                }
            } else {
                parser->state = SD_XML_STATE_ERROR;
                token = SD_XML_TOKEN_ERROR;
                SET_XMLPARSER_ERROR(parser, UNEXPECTED_DATA)
            }
            break;
        case SD_XML_TOKEN_EOF:
            if ( !parser->stack ) {
                parser->state = SD_XML_STATE_READY;
                result = TRUE;
            } else {
                parser->state = SD_XML_STATE_ERROR;
                token = SD_XML_TOKEN_ERROR;
                SET_XMLPARSER_ERROR(parser, INVALID_XML_FORMAT)
            }
            break;
        case SD_XML_TOKEN_ERROR:
            parser->state = SD_XML_STATE_ERROR;
            SET_XMLPARSER_ERROR(parser, INVALID_XML_FORMAT)
            break;
    }

    if ( result && (parser->state != SD_XML_STATE_ERROR) ) {
        result = TRUE;
    } else {
        result = FALSE;
    }

    return result;
}

c_bool
sd_xmlParserParse (
     const c_char         *xmlString,
     sd_xmlParserCallback  callback,
     void                 *argument,
     sd_errorReport       *errorInfo)
{
    c_bool          result = TRUE;
    sd_xmlParser    parser;

    assert(xmlString);

    parser = sd_xmlParserNew(xmlString, callback, argument);
    if ( parser ) {
        while ( result && (parser->state != SD_XML_STATE_READY) ) {
            result = sd_xmlParserGetToken(parser);
        }
        if ( !result ) {
            *errorInfo = parser->errorInfo;
        }
        sd_xmlParserFree(parser);
    } else {
        result = FALSE;
    }

    return result;
}



#define SD_AMPERSAND       '&'
#define SD_UNDERSCORE      '_'
#define SD_QUOTE           '\"'
#define SD_SINGLE_QUOTE    '\''
#define SD_ASSIGN          '='

#define SD_CHARS_CHAR       "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define SD_CHARS_DIGIT      "1234567890"
#define SD_CHARS_SPECIAL    "~!@$^&*()_+-={}|:;?,.\\"
#define SD_CHARS_ESCAPED    "&#;"
#define SD_CHARS_SPACES     " \t\n"
#define SD_CHARS_QUOTES     "\"'"
#define SD_CHARS_IDENT      SD_CHARS_CHAR SD_CHARS_DIGIT "_"
#define SD_CHARS_STRING     SD_CHARS_CHAR SD_CHARS_DIGIT SD_CHARS_SPECIAL SD_SKIP_SPACES
#define SD_CHARS_DATA       SD_CHARS_STRING SD_CHARS_QUOTES SD_CHARS_ESCAPED

#define SD_SKIP_SPACES      SD_CHARS_SPACES
#define SD_SKIP_CHAR        SD_CHARS_CHAR SD_CHARS_DIGIT SD_CHARS_SPECIAL SD_SKIP_SPACES
#define SD_SKIP_INT         SD_CHARS_DIGIT "-"                            SD_SKIP_SPACES
#define SD_SKIP_UINT        SD_CHARS_DIGIT                                SD_SKIP_SPACES
#define SD_SKIP_FLOAT       SD_CHARS_DIGIT "+-Ee.Nan" /* NaN/nan */       SD_SKIP_SPACES
#define SD_SKIP_STRING      SD_CHARS_CHAR SD_CHARS_DIGIT SD_CHARS_SPECIAL SD_SKIP_SPACES
#define SD_SKIP(x)          SD_SKIP_##x


typedef struct {
    const c_char *repr;
    const c_char  value;
} sd_xmlSpecialEntry;

static const sd_xmlSpecialEntry sd_xmlSpecialTable[] = {
    { "gt;",    '>' },
    { "lt;",    '<' },
    { "eq;",    '=' },
    { "amp;",   '&' },
    { "quote;", '"' }
};

static const c_ulong sd_xmlSpecialTableSize = sizeof(sd_xmlSpecialTable) / sizeof(sd_xmlSpecialEntry);

static c_char *
getCharsUntil (
    const c_char *str,
    c_char        c,
    c_ulong      *len)
{
    c_char *res = NULL;
    c_char *p;
    c_ulong l;

    p = strchr(str, c);
    if ( p ) {
        l = (c_ulong)(p - str) + 1;
        res = (c_char *) os_malloc(l+1);
        os_strncpy(res, str, l);
        res[l] = '\0';
        *len = l;
    }

    return res;
}

static c_bool
sd_xmlEscapedChar (
    c_char  *str,
    c_char  *retval,
    c_ulong *processed)
{
    int     base = 10;
    c_char *p;
    c_char *e;
    c_ulong n;
    long    value;
    c_bool  result = FALSE;

    p = getCharsUntil(str, ';', &n);

    if ( p ) {
        if ( (*p != '\0') && (*p == 'x') ) {
            str++;
            base = 16;
        }
        p[n-1] = '\0';

        value = strtol(p, &e, base);

        if ( e && (strlen(e) == 0) && (value >= 0) && (value < 255) ) {
            *retval = (c_char) value;
            *processed = n;
            result = TRUE;
        }
        os_free(p);
    }

    return result;
}

static c_bool
sd_xmlStringProcSpecial (
    c_char *str,
    c_char **retval)
{
    c_char *res;
    c_ulong s = 0;
    c_ulong d = 0;
    c_ulong i,l;
    c_bool result = TRUE;

    assert(str);

    l = strlen(str)+1;
    res = (c_char *) os_malloc(l);
    memset(res, 0, l);

    while ( result && (str[s] != '\0') ) {
        if ( str[s] == SD_AMPERSAND ) {
            s++;
            if ( (str[s] != '\0') && (str[s] == '#') ) {
                s++;
                if ( sd_xmlEscapedChar(&str[s], &res[d], &l) ) {
                    d++;
                    s += l;
                } else {
                    result = FALSE;
                }
            } else {
                result = FALSE;
                for ( i = 0; !result && (i < sd_xmlSpecialTableSize); i++ ) {
                    l = strlen(sd_xmlSpecialTable[i].repr);
                    if ( strncmp(sd_xmlSpecialTable[i].repr, &str[s], l) == 0 ) {
                        s += l;
                        res[d++] = sd_xmlSpecialTable[i].value;
                        result = TRUE;
                    }
                }
            }
        } else {
            res[d++] = str[s++];
        }
    }

    if ( result ) {
        *retval = res;
    } else {
        os_free(res);
    }

    return result;
}


static void
sd_xmlStringSkipSpaces (
    c_char **str)
{
    assert(str);
    assert(*str);

    sd_strSkipChars(str, SD_SKIP_SPACES);
}

static c_char *
sd_xmlStringSkipTrailSpaces (
    c_char *str)
{
    c_long i;

    assert(str);

    i = strlen(str);
    while ( (i > 0) && ((str[i-1] == ' ') || (str[i-1] == '\t') || (str[i-1] == '\n')) ) {
        i--;
    }

    str[i] = '\0';

    return str;
}


#define SD_COMMENT_START "<!--"
#define SD_COMMENT_END   "-->"

static c_bool
sd_xmlStringSkipComment (
    c_char **str)
{
    c_bool valid = TRUE;
    c_char *end;

    assert(str);
    assert(*str);

    sd_xmlStringSkipSpaces(str);

    if ( strncmp(SD_COMMENT_START, *str, strlen(SD_COMMENT_START)) == 0 ) {
        end = strstr(*str, SD_COMMENT_END);
        if ( end ) {
            *str = C_DISPLACE(end, strlen(SD_COMMENT_END));
            valid = sd_xmlStringSkipComment(str);
        } else {
            valid = FALSE;
        }
    }

    return valid;
}

#undef SD_COMMENT_START
#undef SD_COMMENT_END

static sd_xmlTokenKind
sd_xmlStringGetOpeningTag (
    c_char **str,
    c_char **value)
{
    sd_xmlTokenKind token = SD_XML_TOKEN_ERROR;
    c_char *v;

    assert(str);
    assert(*str);

    sd_xmlStringSkipSpaces(str);

    v = sd_strGetChars(str, SD_CHARS_DATA);

    if ( v ) {
        if ( sd_xmlStringProcSpecial(v, value) ) {
            if ( **str == '\0' ) {
                token = SD_XML_TOKEN_ERROR;
            } else  if ( **str == '>' ) {
                *str = &((*str)[1]);
                *value = sd_xmlStringSkipTrailSpaces(*value);
                token = SD_XML_TOKEN_BTAG;
            } else if ( (**str == '/') && ((*str)[1] == '>') ) {
                *str = &((*str)[2]);
                *value = sd_xmlStringSkipTrailSpaces(*value);
                token = SD_XML_TOKEN_STAG;
            } else {
                token = SD_XML_TOKEN_ERROR;
            }
        }
        os_free(v);
    }

    return token;
}

static sd_xmlTokenKind
sd_xmlStringGetClosingTag (
    c_char **str,
    c_char **value)
{
    sd_xmlTokenKind token = SD_XML_TOKEN_ERROR;
    c_char *v;

    assert(str);
    assert(*str);

    sd_xmlStringSkipSpaces(str);

    v = sd_strGetChars(str, SD_CHARS_DATA);

    if ( v ) {
        if ( sd_xmlStringProcSpecial(v, value) ) {
            if ( **str == '>' ) {
                *str = &((*str)[1]);
                *value = sd_xmlStringSkipTrailSpaces(*value);
                token = SD_XML_TOKEN_ETAG;
            } else {
                token = SD_XML_TOKEN_ERROR;
            }
        }
        os_free(v);
    }

    return token;
}

static sd_xmlTokenKind
sd_xmlStringGetData (
    c_char **str,
    c_char **value)
{
    sd_xmlTokenKind token = SD_XML_TOKEN_ERROR;
    c_char *v;

    assert(str);
    assert(*str);

    sd_xmlStringSkipSpaces(str);

    v = sd_strGetChars(str, SD_CHARS_DATA);

    if ( v ) {
        if ( sd_xmlStringProcSpecial(v, value) ) {
            if ( (**str == '0') || (**str == '<') ) {
                *value = sd_xmlStringSkipTrailSpaces(*value);
                token = SD_XML_TOKEN_DATA;
            } else {
                token = SD_XML_TOKEN_ERROR;
            }
        }
        os_free(v);
    }

    return token;
}


static sd_xmlTokenKind
sd_xmlStringGetToken (
    c_char **str,
    c_char **value)
{
    sd_xmlTokenKind token = SD_XML_TOKEN_ERROR;

    assert(str);
    assert(*str);

    if ( sd_xmlStringSkipComment(str) ) {
        if ( **str == '\0' ) {
            token = SD_XML_TOKEN_EOF;
        } else if ( ((*str)[0] == '<') && ((*str)[1] == '/') ) {
            *str = &((*str)[2]);
            token = sd_xmlStringGetClosingTag(str, value);
        } else if ( **str == '<' ) {
            *str = &((*str)[1]);
            token = sd_xmlStringGetOpeningTag(str, value);
        } else {
            token = sd_xmlStringGetData(str, value);
        }
    }

    return token;
}


static c_bool
sd_xmlStringGetIdent (
    c_char **str,
    c_char **ident)
{
    c_bool result = FALSE;

    sd_xmlStringSkipSpaces(str);

    *ident = sd_strGetChars(str, SD_CHARS_IDENT);

    if ( *ident ) {
        if ( IS_ALPHA((*ident)[0]) ) {
            result = TRUE;
        } else {
            os_free(*ident);
            *ident = NULL;
        }
    }
    return result;
}

static c_bool
sd_xmlStringSkipAssign (
    c_char **str)
{
    c_bool result = FALSE;

    sd_xmlStringSkipSpaces(str);

    if ( *str[0] == SD_ASSIGN ) {
        *str = &(*str)[1];
        result = TRUE;
    }

    return result;
}

static c_bool
sd_xmlStringGetQuotedString (
    c_char **str,
    c_char **value)
{
    c_bool result = FALSE;
    c_char *val;
    c_char quote;

    sd_xmlStringSkipSpaces(str);

    if ( IS_QUOTE(**str) ) {
        quote = **str;
        *str = &(*str)[1];
        val = sd_strGetChars(str, SD_CHARS_STRING);
        if ( val ) {
            if ( **str == quote ) {
                *str = &(*str)[1];
                if ( sd_xmlStringProcSpecial(val, value) ) {
                    result = TRUE;
                }
            }
            os_free(val);
        }
    }
    return result;
}

static c_bool
sd_xmlStringIsEmpty (
    c_char **str)
{
    c_bool result = FALSE;

    sd_xmlStringSkipSpaces(str);

    if ( **str == '\0' ) {
        result = TRUE;
    }

    return result;
}

static c_bool
printXmlAttribute (
    sd_xmlNode node,
    void      *arg)
{
    sd_xmlAttribute attribute = sd_xmlAttribute(node);
    OS_UNUSED_ARG(arg);

    printf(" %s=\"%s\"", node->name, attribute->value);

    return TRUE;
}


static c_bool
printXmlElement (
    sd_xmlNode node,
    void       *arg)
{
    sd_xmlElement element = sd_xmlElement(node);
    
    OS_UNUSED_ARG(arg);
    
    printf("<%s", node->name);

    if ( element->attributes ) {
        sd_xmlElementWalkAttributes(element, printXmlAttribute, NULL);
    }

    if ( element->data ) {
        sd_xmlData data = sd_xmlData(element->data);
        printf(">%s</%s>\n", data->data, node->name);
    } else {
        if ( element->children ) {
            printf(">\n");
            sd_xmlElementWalkChildren(element, printXmlElement, NULL);
            printf("</%s>\n", node->name);
        } else {
            printf("/>\n");
        }
    }
    return TRUE;
}

static void
buildXmlElementInfo (
    sd_xmlParserElement element,
    sd_string           message)
{
    c_ulong i;

    if ( element && element->name ) {
        sd_stringAdd(message, "<%s", element->name);
        if ( element->attributes ) {
            for ( i = 0; i < sd_listSize(element->attributes); i++ ) {
                sd_xmlParserAttribute attribute;

                attribute = (sd_xmlParserAttribute) sd_listAt(element->attributes,i);
                if ( attribute->name && attribute->value ) {
                    sd_stringAdd(message, "%s=\"%s\"", attribute->name, attribute->value);
                }
            }
        }
        sd_stringAdd(message, ">", NULL);
    }

}


void
sd_xmlParserSetError (
    sd_xmlParser  parser,
    c_ulong       errorNumber,
    const c_char *message)
{
    sd_xmlParserElement element = NULL;
    sd_xmlParserElement parent  = NULL;

    if ( parser && !parser->errorInfo ) {
        sd_string location = sd_stringNew(256);
        element = sd_xmlParserStackPeek(&parser->stack);
        parent  = sd_xmlParserStackPeek2(&parser->stack);

        buildXmlElementInfo(element, location);
        buildXmlElementInfo(parent, location);

        parser->errorInfo = sd_errorReportNew(errorNumber, message, sd_stringContents(location));
        sd_stringFree(location);
    }
}

