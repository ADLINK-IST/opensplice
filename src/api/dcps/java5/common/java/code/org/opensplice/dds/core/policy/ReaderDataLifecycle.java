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
package org.opensplice.dds.core.policy;

/**
 * This class provides OpenSplice-specific extensions to the
 * {@link org.omg.dds.core.policy.ReaderDataLifecycle} policy.
 *
 */
public interface ReaderDataLifecycle extends
        org.omg.dds.core.policy.ReaderDataLifecycle {
    /**
     * Instance state changes are communicated to a
     * {@link org.omg.dds.sub.DataReader} by means of the sample info
     * accompanying a {@link org.omg.dds.sub.Sample}. If no samples are
     * available in the DataReader, a so-called 'invalid sample' can be injected
     * with the sole purpose of notifying applications of the instance state.
     * <p>
     * ReaderDataLifecycle.Kind drives the behavior of the middleware concerning
     * these invalid samples.
     *
     * <ul>
     * <li><b>NONE</b> - applications will be notified of InstanceState changes
     * only if there is a sample available in the DataReader. The SampleInfo
     * belonging to this sample will contain the updated instance state.
     * <li><b>MINIMUM</b> - the middleware will try to update the
     * {@link org.omg.dds.sub.InstanceState} on available samples in the
     * DataReader. If no sample is available, an invalid sample will be
     * injected. These samples contain only the key values of the instance. The
     * SampleInfo for invalid samples will only have a key value (available
     * through {@link org.opensplice.dds.sub.Sample#getKeyValue()}), and contain
     * the updated {@link org.omg.dds.sub.InstanceState}. This is the default
     * value.
     * <li><b>ALL</b> - every change in the
     * {@link org.omg.dds.sub.InstanceState}will be communicated by a separate
     * invalid sample. Invalid samples are always visible, every time the of an
     * instance changes. This last option has not been implemented yet.
     * </ul>
     */
    public enum Kind {
        NONE, MINIMUM, ALL
    }

    /**
     * Whether or not instances that have been disposed by means of the
     * {@link org.opensplice.dds.topic.Topic#disposeAllData()} method will
     * automatically be purged by the middleware or not.
     *
     * @return true if instances are automatically purged, false otherwise.
     */
    public boolean getAutoPurgeDisposeAll();

    /**
     * Provides access to the ReaderDataLifecycle.Kind that drives invalid
     * sample visibility.
     *
     * @return The invalid sample visibility.
     */
    public Kind getInvalidSampleInvisibility();

    /**
     * Creates a copy of this ReaderDataLifecycle that has its
     * autoPurgeDisposeAll set to true.
     *
     * @return a copy of this policy with autoPurgeDisposeAll set to true.
     */
    public ReaderDataLifecycle withAutoPurgeDisposeAll();

    /**
     * Creates a copy of this ReaderDataLifecycle that has its invalid sample
     * visibility set to the supplied value.
     *
     * @param kind
     *            the invalid sample visibility kind to set.
     * @return a copy of this policy with supplied invalid sample visibility set
     *         to true.
     */
    public ReaderDataLifecycle withInvalidSampleInvisibility(Kind kind);
}
