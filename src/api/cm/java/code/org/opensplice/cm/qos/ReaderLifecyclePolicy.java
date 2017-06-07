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
package org.opensplice.cm.qos;

import org.opensplice.cm.Time;

/**
 * Specifies the behavior of the DataReader with regards to the lifecycle of 
 * the datainstances it manages.
 * 
 * @date Jan 10, 2005 
 */
public class ReaderLifecyclePolicy {
    /**
     * Indicates the duration the DataReader must retain information regarding
     * instances that have the view_state NOT_ALIVE_NO_WRITERS. By default, 
     * unlimited.
     */
    public Time autopurge_nowriter_samples_delay;

    public Time autopurge_disposed_samples_delay;

    public boolean autopurge_dispose_all;

    public boolean enable_invalid_samples;

    public InvalidSampleVisibilityKind invalid_sample_visibility;

    public static final ReaderLifecyclePolicy DEFAULT = new ReaderLifecyclePolicy(Time.infinite, Time.infinite, false, true, InvalidSampleVisibilityKind.MINIMUM_INVALID_SAMPLES);
    
    /**
     * Constructs a new ReaderLifecyclePolicy.
     *
     * @param _autopurge_nowriter_samples_delay The duration to retain 
     *                                          information.
     */
    public ReaderLifecyclePolicy(Time _autopurge_nowriter_samples_delay,
                                 Time _autopurge_disposed_samples_delay,
                                 boolean _autopurge_dispose_all,
                                 boolean _enable_invalid_samples,
                                 InvalidSampleVisibilityKind _invalid_sample_visibility)
    {
        autopurge_nowriter_samples_delay = _autopurge_nowriter_samples_delay;
        autopurge_disposed_samples_delay = _autopurge_disposed_samples_delay;
        autopurge_dispose_all            = _autopurge_dispose_all;
        enable_invalid_samples           = _enable_invalid_samples;
        invalid_sample_visibility        = _invalid_sample_visibility;
    }
    
    public ReaderLifecyclePolicy copy(){
        return new ReaderLifecyclePolicy(
                this.autopurge_nowriter_samples_delay.copy(),
                this.autopurge_disposed_samples_delay.copy(),
                this.autopurge_dispose_all,
                this.enable_invalid_samples,
                this.invalid_sample_visibility);
    }
}
