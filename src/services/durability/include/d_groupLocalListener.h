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

#ifndef D_GROUPLOCALLISTENER_H
#define D_GROUPLOCALLISTENER_H

#include "d__types.h"
#include "v_group.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_groupLocalListener(l) ((d_groupLocalListener)(l))

d_groupLocalListener    d_groupLocalListenerNew                     (d_subscriber subscriber,
                                                                     d_sampleChainListener listener);

void                    d_groupLocalListenerFree                    (d_groupLocalListener listener);

c_bool                  d_groupLocalListenerStart                   (d_groupLocalListener listener);

c_bool                  d_groupLocalListenerStop                    (d_groupLocalListener listener);

void                    d_groupLocalListenerHandleAlignment         (d_groupLocalListener listener,
                                                                     d_group dgroup,
                                                                     d_readerRequest request);

#if defined (__cplusplus)
}
#endif

#endif /* D_GROUPLOCALLISTENER_H */
