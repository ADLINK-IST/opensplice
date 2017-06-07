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
 * Contains all common SPLICE DDS C&M Tooling model components. 
 */
package org.opensplice.common.model;

/**
 * Interface that offers the possibility to be notified on model changes.
 * 
 * @date Apr 26, 2004
 */
public interface ModelListener {
    /**
     * Notifies model events. When a ModelListener is registered with a 
     * ModelRegister component, the ModelRegister will notify the ModelListener
     * when something in the ModelRegister changed. A ModelListener is typically
     * a view component that needs to update itself when a model change occurs.
     * 
     * @param description Description of the event that occurred.
     */
    public void update(String description);
}

