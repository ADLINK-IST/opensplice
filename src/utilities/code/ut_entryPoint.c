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

#include "ut_entryPoint.h"
#include "os.h"

void *
ut_entryPointWrapper(void *arg)
{
    struct ut_entryPointWrapperArg *mwa = (struct ut_entryPointWrapperArg *)arg;
    int result;

    result = mwa->entryPoint(mwa->argc,mwa->argv);

    os_free(mwa->argv);
    os_free(mwa);

    return (void *)result;
}
