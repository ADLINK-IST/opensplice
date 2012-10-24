
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
 * LOGICAL_NAME:    DurabilityDataPublisher.cs
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C# programming language.
 * DATE             September 2010.
 ************************************************************************
 * 
 * This file contains the implementation of the Publisher for the 'Durability' example.
 * 
 ***/
#include <string>
#include <sstream>
#include <iostream>
#include "DDSEntityManager.h"
#include "ccpp_DurabilityData.h"
#include "os.h"

using namespace DDS;
using namespace DurabilityData;

void usage()
{
  //usage : DurabilityDataPublisher <durability_kind> <autodispose_flag>
  //		. durability_kind = transient | persistent
  //		. autodispose_flag = false | true
  cerr << "*** ERROR" << endl;
  cerr << "*** usage : DurabilityDataPublisher <durability_kind> <autodispose_flag> <automatic_flag>"
    << endl;
  cerr << "***         . durability_kind = transient | persistent" << endl;
  cerr << "***         . autodispose_flag = false | true" << endl;
  cerr << "***         . automatic_flag = false | true" << endl;
 exit( - 1);
}

int main(int argc, char *argv[])
{
  bool automatic = true;
  ReturnCode_t status =  - 1;
  os_time delay_180s = { 180, 0 };
  os_time delay_1s = { 1, 0 };

  if (argc < 4)
  {
    usage();
  }
  if ((strcmp(argv[1], "transient") && strcmp(argv[1], "persistent")) || 
    (strcmp(argv[2], "false") && strcmp(argv[2], "true")))
  {
    usage();
  }
  string durability_kind(argv[1]);
  bool autodispose_unregistered_instances = (strcmp(argv[2], "true") == 0);
  DDSEntityManager *mgr = new DDSEntityManager(durability_kind,
    autodispose_unregistered_instances);
  
  automatic = (strcmp(argv[3], "true") == 0);

  // Wait for the Subscriber (case of transient = true && auto_dispose = true)
  os_nanoSleep(delay_1s);

  // create domain participant
  char partition_name[] = "Durability example";
  mgr->createParticipant(partition_name);

  //create type
  MsgTypeSupport_var mt = new MsgTypeSupport();
  mgr->registerType(mt.in());

  //create Topic
  char topic_name[] = "DurabilityData_Msg";
  mgr->createTopic(topic_name);

  //create Publisher
  mgr->createPublisher();

  // create DataWriter
  mgr->createWriter();

  // Publish Events
  ostringstream buf;
  DataWriter_ptr dwriter = mgr->getWriter();
  MsgDataWriter_var DurabilityDataWriter = MsgDataWriter::_narrow(dwriter);

  Msg *instances[10];
  InstanceHandle_t userHandle[10];
  for (int x = 0; x < 10; x++)
  {
    instances[x] = new Msg();
    instances[x]->id = x;
    buf.str(string(""));
    buf << x;
    userHandle[x] = DurabilityDataWriter->register_instance(*instances[x]);
    instances[x]->content = CORBA::string_dup(buf.str().c_str());
    cout << instances[x]->content << endl;
    DurabilityDataWriter->write(*instances[x], userHandle[x]);

  }
  if (! automatic) {
     char c = 0;
     cout << "Enter E to exit" << endl;
     while (c != 'E')
     {
       cin >> c;
     }
  } 
  else {
    cout << "=== sleeping 180s..." << endl;
    os_nanoSleep(delay_180s);   
  }
  //clean up

  // transient and autodispose unregister instances
  if(autodispose_unregistered_instances)
  {
	  for (int x=0; x<10; x++)
	  {		  
		  status = DurabilityDataWriter->unregister_instance(*instances[x], userHandle[x]);
		  checkStatus(status, "DataWriter::unregister_instance");
	  }
  }

  /* Do not release the data-samples in
  order to let persistency test work*/

  /* Remove the DataWriters */
  mgr->deleteWriter(DurabilityDataWriter.in ());

  /* Remove the Publisher. */
  mgr->deletePublisher();

  /* Remove the Topics. */
  mgr->deleteTopic();

  /* Remove Participant. */
  mgr->deleteParticipant();

  return 0;
}
