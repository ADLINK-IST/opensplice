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
 *  
 * Copyright (c) 2007
 * PrismTech Ltd.
 * All rights Reserved.
 * 
 * LOGICAL_NAME:    ListenerDataSubscriber.java
 * FUNCTION:        OpenSplice example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             April 2009.
 ************************************************************************
 * 
 * This file contains the implementation for the 'ListenerDataSubscriber' executable.
 * 
 ***/

import DDS.*;
import ListenerData.MsgDataReaderHelper;
import ListenerData.MsgTypeSupport;

public class ListenerDataSubscriber {

	public static void main(String[] args) {

		DDSEntityManager mgr = new DDSEntityManager();

		// create domain participant
		mgr.createParticipant("Listener example");

		// create type
		MsgTypeSupport st = new MsgTypeSupport();
		mgr.registerType(st);

		// create Topic
		mgr.createTopic("ListenerData_Msg");

		// create Subscriber
		mgr.createSubscriber();

		// create DataReader
		mgr.createReader();

		DataReader dreader = mgr.getReader();
		ListenerDataListener myListener = new ListenerDataListener();
		myListener.m_MsgReader = MsgDataReaderHelper.narrow(dreader);
		ErrorHandler
				.checkHandle(myListener.m_MsgReader, "MsgDataReader_narrow");

		// System.out.println( "=== [Subscriber] set_listener" );
		int mask = DDS.DATA_AVAILABLE_STATUS.value
				| DDS.REQUESTED_DEADLINE_MISSED_STATUS.value;
		myListener.m_MsgReader.set_listener(myListener, mask);
		myListener.m_guardCond.set_trigger_value(false);
		// System.out.println( "=== [SubscriberUsingListener] Ready ..." );
		myListener.m_closed = false;

		// waitset used to avoid spinning in the loop below
		WaitSet ws = new WaitSet();
		ws.attach_condition(myListener.m_guardCond);
		ConditionSeqHolder condSeq = new ConditionSeqHolder();
		int count = 0;
		while (!myListener.m_closed && count < 1500) {

			// To avoid spinning here. We can either use a sleep or better a
			// WaitSet.
			// System.out.println(
			// "=== [SubscriberUsingListener] waiting waitset ..." );

			DDS.Duration_t wait_timeout = new DDS.Duration_t(
					0,
					200000000);

			ws._wait(condSeq, wait_timeout);
			myListener.m_guardCond.set_trigger_value(false);
			++count;
		}

		// cleanup

		mgr.getSubscriber().delete_datareader(myListener.m_MsgReader);
		mgr.deleteSubscriber();
		mgr.deleteTopic();
		mgr.deleteParticipant();
	}
}
