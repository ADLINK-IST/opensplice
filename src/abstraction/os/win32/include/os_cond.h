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
#ifndef OS_WIN32_COND_H
#define OS_WIN32_COND_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_defs.h"

typedef struct cond {
    os_scopeAttr scope;
    long         qId;
    long         state;
} os_os_cond;

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_COND_H */
