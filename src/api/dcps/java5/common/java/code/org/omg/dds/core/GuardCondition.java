/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.omg.dds.core;


/**
 * A GuardCondition object is a specific Condition whose triggerValue is
 * completely under the control of the application. When it is first created,
 * the triggerValue is set to false.
 * <p>
 * The purpose of the GuardCondition is to provide the means for the
 * application to manually wake up a {@link org.omg.dds.core.WaitSet}. This is accomplished by
 * attaching the GuardCondition to the WaitSet and then setting the
 * triggerValue by means of the {@link #setTriggerValue(boolean)} operation.
 *
 * <pre>
 * <b><i>Example</i></b>
 * <code>
 * //Instantiate a GuardCondition.
 * GuardCondition myGC = GuardCondition.newGuardCondition(env);
 * //Attach the GuardCondition to the WaitSet defined as before.
 * waitset.attachCondition(myGC);
 *
 * HashSet&lt;Condition&gt; triggeredConditions = new HashSet&lt;Condition&gt;();
 * Duration timeout = Duration.infiniteDuration(env);
 *
 * // Wait infinitely for the status to trigger.
 * waitset.waitForConditions(triggeredConditions, timeout);
 * // walk over the triggered conditions
 * for (Condition cond : triggeredConditions) {
 *   // check condition and do an action
 * }
 * </code>
 * </pre>
 * In another thread, trigger the GuardCondition.
 * <pre>
 * <code>
 * myGC.setTriggerValue(true);
 * </code>
 * </pre>
 */
public abstract class GuardCondition implements Condition
{
    // -----------------------------------------------------------------------
    // Factory Methods
    // -----------------------------------------------------------------------

    /**
     * Creates a new GuardCondition object which can be used in a @{link org.omg.dds.core.WaitSet}
     * @param env       Identifies the Service instance to which the new
     *                  object will belong.
     */
    public static GuardCondition newGuardCondition(ServiceEnvironment env)
    {
        return env.getSPI().newGuardCondition();
    }



    // -----------------------------------------------------------------------
    // Instance Methods
    // -----------------------------------------------------------------------

    /**
     * This operation sets the triggerValue of the GuardCondition.
     * <p>
     * {@link org.omg.dds.core.WaitSet} objects' behavior depends on the changes of the
     * triggerValue of their attached conditions. Therefore, any WaitSet
     * to which the GuardCondition is attached is potentially affected by
     * this operation.
     *
     * @see     Condition#getTriggerValue()
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     */
    public abstract void setTriggerValue(boolean value);
}
