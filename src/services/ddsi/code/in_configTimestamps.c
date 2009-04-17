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
#include "in__configTimestamps.h"

OS_STRUCT(in_configTimestamps)
{
    os_char* pathName;
    os_boolean isAbsolute;
};


os_char*
in_configTimestampsGetPathName(
    in_configTimestamps _this)
{
    assert(_this);

    return _this->pathName;
}

os_boolean
in_configTimestampsIsAbsolute(
    in_configTimestamps _this)
{
    assert(_this);

    return _this->isAbsolute;
}
