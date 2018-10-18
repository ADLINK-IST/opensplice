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
