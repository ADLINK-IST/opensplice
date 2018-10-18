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
/**
 * This class defines some simple common utility functions for use in Vortex
 * Cafe examples.
 */
class ExampleUtilities {
    public static final long NS_IN_ONE_US = 1000;
    public static final long US_IN_ONE_MS = 1000;
    public static final long US_IN_ONE_SEC = 1000000;

    /**
     * Returns the current time in microseconds
     * 
     * @return the current time in microseconds
     */
    public static long GetTime() {
        return System.nanoTime() / NS_IN_ONE_US;
    }

    /**
     * Class to keep a running average time as well as recording the minimum and
     * maximum times
     */
    public static class TimeStats {
        public java.util.ArrayList<Long> values = new java.util.ArrayList<Long>();
        public double average;
        public long min;
        public long max;
        public long count;

        /**
         * Resets stats variables to zero
         */
        public void Reset() {
            values.clear();
            average = 0;
            min = 0;
            max = 0;
            count = 0;
        }

        /**
         * Updates stats with new time data, keeps a running average as well as
         * recording the minimum and maximum times
         * 
         * @param microseconds
         *            A time in microseconds to add to the stats
         * @return The updated stats
         */
        public TimeStats AddMicroseconds(long microseconds) {
            values.add(microseconds);
            average = (count * average + microseconds) / (count + 1);
            min = count == 0 || microseconds < min ? microseconds : min;
            max = count == 0 || microseconds > max ? microseconds : max;
            count++;
            return this;
        }

        /**
         * Calculates the median time from the stats
         * 
         * @return the median time
         */
        public double GetMedian() {
            double median;

            java.util.Collections.sort(values);

            if (values.size() % 2 == 0) {
                median = (double) (values.get(values.size() / 2 - 1) + values
                        .get(values.size() / 2)) / 2;
            } else {
                median = (double) values.get(values.size() / 2);
            }

            return median;
        }
    }
}
