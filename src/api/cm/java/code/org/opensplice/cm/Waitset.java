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
package org.opensplice.cm;

/**
 * Represents a waitset in SPLICE-DDS.
 * 
 * @date Oct 28, 2004 
 */
public interface Waitset extends Entity {
    public void attach(Entity entity) throws CMException;
    
    public void detach(Entity entity) throws CMException;
    
    public Entity[] _wait() throws CMException;
    
    public Entity[] timedWait(Time time) throws CMException;
    
    public int getEventMask() throws CMException;
    
    public void setEventMask(int mask) throws CMException;
}
