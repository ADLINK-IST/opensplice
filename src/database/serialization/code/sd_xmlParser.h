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
        sd_xmlParser handle);

c_bool
sd_xmlParserParse (
     const c_char         *xmlString,
     sd_xmlParserCallback  callback,
     void                 *argument,
     sd_errorReport       *errorInfo);

void
sd_xmlParserSetError (
    sd_xmlParser  handle,
    c_ulong       errorNumber,
    const c_char *message);



#endif  /* SD_XMLPARSER_H */

