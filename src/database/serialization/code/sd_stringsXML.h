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
#ifndef SD_STRINGSXML_H
#define SD_STRINGSXML_H

#include "c_base.h"

#define SD_DISPLACE(ptr,offset) ((c_char *)(C_DISPLACE(ptr,offset)))

c_char *sd_getTagName(
            const c_char *name,
            c_type type);

c_char *sd_getTypeAttributeFromOpenTag(
            const c_char *openTag);

void    sd_strEscapeXML(
            c_char** src);

void    sd_strUnescapeXML(
            c_char* const* src);

void    sd_strSkipChars(
            c_char **str,
            const c_char *chars);

c_char *sd_strGetChars(
            c_char **str,
            const c_char *chars);

c_char *sd_strGetUptoChars(
            c_char **str,
            const c_char *chars);

c_char *sd_strGetOpeningTag(
            c_char **str);

c_char *sd_strGetClosingTag(
            c_char **str);

void    sd_strReplace(
            c_char *str,
            const c_char *patOld,
            const c_char *patNew);

#endif /* SD_STRINGS_XML */
