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

/* interface */
#include "os_stdlib.h"
#include "nw__plugDataBuffer.h"

/* implementation */
#include "nw_misc.h"
#include "nw__confidence.h"
#include "os_stdlib.h"


/* --------------------------- analysis functions --------------------------- */

nw_bool
nw_plugDataBufferIsLastMessageHolder(
    nw_plugDataBuffer buffer,
    nw_messageHolder messageHolder)
{
    nw_bool result = FALSE;
    nw_seqNr fragmentLength;
    nw_seqNr msgLength;
    nw_seqNr offset;
    
    NW_CONFIDENCE(buffer != NULL);
    
    if (messageHolder != NULL) {
        fragmentLength = nw_plugBufferGetLength(nw_plugBuffer(buffer));
        msgLength = nw_messageHolderGetLength(messageHolder);
        offset = UI(messageHolder) - UI(buffer) + sizeof(messageHolder->length);
        result = ((offset + NW_ALIGN(NW_PLUGDATABUFFER_DATA_ALIGNMENT, msgLength)) == fragmentLength);
    }
    
    return result;
}

nw_bool
nw_plugDataBufferContainsWholeMessages(
    nw_plugDataBuffer buffer)
{
    nw_bool result = FALSE;
    nw_seqNr nrOfMessages;
    
    if (buffer != NULL) {
        nrOfMessages = nw_plugDataBufferGetNrOfMessages(buffer);
        if (nw_plugBufferGetTerminatorFlag(nw_plugBuffer(buffer))) {
            NW_CONFIDENCE(nrOfMessages>0);
            nrOfMessages--;
        }
        if (nw_plugBufferGetFragmentedFlag(nw_plugBuffer(buffer))) {
            NW_CONFIDENCE(nrOfMessages>0);
            nrOfMessages--;
        }
        result = (nrOfMessages > 0);
    }
    return result;
}

nw_messageHolder
nw_plugDataBufferGetNextMessageHolder(
    nw_plugDataBuffer buffer,
    nw_messageHolder prevMessageHolder,
    nw_bool wholeMessagesOnly)
{
    nw_messageHolder result = NULL;
    nw_bool isFragmented;
    nw_bool isLast;
    nw_bool isTerminator;
    
    NW_CONFIDENCE(buffer != NULL);
    
    if (prevMessageHolder != NULL) {
        if (!nw_plugDataBufferIsLastMessageHolder(buffer, prevMessageHolder)) {
            /* Get next from previous */
            result = (nw_messageHolder)NW_ALIGN(NW_PLUGDATABUFFER_DATA_ALIGNMENT,
                UI(NW_MESSAGEHOLDER_DATA(prevMessageHolder)) + 
                nw_messageHolderGetLength(prevMessageHolder));
        }
    } else {
        /* Get first */
        result = NW_PLUGDATABUFFER_FIRSTMESSAGE(buffer);
    }

    if ((result != NULL) && (wholeMessagesOnly)) {
        isFragmented = nw_plugBufferGetFragmentedFlag(nw_plugBuffer(buffer));
        if (isFragmented) {
            isLast = nw_plugDataBufferIsLastMessageHolder(buffer, result);
            if (isLast) {
                result = NULL;
            }
        }
        if (result != NULL) {
           isTerminator = nw_plugBufferGetTerminatorFlag(nw_plugBuffer(buffer));
           if (isTerminator && (prevMessageHolder == NULL)) {
               /* Skip this first message because it is the terminator of a
                * previous message */
               result = (nw_messageHolder)NW_ALIGN(NW_PLUGDATABUFFER_DATA_ALIGNMENT,
                   UI(NW_MESSAGEHOLDER_DATA(result)) + nw_messageHolderGetLength(result));
           }
        }
    }
    
    return result;
}

char *
nw_plugDataBufferToString(
    nw_plugDataBuffer buffer)
{
    char *str;

    str = os_malloc(200);
    NW_CONFIDENCE(buffer != NULL);
    os_sprintf(str,"partitionId(%d),nrOfMessages(%d),packetNr(%d),fragmentedMsgNr(%d),fragmentNr(%d),terminatedMsgNr(%d),terminatingFragmentNr(%d)",
    buffer->partitionId,
    buffer->nrOfMessages,
    buffer->packetNr,
    buffer->fragmentedMsgNr,
    buffer->fragmentNr,
    buffer->terminatedMsgNr,
    buffer->terminatingFragmentNr);

    return str;
}
