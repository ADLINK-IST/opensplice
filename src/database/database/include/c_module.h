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
#ifndef C_MODULE_H
#define C_MODULE_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "c_metabase.h"
#include "c_sync.h"

C_CLASS(c_module);

C_STRUCT(c_module) {
    C_EXTENDS(c_metaObject);
    c_mutex mtx;
    c_scope scope;
};

#ifndef NO_META_CAST
#define c_module(o)         (C_CAST(o,c_module))
#endif

#if defined (__cplusplus)
}
#endif

#endif
