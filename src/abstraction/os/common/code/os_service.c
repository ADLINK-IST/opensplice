/** \file os/common/code/os_service.c
 *  \brief Common OS service
 *
 * Stub for OS service implementation for platforms
 * that don't need it.
 */
#include "os_init.h"

os_result
os_serviceStart(const char *name)
{
    (void) name;
    return os_resultSuccess;
}

os_result
os_serviceStop(void)
{
    return os_resultSuccess;
}

const char *
os_serviceName(void)
{
    return (const char *)0;
}

