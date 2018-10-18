/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef D__REMOTE_READER_LISTENER_H
#define D__REMOTE_READER_LISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

d_remoteReaderListener d_remoteReaderListenerNew          (d_subscriber subscriber);
void                   d_remoteReaderListenerFree         (d_remoteReaderListener listener);
c_bool                 d_remoteReaderListenerStop         (d_remoteReaderListener listener);
c_bool                 d_remoteReaderListenerStart        (d_remoteReaderListener listener);
void                   d_remoteReaderListenerCheckReaders (d_remoteReaderListener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D__REMOTE_READER_LISTENER_H */

