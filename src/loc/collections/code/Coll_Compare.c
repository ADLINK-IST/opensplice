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
#include <string.h>

#include "DLRL_Report.h"
#include "Coll_Compare.h"

int
stringIsLessThen(
    void *left,
    void *right)
{
    char* leftString  =	(char*)left;
    char* rightString =	(char*)right;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_EXIT);

    return (strcmp(leftString, rightString) < 0);
}

int
pointerIsLessThen(
    void *left,
    void *right)
{
    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_EXIT);
    return (left < right);
    /* suppress warnings */
}
