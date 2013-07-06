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

#include "os.h"
#include "c_stringSupport.h"

#include <ctype.h>

c_bool
c_isOneOf (
    c_char c,
    const c_char *symbolList)
{
    const c_char *symbol;

    symbol = symbolList;
    while (symbol != NULL && *symbol != '\0') {
        if (c == *symbol) return TRUE;
        symbol++;
    }
    return FALSE;
}

c_bool
c_isDigit (
    c_char c)
{
    return ((c>='0')&&(c<='9'));
}

c_bool
c_isLetter (
    c_char c)
{
    return (((c>='a')&&(c<='z'))||((c>='A')&&(c<='Z')));
}

c_char *
c_skipSpaces (
    const c_char *str)
{
    if (str == NULL) return NULL;
    while (c_isOneOf(*str, " \t")) str++;
    return (c_char *)str;
}

c_char *
c_skipIdentifier (
    const c_char *str,
    const c_char *puctuationList)
{
    c_char *ptr = (c_char *)str;

    if (ptr == NULL) return NULL;

    if (!c_isLetter(*ptr)) {
        return ptr;
    }
    ptr++;
    while (c_isLetter(*ptr) || c_isDigit(*ptr) || c_isOneOf(*ptr, puctuationList)) {
        ptr++;
    }
    return ptr;
}

c_char *
c_skipUntil (
    const c_char *str,
    const c_char *symbolList)
{
    c_char *ptr = (c_char *)str;

    assert(symbolList != NULL);
    if (ptr == NULL) return NULL;

    while (*ptr != '\0' && !c_isOneOf(*ptr,symbolList)) ptr++;
    return ptr;
}

c_iter
c_splitString(
    const c_char *str,
    const c_char *delimiters)
{
    const c_char *head, *tail;
    c_char *nibble;
    c_iter iter = NULL;
    c_long length;

    if (str == NULL) return NULL;

    tail = str;
    while (*tail != '\0') {
        head = c_skipUntil(tail,delimiters);
        length = abs((c_address)head - (c_address)tail);
        if (length != 0) {
            length++;
            nibble = (c_string)os_malloc(length);
            os_strncpy(nibble,tail,length);
            nibble[length-1]=0;
            iter = c_iterAppend(iter,nibble);
        }
        tail = head;
        if (c_isOneOf(*tail,delimiters)) tail++;
    }
    return iter;
}

c_equality
c_compareString (
    const c_char *s1,
    const c_char *s2)
{
    c_long result;

    result = strcmp(s1,s2);
    if (result < 0) return C_LT;
    if (result > 0) return C_GT;
    return C_EQ;
}

/** \brief string trimmer
*
* Returns a newly allocated string with the same contents as the original string
* without the leading and trailing whitespace characters.
*
* Precondition:
*   s != NULL
* Postcondition:
*   None
*
* Possible results:
* - return trimmed s
* - if original string only contains spaces, return null-terminated empty string
*/
c_char *
c_trimString(
    const c_char *s)
{
    const c_char *begin;
    const c_char *end;
    c_char *result = NULL;
    c_size size;

    if(s){
        /* ltrim
         * While begin is not the end of the string, and is a space, move right.
         * End result: begin points at the first non-space character in the string,
         * or has reached the end.
         */
        begin = s;
        while(*begin != '\0' && isspace(*begin))
        {
            begin++;
        }

        /* rtrim
         * While end is bigger than begin, and the character left of the current is a space
         * move to the left.
         * End result: end points to the first space character after the last non-space
         * character in the string, or to the end of the string.
         */
        end = s + strlen(s); /* end now points to the '\0' character */
        while(end > begin && isspace(*(end-1)))
        {
            end--;
        }

        /* either end-begin = 0, in case of an empty string, or bigger than 0, if
         * there is at least one non-space character.
         */
        assert(end >= begin);
        size = end - begin;
        result = os_malloc((size + 1) * sizeof(c_char)); /* +1 for the '\0' character */
        memcpy(result, begin, size);
        result[size] = '\0'; /* null-terminated string */
    }

    return result;
}

c_bool
c_stringExpressionIsAbsolute(
    const c_char* expression)
{
    c_bool result;

    assert(expression);

    /* Absolute expressions are those which do not contain a '?' or a '*' */
    if(strchr(expression, '*') == NULL && strchr(expression, '?') == NULL)
    {
        result = TRUE;
    } else
    {
        result = FALSE;
    }

    return result;
}

c_bool
c_stringMatchesExpression(
    const c_char* string,
    const c_char* expression)
{
    c_bool result = FALSE;
    c_value matchResult;
    c_value expressionValue;
    c_value stringValue;

    assert(string);
    assert(expression);

    if(c_stringExpressionIsAbsolute(expression))
    {
        if(strcmp(expression, string) == 0)
        {
             result  = TRUE;
        }
    } else
    {
        expressionValue.kind = V_STRING;
        expressionValue.is.String = (c_char*)expression;
        stringValue.kind = V_STRING;
        stringValue.is.String = (c_char*)string;
        matchResult = c_valueStringMatch(expressionValue, stringValue);
        if(matchResult.is.Boolean == TRUE)
        {
            result = TRUE;
        }
    }
    return result;
}

