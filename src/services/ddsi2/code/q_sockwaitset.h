#ifndef Q_SOCKWAITSET_H
#define Q_SOCKWAITSET_H

#include "os_defs.h"
#include "sysdeps.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define OS_EVENT_READ 0x01
#define OS_EVENT_WRITE 0x02
#define OS_EVENT_ACCEPT OS_EVENT_READ

typedef struct os_sockWaitset_s *os_sockWaitset;
struct ddsi_tran_base;

/* Allocates a new socket waitset.
   The waitset is NOT thread-safe, but os_sockWaitsetTrigger may be
   called at any time between os_sockWaitsetNew and
   os_sockWaitsetFree.  Returns NULL on failure, a freshly
   initialised, empty waitset on success. */
os_sockWaitset os_sockWaitsetNew (void);

/* Frees the socket waitset WS.  Any sockets associated with it will
   be dissociated from it. */
void os_sockWaitsetFree (os_sockWaitset ws);

/* Triggers the waitset, from any thread.  It is level
   triggered, when called while no thread is waiting in
   os_sockWaitsetWait the trigger will cause an (early) wakeup on the
   next call to os_sockWaitsetWait.  Returns os_resultSuccess if
   successfully triggered, os_resultInvalid if an error occurs.

   Triggering a waitset may require resources and they may be counted.
   Do not trigger a waitset arbitrarily often without ensuring
   os_sockWaitsetWait is called often enough to let it release any
   resources used.

   Shared state updates preceding os_sockWaitsetTrigger are visible
   following os_sockWaitsetWait. */
os_result os_sockWaitsetTrigger (os_sockWaitset ws);

/* Adds socket SOCK to socket waitset WS, with EVENTS specifying the
   events of interest as a bit-wise or of some of the following flags:

     OS_EVENT_READ   Triggers when data is available for reading
     OS_EVENT_WRITE  Triggers when can accept more data for transmission
     OS_EVENT_ACCEPT Triggers when listener can accept connection 

   A socket may be associated with only one waitset at any time, and
   may be added to the waitset only once.  Failure to comply with this
   restriction results in undefined behaviour.

   Returns os_resultSuccess if the socket was successfully added to
   the waitset, os_resultInvalid if an attempt was made to add a
   socket to a full socket waitset or undefined event flags were
   specified, or os_resultFail if any of the underlying operating
   system operations fail.

   Behaviour is undefined when called after a successful wait but
   before all events had been enumerated.

   Closing socket associated with a waitset is handled gracefully: no
   operations will signal errors because of it. */
os_result os_sockWaitsetAdd (os_sockWaitset ws, struct ddsi_tran_base * base, unsigned events);

/* Drops all sockets from the waitset from index onwards. Index
   0 corresponds to the first socket added to the waitset, index 1 to
   the second, etc. Behaviour is undefined when called after a successful wait
   but before all events had been enumerated.
*/
void os_sockWaitsetPurge (os_sockWaitset ws, unsigned index);

/* Waits until some of the sockets in WS have been triggered or the
   timeout has elapsed.  If timeout_ms is:

     = -1         waits indefinitely
     =  0         polls the state, then returns immediately
     in [1, 999]  times out in timeout_ms ms
     otherwise    invalid

   Returns os_resultSuccess if some socket was triggered, or
   os_resultTimeout if the timeout elapsed before a trigger occurred.
   However, the return may be spurious (i.e., no events, yet no
   timeout signalled), and a timeout does not imply no event has
   occurred (this is by the very nature of asynchronous events), and
   the return value is therefore only indicative of the cause.

   If the return value is os_resultSuccess, the available events MUST
   be enumerated before os_sockWaitsetAdd may be called again.

   If timeout_ms is invalid, the waitset is empty, or one has failed
   to enumerate the events following a preceding call to
   os_sockWaitsetWait that returned os_resultSuccess, the behaviour is
   undefined.

   Shared state updates preceding os_sockWaitsetTrigger are visible
   following os_sockWaitsetWait. */
os_result os_sockWaitsetWait (os_sockWaitset ws, int timeout_ms);

/* Returns the index of the next triggered socket in the socket
   waitset WS, or -1 if the set of available events has been
   exhausted.  Index 0 is the first socket added to the waitset, index
   1 the second, &c.

   Following a call to os_sockWaitsetWait on WS that returned
   os_resultSuccess, one MUST enumerate all available events before
   os_sockWaitsetWait may be called again on the WS.

   If the return value is >= 0, *sock contains the socket and *events
   the available events on that socket.  For the defined events, see
   os_sockWaitsetAdd. The value of any bits in the returned
   events other than those specified in os_sockWaitsetAdd is
   undefined. */
int os_sockWaitsetNextEvent (os_sockWaitset ws, struct ddsi_tran_base ** base, unsigned *events);

/* Remove connection */
void os_sockWaitsetRemove (os_sockWaitset ws, struct ddsi_tran_base * base);

#if defined (__cplusplus)
}
#endif

#endif /* Q_SOCKWAITSET_H */

/* SHA1 not available (unoffical build.) */
