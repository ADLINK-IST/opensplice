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
#ifndef SD_TYPEINFOPARSER_H
#define SD_TYPEINFOPARSER_H

#include "c_typebase.h"
#include "sd_list.h"
#include "sd_errorReport.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef enum {
    SD_TYPEINFO_KIND_MODULE,
    SD_TYPEINFO_KIND_STRUCT,
    SD_TYPEINFO_KIND_MEMBER,
    SD_TYPEINFO_KIND_UNION,
    SD_TYPEINFO_KIND_UNIONCASE,
    SD_TYPEINFO_KIND_UNIONSWITCH,
    SD_TYPEINFO_KIND_UNIONLABEL,
    SD_TYPEINFO_KIND_TYPEDEF,
    SD_TYPEINFO_KIND_ENUM,
    SD_TYPEINFO_KIND_ENUMLABEL,
    SD_TYPEINFO_KIND_TYPE,
    SD_TYPEINFO_KIND_ARRAY,
    SD_TYPEINFO_KIND_SEQUENCE,
    SD_TYPEINFO_KIND_STRING,
    SD_TYPEINFO_KIND_CHAR,
    SD_TYPEINFO_KIND_BOOLEAN,
    SD_TYPEINFO_KIND_OCTET,
    SD_TYPEINFO_KIND_SHORT,
    SD_TYPEINFO_KIND_USHORT,
    SD_TYPEINFO_KIND_LONG,
    SD_TYPEINFO_KIND_ULONG,
    SD_TYPEINFO_KIND_LONGLONG,
    SD_TYPEINFO_KIND_ULONGLONG,
    SD_TYPEINFO_KIND_FLOAT,
    SD_TYPEINFO_KIND_DOUBLE,
    SD_TYPEINFO_KIND_TIME,
    SD_TYPEINFO_KIND_UNIONLABELDEFAULT
} sd_typeInfoKind;

typedef enum {
    SD_TYPEINFO_ATTR_KIND_NUMBER,
    SD_TYPEINFO_ATTR_KIND_STRING
} sd_typeInfoAttrKind;

typedef struct sd_typeInfoAttribute_s {
    c_char *name;
    sd_typeInfoAttrKind d;
    union {
        c_long  nvalue;
        c_char *svalue;
    } u;
} sd_typeInfoAttribute;

typedef void *sd_typeInfoHandle;

typedef c_bool (*sd_typeInfoCallback) (
    sd_typeInfoKind kind, c_char *name,
    sd_list attributes, void *argument, sd_typeInfoHandle handle);

OS_API c_bool
sd_typeInfoParserParse (
    const c_char        *description,
    sd_typeInfoCallback  callback,
    void                *argument,
    sd_errorReport      *errorInfo);

OS_API c_bool
sd_typeInfoParserNext (
    sd_typeInfoHandle    handle,
    sd_typeInfoCallback  callback,
    void                *argument);


OS_API c_long
sd_findAttributeNumber (
    sd_list       attributes,
    const c_char *name);

OS_API const c_char *
sd_findAttributeValue (
    sd_list       attributes,
    const c_char *name);

#undef OS_API

#endif  /* SD_TYPEINFOPARSER_H */

