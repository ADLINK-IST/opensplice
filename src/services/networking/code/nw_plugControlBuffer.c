
/* interface */
#include "nw__plugControlBuffer.h"

/* implementation */
#include "nw_misc.h"
#include "nw__confidence.h"

#define UI(val) ((nw_length)(val))

nw_plugControlMessage
nw_plugControlBufferGetNextMessage(
    nw_plugControlBuffer buffer,
    nw_plugControlMessage prevMessage,
    nw_bool *more)
{
    nw_plugControlMessage result;
    nw_seqNr totalNrOfMessages;
    nw_seqNr lastMessageNr;
    
    totalNrOfMessages = nw_plugControlBufferGetNrOfMessages(buffer);
    NW_CONFIDENCE(totalNrOfMessages > 0);
    if (prevMessage == NULL) {
        lastMessageNr = 0;
        result = NW_PLUGCONTROLBUFFER_FIRSTMESSAGE(buffer);
    } else {
        NW_CONFIDENCE(((UI(prevMessage) - UI(NW_PLUGCONTROLBUFFER_FIRSTMESSAGE(buffer))) % NW_CONTROL_MESSAGE_SIZE) == 0);
        lastMessageNr = (UI(prevMessage) - UI(NW_PLUGCONTROLBUFFER_FIRSTMESSAGE(buffer)))/
            NW_CONTROL_MESSAGE_SIZE + 1;
        if (lastMessageNr < totalNrOfMessages) {
            result = (nw_plugControlMessage)(UI(prevMessage) + NW_CONTROL_MESSAGE_SIZE);
        } else {
            result = NULL;
        }
    }
    
    *more = ((lastMessageNr+1) < totalNrOfMessages);
    return result;
}
