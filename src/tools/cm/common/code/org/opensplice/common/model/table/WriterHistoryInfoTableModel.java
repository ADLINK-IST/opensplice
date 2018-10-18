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
package org.opensplice.common.model.table;

import java.util.Date;

import org.opensplice.cm.data.GID;
import org.opensplice.cm.data.Message;
import org.opensplice.cm.data.Sample;


/**
 * Model that holds the history of one specific Sample.
 *
 * @date Nov 17, 2004
 */
public class WriterHistoryInfoTableModel extends SampleInfoTableModel {
    /**
     * Constructs a new HistoryTableModel.
     *
     */
    public WriterHistoryInfoTableModel(){
        super();
    }

    /**
     * Administrates the Sample info oof the supplied Sample. It replaces the
     * possible previous info.
     *
     * @param data The Sample, which info must be administrated.
     * @return true if the supplied Sample and its Message are not null, false
     *              otherwise.
     */
    @Override
    public boolean setData(Sample data){
        Message msg = data.getMessage();
        boolean success = false;
        int row = 0;

        if(msg != null){
            this.setValueAt("N/A", row++, 1);
            this.setValueAt("N/A", row++, 1);
            this.setValueAt("N/A", row++, 1);

            this.setValueAt("N/A", row++, 1);
            this.setValueAt("N/A", row++, 1);

            this.setValueAt("N/A", row++, 1);

            String date = "(" + new Date(msg.getWriteTimeSec() * 1000) + ")";
            this.setValueAt(Long.toString(msg.getWriteTimeSec()) + "s. " +
                            Long.toString(msg.getWriteTimeNanoSec()) + "ns. " +
                            date,
                            row++, 1);
            this.setValueAt("N/A", row++, 1);

            GID gid = msg.getWriterGid();

            if(gid != null){
                this.setValueAt(Long.toString(gid.getLocalId()), row++, 1);
                this.setValueAt(Long.toString(gid.getSystemId()), row++, 1);
            } else {
                this.setValueAt("N/A", row++, 1);
                this.setValueAt("N/A", row++, 1);
                this.setValueAt("N/A", row++, 1);
            }
            gid = msg.getInstanceGid();

            if(gid != null){
                this.setValueAt(Long.toString(gid.getLocalId()), row++, 1);
                this.setValueAt(Long.toString(gid.getSystemId()), row++, 1);
            } else {
                this.setValueAt("N/A", row++, 1);
                this.setValueAt("N/A", row++, 1);
                this.setValueAt("N/A", row++, 1);
            }
            this.setValueAt(Long.toString(msg.getSampleSequenceNumber()), row++, 1);
            this.setValueAt(msg.getQos().getReliabilityKind().toString(), row++, 1);

            success = true;
        }
        return success;
    }
}
