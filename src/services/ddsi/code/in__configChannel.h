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
#ifndef IN_CONFIG_CHANNEL_H
#define IN_CONFIG_CHANNEL_H

#include "in__object.h"

/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif


/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 */
#define in_configChannel(_this) ((in_configChannel)_this)

OS_STRUCT(in_configChannel)
{
    in_configChannelKind kind;
    in_configDdsiService owningService;
    os_boolean isEnabled;
    os_ushort portNr;
    os_char* pathName;
    os_uint32 fragmentSize;
    /* sending parameters */
    os_uint32 receiveBufferSize;
    /* receiving parameter */
    os_int differentiatedServicesField; /* native int-type, TOS parameter */
};

os_boolean
in_configChannelInit(
    in_configChannel _this,
    in_configChannelKind kind,
    os_boolean isEnabled,
    in_configDdsiService owningService);

void
in_configChannelDeinit(
    in_configChannel _this);

in_configDdsiService
in_configChannelGetDdsiService(
    in_configChannel _this);

os_boolean
in_configChannelIsEnabled(
    in_configChannel _this);

os_ushort
in_configChannelGetPortNr(
    in_configChannel _this);

/* TODO add to design */
os_uint32
in_configChannelGetFragmentSize(
    in_configChannel _this);

void
in_configChannelSetFragmentSize(
    in_configChannel _this,
    os_uint32 fragmentSize);

/* TODO add to design */
os_boolean
in_configChannelSupportsControl(
    in_configChannel _this);

void
in_configChannelSetPortNr(
    in_configChannel _this,
    os_ushort portNr);

os_char*
in_configChannelGetPathName(
		in_configChannel _this);

/** shortcuts, mapping to in_configPartitioningGetGlobalPartitionAddress
 * TODO: add to design document */
os_char*
in_configChannelGetGlobalPartitionAddress(
		in_configChannel _this);


/** shortcuts, mapping to in_configDdsiServiceGetInterfaceId
 * TODO: add to design document */
os_char*
in_configChannelGetInterfaceId(
		in_configChannel _this);


/** TODO: add to design document */
os_uint32
in_configChannelGetReceiveBufferSize(
		in_configChannel _this);


/** TODO: add to design document */
os_uint
in_configChannelGetDifferentiatedServicesField(
		in_configChannel _this);

/** TODO: add to design document */
os_time
in_configChannelGetIOTimeout(
		in_configChannel _this);

in_configChannelKind
in_configChannelGetKind(
    in_configChannel _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CONFIG_CHANNEL_H */
