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
package org.opensplice.common.model.table;

import java.util.ArrayList;

import org.opensplice.cm.Reader;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.State;
import org.opensplice.cm.data.UserData;
import org.opensplice.cmdataadapter.CmDataException;
import org.opensplice.cmdataadapter.TypeInfo;
import org.opensplice.common.CommonException;

public class CoherentSetSampleTableModel extends CoherentSetAbstractTableModel<Sample> {

    private static final long serialVersionUID = -2267097614725260743L;

    public CoherentSetSampleTableModel() {
        rows = new ArrayList<DataRowTuple<Sample>>();
    }

    /**
     * Adds the supplied data to the table.
     *
     * @param sample
     *            The Sample object that was retrieved from a data reader. The
     *            sample's UserData is unadapted. If {@link this#getData(int)}
     *            is used to get the Sample for display purposes, a
     *            {@link TypeInfo#adaptDataForRead(org.opensplice.cmdataadapter.TypeInfo.TypeEvolution, Sample)
     *            should be invoked.}
     * @param reader
     *            The Reader that was used to read the data.
     * @param typeInfo
     *            The TypeInfo object associated with the Reader's backing data
     *            type. It is used to acquire the type's keys.
     * @throws CommonException
     *             If there was an error while acquired the key fields from the
     *             typeInfo
     */
    public void addData(Sample sample, Reader reader, TypeInfo typeInfo) throws CommonException {
        rows.add(new DataRowTuple<Sample>(sample, sample.getState(), reader));
        if (!keyFieldMap.containsKey(reader)) {
            try {
                String[] keys = typeInfo.getKeys();
                for (int i = 0; i < keys.length; i++) {
                    if (keys[i].startsWith("userData.")) {
                        keys[i] = keys[i].substring(9);
                    }
                }
                keyFieldMap.put(reader, keys);
            } catch (CmDataException e) {
                throw new CommonException(e.getMessage());
            }
        }
        fireTableDataChanged();
    }

    public void addDummyRow() {
        rows.add(new DataRowTuple<Sample>(null, null, null));
        fireTableDataChanged();
    }

    public Sample getSample(int rowIndex) {
        return rows.get(rowIndex).data;
    }

    @Override
    public String getColumnName(int index) {
        switch (index) {
        case 0:
            return "Instance";
        case 1:
            return "Read State";
        case 2:
            return "Source reader";
        default:
            return "";
        }
    }

    @Override
    protected UserData getRowUserData(Sample tupleData) {
        return tupleData.getMessage().getUserData();
    }

    @Override
    protected String getStateString(State state) {
        String result = "";
        if (state.test(State.DISPOSED)) {
            result += "DISPOSED, ";
        } else if (state.test(State.NOWRITERS)) {
            result += "NOWRITERS, ";
        } else if (!state.test(State.DISPOSED | State.NOWRITERS)) {
            result += "ALIVE, ";
        }
        if (state.test(State.NEW)) {
            result += "NEW";
        } else {
            result += "NOT_NEW";
        }
        return result;
    }
}
