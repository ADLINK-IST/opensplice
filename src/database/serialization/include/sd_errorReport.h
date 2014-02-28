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
#ifndef SD_ERRORREPORT_H
#define SD_ERRORREPORT_H

#include "c_base.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* Error numbers which might occur for the XML type serializer */

#define SD_ERRNO_METADATA_SPEC_INVALID       301U
#define SD_ERRNO_MODULE_SPEC_INVALID         302U
#define SD_ERRNO_TYPE_SPEC_INVALID           303U
#define SD_ERRNO_TYPEDEF_SPEC_INVALID        304U
#define SD_ERRNO_STRUCT_SPEC_INVALID         305U
#define SD_ERRNO_MEMBER_SPEC_INVALID         306U
#define SD_ERRNO_ENUM_SPEC_INVALID           307U
#define SD_ERRNO_UNION_SPEC_INVALID          308U
#define SD_ERRNO_UNIONCASE_SPEC_INVALID      309U
#define SD_ERRNO_SWITCHTYPE_SPEC_INVALID     310U
#define SD_ERRNO_ELEMENT_SPEC_INVALID        311U
#define SD_ERRNO_LABEL_SPEC_INVALID          312U
#define SD_ERRNO_SEQUENCE_SPEC_INVALID       313U
#define SD_ERRNO_ARRAY_SPEC_INVALID          314U
#define SD_ERRNO_STRING_SPEC_INVALID         315U
#define SD_ERRNO_NAME_ATTRIBUTE_MISSING      316U
#define SD_ERRNO_SIZE_ATTRIBUTE_INVALID      317U
#define SD_ERRNO_VALUE_ATTRIBUTE_INVALID     318U
#define SD_ERRNO_VERSION_ATTRIBUTE_MISSING   319U
#define SD_ERRNO_COLLECTION_TYPE_MISSING     320U
#define SD_ERRNO_LENGTH_ATTRIBUTE_MISSING    321U
#define SD_ERRNO_SCOPE_INVALID               322U

#define SD_ERRNO_METADATA_WRONG_VERSION      323U
#define SD_ERRNO_METADATA_DEFINE_FAILED      324U
#define SD_ERRNO_METADATA_RESOLVE_FAILED     325U
#define SD_ERRNO_METADATA_BIND_FAILED        326U
#define SD_ERRNO_METADATA_DECLARE_FAILED     327U
#define SD_ERRNO_MEMORY_ALLOCATION_FAILED    328U

#define SD_MESSAGE_METADATA_SPEC_INVALID     "metadata specification invalid"
#define SD_MESSAGE_MODULE_SPEC_INVALID       "module specification invalid"
#define SD_MESSAGE_TYPE_SPEC_INVALID         "type specification invalid"
#define SD_MESSAGE_TYPEDEF_SPEC_INVALID      "typedef specification invalid"
#define SD_MESSAGE_STRUCT_SPEC_INVALID       "structure specification invalid"
#define SD_MESSAGE_MEMBER_SPEC_INVALID       "member specification invalid"
#define SD_MESSAGE_ENUM_SPEC_INVALID         "enum specification invalid"
#define SD_MESSAGE_UNION_SPEC_INVALID        "union specification invalid"
#define SD_MESSAGE_UNIONCASE_SPEC_INVALID    "unioncase specification invalid"
#define SD_MESSAGE_SWITCHTYPE_SPEC_INVALID   "switch type specification invalid"
#define SD_MESSAGE_ELEMENT_SPEC_INVALID      "element specification invalid"
#define SD_MESSAGE_LABEL_SPEC_INVALID        "label specification invalid"
#define SD_MESSAGE_SEQUENCE_SPEC_INVALID     "sequence specification invalid"
#define SD_MESSAGE_ARRAY_SPEC_INVALID        "array specification invalid"
#define SD_MESSAGE_STRING_SPEC_INVALID       "string specification invalid"
#define SD_MESSAGE_NAME_ATTRIBUTE_MISSING    "name attribute missing"
#define SD_MESSAGE_SIZE_ATTRIBUTE_INVALID    "size attribute invalid"
#define SD_MESSAGE_VALUE_ATTRIBUTE_INVALID   "value attribute invalid"
#define SD_MESSAGE_VERSION_ATTRIBUTE_MISSING "version attribute missing"
#define SD_MESSAGE_COLLECTION_TYPE_MISSING   "collection type missing"
#define SD_MESSAGE_LENGTH_ATTRIBUTE_MISSING  "length attribute missing"
#define SD_MESSAGE_SCOPE_INVALID             "scope specification invalid"

#define SD_MESSAGE_METADATA_WRONG_VERSION    "wrong version"
#define SD_MESSAGE_METADATA_DEFINE_FAILED    "metadata define failed"
#define SD_MESSAGE_METADATA_RESOLVE_FAILED   "metadata resolve failed"
#define SD_MESSAGE_METADATA_BIND_FAILED      "metadata bind failed"
#define SD_MESSAGE_METADATA_DEFINE_FAILED    "metadata define failed"
#define SD_MESSAGE_METADATA_DECLARE_FAILED   "metadata declare failed"
#define SD_MESSAGE_MEMORY_ALLOCATION_FAILED  "memory allocation failed"

C_CLASS(sd_errorReport);
C_STRUCT(sd_errorReport) {
    c_ulong  errorNumber;
    c_char  *message;
    c_char  *location;
};

OS_API sd_errorReport
sd_errorReportNew (
    c_ulong       errorNumber,
    const c_char *message,
    const c_char *location);

OS_API void
sd_errorReportFree (
    sd_errorReport errorReport);

#undef OS_API

#endif  /* SD_ERRORREPORT_H */

