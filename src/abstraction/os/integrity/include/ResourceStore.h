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
#ifndef RESOURCE_STORE_H
#define RESOURCE_STORE_H

#include <INTEGRITY.h>

enum Command 
{
   RSCreateSemaphore,
   RSCreateCondition,
   RSGetSemaphore,
   RSGetCondition,
   RSRemoveCondition,
   RSRemoveSemaphore,
   RSShutdown
};

Error rsReceiveReturn(void);

#define DDS_RES_CONN_LOCK_NAME "DDS_ConnectionLock"
#define DDS_RES_CONN_NAME "DDS_Connection"
#define DDS_RES_PASSWD "dds_res"
#define DDS_MAX_SEMAPHORES 4096
#define DDS_MAX_CONDITIONS 4096
#define DDS_MAX_TRIES_TO_LOCATE_RESOURCESTORE 80

extern Semaphore os_connLock;
extern Connection os_conn;

#endif
