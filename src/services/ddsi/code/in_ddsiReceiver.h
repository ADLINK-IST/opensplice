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
#ifndef IN_DDSIRECEIVER_H_
#define IN_DDSIRECEIVER_H_

#include "in_commonTypes.h"
#include "in_abstractReceiveBuffer.h"

#if defined (__cplusplus)
extern "C" {
#endif

os_boolean
in_ddsiReceiverInit(in_ddsiReceiver _this,
        in_abstractReceiveBuffer receiveBuffer);

void
in_ddsiReceiverDeinit(in_ddsiReceiver _this);

#if defined (__cplusplus)
}
#endif


#endif /* IN_DDSIRECEIVER_H_ */
