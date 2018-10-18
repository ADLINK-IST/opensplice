
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
#include "vortex_os.h"

#include "example_main.h"
#define MAX_MSG_LEN 256

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

int OSPL_MAIN (int argc, char *argv[])
{
  bool automatic = true;
  ReturnCode_t status =  - 1;
  os_time delay = { 30, 0 };
  os_time delay_1s = { 1, 0 };
  char buf[MAX_MSG_LEN];

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
  bool isPersistent = (strcmp(argv[1], "persistent") == 0);
  bool autodispose_unregistered_instances = (strcmp(argv[2], "true") == 0);
  DDSEntityManager mgr (durability_kind, autodispose_unregistered_instances);

  automatic = (strcmp(argv[3], "true") == 0);

  // Wait for the Subscriber (case of transient = true && auto_dispose = true)
  os_nanoSleep(delay_1s);

  // create domain participant
  char partition_name[] = "Durability example";
  mgr.createParticipant(partition_name);

  //create type
  MsgTypeSupport_var mt = new MsgTypeSupport();
  mgr.registerType(mt.in());

  //create Topic
  std::string topic_name = "CPPDurabilityData_Msg";
  if (isPersistent) {
      topic_name = "PersistentCPPDurabilityData_Msg";
  }
  mgr.createTopic(topic_name.c_str());


  //create Publisher
  mgr.createPublisher();

  // create DataWriter
  mgr.createWriter();

  // Publish Events
  DataWriter_var dwriter = mgr.getWriter();
  MsgDataWriter_var DurabilityDataWriter = MsgDataWriter::_narrow(dwriter.in());

  Msg_var instances[10];
  InstanceHandle_t userHandle[10];
  for (int x = 0; x < 10; x++)
  {
    instances[x] = new Msg();
    instances[x]->id = x;
    snprintf(buf, MAX_MSG_LEN, "%d", x);
    userHandle[x] = DurabilityDataWriter->register_instance(*instances[x]);
    instances[x]->content = DDS::string_dup(buf);
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
    //cout << "=== sleeping 30s..." << endl;
    os_nanoSleep(delay);
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
  mgr.deleteWriter(DurabilityDataWriter.in ());

  /* Remove the Publisher. */
  mgr.deletePublisher();

  /* Remove the Topics. */
  mgr.deleteTopic();

  /* Remove Participant. */
  mgr.deleteParticipant();

  return 0;
}
