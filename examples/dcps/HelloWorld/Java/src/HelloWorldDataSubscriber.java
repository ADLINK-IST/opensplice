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
 * LOGICAL_NAME:    HelloWorldSubscriber.java
 * FUNCTION:        Publisher's main for the HelloWorld OpenSplice programming example.
 * MODULE:          OpenSplice HelloWorld example for the java programming language.
 * DATE             September 2010.
 ************************************************************************/

import DDS.ANY_INSTANCE_STATE;
import DDS.ANY_SAMPLE_STATE;
import DDS.ANY_VIEW_STATE;
import DDS.DataReader;
import DDS.LENGTH_UNLIMITED;
import DDS.SampleInfoSeqHolder;
import HelloWorldData.MsgDataReader;
import HelloWorldData.MsgDataReaderHelper;
import HelloWorldData.MsgSeqHolder;
import HelloWorldData.MsgTypeSupport;

public class HelloWorldDataSubscriber {

	public static void main(String[] args) {
		DDSEntityManager mgr = new DDSEntityManager();
		String partitionName = "HelloWorld example";

		// create Domain Participant
		mgr.createParticipant(partitionName);

		// create Type
		MsgTypeSupport msgTS = new MsgTypeSupport();
		mgr.registerType(msgTS);

		// create Topic
		mgr.createTopic("HelloWorldData_Msg");

		// create Subscriber
		mgr.createSubscriber();

		// create DataReader
		mgr.createReader();

		// Read Events

		DataReader dreader = mgr.getReader();
		MsgDataReader HelloWorldReader = MsgDataReaderHelper.narrow(dreader);

		MsgSeqHolder msgSeq = new MsgSeqHolder();
		SampleInfoSeqHolder infoSeq = new SampleInfoSeqHolder();
		boolean terminate = false;
		int count = 0;
		while (!terminate && count < 1500) { // We dont want the example to run indefinitely
			HelloWorldReader.take(msgSeq, infoSeq, LENGTH_UNLIMITED.value,
					ANY_SAMPLE_STATE.value, ANY_VIEW_STATE.value,
					ANY_INSTANCE_STATE.value);
			for (int i = 0; i < msgSeq.value.length; i++) {
				if (msgSeq.value[i].message.equals("Hello World")) {
					System.out.println("=== [Subscriber] message received :");
					System.out.println("    userID  : "
							+ msgSeq.value[i].userID);
					System.out.print("    Message : \""
							+ msgSeq.value[i].message + "\"");
					terminate = true;
				}
			}
			try
			{
				Thread.sleep(200);
			}
			catch(InterruptedException ie)
			{
				// nothing to do
			}
			++count;
			
		}
                HelloWorldReader.return_loan(msgSeq, infoSeq);
		
		// clean up
		mgr.getSubscriber().delete_datareader(HelloWorldReader);
		mgr.deleteSubscriber();
		mgr.deleteTopic();
		mgr.deleteParticipant();

	}
}
