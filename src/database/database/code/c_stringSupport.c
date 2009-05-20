/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include "os.h"
#include "c_stringSupport.h"

#include "ctype.h"

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
            strncpy(nibble,tail,length);
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
