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
/* local function of the required posix function, Solaris 9 does not contain strerror_r
 */
char * strerror_r(int errnum, char *buf, size_t n)
{
    sprintf(buf, "%d", errnum);
    return buf;
}

#include "../solaris10/code/os_stdlib.c"
