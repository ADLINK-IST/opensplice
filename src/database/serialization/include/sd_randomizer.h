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
/** \file services/serialization/include/sd_randomizer.h
 *  \brief Declaration of the \b randomizer class for creating random instances
 *         of database objects.
 */

#ifndef SD_RANDOMIZER_H
#define SD_RANDOMIZER_H

#include "c_typebase.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(sd_randomizer);

OS_API sd_randomizer
sd_randomizerNew(
    c_base base);
    
OS_API void
sd_randomizerInit(
    sd_randomizer randomizer,
    c_ulong seed);
    
OS_API c_object
sd_randomizerRandomInstance(
    sd_randomizer randomizer,
    const c_char *typeName);
    
OS_API void
sd_randomizerFree(
    sd_randomizer randomizer);

#undef OS_API

#endif  /* SD_RANDOMIZER_H */
