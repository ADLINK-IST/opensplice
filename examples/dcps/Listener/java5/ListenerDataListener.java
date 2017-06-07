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
import java.util.ArrayList;
import java.util.List;

import org.omg.dds.core.GuardCondition;
import org.omg.dds.core.event.DataAvailableEvent;
import org.omg.dds.core.event.RequestedDeadlineMissedEvent;
import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.DataReaderAdapter;
import org.omg.dds.sub.Sample;
import org.omg.dds.sub.Subscriber.DataState;
import org.omg.dds.sub.ViewState;

import ListenerData.*;

public class ListenerDataListener extends DataReaderAdapter<Msg> {

	boolean closed = false;
	GuardCondition guardCond = null;

	public ListenerDataListener(GuardCondition myGC) {
	    guardCond = myGC;
    }

	@Override
    public void onDataAvailable(DataAvailableEvent<Msg> status) {
        /* Handle the incoming data here. */
		DataReader<Msg> reader = status.getSource();
		DataState ds = reader.getParent().createDataState();
		ds = ds.withAnySampleState()
		       .with(ViewState.NEW)
		       .withAnyInstanceState();
		List<Sample<Msg>> samples = new ArrayList<Sample<Msg>>();

		reader.select().dataState(ds).read(samples);

		boolean hasValidData = false;
		if (samples.size() > 0) {
			System.out.println("=== [ListenerDataListener.on_data_available] - msgList.length : "+ samples.size());
			for (Sample<Msg> sample : samples) {
			    Msg msg = sample.getData();
				if (msg != null) {
					hasValidData = true;
					System.out.println("    --- message received ---");
					System.out.println("    userID  : " + msg.userID);
					System.out.println("    Message : \"" + msg.message+ "\"");
				}
			}

			if (hasValidData) {
				// unblock the Waitset in Subscriber main loop
			    guardCond.setTriggerValue(true);
			} else {
				System.out.println("=== [ListenerDataListener.on_data_available] ===> hasValidData is false!");
			}
		}
	}

	@Override
	public void onRequestedDeadlineMissed(RequestedDeadlineMissedEvent<Msg> status) {

		System.out.println("=== [ListenerDataListener.on_requested_deadline_missed] : triggered");
		System.out.println("=== [ListenerDataListener.on_requested_deadline_missed] : stopping");
		// unblock the waitset in Subscriber main loop
		guardCond.setTriggerValue(true);
		closed = true;
	}
}
