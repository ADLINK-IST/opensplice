
#ifndef NW_PLUGRECEIVECHANNEL_H
#define NW_PLUGRECEIVECHANNEL_H

#include "nw_plugTypes.h"
#include "nw_plugChannel.h"

NW_CLASS(nw_plugReceiveChannel);

/* Note the symmetry between the read and write functions:
 * 
 * 
 *  |                          >------> ReadWait
 * \|/  WriteMessageStart      |      |   ReadMessageStart
 *  |     WriteGetNextFragment ^      |     ReadGetNextFragment
 *  |   WriteMessageEnd        |     \|/  ReadMessageEnd
 *  | WriteFlush               |      |   
 *  >---->---->---->---->---->-^      |
 */
/* Read functions and corresponding buffer management functions */

/* Read data from socket into internal buffers and handle control messages
 */
void           nw_plugReceiveChannelProcessIncoming(
                   nw_plugChannel channel); 


/* Blocking call returning a pointer to the next message. Function returns
 * {NULL, 0} if nw_plugReceiveChannelWakeup has been called */
c_bool         nw_plugReceiveChannelMessageStart(
                   nw_plugChannel channel,
                   nw_data *data,
                   nw_length *length,
                   nw_address *senderAddress);
                   
/* Non-blocking call returning the remaining part of a message if fragmentation
 * has occurred over the previous buffer */
void           nw_plugReceiveChannelGetNextFragment(
                   nw_plugChannel channel,
                   nw_data *data,
                   nw_length *length);

/* Non-blocking call that throws away all buffers related to the current message */
void           nw_plugReceiveChannelMessageIgnore(
                   nw_plugChannel channel);
                   
/* Non-blocking call freeing the resources used for the message retrieved
 * with the MessageStart function */
void           nw_plugReceiveChannelMessageEnd(
                   nw_plugChannel channel);

/* Wake up from the wait; this can be called for any reason. As a result,
 * the nw_plugReceiveChannelMessageStart will return with the value FALSE */
void           nw_plugReceiveChannelWakeUp(
                   nw_plugChannel channel);

#endif /*NW_PLUGRECEIVECHANNEL_H */
