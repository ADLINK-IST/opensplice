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
#include "in__configDebug.h"

OS_STRUCT(in_configDebug)
{
    os_char* pathName;
    os_uint32 wait;
};

os_uint32
in_configDebugGetWaitTime(
    in_configDebug _this)
{
    assert(_this);

    return _this->wait;
}

os_char*
in_configDebugGetPathName(
    in_configDebug _this)
{
    assert(_this);

    return _this->pathName;
}
