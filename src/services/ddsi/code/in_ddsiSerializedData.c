/*
 * in_ddsiSerializedData.c
 *
 *  Created on: Mar 16, 2009
 *      Author: frehberg
 */

/* **** interface headers **** */
#include "in_ddsiSerializedData.h"

/* **** implementation headers **** */

/* **** private functions **** */

/* **** public functions **** */

/** init
 *
 */
os_boolean
in_ddsiSerializedDataInit(in_ddsiSerializedData _this,
        in_octet *begin,
        os_size_t length,
        in_ddsiCodecId codecId,
        os_ushort flags)
{
    _this->begin = begin;
    _this->length = length;
    _this->codecId = codecId;
    _this->flags = flags;
    return OS_TRUE;
}

/** deinit
 *
 */
void
in_ddsiSerializedDataDeinit(in_ddsiSerializedData _this)
{
    _this->begin = NULL;
    _this->length = 0;
}

