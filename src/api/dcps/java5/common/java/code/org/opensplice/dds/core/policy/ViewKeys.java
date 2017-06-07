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

import java.util.Collection;
import java.util.Set;

/**
 * This QosPolicy is used to set the key list of a DataReaderView. A
 * DataReaderView allows a different view, defined by this key list, on the data
 * set of the DataReader from which it is created.
 * <p>
 * Operations that operate on instances or instance handles, such as
 * lookupInstance or getKeyValue, respect the alternative key-list and work as
 * expected. However, since the mapping of writer instances to reader instances
 * is no longer trivial (one writer instance may now map to more than one
 * matching reader instance and vice versa), a writer instance will no longer be
 * able to fully determine the lifecycle of its matching reader instance, nor
 * the value its view_state and instance_state.
 * <p>
 * In fact, the view sample will always copy the view_state and instance_state
 * values from the reader sample to which it is slaved. If both samples preserve
 * a 1 - 1 correspondence with respect to their originating instances (this may
 * sometimes be the case even when an alternative keylist is provided, i.e. when
 * one reader instance never maps to more than one view instance and vice versa)
 * then the resulting InstanceState and ViewState still have a valid semantical
 * meaning. If this 1 - 1 correspondence cannot be guaranteed, the resulting
 * InstanceState and ViewState are semantically meaningless and should not be
 * used to derive any conclusion regarding the lifecycle of a view instance.
 * <p>
 * By default, the ViewKeyQosPolicy is disabled.
 * <p>
 * This QosPolicy is applicable to a DataReaderView only, and cannot be changed
 * after the DataReaderView is created.
 */
public interface ViewKeys extends QosPolicy.ForView {
    /**
     * @return an unmodifiable collection of view keys.
     */
    public Set<String> getKey();

    /**
     * Copy this policy and override the value of the property.
     * @param keyList           A collection of strings with one or more names of the data
     *                          set of the DataReader acting as alternative keys.
     * @return a new ViewKeys policy
     */
    public ViewKeys withKey(Collection<String> keyList);

    /**
     * Copy this policy and override the value of the property.
     * @param keyList           A strings with one name of the data
     *                          set of the DataReader acting as alternative keys.
     * @return a new ViewKeys policy
     */
    public ViewKeys withKey(String keyList);

    /**
     * Copy this policy and override the value of the property.
     * @param keyList           An arbitrary number of strings with one or more names of the data
     *                          set of the DataReader acting as alternative keys.
     * @return a new ViewKeys policy
     */
    public ViewKeys withKey(String... keyList);
}
