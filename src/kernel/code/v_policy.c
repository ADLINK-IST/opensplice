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

#include "v__policy.h"

#include "c_stringSupport.h"

#include "os.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/

/**************************************************************
 * Protected functions
 **************************************************************/
v_partitionPolicy
v_partitionPolicyAdd(
    v_partitionPolicy p,
    const c_char *expr,
    c_base base)
{
    v_partitionPolicy newPolicy;
    c_char *str;
    c_long size;

    newPolicy = NULL;
    if ((p != NULL) &&
        (strstr(p, expr) == NULL)) /* not in partitionPolicy yet */
    {
        size = strlen(p) + 1 + strlen(expr) + 1;
        str = os_malloc(size);
        if (str != NULL) {
            os_strncpy(str, p, size);
            str = os_strcat(str, ",");
            str = os_strcat(str, expr);
            newPolicy = c_stringNew(base, str);
            os_free(str);
        }
    } else {
        newPolicy = c_stringNew(base, expr);
    }

    return newPolicy;
}

v_partitionPolicy
v_partitionPolicyRemove(
    v_partitionPolicy p,
    const c_char *expr,
    c_base base)
{
    v_partitionPolicy newPolicy;
    c_char *str;
    c_char *start; /* start of expr in p */
    int len;

    newPolicy = NULL;
    if (p != NULL) {
        if (strcmp(p, expr) != 0) {
            len = strlen(p);
            str = (c_char *)os_malloc(len + 1);
            if (str != NULL) {
                start = strstr(p, expr);
                assert(start != NULL);
                assert((c_address)start >= (c_address)p);
                os_strncpy(str, p, (c_address)start - (c_address)p); /* includes ',' */
                str[(c_address)start - (c_address)p] = 0; /* make '\0' terminated */
                if (strcmp(start, expr) != 0) { /* not at the end */
                    os_strcat(str, (c_char *)((c_address)start + strlen(expr) + 1 /* , */));
                }
                newPolicy = c_stringNew(base, str);
                os_free(str);
            }
        }
    }

    return newPolicy;
}

c_iter
v_partitionPolicySplit(
    v_partitionPolicy p)
{
    const c_char *head, *tail;
    c_char *nibble;
    c_iter iter = NULL;
    c_long length;
    const c_char *delimiters = ",";

    if (p == NULL) return NULL;

    head = p;
    do {
        tail = c_skipUntil(head,delimiters);
        length = abs((c_address)tail - (c_address)head);
        if (length != 0) {
            length++;
            nibble = (c_string)os_malloc(length);
            os_strncpy(nibble, head, length);
            nibble[length-1]=0;
            iter = c_iterAppend(iter, nibble);
        } else {
            /* head points to one of the delimiters, so we
               add an empty string */
            length = 1;
            nibble = (c_string)os_malloc(length);
            nibble[length - 1 ] = 0;
            iter = c_iterAppend(iter, nibble);
        }
        head = tail;
        if (c_isOneOf(*head, delimiters)) {
            /* if the string ends with a delimiter, we also add an empty string */
            head++;
            if (*head == '\0') {
                length = 1;
                nibble = (c_string)os_malloc(length);
                nibble[length - 1 ] = 0;
                iter = c_iterAppend(iter, nibble);
            }
        }
    } while (*head != '\0');

    return iter;
}

/**************************************************************
 * Public functions
 **************************************************************/
