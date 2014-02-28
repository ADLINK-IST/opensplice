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
#ifndef UT_CRC_H
#define UT_CRC_H

#include "os_defs.h"
#include "os_classbase.h"


#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"
#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_CLASS(ut_crc);

#define UT_CRC_KEY 0x04c11db7

OS_API ut_crc
ut_crcNew(
        os_uint32 key);

OS_API os_uint32
ut_crcCalculate(
    ut_crc _this,
    void *buf,
    os_uint32 length);

OS_API void
ut_crcFree(
    ut_crc _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* UT_CRC_H */
