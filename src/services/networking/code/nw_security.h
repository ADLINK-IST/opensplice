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

#ifndef NW_SECURITY_H_
#define NW_SECURITY_H_

#ifdef NW_SECURITY
#include "nw_privateSecurity.h"
	
#else

#define NW_SECURITY_CHECK_FOR_PUBLISH_PERMISSION_OF_SENDER_ON_SENDER_SIDE(entry) (OS_TRUE)
#define NW_SECURITY_CHECK_FOR_PUBLISH_PERMISSION_OF_SENDER_ON_RECEIVER_SIDE(senderInfo, partitionName, topic) (OS_TRUE)
#define NW_SECURITY_CHECK_FOR_SUBSCRIBE_PERMISSION_OF_RECEIVER(entry) (OS_TRUE)

#define NW_SECURITY_DISABLED (1)

#define NW_SECURITY_MODULE_INIT(service)  
#define NW_SECURITY_MODULE_DEINIT()

#define NW_SECURITY_DECODER_HOOK(_channel) (NULL)
#define NW_SECURITY_DECODER_HOOK_DECL
#define NW_SECURITY_DECODER_INIT(_obj,_nodeId,_partitions) 
#define NW_SECURITY_DECODER_DEINIT(_obj) 

/** Decode the specified cipherText buffer. 
 * 
 *  \param _plainText, 0 on error, otherwise positive integer. In case of error the 
 * cipherText buffer is in undefined state */
#define NW_SECURITY_DECODER_PROCESS(_plainTextLength,_channel,_senderInfo,_cipherText,_cipherTextLength,_maxFragmentLength) \
	_plainTextLength = _cipherTextLength
    	
#define NW_SECURITY_RESERVED_BUFFER_SPACE(sendChannel) (0L)
 
#define NW_SECURITY_ENCODER_HOOK(_channel) (NULL)
#define NW_SECURITY_ENCODER_HOOK_DECL 
#define NW_SECURITY_ENCODER_INIT(_obj,_nodeId,_partitions)
#define NW_SECURITY_ENCODER_DEINIT(_obj) 
#define NW_SECURITY_ENCODER_PROCESS(_succeeded,_channel,_partitionId,_text,_textLengthInOut,_maxTextLength)    \
 		(_succeeded) = TRUE; 

/* do {  os_time nano =  {0, 100*1000*1000}; os_nanoSleep(nano); (_succeeded) = TRUE; } while(0) */
	

   	
#endif

#endif /* NW_SECURITY_H_ */
