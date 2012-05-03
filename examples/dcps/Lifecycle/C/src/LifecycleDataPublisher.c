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
#endif
#include <string.h>
#include "dds_dcps.h"
#include "CheckStatus.h"
#include "DDSEntitiesManager.h"
#include "LifecycleData.h"

void usage()
{
   fprintf(stderr, "\n*** ERROR");
   fprintf(stderr, "\n***    usage: LifecyclePublisher <autodispose_flag> <writer_action>");
   fprintf(stderr, "\n***         . autodispose_flag = false | true");
   fprintf(stderr, "\n***         . dispose | unregister | stoppub");
   exit(-1);
}

main(int argc, const char *argv[])
{
   os_time os_delay_500ms = {  0, 500000000  };
   os_time os_delay_200ms =  {  0, 200000000  };
   os_time os_delay_2ms = {  0, 2000000  };
   int x, i;
   DDS_boolean autodispose_unregistered_instances = FALSE;

   DDS_Publisher writerStatePublisher;
   DDS_DataWriter writerStateWriter;
   DDS_Publisher msgPublisher;
   DDS_DataWriter msgWriter;
   DDS_DataWriter msgWriter_stopper;

   int messageLength;
   LifecycleData_Msg *sample_Msg;
    
   // Force the output to be unbuffered.
   setbuf(stdout, (char *) 0);

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
   g_msgTypeName = (char*) LifecycleData_MsgTypeSupport_get_type_name(g_msgTypeSupport);
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
        messageLength = strlen("Lifecycle_1");
        sample_Msg->message = DDS_string_alloc(messageLength);
        snprintf(sample_Msg->message, messageLength + 1, "%s", "Lifecycle_1");
        messageLength = strlen("SAMPLE_SENT -> INSTANCE_DISPOSED -> DATAWRITER_DELETED");
        sample_Msg->writerStates = DDS_string_alloc(messageLength);
        snprintf(sample_Msg->writerStates, messageLength + 1, "%s", "SAMPLE_SENT -> INSTANCE_DISPOSED -> DATAWRITER_DELETED");
        printf("\n=== [Publisher]  :");
        printf("\n    userID  : %d", sample_Msg->userID);
        printf("\n    Message : %s", sample_Msg->message);
        printf("\n    writerStates : %s", sample_Msg->writerStates);
        g_status = LifecycleData_MsgDataWriter_write(msgWriter, sample_Msg, 0);
        checkStatus(g_status, "MsgDataWriter_write");
	os_nanoSleep(os_delay_500ms);
	printf("\n=== [Publisher]  : SAMPLE_SENT");
	
        // Dispose instance
        g_status = LifecycleData_MsgDataWriter_dispose(msgWriter, sample_Msg, 0);
        checkStatus(g_status, "LifecycleData_MsgDataWriter_dispose");
	printf("\n=== [Publisher]  : INSTANCE_DISPOSED");
	
        // OpenSplice will recursively free the content of the sample,
        // provided it has all been allocated through the DDS_xxxx_alloc API...
        DDS_free(sample_Msg);

      }
    else if (strcmp(argv[2], "unregister") == 0)
      {

        // Publish a Msg Sample and unregister the instance
        LifecycleData_Msg *sample_Msg = LifecycleData_Msg__alloc();
        sample_Msg->userID = 1;
        messageLength = strlen("Lifecycle_2");
        sample_Msg->message = DDS_string_alloc(messageLength);
        snprintf(sample_Msg->message, messageLength + 1, "%s", "Lifecycle_2");
        messageLength = strlen("SAMPLE_SENT -> INSTANCE_UNREGISTERED -> DATAWRITER_DELETED");
        sample_Msg->writerStates = DDS_string_alloc(messageLength);
        snprintf(sample_Msg->writerStates, messageLength + 1, "%s", "SAMPLE_SENT -> INSTANCE_UNREGISTERED -> DATAWRITER_DELETED");
        printf("\n=== [Publisher]  :");
        printf("\n    userID  : %d", sample_Msg->userID);
        printf("\n    Message : %s", sample_Msg->message);
        printf("\n    writerStates : %s", sample_Msg->writerStates);
        g_status = LifecycleData_MsgDataWriter_write(msgWriter, sample_Msg, 0);
        checkStatus(g_status, "MsgDataWriter_write");
        os_nanoSleep(os_delay_500ms);
	printf("\n=== [Publisher]  : SAMPLE_SENT");

        // Unregister instance : the auto_dispose_unregistered_instances flag
        // is currently ignored and the instance is never disposed automatically
        g_status = LifecycleData_MsgDataWriter_unregister_instance(msgWriter, sample_Msg, 0);
        checkStatus(g_status, "LifecycleData_MsgDataWriter_unregister_instance");
	printf("\n=== [Publisher]  : INSTANCE_UNREGISTERED");
 	
        // OpenSplice will recursively free the content of the sample,
        // provided it has all been allocated through the DDS_xxxx_alloc API...
        DDS_free(sample_Msg);
      }
    else if (strcmp(argv[2], "stoppub") == 0)
      {
        // Publish a Msg Sample
        LifecycleData_Msg *sample_Msg = LifecycleData_Msg__alloc();
        sample_Msg->userID = 1;
        messageLength = strlen("Lifecycle_3");
        sample_Msg->message = DDS_string_alloc(messageLength);
        snprintf(sample_Msg->message, messageLength + 1, "%s", "Lifecycle_3");
        messageLength = strlen("SAMPLE_SENT -> DATAWRITER_DELETED");
        sample_Msg->writerStates = DDS_string_alloc(messageLength);
        snprintf(sample_Msg->writerStates, messageLength + 1, "%s", "SAMPLE_SENT -> DATAWRITER_DELETED");
        printf("\n=== [Publisher]  :");
        printf("\n    userID  : %d", sample_Msg->userID);
        printf("\n    Message : %s", sample_Msg->message);
        printf("\n    writerStates : %s", sample_Msg->writerStates);
        g_status = LifecycleData_MsgDataWriter_write(msgWriter, sample_Msg, 0);
        checkStatus(g_status, "MsgDataWriter_write");
        os_nanoSleep(os_delay_500ms);
	printf("\n=== [Publisher]  : SAMPLE_SENT");       
	// OpenSplice will recursively free the content of the sample,
        // provided it has all been allocated through the DDS_xxxx_alloc API...
        DDS_free(sample_Msg);
      }
   // let the subscriber treat the previous writer state !!!!
   printf("\n=== [Publisher] waiting 500ms to let the subscriber treat the previous write state ...");
   os_nanoSleep(os_delay_500ms); 
  
   /* Remove the DataWriters */
   deleteDataWriter(msgPublisher, &msgWriter);
   printf("\n=== [Publisher]  : DATAWRITER_DELETED");
   os_nanoSleep(os_delay_500ms);
   
   printf("\n=== [Publisher]  : Sending a message to stop the subscriber");
   /* send a Msg sample to stop the subscriber */
   sample_Msg = LifecycleData_Msg__alloc();
   sample_Msg->userID = 1;
   messageLength = strlen("Lifecycle_4");
   sample_Msg->message = DDS_string_alloc(messageLength);
   snprintf(sample_Msg->message, messageLength + 1, "%s", "Lifecycle_4");
   messageLength = strlen("STOPPING_SUBSCRIBER");
   sample_Msg->writerStates = DDS_string_alloc(messageLength);
   snprintf(sample_Msg->writerStates, messageLength + 1, "%s", "STOPPING_SUBSCRIBER");
   printf("\n=== [Publisher]  :");
   printf("\n    userID  : %d", sample_Msg->userID);
   printf("\n    Message : %s", sample_Msg->message);
   printf("\n    writerStates : %s\n", sample_Msg->writerStates);
   g_status = LifecycleData_MsgDataWriter_write(msgWriter_stopper, sample_Msg, 0);
   checkStatus(g_status, "MsgDataWriter_write");
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
