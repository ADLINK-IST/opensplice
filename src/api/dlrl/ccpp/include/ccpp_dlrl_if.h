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
#include "os_if.h"

#ifdef OS_DLRL_API
#undef OS_DLRL_API
#endif

#ifdef OSPL_BUILD_DLRLCCPP
#define OS_DLRL_API OS_API_EXPORT
#else
#define OS_DLRL_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */
