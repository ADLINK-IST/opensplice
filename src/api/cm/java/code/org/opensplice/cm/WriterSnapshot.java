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
package org.opensplice.cm;

import org.opensplice.cm.meta.MetaType;

/**
 * Represents a snapshot of the history of a Writer.
 * 
 * @date Nov 17, 2004 
 */
public interface WriterSnapshot extends Snapshot {
    
    /**
     * Provides access to userDataType.
     * 
     * @return Returns the userDataType.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - WriterSnapshot is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     * @throws DataTypeUnsupportedException Thrown when:
     *                                      - Data type of the writer is not
     *                                        supported by this API.
     */
    @Override
    public MetaType getUserDataType() throws DataTypeUnsupportedException, CMException;

    /**
     * Provides access to writer.
     * 
     * @return Returns the writer.
     */
    public Writer getWriter();
}
