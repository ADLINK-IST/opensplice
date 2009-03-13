#ifndef IN_FACTORY_H
#define IN_FACTORY_H

#include "in__configChannel.h"

#include "in_transport.h"
#include "in_transportSender.h"
#include "in_transportReceiver.h"
#include "in_streamReader.h"
#include "in_streamWriter.h"
#include "in_channel.h"
#include "in_connectivityAdmin.h"
#include "in_streamPair.h"
/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif

in_transport
in_factoryCreateTransport(
    in_configChannel channelConfig);

/**  deprecated */
in_streamPair
in_factoryCreateStream(
		in_configChannel config);

/** */
in_streamReader
in_factoryCreateStreamReader(
	in_configChannel config,
	in_transportReceiver receiver,
	in_connectivityAdmin connectivityAdmin);

in_streamWriter
in_factoryCreateStreamWriter(
		in_configChannel config,
		in_transportSender sender,
		in_connectivityAdmin connectivityAdmin);

in_streamReader
in_factoryCreateStreamReader(
		in_configChannel config,
		in_transportReceiver receiver,
		in_connectivityAdmin connectivityAdmin);

in_channel
in_factoryCreateChannel(
    in_configChannel channelConfig);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_FACTORY_H */

