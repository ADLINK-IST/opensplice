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

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.swing.table.AbstractTableModel;

import org.opensplice.cm.Entity;
import org.opensplice.cm.data.State;
import org.opensplice.cm.data.UserData;

/**
 * An implementation of AbstractTableModel for displaying a coherent set of user
 * data. Organizes UserData key fields and values, the write state, and source
 * writer for all samples for the coherent group.
 *
 * @date Jul 27, 2016
 */
public abstract class CoherentSetAbstractTableModel<T> extends AbstractTableModel {

    private static final long serialVersionUID = -2997882516932159985L;

    protected List<DataRowTuple<T>> rows;
    protected Map<Entity, String[]> keyFieldMap;

    public CoherentSetAbstractTableModel() {
        keyFieldMap = new HashMap<Entity, String[]>();
    }

    public Entity getEntity(int rowIndex) {
        return rows.get(rowIndex).entity;
    }

    /**
     * Clears the backing data structure for the table. Updates the view table.
     */
    public void clear() {
        rows.clear();
        fireTableDataChanged();
    }

    @Override
    public int getColumnCount() {
        return 3;
    }

    @Override
    public int getRowCount() {
        return rows.size();
    }

    @Override
    public Object getValueAt(int rowIndex, int columnIndex) {
        DataRowTuple<T> tuple = rows.get(rowIndex);
        switch (columnIndex) {
        case 0:
            if (tuple.entity != null && tuple.data != null) {
                StringBuilder keyvalues = new StringBuilder();
                for (String key : keyFieldMap.get(tuple.entity)) {
                    if (keyvalues.length() != 0) {
                        keyvalues.append(", ");
                    }
                    keyvalues.append(key);
                    keyvalues.append(" : ");
                    keyvalues.append(getRowUserData(tuple.data).getFieldValue(key));
                }
                return keyvalues.toString();
            }
            break;
        case 1:
            if (tuple.state != null) {
                return getStateString(tuple.state);
            }
            break;
        case 2:
            if (tuple.entity != null) {
                return tuple.entity.getName();
            }
            break;
        default:
        }
        return null;
    }

    protected abstract UserData getRowUserData(T tupleData);

    protected abstract String getStateString(State state);

    /**
     * The backing data structure that is modeled in the table.
     */
    protected static class DataRowTuple<T> {
        protected final T data;
        protected final State state;
        protected final Entity entity;

        protected DataRowTuple(T _data, State _state, Entity _entity) {
            data = _data;
            state = _state;
            entity = _entity;
        }
    }
}
