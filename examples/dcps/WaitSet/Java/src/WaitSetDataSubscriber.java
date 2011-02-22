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
 * LOGICAL_NAME:    WaitSetDataSubscriber.java
 * FUNCTION:        OpenSplice example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             April 2009.
 ************************************************************************
 * 
 * This file contains the implementation for the 'WaitSetDataSubscriber' executable.
 * 
 ***/

import DDS.*;
import WaitSetData.*;

public class WaitSetDataSubscriber {


	public static void main(String args[]) {

		DDS.Duration_t wait_timeout = new DDS.Duration_t (20, 0);

		MsgSeqHolder msgList = new MsgSeqHolder();
		SampleInfoSeqHolder infoSeq = new SampleInfoSeqHolder();

		DDSEntityManager mgr = new DDSEntityManager();

		// create domain participant
		String partition_name = "WaitSet example";
		mgr.createParticipant(partition_name);

		// create type
		MsgTypeSupport st = new MsgTypeSupport();
		mgr.registerType(st);

		// create Topic
		String topic_name = "WaitSetData_Msg";
		mgr.createTopic(topic_name);

		// create Subscriber
		mgr.createSubscriber();

		// create DataReader
		mgr.createReader();

		DataReader dreader = mgr.getReader();
		MsgDataReader MsgReader = MsgDataReaderHelper.narrow(dreader);
		ErrorHandler.checkHandle(MsgReader, "MsgDataReader::_narrow");

		/* 1- Create a ReadCondition that will contain new Msg only */
		ReadCondition newMsg = MsgReader.create_readcondition(
				NOT_READ_SAMPLE_STATE.value, NEW_VIEW_STATE.value,
				ALIVE_INSTANCE_STATE.value);
		ErrorHandler.checkHandle(newMsg, "DDS.DataReader.create_readcondition");

		/* 2- Create QueryCondition */
		// create a query string
		String[] queryStr = new String[1];
		queryStr[0] = "Hello again";
		// Create QueryCondition
		System.out.println("=== [WaitSetDataSubscriber] Query : message = "
				+ '"' + "Hello again" + '"');
		QueryCondition queryCond = MsgReader.create_querycondition(
				ANY_SAMPLE_STATE.value, ANY_VIEW_STATE.value,
				ANY_INSTANCE_STATE.value, "message=%0", queryStr);
		ErrorHandler.checkHandle(queryCond, "create_querycondition");

		/*
		 * 3- Obtain a StatusCondition associated to a Writer and that triggers
		 * only when the Writer changes Liveliness
		 */
		StatusCondition leftMsgWriter = MsgReader.get_statuscondition();
		ErrorHandler.checkHandle(leftMsgWriter,
				"DDS.DataReader.get_statuscondition");
		int status = leftMsgWriter
				.set_enabled_statuses(LIVELINESS_CHANGED_STATUS.value);
		ErrorHandler.checkStatus(status,
				"DDS.StatusCondition.set_enabled_statuses");

		/* 4- Create a GuardCondition which will be used to close the subscriber */
	        GuardCondition escape;
		escape = new GuardCondition();

		/*
		 * Create a waitset and add the 4 Conditions created above :
		 * ReadCondition, QueryCondition, StatusCondition, GuardCondition
		 */
		WaitSet newMsgWS = new WaitSet();
		status = newMsgWS.attach_condition(newMsg); // ReadCondition
		ErrorHandler.checkStatus(status,
				"DDS.WaitSetData.attach_condition (newMsg)");
		status = newMsgWS.attach_condition(queryCond); // QueryCondition
		ErrorHandler.checkStatus(status,
				"DDS.WaitSetData.attach_condition (queryCond)");
		status = newMsgWS.attach_condition(leftMsgWriter); // StatusCondition
		ErrorHandler.checkStatus(status,
				"DDS.WaitSetData.attach_condition (leftMsgWriter)");
		status = newMsgWS.attach_condition(escape); // GuardCondition
		ErrorHandler.checkStatus(status,
				"DDS.WaitSetData.attach_condition (escape)");

		/*
		 * Initialize and pre-allocate the GuardList used to obtain the
		 * triggered Conditions.
		 */
		ConditionSeqHolder guardList = new ConditionSeqHolder();

		System.out.println("=== [WaitSetDataSubscriber] Ready ...");

		// var used to manage the status condition
		int prevCount = 0;
		LivelinessChangedStatusHolder livChangStatus = new LivelinessChangedStatusHolder();

		boolean closed = false;
		boolean escaped = false;
		boolean writerLeft = false;
		int count = 0;
		while (!closed && count < 20 ) {
			/*
			 * Wait until at least one of the Conditions in the waitset
			 * triggers.
			 */
			status = newMsgWS._wait(guardList, wait_timeout);
			if (status == RETCODE_OK.value ) {
			/* Walk over all guards to display information */
			for (int i = 0; i < guardList.value.length; i++) {
				if (guardList.value[i] == newMsg) {
					/* The newMsg ReadCondition contains data */
					status = MsgReader.take_w_condition(msgList, infoSeq,
							LENGTH_UNLIMITED.value, newMsg);
					ErrorHandler.checkStatus(status,
							"WaitSetData::MsgDataReader::take_w_condition");

					for (int j = 0; j < msgList.value.length; j++) {
						System.out.println("    --- New message received ---");
						System.out.println("    userID  : "
								+ msgList.value[j].userID);
						System.out.println("    Message : \""
								+ msgList.value[j].message + "\"");
					}
					status = MsgReader.return_loan(msgList, infoSeq);
					ErrorHandler.checkStatus(status,
							"WaitSetData::MsgDataReader::return_loan");
				} else if (guardList.value[i] == queryCond) {
					/* The queryCond QueryCondition contains data */
					status = MsgReader.take_w_condition(msgList, infoSeq,
							LENGTH_UNLIMITED.value, queryCond);
					ErrorHandler.checkStatus(status,
							"WaitSetData::MsgDataReader::take_w_condition");

					for (int j = 0; j < msgList.value.length; j++) {
						System.out
								.println("    --- message received (with QueryCOndition on message field) ---");
						System.out.println("    userID  : "
								+ msgList.value[j].userID);
						System.out.println("    Message : \""
								+ msgList.value[j].message + "\"");
					}
					status = MsgReader.return_loan(msgList, infoSeq);
					ErrorHandler.checkStatus(status,
							"WaitSetData::MsgDataReader::return_loan");
				} else if (guardList.value[i] == leftMsgWriter) {
					/*
					 * Some liveliness has changed (either a DataWriter joined
					 * or a DataWriter left)
					 */
					status = MsgReader
							.get_liveliness_changed_status(livChangStatus);
					ErrorHandler.checkStatus(status,
							"DDS::DataReader::get_liveliness_changed_status");
					if (livChangStatus.value.alive_count < prevCount) {
						/* a DataWriter lost its liveliness */
						System.out
								.println("!!! a MsgWriter lost its liveliness");
					 System.out.println("=== Triggering escape condition");
                                        status = escape.set_trigger_value(true);
                                        ErrorHandler.checkStatus(status,
                                           "DDS.GuardCondition.set_trigger_value");
			writerLeft = true;
					} else {
						/* a DataWriter joined */
						System.out.println("!!! a MsgWriter joined");
					}
					prevCount = livChangStatus.value.alive_count;
				} else if (guardList.value[i] == escape) {
					// SubscriberUsingWaitset terminated.
					System.out.println("!!! escape condition triggered - count = " + count);
					escaped = true;
					status = escape.set_trigger_value(false);
			                ErrorHandler.checkStatus(status,
					   "DDS.GuardCondition.set_trigger_value");
				} else {
					assert (false); // error
				}
				;
			} /* for */
			}
			else if (status != RETCODE_TIMEOUT.value) {
			  // DDS_RETCODE_TIMEOUT is considered as an error
			  // only after it has occurred count times
			  ErrorHandler.checkStatus(status, "DDS::WaitSetData::wait");
			} else {
			  System.out.println("!!! [INFO] WaitSet timedout - count = " + count);
			}
			++count;
			closed = escaped && writerLeft;
		} /* while (!closed) */
		if (count >= 20)  
		    System.out.println("*** Error : Timed out - count = " + count + " ***");

		/* Remove all Conditions from the WaitSetData. */
		status = newMsgWS.detach_condition(escape);
		ErrorHandler.checkStatus(status,
				"DDS::WaitSetData::detach_condition (escape)");
		status = newMsgWS.detach_condition(newMsg);
		ErrorHandler.checkStatus(status,
				"DDS::WaitSetData::detach_condition (newMsg)");
		status = newMsgWS.detach_condition(leftMsgWriter);
		ErrorHandler.checkStatus(status,
				"DDS::WaitSetData::detach_condition (leftMsgWriter)");
		status = newMsgWS.detach_condition(queryCond);
		ErrorHandler.checkStatus(status,
				"DDS::WaitSetData::detach_condition (queryCond)");

		System.out.println("=== [Subscriber] Closed");

		// cleanup
		// Free all resources and delete participant
		MsgReader.delete_readcondition(newMsg);
		MsgReader.delete_readcondition(queryCond);
		mgr.deleteReader(MsgReader);
		mgr.deleteSubscriber();
		mgr.deleteTopic();		
		mgr.deleteParticipant();

	}
}
