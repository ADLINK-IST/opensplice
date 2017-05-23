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
#include "os_heap.h"
#include "os_report.h"
#include "os_stdlib.h"

#include "sd_string.h"



#define SD_STRING_MINSIZE    64
#define SD_STRING_INCREMENT 256


C_STRUCT(sd_string) {
    c_char *buffer;
    c_ulong index;
    c_ulong size;
};

sd_string
sd_stringNew (
    c_ulong size)
{
    sd_string str;

    size = (size > SD_STRING_MINSIZE) ? size : SD_STRING_MINSIZE;

    str = os_malloc(C_SIZEOF(sd_string));
    str->buffer = (c_char *) os_malloc(size);
    str->size   = size;
    str->index  = 0UL;
    memset(str->buffer, 0, size);
    return str;
}

void
sd_stringFree (
    sd_string str)
{
    if ( str ) {
        if ( str->buffer ) {
            os_free(str->buffer);
        }
        os_free(str);
    }
}


static c_ulong
sd_stringRealloc (
    sd_string str)
{
    c_ulong  size;
    c_char  *buffer;

    assert(str);

    size = str->size + SD_STRING_INCREMENT;
    buffer = os_malloc(size);
    memcpy(buffer, str->buffer, str->index);
    memset(&buffer[str->index], 0, size - str->index);
    os_free(str->buffer);
    str->buffer = buffer;
    str->size   = size;
    return str->size;
}


#define SD_FREE(s)    (s->size - s->index)
#define SD_POINTER(s) (&s->buffer[s->index])

void
sd_stringAddImpl (
    sd_string     str,
    const c_char *format,
    va_list       args)
{
    c_ulong l;
    c_bool  ready = FALSE;

    while ( !ready ) {
        l = (c_ulong) os_vsnprintf(SD_POINTER(str), SD_FREE(str), format, args);
        if ( l < SD_FREE(str) ) {
            str->index += l;
            ready = TRUE;
        } else {
            sd_stringRealloc(str);
            if ( l >= SD_FREE(str) ) {
                ready = TRUE;
            }
        }
    }
}
#undef SD_FREE
#undef SD_POINTER

void
sd_stringAdd (
    sd_string     str,
    const c_char *format,
    ...)
{
    va_list args;

    va_start(args, format);
    sd_stringAddImpl(str, format, args);
    va_end(args);
}

const c_char *
sd_stringContents (
    sd_string str)
{
    const c_char *buffer = NULL;

    assert(str);
    if ( str ) {
        buffer = str->buffer;
    }
    return buffer;
}

