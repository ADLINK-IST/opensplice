/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

/************************************************************************
 * LOGICAL_NAME:    LifecyclePublisher.cpp
 * FUNCTION:        Publisher's main for the Lifecycle OpenSplice example.
 * MODULE:          OpenSplice Lifecycle example for the C programming language.
 * DATE             September 2010.
 ************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include "os_stdlib.h"
#endif
#include <string.h>
#include "dds_dcps.h"
#include "CheckStatus.h"
#include "DDSEntitiesManager.h"
#include "LifecycleData.h"

#include "example_main.h"

void usage()
{
   fprintf(stderr, "\n*** ERROR");
   fprintf(stderr, "\n***    usage: LifecyclePublisher <autodispose_flag> <writer_action>");
   fprintf(stderr, "\n***         . autodispose_flag = false | true");
   fprintf(stderr, "\n***         . dispose | unregister | stoppub");
   exit(-1);
}

int OSPL_MAIN (int argc, const char *argv[])
{
   os_time os_delay_500ms = {  0, 500000000  };
   os_time os_delay_200ms =  {  0, 200000000  };
   DDS_boolean autodispose_unregistered_instances = FALSE;

   DDS_Publisher msgPublisher;
   DDS_DataWriter msgWriter;
   DDS_DataWriter msgWriter_stopper;

   DDS_unsigned_long messageLength;
   LifecycleData_Msg *sample_Msg;

   printf("\n\n Starting LifecyclePublisher...");

   if( argc < 3 )
   {
      usage();
   }
   if ((strcmp(argv[1], "false") != 0) &&
     (strcmp(argv[1], "true") != 0) &&
     (strcmp(argv[2], "dispose") != 0) &&
     (strcmp(argv[2], "unregister") != 0) &&
     (strcmp(argv[2], "stoppub") != 0))
   {
    usage();
   }

   autodispose_unregistered_instances = (strcmp(argv[1], "true") == 0);

   // First initialize the Topics and their DDS Entities

   // Create DDS DomainParticipant
   createParticipant("Lifecycle example");

   //------------------ Msg topic --------------------//

   // Register Msg Topic's type in the DDS Domain.
   g_msgTypeSupport = LifecycleData_MsgTypeSupport__alloc();
   checkHandle(g_msgTypeSupport, "LifecycleData_MsgTypeSupport__alloc");
   registerMsgType(g_msgTypeSupport);
   // Create Msg Topic in the DDS Domain.
   g_msgTypeName = LifecycleData_MsgTypeSupport_get_type_name(g_msgTypeSupport);
   g_msgTopic = createTopic("Lifecycle_Msg", g_msgTypeName);
   DDS_free(g_msgTypeName);
   DDS_free(g_msgTypeSupport);

   // Create the Publisher's in the DDS Domain.
   msgPublisher = createPublisher();

   // Request a Writer from the the Publisher.
   msgWriter = createDataWriter(msgPublisher, g_msgTopic, autodispose_unregistered_instances);
   msgWriter_stopper = createDataWriter(msgPublisher, g_msgTopic, autodispose_unregistered_instances);
   //End initialization.
   os_nanoSleep(os_delay_200ms);

   // Start publishing...
   printf("\n=== [Publisher] Ready...");

  if (strcmp(argv[2], "dispose") == 0)
      {
        // Publish a Msg Sample and dispose the instance
         LifecycleData_Msg *sample_Msg = LifecycleData_Msg__alloc();
        sample_Msg->userID = 1;
        messageLength = sizeof("Lifecycle_1") - 1;
        sample_Msg->message = DDS_string_alloc(messageLength);
        snprintf(sample_Msg->message, messageLength + 1, "%s", "Lifecycle_1");
        messageLength = sizeof("SAMPLE_SENT -> INSTANCE_DISPOSED -> DATAWRITER_DELETED") - 1;
        sample_Msg->writerStates = DDS_string_alloc(messageLength);
        snprintf(sample_Msg->writerStates, messageLength + 1, "%s", "SAMPLE_SENT -> INSTANCE_DISPOSED -> DATAWRITER_DELETED");
        printf("\n=== [Publisher]  :");
        printf("\n    userID  : %d", sample_Msg->userID);
        printf("\n    Message : %s", sample_Msg->message);
        printf("\n    writerStates : %s", sample_Msg->writerStates);
        g_status = LifecycleData_MsgDataWriter_write(msgWriter, sample_Msg, DDS_HANDLE_NIL);
        checkStatus(g_status, "MsgDataWriter_write");
        os_nanoSleep(os_delay_500ms);
        printf("\n=== [Publisher]  : SAMPLE_SENT");
        fflush(stdout);
        // Dispose instance
        g_status = LifecycleData_MsgDataWriter_dispose(msgWriter, sample_Msg, DDS_HANDLE_NIL);
        checkStatus(g_status, "LifecycleData_MsgDataWriter_dispose");
        printf("\n=== [Publisher]  : INSTANCE_DISPOSED");
        fflush(stdout);
        // OpenSplice will recursively free the content of the sample,
        // provided it has all been allocated through the DDS_xxxx_alloc API...
        DDS_free(sample_Msg);

      }
    else if (strcmp(argv[2], "unregister") == 0)
      {

        // Publish a Msg Sample and unregister the instance
        LifecycleData_Msg *sample_Msg = LifecycleData_Msg__alloc();
        sample_Msg->userID = 1;
        messageLength = sizeof("Lifecycle_2") - 1;
        sample_Msg->message = DDS_string_alloc(messageLength);
        snprintf(sample_Msg->message, messageLength + 1, "%s", "Lifecycle_2");
        messageLength = sizeof("SAMPLE_SENT -> INSTANCE_UNREGISTERED -> DATAWRITER_DELETED") - 1;
        sample_Msg->writerStates = DDS_string_alloc(messageLength);
        snprintf(sample_Msg->writerStates, messageLength + 1, "%s", "SAMPLE_SENT -> INSTANCE_UNREGISTERED -> DATAWRITER_DELETED");
        printf("\n=== [Publisher]  :");
        printf("\n    userID  : %d", sample_Msg->userID);
        printf("\n    Message : %s", sample_Msg->message);
        printf("\n    writerStates : %s", sample_Msg->writerStates);
        g_status = LifecycleData_MsgDataWriter_write(msgWriter, sample_Msg, DDS_HANDLE_NIL);
        checkStatus(g_status, "MsgDataWriter_write");
        os_nanoSleep(os_delay_500ms);
        printf("\n=== [Publisher]  : SAMPLE_SENT");
        fflush(stdout);
        // Unregister instance : the auto_dispose_unregistered_instances flag
        // is currently ignored and the instance is never disposed automatically
        g_status = LifecycleData_MsgDataWriter_unregister_instance(msgWriter, sample_Msg, 0);
        checkStatus(g_status, "LifecycleData_MsgDataWriter_unregister_instance");
        printf("\n=== [Publisher]  : INSTANCE_UNREGISTERED");
        fflush(stdout);
        // OpenSplice will recursively free the content of the sample,
        // provided it has all been allocated through the DDS_xxxx_alloc API...
        DDS_free(sample_Msg);
      }
    else if (strcmp(argv[2], "stoppub") == 0)
      {
        // Publish a Msg Sample
        LifecycleData_Msg *sample_Msg = LifecycleData_Msg__alloc();
        sample_Msg->userID = 1;
        messageLength = sizeof("Lifecycle_3") - 1;
        sample_Msg->message = DDS_string_alloc(messageLength);
        snprintf(sample_Msg->message, messageLength + 1, "%s", "Lifecycle_3");
        messageLength = sizeof("SAMPLE_SENT -> DATAWRITER_DELETED") - 1;
        sample_Msg->writerStates = DDS_string_alloc(messageLength);
        snprintf(sample_Msg->writerStates, messageLength + 1, "%s", "SAMPLE_SENT -> DATAWRITER_DELETED");
        printf("\n=== [Publisher]  :");
        printf("\n    userID  : %d", sample_Msg->userID);
        printf("\n    Message : %s", sample_Msg->message);
        printf("\n    writerStates : %s", sample_Msg->writerStates);
        g_status = LifecycleData_MsgDataWriter_write(msgWriter, sample_Msg, DDS_HANDLE_NIL);
        checkStatus(g_status, "MsgDataWriter_write");
        os_nanoSleep(os_delay_500ms);
        printf("\n=== [Publisher]  : SAMPLE_SENT");
        fflush(stdout);
        // OpenSplice will recursively free the content of the sample,
        // provided it has all been allocated through the DDS_xxxx_alloc API...
        DDS_free(sample_Msg);
      }
   // let the subscriber treat the previous writer state !!!!
   printf("\n=== [Publisher] waiting 500ms to let the subscriber treat the previous write state ...");
   fflush(stdout);
   os_nanoSleep(os_delay_500ms);

   /* Remove the DataWriters */
   deleteDataWriter(msgPublisher, &msgWriter);
   printf("\n=== [Publisher]  : DATAWRITER_DELETED");
   fflush(stdout);
   os_nanoSleep(os_delay_500ms);
   printf("\n=== [Publisher]  : Sending a message to stop the subscriber");
   /* send a Msg sample to stop the subscriber */
   sample_Msg = LifecycleData_Msg__alloc();
   sample_Msg->userID = 1;
   messageLength = sizeof("Lifecycle_4") - 1;
   sample_Msg->message = DDS_string_alloc(messageLength);
   snprintf(sample_Msg->message, messageLength + 1, "%s", "Lifecycle_4");
   messageLength = sizeof("STOPPING_SUBSCRIBER") - 1;
   sample_Msg->writerStates = DDS_string_alloc(messageLength);
   snprintf(sample_Msg->writerStates, messageLength + 1, "%s", "STOPPING_SUBSCRIBER");
   printf("\n=== [Publisher]  :");
   printf("\n    userID  : %d", sample_Msg->userID);
   printf("\n    Message : %s", sample_Msg->message);
   printf("\n    writerStates : %s\n", sample_Msg->writerStates);
   g_status = LifecycleData_MsgDataWriter_write(msgWriter_stopper, sample_Msg, DDS_HANDLE_NIL);
   checkStatus(g_status, "MsgDataWriter_write");
   fflush(stdout);
   os_nanoSleep(os_delay_500ms);
   // OpenSplice will recursively free the content of the sample,
   // provided it has all been allocated through the DDS_xxxx_alloc API...
   DDS_free(sample_Msg);
   /* Remove the DataWriter */
   deleteDataWriter(msgPublisher, &msgWriter_stopper);

   // Cleanup DDS from the created Entities.
   deletePublisher(&msgPublisher);
   deleteTopic(g_msgTopic);
   deleteParticipant();

   return 0;
}
