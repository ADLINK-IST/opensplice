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

    str = (sd_string) os_malloc(C_SIZEOF(sd_string));
    if ( str ) {
        str->buffer = (c_char *) os_malloc(size);
        str->size   = size;
        str->index  = 0UL;
        if ( str->buffer ) {
            memset(str->buffer, 0, size);
        } else {
            OS_REPORT(OS_ERROR, "sd_string", 0, "memory allocation failed");
            os_free(str);
            str = NULL;
        }
    } else {
        OS_REPORT(OS_ERROR, "sd_string", 0, "memory allocation failed");
    }
    
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
    buffer = (c_char *) os_malloc(size);
    if ( buffer ) {
        memcpy(buffer, str->buffer, str->index);
        memset(&buffer[str->index], 0, size - str->index);
        os_free(str->buffer);
        str->buffer = buffer;
        str->size   = size;
    } else {
        OS_REPORT(OS_ERROR, "sd_string", 0, "memory allocation failed");
    }
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
        l = os_vsnprintf(SD_POINTER(str), SD_FREE(str), format, args);
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

