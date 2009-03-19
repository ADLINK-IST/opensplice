#ifndef NW_PLUGSENDCHANNEL_H
#define NW_PLUGSENDCHANNEL_H

#include "nw_plugTypes.h"
#include "nw_plugChannel.h"

NW_CLASS(nw_plugSendChannel);
                         
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

/* Starting the writing of a message, returns a buffer of length to write into */
nw_bool        nw_plugSendChannelMessageStart(
                   nw_plugChannel channel,
                   nw_data *buffer,
                   nw_length *length,
                   nw_partitionId partitionId,
                   nw_signedLength *bytesLeft);
                   
/* Retrieve a buffer for copying data into if the previous buffer was not
 * sufficiently long. NOTE: This function currently flushes the available
 * data to the network */
void           nw_plugSendChannelGetNextFragment(
                   nw_plugChannel channel,
                   nw_data *buffer,
                   nw_length *length);

/* Indicate the end of a message */
void           nw_plugSendChannelMessageEnd(
                   nw_plugChannel channel,
                   nw_data buffer);

/* Do a flush of data messages to the network.
 * bytesLeft is in/out indicating the number of bytes allowed to be flushed.
 * Flushes to the network go in chunks of complete fragments.
 * Function returns true if all packets were flushed, false otherwise */

nw_bool        nw_plugSendChannelMessagesFlush(
                   nw_plugChannel channel,
                   nw_bool all,
                   nw_signedLength *bytesLeft /* in/out */);
                   
/* Perform periodic actions:
  *  - send control messages 
  *  - resend data
  *  - process discovery messages
  *  - determine MaxBurstSize for the coming period
  */ 
void           nw_plugSendChannelPeriodicAction(
                   nw_plugChannel channel,
                   nw_signedLength *credits /* in/out */);
                   

#endif /* NW_PLUGSENDCHANNEL_H */
