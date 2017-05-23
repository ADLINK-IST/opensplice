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

/**
 * The Throughput example measures data throughput in bytes per second. The
 * publisher allows you to specify a payload size in bytes as well as allowing
 * you to specify whether to send data in bursts. The publisher will continue to
 * send data forever unless a time out is specified. Thesamples.clear()
 * subscriber will receive data and output the total amount received and the
 * data rate in bytes per second. It will also indicate if any samples were
 * received out of order. A maximum number of cycles can be specified and once
 * this has been reached the subscriber will terminate and output totals and
 * averages.
 *
 * This class performs the publisher role in this example.
 */
class Publisher_impl {
    /**
     * Performs the publisher role in this example.
     */
    public void run(String args[]) {
        Exception usage = new Exception(
                "Usage (parameters must be supplied in order):\n"
                        + "./publisher [payloadSize (bytes)] [burstInterval (ms)] [burstSize (samples)] [timeOut (seconds)] [partitionName]\n"
                        + "  burstSize must be > 0\n" + "Defaults: \n"
                        + "./publisher 8192 0 1 0 \"Throughput example\"");
        try {
            /*
             * Get the program parameters Parameters: publisher [payloadSize]
             * [burstInterval] [burstSize] [timeOut] [partitionName]
             */
            int payloadSize = 8192;
            long burstInterval = 0;
            long timeOut = 0;
            long burstSize = 1;
            String partitionName = "Throughput example";
            if (args.length == 1
                    && (args[0].equals("-h") || args[0].equals("--help"))) {

                throw usage;
            }
            if (args.length > 0) {
                payloadSize = Integer.parseInt(args[0]); // The size of the
                                                         // payload in bytes
            }
            if (args.length > 1) {
                burstInterval = Integer.parseInt(args[1]); // The time interval
                                                           // between each
                                                           // burst in ms
            }
            if (args.length > 2) {
                burstSize = Integer.parseInt(args[2]); // The number of samples
                                                       // to send each burst
                if (burstSize <= 0) {
                    throw usage;
                }
            }
            if (args.length > 3) {
                timeOut = Integer.parseInt(args[3]); // The number of seconds
                                                     // the publisher should
                                                     // run for (0 =
                                                     // infinite)
            }
            if (args.length > 4) {
                partitionName = args[4]; // The name of the partition
            }

            System.out.println("payloadSize: " + payloadSize
                    + " | burstInterval: " + burstInterval + " | burstSize: "
                    + burstSize + " | timeOut: " + timeOut
                    + " | partitionName: " + partitionName + "\n");

            /* Initialize entities */
            final PubEntities e = new PubEntities(partitionName);

            Runtime.getRuntime().addShutdownHook(new Thread() {
                public void run() {
                    e.terminated.setTriggerValue(true);
                }
            });

            /* Fill the sample payload with data */
            ThroughputModule.DataType sample = new ThroughputModule.DataType(0,
                    new byte[payloadSize]);
            for (int i = 0; i < payloadSize; i++) {
                sample.payload[i] = 'a';
            }

            /*
             * Register the sample instance and write samples repeatedly or
             * until time out
             */
            e.writer.registerInstance(sample);
            System.out.println("Writing samples...");

            long burstCount = 0;
            boolean timedOut = false;
            long pubStart = ExampleUtilities.GetTime();
            long burstStart = ExampleUtilities.GetTime();

            while (!e.terminated.getTriggerValue() && !timedOut) {
                /* Write data until burst size has been reached */
                if (burstCount < burstSize) {
                    e.writer.write(sample);
                    sample.count++;
                    burstCount++;
                }
                /* Sleep until burst interval has passed */
                else if (burstInterval > 0) {
                    long time = ExampleUtilities.GetTime();
                    long deltaTime = (time - burstStart)
                            / ExampleUtilities.US_IN_ONE_MS;
                    if (deltaTime < burstInterval) {
                        Thread.sleep(burstInterval - deltaTime);
                    }
                    burstStart = ExampleUtilities.GetTime();
                    burstCount = 0;
                } else {
                    burstCount = 0;
                }

                if (timeOut > 0) {
                    if (((ExampleUtilities.GetTime() - pubStart) / ExampleUtilities.US_IN_ONE_SEC) > timeOut) {
                        timedOut = true;
                    }
                }
            }
            if (e.terminated.getTriggerValue()) {
                System.out.println("Terminated, " + sample.count
                        + " samples written.");
            } else {
                System.out.println("Timed out, " + sample.count
                        + " samples written.");
            }
            e.participant.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
