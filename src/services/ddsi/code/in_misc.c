
/* interface */
#include "in_misc.h"

/* implementation */
#include "u_user.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "c_metabase.h"

#if 0

struct createTypeArg
{
    const c_char *typeName;
    c_object instanceOfType; /* result value */
};

static void
createType(
    v_entity entity,
    c_voidp arg)
{
    c_base base;
    c_type type;
    v_kernel kernel;
    struct createTypeArg *argument = (struct createTypeArg *)arg;

    kernel = v_objectKernel(entity);
    base = c_getBase(entity);
    type = c_resolve(base, argument->typeName);
    argument->instanceOfType = (c_object)v_new(kernel, type);
}

c_object
in_participantCreateType(
    u_participant participant,
    const c_char *typeName)
{
    struct createTypeArg arg;

    arg.typeName = typeName;
    arg.instanceOfType = NULL;
    u_entityAction(u_entity(participant), createType, &arg);

    return arg.instanceOfType;
}

#endif

char *
in_stringDup(
    const char *string)
{
    unsigned int size;
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
in_dumpToString(
    void *data,
    unsigned int length)
{
#define IN_ASCII_SPACE          (32U)  /* SP: Lowest printable character      */
#define IN_ASCII_TILDE         (126U)  /* ~: Highest printable character      */
#define IN_ASCII_PERCENT        (37U)  /* %: Avoid printf confusion           */
#define IN_ASCII_DOT            (46U)  /* .: Replacer for non-printable chars */
#define IN_ASCII_ZERO           (48U)  /* 0: Base for digits                  */
#define IN_ASCII_A              (65U)  /* A: Base for hex-digits > 10         */
#define IN_SEP_REPLACER         (c_octet)'\n'
#define IN_ASCII(i)             (c_octet)(                 \
                                  ((unsigned int)(i)<10U)?  \
                                  ((unsigned int)(i)+IN_ASCII_ZERO):  \
                                  ((unsigned int)(i)+ IN_ASCII_A - 10U))
#define IN_ASCII_MSB(i)         (c_octet)(IN_ASCII(((unsigned int)(i)>>4)))
#define IN_ASCII_LSB(i)         (c_octet)(IN_ASCII(((unsigned int)(i)&15U)))
#define IN_ASCII_PRINTABLE(i)   (c_octet)(                      \
                                  ( ((unsigned int)(i)>= IN_ASCII_SPACE) && \
                                    ((unsigned int)(i)<= IN_ASCII_TILDE) && \
                                    ((unsigned int)(i)!= IN_ASCII_PERCENT) )? \
                                  (unsigned int)(i):            \
                                  IN_ASCII_DOT)
#define IN_LINE_WIDTH           (16U)
#define IN_BUFSIZE              (32U)
#define IN_BYTESEP              (c_octet)' '
#define IN_LINESEP              (c_octet)'\n'

    c_ulong totSize, nLines;
    c_char *result;
    c_octet *currentDest, *currentSrc;
    c_ulong line, i, nThisLine;

    nLines = ((length-1U) / IN_LINE_WIDTH) + 1U;
    totSize = ((4U*IN_LINE_WIDTH) + 1U) * nLines;
    totSize++; /* '\0' */
/* QAC EXPECT 5007; warning is bug which will be repaired */                      
    result = (c_char *)os_malloc(totSize);

    /* Prepend version info */
    /* The actual serialized data */
    currentSrc = data;
    currentDest = (c_octet *)result;
    for (line=0; line < nLines; line++) {
        nThisLine = (((line+1U) < nLines) ? 
                     IN_LINE_WIDTH : 
                     (((length - 1U) % IN_LINE_WIDTH) + 1U));
        for (i=0; i < IN_LINE_WIDTH; i++) {
            if (i < nThisLine) {
                currentDest[(3U*i)] = IN_ASCII_MSB(currentSrc[i]);
                currentDest[(3U*i) + 1U] = IN_ASCII_LSB(currentSrc[i]);
            } else {
                currentDest[(3U*i)] = IN_BYTESEP;
                currentDest[(3U*i) + 1U] = IN_BYTESEP;
            }
            currentDest[(3U*i) + 2U] = IN_BYTESEP;
        }
        currentDest = &(currentDest[3U*IN_LINE_WIDTH]);
        for (i=0; i < nThisLine; i++) {
            currentDest[i] = IN_ASCII_PRINTABLE(currentSrc[i]);
        }
        currentDest = &(currentDest[nThisLine]);
        currentSrc = &(currentSrc[nThisLine]);

        *currentDest = IN_LINESEP;
        currentDest = &(currentDest[1]);
    }
    *currentDest = 0;

    return result;

#undef IN_ASCII
#undef IN_ASCII_PRINTABLE
#undef IN_ASCII_LSB
#undef IN_ASCII_MSB
#undef IN_SEP_REPLACER
#undef IN_LINE_WIDTH
#undef IN_BUFSIZE
#undef IN_BYTESEP
#undef IN_LINESEP
}
