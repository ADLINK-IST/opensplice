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
#include "sd_stringsXML.h"


#include "os_heap.h"
#include "os_stdlib.h"
#include "sd_misc.h"
#include "sd__confidence.h"

/* ---------------------------- String helpers ---------------------------- */

/** \brief This macro is used for offsetting into the XML data.
 *
 *  This encapsulated implementation of offsetting a memory pointer is
 *  used in order to avoid code repetition.
 */
#define SD_DISPLACE(ptr,offset) ((c_char *)(C_DISPLACE(ptr,offset)))

#define SD_ELEMENT_NAME "elmt"

/** \brief Translate a type name of an object to a XML tagname.
 *
 *  XML tagnames must start with ":" | [A-Z] | "_" | [a-z] (or a bunch of
 *  #xWHATEFFs, and may not contain "-" | "." | [0-9] (plus again a bunch of
 *  #xWHATEFFs) for the rest of the tagname. Names may NOT start with whatever
 *  (non-/partially) capitalized form of the word "xml". Furthermore can tagnames
 *  not contain any of the delimiter characters like "<", "=", "&". Therefor
 *  typenames have to be translated, cause the can be v_message<myType>.
 *  Anonymous members are interpreted as collection elements having the name "elmt".
 *
 *  The caller is supposed to os_free the result.
 *
 *  \param name The original name to be translated.
 *  \param type The metadata information of the object.
 *  \return A string which is a valid XML tag.
 */
/* Non-static for reuse by descendants */
c_char *
sd_getTagName(
    const c_char *name,
    c_type type)
{
    c_char *result;

    if (name) {
        result = sd_stringDup(name);
    } else {
        switch (c_baseObject(type)->kind) {
        case M_STRUCTURE:
        case M_INTERFACE:
        case M_CLASS:
            result = sd_getScopedTypeName(type, "..");
        break;
        default:
            result = sd_stringDup(SD_ELEMENT_NAME);
        break;
        }
    }

    return result;
}

#undef SD_ELEMENT_NAME


#define SD__BUFSIZE_STRLEN 127
#define SD__BUFSIZE_STRLEN_STR SD__STRINGIFY(SD__BUFSIZE_STRLEN)
#define SD__BUFSIZE (SD__BUFSIZE_STRLEN + 1)

#define SD__STRINGIFY(x) SD__STRINGIFY1 (x)
#define SD__STRINGIFY1(x) #x

c_char *
sd_getTypeAttributeFromOpenTag(
    const c_char *openTag)
{
    int start, end;
    int resultLen, openTagOffset = 0;
    c_char *result, *tmpResult, *resultPos;

    resultLen = SD__BUFSIZE;
    result = (c_char *)os_malloc(resultLen);

    /* Scan for 'object type="XMLEscapedTypeName"' */
    /* TODO: Remove sscanf; it is quite unfit for this purpose (see below effort
     * to retrieve a string of unknown length). */
    if(sscanf(openTag, "object%*[ ]type=\"%n%"SD__BUFSIZE_STRLEN_STR"[^\"]%n", &start, result, &end) == 1){
        while((end - start) == SD__BUFSIZE_STRLEN){
            /* Truncation; realloc and scan the rest of the string */
            tmpResult = (c_char *)os_realloc(result, resultLen + SD__BUFSIZE_STRLEN);
            if(tmpResult){
                result = tmpResult;
                resultPos = result + resultLen - 1; /* Append at position of current NULL-terminator */
                resultLen += SD__BUFSIZE_STRLEN;
                openTagOffset += end;
                /* Scan for the rest of the typeName, TODO: check successful parse */
                sscanf(openTag + openTagOffset, "%n%"SD__BUFSIZE_STRLEN_STR"[^\"]%n", &start, resultPos, &end); /* start always 0 */
            } else {
                /* TODO: report out-of-resources */
                os_free(result);
                result = NULL;
                break;
            }
        }
    } else {
        os_free(result);
        result = NULL;
    }

    sd_strUnescapeXML(&result);

    return result;
}

#define SD_ESCAPE(dst, dstPos, srcPos, escSeq) \
do { \
    memcpy (&((dst)[dstPos]), escSeq, sizeof (escSeq) - 1); \
    dstPos += sizeof(escSeq) - 1; \
    srcPos++; \
} while (0)



void
sd_strEscapeXML(
    c_char** src)
{
    const c_ulong step = 12; /* Allows 2 pairs of <> to be replaced by &lt;&gt; */
    c_ulong dstLen = 0;
    c_ulong srcPos, dstPos;
    c_char *newDst, *dst;

    if(!src || !(*src)){
        return;
    }

    srcPos = 0;
    dstPos = 0;

    dst = NULL;

    /* typeName needs escaping since it may contain < and > */
    do{
       /* Allocate enough memory for (worst-case) result; re-alloc if needed */
       if ((dstPos + sizeof("&amp;") - 1) >= dstLen) {
           if(dstLen == 0){
               dstLen = strlen(*src);
           }
           dstLen += step;
           newDst = (c_char *)os_realloc(dst, dstLen);
           if (newDst == NULL){
               /* Out of resources */
               os_free(dst);
               dst = NULL;
               break;
           }
           dst = newDst;
       }

       /* Scan the string for character-data */
       switch((*src)[srcPos]){
           case '<':
               SD_ESCAPE(dst, dstPos, srcPos, "&lt;");
               break;
           case '>':
               SD_ESCAPE(dst, dstPos, srcPos, "&gt;");
               break;
           case '&':
               SD_ESCAPE(dst, dstPos, srcPos, "&amp;");
               break;
           default:
               /* Copy character */
               dst[dstPos++] = (*src)[srcPos++];
               break;
       }
   } while(dst && (dst[dstPos - 1] != '\0'));

   os_free(*src);
   *src = dst;
}

