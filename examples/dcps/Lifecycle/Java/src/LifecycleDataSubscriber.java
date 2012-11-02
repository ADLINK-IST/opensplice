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

 * LOGICAL_NAME:    LifecycleSubscriber.java

 * FUNCTION:        OpenSplice Tutorial example code.

 * MODULE:          Tutorial for the Java programming language.

 * DATE             September 2010.

 ************************************************************************

 * 
 * This file contains the implementation for the 'LifecycleSubscriber' executable.

 * 
 ***/

import DDS.*;
import LifecycleData.*;

public class LifecycleDataSubscriber {
	static String sSampleState[] = { "READ_SAMPLE_STATE",
			"NOT_READ_SAMPLE_STATE" };
	static String sViewState[] = { "NEW_VIEW_STATE", "NOT_NEW_VIEW_STATE" };
	static String sInstanceState[] = { "ALIVE_INSTANCE_STATE",
			"NOT_ALIVE_DISPOSED_INSTANCE_STATE",
			"NOT_ALIVE_NO_WRITERS_INSTANCE_STATE" };

	static int index(int i) {
		int j = (int) (Math.log10((float) i) / Math.log10((float) 2));
		return j;
	}

	public static void main(String[] args) {
		MsgSeqHolder msgList = new MsgSeqHolder();
		SampleInfoSeqHolder infoSeq = new SampleInfoSeqHolder();


		// ------------------ Msg topic --------------------//

		DDSEntityManager mgr = new DDSEntityManager();

		// create domain participant
		mgr.createParticipant("Lifecycle example");

		// create type
		MsgTypeSupport mt = new MsgTypeSupport();
		mgr.registerType(mt);

		// create Topic
		mgr.createTopic("Lifecycle_Msg");

		// create Subscriber
		mgr.createSubscriber();

		// create DataReader
		mgr.createReader();

		DataReader dreader = mgr.getReader();
		MsgDataReader LifecycleReader = MsgDataReaderHelper.narrow(dreader);
		ErrorHandler.checkHandle(LifecycleReader, "MsgDataReader_narrow");

		boolean closed = false;
		int status;
		int nbIter = 1;
		int nbIterMax = 100;
		while ((!closed) && (nbIter < nbIterMax)){
			status = LifecycleReader.read(msgList, infoSeq,
						LENGTH_UNLIMITED.value, ANY_SAMPLE_STATE.value,
						ANY_VIEW_STATE.value, ANY_INSTANCE_STATE.value);
			ErrorHandler.checkStatus(status, "msgDataReader::read");
			for (int j = 0; j < msgList.value.length; j++) {
				System.out.println('\n' + "Message : " + msgList.value[j].message);
				System.out.println("writerStates : " + msgList.value[j].writerStates);
				System.out.println("valid_data : 1");
				System.out.println("sample_state:"+ sSampleState[index(infoSeq.value[j].sample_state)]
						  + "-view_state:" + sViewState[index(infoSeq.value[j].view_state)]
						  + "-instance_state:" + sInstanceState[index(infoSeq.value[j].instance_state)]);
				// sleep(200);
				try {
					Thread.sleep(200);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				closed = (msgList.value[j].writerStates.equals("STOPPING_SUBSCRIBER"));
			}
			status = LifecycleReader.return_loan(msgList, infoSeq);
			ErrorHandler.checkStatus(status, "MsgDataReader.return_loan");
			// sleep(2);
			try {
				Thread.sleep(20);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			nbIter++;
		}
		System.out.println("=== [Subscriber] stopping after " + nbIter + "iterations - closed = " + closed);
		if ( nbIter == nbIterMax) System.out.println("*** Error : max " + nbIterMax +   "iterations reached");

		// cleanup

		mgr.getSubscriber().delete_datareader(LifecycleReader);

		mgr.deleteSubscriber();
		mgr.deleteTopic();
		mgr.deleteParticipant();

	}
}
