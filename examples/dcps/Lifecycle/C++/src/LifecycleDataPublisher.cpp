
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
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             September 2010.
 ************************************************************************
 * 
 * This file contains the implementation for the 'LifecyclePublisher' executable.
 * 
 ***/
#include <string>
#include <sstream>
#include <iostream>
#include "DDSEntityManager.h"
#include "ccpp_LifecycleData.h"
#include "os.h"


using namespace DDS;
using namespace LifecycleData;

void usage()
{
  //usage : LifecyclePublisher <autodispose_flag>
  //		. autodispose_flag = false | true
  cerr << "*** ERROR" << endl;
  cerr << "*** usage : LifecyclePublisher <autodispose_flag> <writer_action>" << endl;
  cerr << "***         . autodispose_flag = false | true" << endl;
  cerr << "***         . dispose | unregister | stoppub" << endl;
  exit( - 1);
}

int main(int argc, char *argv[])
{
  os_time delay_200ms = { 0, 200000000 };
  os_time delay_500ms = { 0, 500000000 };
  cout << "argc=" << argc << endl;
  if (argc < 3)
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
 
  bool autodispose_unregistered_instances = (strcmp(argv[1], "true") == 0);

  // create domain participant
  char partition_name[] = "Lifecycle example";
 
  //------------------ Msg topic --------------------//
  DDSEntityManager *mgr = new DDSEntityManager
    (autodispose_unregistered_instances);
  // create domain participant
  mgr->createParticipant(partition_name);
  //create type
  MsgTypeSupport_var mt = new MsgTypeSupport();
  mgr->registerType(mt.in());

  //create Topic
  char topic_name[] = "Lifecycle_Msg";
  mgr->createTopic(topic_name);

  //create Publisher
  mgr->createPublisher();

  // create DataWriters
  mgr->createWriters();
  DataWriter_ptr dwriter = mgr->getWriter();
  MsgDataWriter_var LifecycleWriter = MsgDataWriter::_narrow(dwriter);
  DataWriter_ptr dwriter_stopper = mgr->getWriter_stopper();
  MsgDataWriter_var LifecycleWriter_stopper = MsgDataWriter::_narrow(dwriter_stopper);
  os_nanoSleep(delay_500ms); 
 
  ReturnCode_t status;
  if (strcmp(argv[2], "dispose") == 0) 
      {
        Msg *msgInstance = new Msg();
        msgInstance->userID = 1;
        msgInstance->message = CORBA::string_dup("Lifecycle_1");
        msgInstance->writerStates = CORBA::string_dup("SAMPLE_SENT -> INSTANCE_DISPOSED -> DATAWRITER_DELETED");
        cout << "=== [Publisher]  :" << endl;
        cout << "    userID  : " << msgInstance->userID << endl;
        cout << "    Message : \"" << msgInstance->message << "\"" << endl;
        cout << "    writerStates : \"" << msgInstance->writerStates << "\"" << endl;
        status = LifecycleWriter->write(*msgInstance, NULL);
        checkStatus(status, "MsgDataWriter::write");
        os_nanoSleep(delay_500ms);	    //
        cout << "=== [Publisher]  : SAMPLE_SENT" << endl;

        // Dispose instance
        status = LifecycleWriter->dispose(*msgInstance, NULL);
        checkStatus(status, "MsgDataWriter::dispose");
		cout << "=== [Publisher]  : INSTANCE_DISPOSED" << endl;
        /* Release the data-samples. */
        delete msgInstance;
        // msg allocated on heap: explicit de-allocation required!!
      }
    else if (strcmp(argv[2], "unregister") == 0)
      {
        Msg *msgInstance = new Msg();
        msgInstance->userID = 2;
        msgInstance->message = CORBA::string_dup("Lifecycle_2");
        msgInstance->writerStates = CORBA::string_dup("SAMPLE_SENT -> INSTANCE_UNREGISTERED -> DATAWRITER_DELETED");
        cout << "=== [Publisher]  :" << endl;
        cout << "    userID  : " << msgInstance->userID << endl;
        cout << "    Message : \"" << msgInstance->message << "\"" << endl;
        cout << "    writerStates : \"" << msgInstance->writerStates << "\"" << endl;
        status = LifecycleWriter->write(*msgInstance, NULL);
        checkStatus(status, "MsgDataWriter::write");
        os_nanoSleep(delay_500ms);	    //
        cout << "=== [Publisher]  : SAMPLE_SENT" << endl;
		
        // Unregister instance : the auto_dispose_unregistered_instances flag
        // is currently ignored and the instance is never disposed automatically
        status = LifecycleWriter->unregister_instance(*msgInstance, NULL);
        checkStatus(status, "MsgDataWriter::unregister_instance");
		cout << "=== [Publisher]  : INSTANCE_UNREGISTERED" << endl;
 
        /* Release the data-samples. */
        delete msgInstance;
        // msg allocated on heap: explicit de-allocation required!!
      }
    else if (strcmp(argv[2], "stoppub") == 0)
      {
        Msg *msgInstance = new Msg();
        msgInstance->userID = 3;
        msgInstance->message = CORBA::string_dup("Lifecycle_3");
        msgInstance->writerStates = CORBA::string_dup("SAMPLE_SENT -> DATAWRITER_DELETED");
        cout << "=== [Publisher]  :" << endl;
        cout << "    userID  : " << msgInstance->userID << endl;
        cout << "    Message : \"" << msgInstance->message << "\"" << endl;
        cout << "    writerStates : \"" << msgInstance->writerStates << "\"" << endl;
        status = LifecycleWriter->write(*msgInstance, NULL);
        checkStatus(status, "MsgDataWriter::write");
        os_nanoSleep(delay_500ms);	    //
        cout << "=== [Publisher]  : SAMPLE_SENT" << endl;
        /* Release the data-samples. */
        delete msgInstance;
        // msg allocated on heap: explicit de-allocation required!!
      }


  // let the subscriber treat the previous writer state !!!!
  cout << "=== [Publisher] waiting 500ms to let the subscriber treat the previous write state ..." << endl;
  os_nanoSleep(delay_500ms); 
  
  /* Remove the DataWriters */
  mgr->deleteWriter(LifecycleWriter.in ());
  os_nanoSleep(delay_500ms); 
  cout << "=== [Publisher]  : DATAWRITER_DELETED" << endl;

  
  // Stop the subscriber
 
  Msg *msgInstance = new Msg();
  msgInstance->userID = 4;
  msgInstance->message = CORBA::string_dup("Lifecycle_4");
  msgInstance->writerStates = CORBA::string_dup("STOPPING_SUBSCRIBER");
  cout << "=== [Publisher]  :" << endl;
  cout << "    userID  : " << msgInstance->userID << endl;
  cout << "    Message : \"" << msgInstance->message << "\"" << endl;
  cout << "    writerStates : \"" << msgInstance->writerStates << "\"" << endl;
  status = LifecycleWriter_stopper->write(*msgInstance, NULL);
  checkStatus(status, "MsgDataWriter::write");
  os_nanoSleep(delay_500ms);	    //
  //cout << "=== [Publisher]  : SAMPLE_SENT" << endl;
   /* Release the data-samples. */
  delete msgInstance;
  // msg allocated on heap: explicit de-allocation required!!
  /* Remove the DataWriter_stopper */
  mgr->deleteWriter(LifecycleWriter_stopper.in ());

  /* Remove the Publisher. */
  mgr->deletePublisher();

  /* Remove the Topics. */
  mgr->deleteTopic();

  mgr->deleteParticipant();

  delete mgr;

  return 0;
}
