/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "in_factory.h"

#include "in_ddsiStreamWriterImpl.h"
#include "in_ddsiStreamReaderImpl.h"

in_transport
in_factoryCreateTransport(
    in_configChannel channelConfig)
{
    assert(channelConfig);

    return NULL;
}

/**  deprecated */
in_streamPair
in_factoryCreateStream(
		in_configChannel config)
{

	return NULL;
}



/** */
in_streamWriter
in_factoryCreateStreamWriter(
	in_configChannel config,
	in_transportSender sender,
	in_connectivityAdmin connectivityAdmin)
{
	in_streamWriter result = NULL;

    assert(config);
    assert(sender);
    assert(connectivityAdmin);

    result =
    	in_streamWriter(
    		in_ddsiStreamWriterImplNew(
    			config,
    			sender,
    			connectivityAdmin));

    return result;
}


/** */
in_streamReader
in_factoryCreateStreamReader(
	in_configChannel config,
	in_transportReceiver receiver,
	in_connectivityAdmin connectivityAdmin)
{
	in_streamReader result = NULL;

    assert(config);
    assert(receiver);
    assert(connectivityAdmin);

    result =
    	in_streamReader(
    		in_ddsiStreamReaderImplNew(
    			config,
    			receiver,
    			connectivityAdmin));

    return result;
}

in_channel
in_factoryCreateChannel(
    in_configChannel config)
{
    assert(config);

    return NULL;
}


