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
#include "os_if.h"

#if defined OSPL_BUILD_DCPSCCPP || defined OSPL_BUILD_DLRLCCPP
#define SACPP_API OS_API_EXPORT
#else
#define SACPP_API OS_API_IMPORT
#endif

#if defined OSPL_BUILD_DLRLCCPP
#define SACPP_DLRL_API OS_API_EXPORT
#else
#define SACPP_DLRL_API OS_API_IMPORT
#endif

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */
