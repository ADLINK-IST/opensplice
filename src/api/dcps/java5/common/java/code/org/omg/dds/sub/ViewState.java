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

package org.omg.dds.sub;

/**
 * For each instance (identified by the key), the Data Distribution Service internally maintains a
 * ViewState relative to each DataReader.
 * The ViewState can either be NEW or NOT_NEW.
 */
public enum ViewState {
    // -----------------------------------------------------------------------
    // States
    // -----------------------------------------------------------------------
    /**
     *  NEW indicates that either this is the first time that the DataReader has ever accessed samples
     *  of that instance, or else that the DataReader has accessed previous samples of the instance,
     *  but the instance has since been reborn (i.e. becomes not-alive and then alive again).
     */
    NEW(0x0001 << 0),
    /**
     * NOT_NEW indicates that the DataReader has already accessed samples of the same instance
     * and that the instance has not been reborn since.
     */
    NOT_NEW(0x0001 << 1);



    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    public final int value;



    // -----------------------------------------------------------------------
    // Object Life Cycle
    // -----------------------------------------------------------------------

    private ViewState(int value) {
        this.value = value;
    }

}
