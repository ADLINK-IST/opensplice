/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

/* interface */
#include "nw_misc.h"

/* implementation */
#include "u_user.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "c_metabase.h"



char *
nw_stringDup(
    const char *string)
{
    os_uint32 size;
    char *result = NULL;
    
    if (string) {
        size = strlen(string);
        size++; /* '\0'*/
        result = os_malloc(size);
        if (result != NULL) {
            os_strncpy(result, string, size);
        }
    }
    
    return result;
}

/* --------------------------------- hexdump -------------------------------- */

c_char *
nw_dumpToString(
    void *data,
    os_uint32 length)
{
#define NW_ASCII_SPACE          (32U)  /* SP: Lowest printable character      */
#define NW_ASCII_TILDE         (126U)  /* ~: Highest printable character      */
#define NW_ASCII_PERCENT        (37U)  /* %: Avoid printf confusion           */
#define NW_ASCII_DOT            (46U)  /* .: Replacer for non-printable chars */
#define NW_ASCII_ZERO           (48U)  /* 0: Base for digits                  */
#define NW_ASCII_A              (65U)  /* A: Base for hex-digits > 10         */
#define NW_SEP_REPLACER         (c_octet)'\n'
#define NW_ASCII(i)             (c_octet)(                 \
                                  ((os_uint32)(i)<10U)?  \
                                  ((os_uint32)(i)+NW_ASCII_ZERO):  \
                                  ((os_uint32)(i)+ NW_ASCII_A - 10U))
#define NW_ASCII_MSB(i)         (c_octet)(NW_ASCII(((os_uint32)(i)>>4)))
#define NW_ASCII_LSB(i)         (c_octet)(NW_ASCII(((os_uint32)(i)&15U)))
#define NW_ASCII_PRINTABLE(i)   (c_octet)(                      \
                                  ( ((os_uint32)(i)>= NW_ASCII_SPACE) && \
                                    ((os_uint32)(i)<= NW_ASCII_TILDE) && \
                                    ((os_uint32)(i)!= NW_ASCII_PERCENT) )? \
                                  (os_uint32)(i):            \
                                  NW_ASCII_DOT)
#define NW_LINE_WIDTH           (16U)
#define NW_BUFSIZE              (32U)
#define NW_BYTESEP              (c_octet)' '
#define NW_LINESEP              (c_octet)'\n'

    c_ulong totSize, nLines;
    c_char *result;
    c_octet *currentDest, *currentSrc;
    c_ulong line, i, nThisLine;

    nLines = ((length-1U) / NW_LINE_WIDTH) + 1U;
    totSize = ((4U*NW_LINE_WIDTH) + 1U) * nLines;
    totSize++; /* '\0' */
/* QAC EXPECT 5007; warning is bug which will be repaired */                      
    result = (c_char *)os_malloc(totSize);

    /* Prepend version info */
    /* The actual serialized data */
    currentSrc = data;
    currentDest = (c_octet *)result;
    for (line=0; line < nLines; line++) {
        nThisLine = (((line+1U) < nLines) ? 
                     NW_LINE_WIDTH : 
                     (((length - 1U) % NW_LINE_WIDTH) + 1U));
        for (i=0; i < NW_LINE_WIDTH; i++) {
            if (i < nThisLine) {
                currentDest[(3U*i)] = NW_ASCII_MSB(currentSrc[i]);
                currentDest[(3U*i) + 1U] = NW_ASCII_LSB(currentSrc[i]);
            } else {
                currentDest[(3U*i)] = NW_BYTESEP;
                currentDest[(3U*i) + 1U] = NW_BYTESEP;
            }
            currentDest[(3U*i) + 2U] = NW_BYTESEP;
        }
        currentDest = &(currentDest[3U*NW_LINE_WIDTH]);
        for (i=0; i < nThisLine; i++) {
            currentDest[i] = NW_ASCII_PRINTABLE(currentSrc[i]);
        }
        currentDest = &(currentDest[nThisLine]);
        currentSrc = &(currentSrc[nThisLine]);

        *currentDest = NW_LINESEP;
        currentDest = &(currentDest[1]);
    }
    *currentDest = 0;

    return result;

#undef NW_ASCII
#undef NW_ASCII_PRINTABLE
#undef NW_ASCII_LSB
#undef NW_ASCII_MSB
#undef NW_SEP_REPLACER
#undef NW_LINE_WIDTH
#undef NW_BUFSIZE
#undef NW_BYTESEP
#undef NW_LINESEP
}