#define SD_UNESCAPE(dst, dstPos, srcPos, escSeq, escChar) \
do { \
    (dst)[dstPos++] = escChar; \
    srcPos += sizeof(escSeq) - 1; \
} while (0)

#define SD_SUBSTR_MATCHES(src, srcPos, str) (strncmp(str, &((src)[srcPos]), sizeof(str) - 1) == 0)

#define SD_IF_MATCHES_UNESCAPE(src, srcPos, dstPos, str, chr) \
if(SD_SUBSTR_MATCHES(src, srcPos, str)){ \
    SD_UNESCAPE(src, dstPos, srcPos, str, chr); \
}

void
sd_strUnescapeXML(
    c_char * const* src)
{
    c_ulong srcPos, dstPos;

    if(!src || !(*src)){
        return;
    }

    srcPos = 0;
    dstPos = 0;

    do{
       /* strlen should reduce through this operation, so no need to do any
        * (re-)allocations; the unescaping is done in-place. */

       /* Scan the string for character-data */
       switch((*src)[srcPos]){
           case '&':
               SD_IF_MATCHES_UNESCAPE(*src, srcPos, dstPos, "&lt;", '<')
               else
               SD_IF_MATCHES_UNESCAPE(*src, srcPos, dstPos, "&gt;", '>')
               else
               SD_IF_MATCHES_UNESCAPE(*src, srcPos, dstPos, "&amp;", '&')
               else
               SD_IF_MATCHES_UNESCAPE(*src, srcPos, dstPos, "&quot;", '"')
               else
               SD_IF_MATCHES_UNESCAPE(*src, srcPos, dstPos, "&apos;", '\'')
               else {
                   /* Copy character; this can be a (valid) numerical character
                    * reference &#nnnn; (or &xhhhh;). These are not unescaped. */
                  (*src)[dstPos++] = (*src)[srcPos++];
               }
               break;
           default:
               /* Copy character */
               assert(dstPos <= srcPos);
               (*src)[dstPos++] = (*src)[srcPos++];
               break;
       }
   } while((*src)[dstPos - 1] != '\0');

   assert(dstPos <= srcPos);
}

#undef SD_IF_MATCHES_UNESCAPE
#undef SD_SUBSTR_MATCHES
#undef SD_UNESCAPE

/** \brief A helper function for skipping chars in a string
 *
 *  All characters in the given array are skipped.
 *
 *  \param str In/out parameter which contains the string.
 *             After execution, it will point to the new location.
 *  \param chars Array containing all characters to be skipped.
 */

void
sd_strSkipChars(
    c_char **str,
    const c_char *chars)
{
    c_ulong count;

    count = strspn(*str, chars);
    *str = SD_DISPLACE(*str, count);
}

/** \brief A helper function for retrieving a substring containing
 *         specific characters and simultaneously skipping the
 *         substring.
 *
 *  The caller has to os_free the returned value.
 *
 *  \param str In/out parameter which contains the string.
 *             After execution, it will point to the new location.
 *  \param chars Array containing all characters to be included.
 */

c_char *
sd_strGetChars(
    c_char **str,
    const c_char *chars)
{
    c_ulong count;
    c_char *result;

    count = strspn(*str, chars);
    result = (c_char *)os_malloc(count+1U);
    os_strncpy(result, *str, count);
    result[count] = 0;
    *str = SD_DISPLACE(*str, count);

    return result;
}


c_char *
sd_strGetUptoChars(
    c_char **str,
    const c_char *chars)
{
    c_ulong count;
    c_char *result;

    count = strcspn(*str, chars);
    result = (c_char *)os_malloc(count+1U);
    os_strncpy(result, *str, count);
    result[count] = 0;
    *str = SD_DISPLACE(*str, count);

    return result;
}


#define SD_TAG_OPENER '<'
#define SD_CDATA_OPENER_CHAR '!'
#define SD_CLOSING_TAG_CHAR '/'
#define SD_TAG_CLOSER_STR ">"
#define SD_CHARS_SPACES  " \t\n"

c_char *
sd_strGetOpeningTag(
    c_char **str)
{
    c_char *result = NULL;

    sd_strSkipChars(str, SD_CHARS_SPACES);
    if ((int)**str == SD_TAG_OPENER) {
        *str = &((*str)[1]);

        result = sd_strGetUptoChars(str, SD_TAG_CLOSER_STR);
        *str = &((*str)[1]);
    }

    return result;
}

c_char *
sd_strGetClosingTag(
    c_char **str)
{
    c_char *result = NULL;

    sd_strSkipChars(str, SD_CHARS_SPACES);
    if ((int)**str == SD_TAG_OPENER && ((*str)[1] != SD_CDATA_OPENER_CHAR)) {
        *str = &((*str)[1]);
        if ((int)**str == SD_CLOSING_TAG_CHAR) {
            *str = &((*str)[1]);

            result = sd_strGetUptoChars(str, SD_TAG_CLOSER_STR);
            *str = &((*str)[1]);
        }
    }

    return result;
}

#undef SD_CHARS_SPACES
#undef SD_CDATA_OPENER_CHAR
#undef SD_TAG_OPENER
#undef SD_CLOSING_TAG_CHAR
#undef SD_TAG_CLOSER_STR

void
sd_strReplace(
    c_char *str,
    const c_char *patOld,
    const c_char *patNew)
{
    c_char *patStart = str;
    c_ulong i, len;

    SD_CONFIDENCE(strlen(patNew) == strlen(patOld));

    len = strlen(patNew);
    patStart = strstr(patStart, patOld);
    while (patStart) {
        for (i=0; i<len; i++) {
            patStart[i] = patNew[i];
        }
        patStart = strstr(patStart, patOld);
    }
}
