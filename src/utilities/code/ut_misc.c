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

#include "ut_misc.h"
#include "os.h"
#include "stdlib.h"

int
ut_patternMatch(
    const char* str,
    const char* pattern)
{
    int   stop = FALSE;
    int   matches = FALSE;
    char* strRef = NULL;
    char* patternRef = NULL;

    /* QAC EXPECT 2106,2100; */
    while ((*str != 0) && (*pattern != 0) && (stop == FALSE)) {
        /* QAC EXPECT 2106,3123; */
        if (*pattern == '*') {
            /* QAC EXPECT 0489; */
            pattern++;
            /* QAC EXPECT 2106; */
            while ((*str != 0) && (*str != *pattern)) {
                /* QAC EXPECT 0489; */
                str++;
            }
            /* QAC EXPECT 2106; */
            if (*str != 0) {
                /* QAC EXPECT 0489; */
                strRef = (char*)(str+1); /* just behind the matching char */
                patternRef = (char*)(pattern-1); /* on the '*' */
            }
        /* QAC EXPECT 2106,3123; */
        } else if (*pattern == '?') {
            /* QAC EXPECT 0489; */
            pattern++;
            /* QAC EXPECT 0489; */
            str++;
        /* QAC EXPECT 2004,3401,0489,2106; */
        } else if (*pattern++ != *str++) {
            if (strRef == NULL) {
                matches = FALSE;
                stop = TRUE;
            } else {
                str = strRef;
                pattern = patternRef;
                strRef = NULL;
            }
        }
    }
    /* QAC EXPECT 3892,2106,2100; */
    if ((*str == (char)0) && (stop == FALSE)) {
        /* QAC EXPECT 2106,3123; */
        while (*pattern == '*') {
            /* QAC EXPECT 0489; */
            pattern++;
        }
        /* QAC EXPECT 3892,2106; */
        if (*pattern == (char)0) {
            matches = TRUE;
        } else {
            matches = FALSE;
        }
    } else {
        matches = FALSE;
    }
    return matches;
    /* QAC EXPECT 5101; */
}
