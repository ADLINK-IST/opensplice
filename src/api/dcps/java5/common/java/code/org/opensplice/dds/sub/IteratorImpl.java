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
package org.opensplice.dds.sub;

import java.lang.reflect.Field;

import org.opensplice.dds.core.DDSExceptionImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

import DDS.SampleInfoSeqHolder;

public class IteratorImpl<TYPE> extends AbstractIterator<TYPE> {
    private TYPE[] data;

    public IteratorImpl(OsplServiceEnvironment environment,
            DataReaderImpl<TYPE> reader, Object sampleSeqHolder,
            Field dataSeqHolderValue, SampleInfoSeqHolder infoSeqHolder) {
        super(environment, reader, sampleSeqHolder, dataSeqHolderValue,
                infoSeqHolder);
    }

    @SuppressWarnings("unchecked")
    @Override
    protected SampleImpl<TYPE>[] setupSampleList() {
        try {
            this.data = (TYPE[]) dataSeqHolderValue.get(sampleSeqHolder);

        } catch (SecurityException e) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Not allowed to access field "
                            + dataSeqHolderValue.getName() + " in "
                            + dataSeqHolderValue.getClass().getName() + "("
                            + e.getMessage() + ").");
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(environment, "Cannot find "
                    + dataSeqHolderValue.getName() + " in "
                    + dataSeqHolderValue.getClass().getName() + "("
                    + e.getMessage() + ").");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(environment, "No access to field "
                    + dataSeqHolderValue.getName() + " in "
                    + dataSeqHolderValue.getClass().getName() + "("
                    + e.getMessage() + ").");
        }
        return new SampleImpl[this.data.length];
    }

    @Override
    protected TYPE getData(int index) {
        return this.data[index];
    }
}
