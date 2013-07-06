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

#ifndef OS_SOLARIS__SHAREDMEM_H
#define OS_SOLARIS__SHAREDMEM_H

#if defined (__cplusplus)
extern "C" {
#endif

/** \brief Initialize shared memory module
 */
void
os_sharedMemoryInit (
    void);

/** \brief Deinitialize shared memory module
 */
void
os_sharedMemoryExit (
    void);

#if defined (__cplusplus)
}
#endif

#endif /* OS_SOLARIS__SHAREDMEM_H */
