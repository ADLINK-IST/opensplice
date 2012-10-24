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
 *  XML tagnames are not allowed to contain any colons (:). Therefore,
 *  typenames have to be translated. Also, anonymous members are
 *  interpreted as collection elements having the name "elmt".
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
    if ((int)**str == SD_TAG_OPENER) {
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
