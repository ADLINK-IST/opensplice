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
import org.omg.dds.core.Duration;
import org.omg.dds.sub.InstanceState;
import org.omg.dds.sub.Sample;

/**
 * The RoundTrip example consists of a Ping and a Pong application. Ping sends
 * sample to Pong by writing to the Ping partition which Pong subscribes to.
 * Pong them sends them back to Ping by writing on the Pong partition which Ping
 * subscribes to. Ping measure the amount of time taken to write and read each
 * sample as well as the total round trip time to send a sample to Pong and
 * receive it back.
 * 
 * This class performs the Pong role in this example.
 */
class Pong_impl {

	/**
	 * Performs the Pong role in this example.
	 */
	public void run(String args[]) {
		try {

			/*
			 * Initialize entities
			 */
			final Entities e = new Entities("pong", "ping");

			Runtime.getRuntime().addShutdownHook(new Thread() {
				public void run() {
					e.terminated.setTriggerValue(true);
				}
			});

			System.out.println("Waiting for samples from ping to send back...");

			Duration waitTimeout = e.env.getSPI().infiniteDuration();

			List<Sample<RoundTripModule.DataType>> samples = new ArrayList<Sample<RoundTripModule.DataType>>(
					10);

			while (!e.terminated.getTriggerValue()) {
				/*
				 * Wait for a sample from ping
				 */
				e.waitSet.waitForConditions(waitTimeout);
				/*
				 * Take sample
				 */
				e.reader.take(samples);

				for (int i = 0; !e.terminated.getTriggerValue()
						&& i < samples.size(); i++) {
					/*
					 * If writer has been disposed terminate pong
					 */
					Sample<RoundTripModule.DataType> sample = samples.get(i);
					if (sample.getInstanceState() == InstanceState.NOT_ALIVE_DISPOSED) {
						System.out
								.println("Received termination request. Terminating.");
						e.terminated.setTriggerValue(true);
						break;
					}
					/*
					 * If sample is valid, send it back to ping
					 */
					else if (sample.getData() != null) {
						e.writer.write(sample.getData());
					}

				}
			}
			e.participant.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
