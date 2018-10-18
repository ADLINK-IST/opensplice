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
package org.opensplice.dds.core.policy;

import java.util.Collection;
import java.util.Set;

import org.omg.dds.core.policy.QosPolicy;

/**
 * By using the SubscriptionKey QosPolicy, a DataReader can force its own
 * key-list definition on data samples. The consequences are that the DataReader
 * will internally keep track of instances based on its own key list, instead of
 * the key list dictated by the Topic. Operations that operate on instances or
 * instance handles, such as
 * {@link org.omg.dds.sub.DataReader#lookupInstance(Object)} or
 * {@link org.omg.dds.sub.DataReader#getKeyValue(org.omg.dds.core.InstanceHandle)}
 * , respect the alternative key-list and work as expected.
 * <p>
 * However, since the mapping of writer instances to reader instances is no
 * longer trivial (one writer instance may now map to more than one matching
 * reader instance and vice versa), a writer instance will no longer be able to
 * fully determine the lifecycle of its matching reader instance, nor the value
 * its {@link org.omg.dds.sub.ViewState} and
 * {@link org.omg.dds.sub.InstanceState}.
 * <p>
 * In fact, by diverting from the conceptual 1 - 1 mapping between writer
 * instance and reader instance, the writer can no longer keep an (empty) reader
 * instance ALIVE by just refusing to unregister its matching writer instance.
 * That means that when a reader takes all samples from a particular reader
 * instance, that reader instance will immediately be removed from the reader's
 * administration. Any subsequent reception of a message with the same keys will
 * re-introduce the instance into the reader administration, setting its
 * view_state back to NEW. Compare this to the default behavior, where the
 * reader instance will be kept alive as long as the writer does not unregister
 * it. That causes the view_state in the reader instance to remain NOT_NEW, even
 * if the reader has consumed all of its samples prior to receiving an update.
 * <p>
 * Another consequence of allowing an alternative keylist is that events that
 * are communicated by invalid samples (i.e. samples that have only initialized
 * their keyfields) may no longer be interpreted by the reader to avoid
 * situations in which uninitialized non-keyfields are treated as keys in the
 * alternative keylist. This effectively means that all invalid samples (e.g.
 * unregister messages and both implicit and explicit dispose messages) will be
 * skipped and can no longer affect the InstanceState, which will therefore
 * remain ALIVE. The only exceptions to this are the messages that are
 * transmitted explicitly using the
 * {@link org.opensplice.dds.pub.DataWriter#writeDispose(Object)} call, which
 * always includes a full and valid sample and can therefore modify the
 * InstanceState to NOT_ALIVE_DISPOSED.
 * <p>
 * By default, the SubscriptionKey QosPolicy is not used.
 * <p>
 * This QosPolicy is applicable to a DataReader only, and cannot be changed
 * after the DataReader is enabled.
 */
public interface SubscriptionKeys extends QosPolicy.ForDataReader {

    /**
     * @return an unmodifiable collection of subscription keys.
     */
    public Set<String> getKey();

    /**
     * Copy this policy and override the value of the property.
     * @param keyList           A collection of strings with one or more names of topic fields acting as alternative keys.
     * @return a new SubscriptionKeys policy
     */
    public SubscriptionKeys withKey(Collection<String> keyList);

    /**
     * Copy this policy and override the value of the property.
     * @param keyList           A strings with one name of topic fields acting as alternative key.
     * @return a new SubscriptionKeys policy
     */
    public SubscriptionKeys withKey(String keyList);

    /**
     * Copy this policy and override the value of the property.
     * @param keyList           An arbitrary number of strings with one or more names of topic fields acting as alternative keys.
     * @return a new SubscriptionKeys policy
     */
    public SubscriptionKeys withKey(String... keyList);
}
