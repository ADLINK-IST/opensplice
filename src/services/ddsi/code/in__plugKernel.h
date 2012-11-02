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
#ifndef IN__PLUGKERNEL_H_
#define IN__PLUGKERNEL_H_

#include "in__object.h"
#include "u_service.h"
#include "u_networkReader.h"

#if defined (__cplusplus)
extern "C" {
#endif


#define in_plugKernel(p) ((in_plugKernel)(p))

#define in_plugKernelFree(p) in_objectFree(in_object(p))

#define in_plugKernelKeep(p) in_plugKernel(in_objectKeep(in_object(p)))

#define in_plugKernelIsValid(p) \
    in_objectIsValidWithKind(in_object(p), IN_OBJECT_KIND_PLUG_KERNEL)

in_plugKernel
in_plugKernelNew(
    u_service service);

u_service
in_plugKernelGetService(
    in_plugKernel _this);

v_topic
in_plugKernelLookupTopic(
    in_plugKernel _this,
    const c_char* topic_name);

u_networkReader
in_plugKernelGetNetworkReader(
    in_plugKernel _this);

c_base
in_plugKernelGetBase(
    in_plugKernel _this);

#if defined (__cplusplus)
}
#endif

#endif /* IN__PLUGKERNEL_H_ */
