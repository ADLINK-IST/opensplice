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

import org.opensplice.cm.Writer;
import org.opensplice.cm.data.State;
import org.opensplice.cm.data.UserData;
import org.opensplice.cmdataadapter.CmDataException;
import org.opensplice.cmdataadapter.TypeInfo;
import org.opensplice.common.CommonException;

public class CoherentSetUserDataTableModel extends CoherentSetAbstractTableModel<UserData> {

    private static final long serialVersionUID = 3044931973241694391L;

    public CoherentSetUserDataTableModel() {
        rows = new ArrayList<DataRowTuple<UserData>>();
    }

    /**
     * Adds the supplied data to the table. It is expected that the Writer has
     * already written the data at this point.
     *
     * @param data
     *            The UserData object that was constructed by a user data
     *            editor. The UserData is adapted, that is, the supplied data
     *            has not been passed through a call to
     *            {@link TypeInfo#adaptDataForWrite(org.opensplice.cmdataadapter.TypeInfo.TypeEvolution, UserData)}
     * @param writeState
     *            The State that the data was written as. State is expected to
     *            be one of WRITE, DISPOSE, REGISTER, UNREGISTER, or (WRITE |
     *            DISPOSE).
     * @param writer
     *            The Writer that was used to write the data.
     * @param typeInfo
     *            The TypeInfo object associated with the Writer's backing data
     *            type. It is used to acquire the type's keys.
     * @throws CommonException
     *             If there was an error while acquired the key fields from the
     *             typeInfo
     */
    public void addData(UserData data, State writeState, Writer writer, TypeInfo typeInfo) throws CommonException {
        rows.add(new DataRowTuple<UserData>(data, writeState, writer));
        if (!keyFieldMap.containsKey(writer)) {
            try {
                String[] keys = typeInfo.getKeys();
                for (int i = 0; i < keys.length; i++) {
                    if (keys[i].startsWith("userData.")) {
                        keys[i] = keys[i].substring(9);
                    }
                }
                keyFieldMap.put(writer, keys);
            } catch (CmDataException e) {
                throw new CommonException(e.getMessage());
            }
        }
        fireTableDataChanged();
    }

    public UserData getData(int rowIndex) {
        return rows.get(rowIndex).data;
    }

    @Override
    public String getColumnName(int index) {
        switch (index) {
        case 0:
            return "Instance";
        case 1:
            return "Write State";
        case 2:
            return "Source Writer";
        default:
            return "";
        }
    }

    @Override
    protected UserData getRowUserData(UserData tupleData) {
        return tupleData;
    }

    @Override
    protected String getStateString(State state) {
        String result = "";
        if (state.getValue() == State.WRITE) {
            result = "WRITE";
        } else if (state.getValue() == State.DISPOSED) {
            result = "DISPOSED";
        } else if (state.getValue() == (State.WRITE | State.DISPOSED)) {
            result = "WRITE DISPOSED";
        } else if (state.getValue() == State.REGISTER) {
            result = "REGISTERED";
        } else if (state.getValue() == State.UNREGISTER) {
            result = "UNREGISTERED";
        }
        return result;
    }
}
