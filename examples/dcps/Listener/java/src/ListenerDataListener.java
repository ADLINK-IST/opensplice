/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
import ListenerData.*;

public class ListenerDataListener implements DDS.DataReaderListener {

	boolean m_closed;
	MsgDataReader m_MsgReader;
	DDS.GuardCondition m_guardCond = new GuardCondition();

	@Override
	public void on_data_available(DataReader arg0) {
		System.out.println("AVAILABLE");
		int status;
		MsgSeqHolder msgList = new MsgSeqHolder();
		SampleInfoSeqHolder infoSeq = new SampleInfoSeqHolder();
		status = m_MsgReader.read(msgList, infoSeq, LENGTH_UNLIMITED.value,
				ANY_SAMPLE_STATE.value, NEW_VIEW_STATE.value,
				ANY_INSTANCE_STATE.value);
		ErrorHandler.checkStatus(status, "MsgDataReader.read");

		Msg[] data = msgList.value;
		boolean hasValidData = false;
		if (data.length > 0) {
			System.out
					.println("=== [ListenerDataListener.on_data_available] - msgList.length : "
							+ data.length);

			int i = 0;
			do {
				if (infoSeq.value[i].valid_data) {
					hasValidData = true;
					System.out.println("    --- message received ---");
					System.out.println("    userID  : " + data[i].userID);
					System.out.println("    Message : \"" + data[i].message
							+ "\"");
				}
			} while (++i < data.length);

			if (hasValidData) {
				// unblock the wCaitset in Subscriber main loop
				m_guardCond.set_trigger_value(true);
			} else
				System.out
						.println("=== [ListenerDataListener.on_data_available] ===> hasValidData is false!");

			status = m_MsgReader.return_loan(msgList, infoSeq);
			ErrorHandler.checkStatus(status, "MsgDataReader.return_loan");
		}
	}

	@Override
	public void on_liveliness_changed(DataReader arg0,
			LivelinessChangedStatus arg1) {

		System.out
				.println("=== [ListenerDataListener.on_liveliness_changed] : triggered");
		// unblock the waitset in Subscriber main loop
		m_guardCond.set_trigger_value(true);
	}

	@Override
	public void on_requested_deadline_missed(DataReader arg0,
			RequestedDeadlineMissedStatus arg1) {

		// System.out.println("=== [ListenerDataListener.on_requested_deadline_missed] : triggered");
		// System.out.println("=== [ListenerDataListener.on_requested_deadline_missed] : stopping");
		m_closed = true;
		// unblock the waitset in Subscriber main loop
		m_guardCond.set_trigger_value(true);
	}

	@Override
	public void on_requested_incompatible_qos(DataReader arg0,
			RequestedIncompatibleQosStatus arg1) {

		System.out
				.println("=== [ListenerDataListener.on_requested_incompatible_qos] : triggered");
		// unblock the waitset in Subscriber main loop
		m_guardCond.set_trigger_value(true);
	}

	@Override
	public void on_sample_lost(DataReader arg0, SampleLostStatus arg1) {

		System.out
				.println("=== [ListenerDataListener.on_sample_lost] : triggered");
		// unblock the waitset in Subscriber main loop
		m_guardCond.set_trigger_value(true);
	}

	@Override
	public void on_sample_rejected(DataReader arg0, SampleRejectedStatus arg1) {

		System.out
				.println("=== [ListenerDataListener.on_sample_rejected] : triggered");
		// unblock the waitset in Subscriber main loop
		m_guardCond.set_trigger_value(true);
	}

	@Override
	public void on_subscription_matched(DataReader arg0,
			SubscriptionMatchedStatus arg1) {

		System.out
				.println("=== [ListenerDataListener.on_subscription_matched] : triggered");
		// unblock the waitset in Subscriber main loop
		m_guardCond.set_trigger_value(true);
	}

}
