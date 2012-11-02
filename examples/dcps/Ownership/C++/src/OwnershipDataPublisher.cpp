
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
 * LOGICAL_NAME:    OwnershipDataPublisher.h
 * FUNCTION:        OpenSplice Ownership example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             September 2010.
 ************************************************************************
 * 
 * This file contains the implementation for the 'OwnershipDataPublisher' executable.
 * 
 ***/

#include "OwnershipDataPublisher.h"
#include "os.h"



void OwnershipDataPublisher::initPublisher(string pub, CORBA::Long strength)
{
  mgr = new DDSEntityManager();
  char partition_name[] = "Ownership example";
  mgr->createParticipant(partition_name);
 //create type
  StockTypeSupport_var st = new StockTypeSupport();
  mgr->registerType(st.in());
  //create Topic
  char topic_name[] = "StockTrackerExclusive";
  mgr->createTopic(topic_name);
  //create Publisher
  mgr->createPublisher();

  // create DataWriter
  mgr->createWriter();

  dWriter = mgr->getWriter();
  OwnershipDataDataWriter = StockDataWriter::_narrow(dWriter);

  m_instance = new Stock();

  m_instance->ticker = CORBA::string_dup("MSFT");
  m_instance->price = 0.0;
  m_instance->publisher = CORBA::string_dup(pub.c_str());
  m_instance->strength = strength;
  //cout <<m_instance->ticker<<"/"<<m_instance->price<<"/"<<m_instance->publisher<< endl;
  userHandle = OwnershipDataDataWriter->register_instance(*m_instance);

}

OwnershipDataPublisher::OwnershipDataPublisher(string pub, CORBA::Long strength)
{
  initPublisher(pub, strength);
}


void OwnershipDataPublisher::setStrength(CORBA::Long strength)
{
  DataWriterQos dw_qos;
  status = dWriter->get_qos(dw_qos);
  checkStatus(status, "getQos");
  dw_qos.ownership_strength.value = strength;
  status = dWriter->set_qos(dw_qos);
  checkStatus(status, "setQos");
}

void OwnershipDataPublisher::publishEvent(float price, string pub)
{
  m_instance->price = price;
  m_instance->publisher = CORBA::string_dup(pub.c_str());
  OwnershipDataDataWriter->write(*m_instance, userHandle);
}

void OwnershipDataPublisher::dispose()
{
   /* Remove the DataWriters */
   mgr->deleteWriter(OwnershipDataDataWriter.in ());

   /* Remove the Publisher. */
   mgr->deletePublisher();

   /* Remove the Topics. */
   mgr->deleteTopic();

   /* Remove Participant. */
   mgr->deleteParticipant();

   delete mgr;
}

int main(int argc, char *argv[])
{
  os_time delay_200ms = { 0, 200000000 };
  os_time delay_2s = { 2, 0 };

  // usage : Publisher.exe <publisher_name> <ownership_strength> <stop_subscriber_flag>
  if (argc < 5)
  {
    cerr <<
      "*** [Publisher] usage : Publisher.exe <publisher_name> <ownership_strength> <nb_iterations> <stop_subscriber_flag>" << endl;
    exit( - 1);
  }

  OwnershipDataPublisher *pub;
  string publisher_name = argv[1];
  int ownership_strength = atoi(argv[2]);
  int nb_iteration = atoi(argv[3]);
  pub = new OwnershipDataPublisher(publisher_name, ownership_strength);
  pub->setStrength(ownership_strength);
  bool stop_subscriber = (atoi(argv[4]) == 1);

  //Publisher publishes the prices in dollars
  cout << "=== [Publisher] Publisher " << publisher_name << " with strength : "
    << ownership_strength;
  cout << " / sending " << nb_iteration << " prices ..." << " stop_subscriber flag=" << argv[4] << endl;
  // The subscriber should display the prices sent by the publisher with the highest ownership strength
  float price = 10.0f;
  for (int x = 0; x < nb_iteration; x++)
  {
    pub->publishEvent(price, publisher_name);
    os_nanoSleep(delay_200ms);
    price = price + 0.5;
  }
  os_nanoSleep(delay_2s);

  if (stop_subscriber)
  {
    // send a price = -1 to stop subscriber
    price = -1.0f;
    cout << "=== Stopping the subscriber" << endl;
    pub->publishEvent(price, publisher_name);
  }
  pub->dispose();
  delete pub;

  return 0;
};
