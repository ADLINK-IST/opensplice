/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
