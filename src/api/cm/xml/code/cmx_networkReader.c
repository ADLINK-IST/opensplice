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
#include "cmx__networkReader.h"
#include "v_networkReader.h"
#include <stdio.h>
#include "os_stdlib.h"

c_char*
cmx_networkReaderInit(
    v_networkReader entity)
{
    char buf[512];
    v_networkReader reader;
    
    reader = v_networkReader(entity);
    os_sprintf(buf, "<kind>NETWORKREADER</kind>");
    
    return (c_char*)(os_strdup(buf));
}
