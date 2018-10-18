/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef SD_XMLPARSER_H
#define SD_XMLPARSER_H

#include "c_base.h"

#include "sd_list.h"
#include "sd__deepwalkMeta.h"
#include "sd_errorReport.h"

C_CLASS(sd_xmlParser);

typedef enum {
    SD_XML_PARSER_KIND_ELEMENT_START,
    SD_XML_PARSER_KIND_ELEMENT_END,
    SD_XML_PARSER_KIND_DATA
} sd_xmlParserKind;

C_CLASS(sd_xmlParserAttribute);
C_STRUCT(sd_xmlParserAttribute) {
    c_char *name;
    c_char *value;
};

C_CLASS(sd_xmlParserElement);
C_STRUCT(sd_xmlParserElement) {
    c_char  *name;
    sd_list  attributes;
    c_char  *data;
};

typedef c_bool (*sd_xmlParserCallback)(
        sd_xmlParserKind kind,
        sd_xmlParserElement element,
        void *argument,
        sd_xmlParser handle) __nonnull((3, 4)) __attribute_warn_unused_result__;

c_bool
sd_xmlParserParse (
     const c_char         *xmlString,
     sd_xmlParserCallback  callback,
     void                 *argument,
     sd_errorReport       *errorInfo) __nonnull_all__ __attribute_warn_unused_result__;

void
sd_xmlParserSetError (
    sd_xmlParser  handle,
    c_ulong       errorNumber,
    const c_char *message) __nonnull_all__;

#endif  /* SD_XMLPARSER_H */

