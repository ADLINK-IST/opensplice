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
#ifndef IN_DDSISERIALIZEDDATA_H_
#define IN_DDSISERIALIZEDDATA_H_

#include "in_commonTypes.h"
#include "in_ddsiDefinitions.h"

#if defined (__cplusplus)
extern "C" {
#endif

/** class
 *
 */
OS_STRUCT(in_ddsiSerializedData)
{
    in_octet *begin;  /* first octet of encapsulation header */
    os_size_t length; /* matching type for pointer arithmetics */

    /* The encapsulation header, using the same endianess as the Data-submessage-body */
    in_ddsiCodecId codecId; /* the code beind declared in encapsulation header */
    os_ushort flags; /* reserved for later user */
};

/** init
 *
 */
os_boolean
in_ddsiSerializedDataInit(in_ddsiSerializedData _this,
        in_octet *begin,
        os_size_t length,
        in_ddsiCodecId codecId,
        os_ushort flags);

/** deinit
 *
 */
void
in_ddsiSerializedDataDeinit(in_ddsiSerializedData _this);


#if defined (__cplusplus)
}
#endif


#endif /* IN_DDSISERIALIZEDDATA_H_ */
