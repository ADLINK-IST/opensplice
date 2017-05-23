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
package org.opensplice.dds.sub;

import java.lang.reflect.Field;

import org.omg.dds.sub.Sample;
import org.opensplice.dds.core.AlreadyClosedExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.UnsupportedOperationExceptionImpl;

import DDS.SampleInfoSeqHolder;

public abstract class AbstractIterator<TYPE> implements Sample.Iterator<TYPE> {
    protected final AbstractDataReader<TYPE> reader;
    protected final OsplServiceEnvironment environment;
    protected final SampleInfoSeqHolder infoSeqHolder;
    protected final Object sampleSeqHolder;
    protected final Field dataSeqHolderValue;
    private final SampleImpl<TYPE>[] samples;
    private int currentIndex;
    private int initUntil;

    public AbstractIterator(OsplServiceEnvironment environment,
            AbstractDataReader<TYPE> reader, Object sampleSeqHolder,
            Field dataSeqHolderValue, SampleInfoSeqHolder infoSeqHolder) {
        this.environment = environment;
        this.reader = reader;
        this.infoSeqHolder = infoSeqHolder;
        this.sampleSeqHolder = sampleSeqHolder;
        this.dataSeqHolderValue = dataSeqHolderValue;
        this.currentIndex = 0;
        this.initUntil = -1;
        this.samples = this.setupSampleList();
        this.reader.registerIterator(this);
    }

    protected abstract SampleImpl<TYPE>[] setupSampleList();

    protected abstract TYPE getData(int index);

    @Override
    public boolean hasNext() {
        if (this.currentIndex == -1) {
            throw new AlreadyClosedExceptionImpl(this.environment,
                    "Iterator already closed.");
        }
        if (infoSeqHolder.value.length > this.currentIndex) {
            return true;
        }
        return false;
    }

    @Override
    public boolean hasPrevious() {
        if (this.currentIndex == -1) {
            throw new AlreadyClosedExceptionImpl(this.environment,
                    "Iterator already closed.");
        }
        if ((this.currentIndex - 1) > 0) {
            return true;
        }
        return false;
    }

    @Override
    public int nextIndex() {
        if (this.currentIndex == -1) {
            throw new AlreadyClosedExceptionImpl(this.environment,
                    "Iterator already closed.");
        }
        return this.currentIndex;
    }

    @Override
    public int previousIndex() {
        if (this.currentIndex == -1) {
            throw new AlreadyClosedExceptionImpl(this.environment,
                    "Iterator already closed.");
        }
        return this.currentIndex - 1;
    }

    @Override
    public void close() {
        if (this.currentIndex == -1) {
            throw new AlreadyClosedExceptionImpl(this.environment,
                    "Iterator already closed.");
        }
        this.currentIndex = -1;
        this.reader.returnLoan(this.sampleSeqHolder, this.infoSeqHolder);
        this.reader.deregisterIterator(this);
    }

    @Override
    public void remove() {
        throw new UnsupportedOperationExceptionImpl(this.environment,
                "Cannot remove() from Sample.Iterator.");
    }

    @Override
    public void set(Sample<TYPE> o) {
        throw new UnsupportedOperationExceptionImpl(this.environment,
                "Cannot set() in Sample.Iterator.");
    }

    @Override
    public void add(Sample<TYPE> o) {
        throw new UnsupportedOperationExceptionImpl(this.environment,
                "Cannot add() to Sample.Iterator.");
    }

    @Override
    public Sample<TYPE> next() {
        if (this.currentIndex == -1) {
            throw new AlreadyClosedExceptionImpl(this.environment,
                    "Iterator already closed.");
        }
        int index = this.currentIndex++;

        if (this.initUntil < index) {
            this.samples[index] = new SampleImpl<TYPE>(this.environment,
                    this.getData(index), this.infoSeqHolder.value[index]);
            this.initUntil++;
        }
        return this.samples[index];
    }

    @Override
    public Sample<TYPE> previous() {
        if (this.currentIndex == -1) {
            throw new AlreadyClosedExceptionImpl(this.environment,
                    "Iterator already closed.");
        }
        return this.samples[--this.currentIndex];
    }
}
