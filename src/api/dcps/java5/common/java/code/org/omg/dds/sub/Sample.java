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

import java.io.Serializable;
import java.util.ListIterator;

import org.omg.dds.core.DDSObject;
import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.Time;


/**
 * A Sample represents an atom of data information (i.e., one value for one
 * instance) as returned by a {@link org.omg.dds.sub.DataReader}'s read or take
 * operations. It consists of two parts: the Data ({@link #getData()}) and the
 * "Sample Info" (the remainder of the methods defined by this interface).
 * <p>
 * <b>Interpretation of the Sample Info</b>
 * <p>
 * In addition to the data value itself, the Sample contains information
 * pertaining to it:
 *
 * <ul>
 * <li>The sampleState ({@link #getSampleState()}) of the Data value (i.e., if
 * the sample has already been {@link org.omg.dds.sub.SampleState#READ} or
 * {@link org.omg.dds.sub.SampleState#NOT_READ} by that same
 * {@link org.omg.dds.sub.DataReader}).</li>
 * <li>The viewState ({@link #getViewState()}) of the related instance (i.e., if
 * the current generation of the instance is
 * {@link org.omg.dds.sub.ViewState#NEW} or
 * {@link org.omg.dds.sub.ViewState#NOT_NEW} for that DataReader).</li>
 * <li>The instanceState ({@link #getInstanceState()}) of the related instance
 * (i.e., if the instance is {@link org.omg.dds.sub.InstanceState#ALIVE},
 * {@link org.omg.dds.sub.InstanceState#NOT_ALIVE_DISPOSED}, or
 * {@link org.omg.dds.sub.InstanceState#NOT_ALIVE_NO_WRITERS}).</li>
 * <li>The "valid data" flag, corresponding to whether {@link #getData()} return
 * a non-null value. Some samples do not contain data, instead indicating only a
 * change on the instanceState of the corresponding instance.</li>
 * <li>The values of disposedGenerationCount (
 * {@link #getDisposedGenerationCount()}) and noWritersGenerationCount (
 * {@link #getNoWritersGenerationCount()}) for the related instance at the time
 * the sample was received. These counters indicate the number of times the
 * instance had become ALIVE (with instanceState =
 * {@link org.omg.dds.sub.InstanceState#ALIVE}) at the time the sample was
 * received.</li>
 * <li>The sampleRank ({@link #getSampleRank()}) and generationRank (
 * {@link #getGenerationRank()}) of the sample within the returned sequence.
 * These ranks provide a preview of the samples that follow within the sequence
 * returned by the {@link org.omg.dds.sub.DataReader#read()} or
 * {@link org.omg.dds.sub.DataReader#take()} operations.</li>
 * <li>The absoluteGenerationRank ({@link #getAbsoluteGenerationRank()}) of the
 * sample within the DataReader. This rank provides a preview of what is
 * available within the DataReader.</li>
 * <li>The sourceTimestamp ({@link #getSourceTimestamp()}) of the sample. This
 * is the time stamp provided by the {@link org.omg.dds.pub.DataWriter} at the
 * time the sample was produced.</li>
 * <li>The instanceHandle ({@link #getInstanceHandle()}) that identifies locally
 * the corresponding instance.</li>
 * <li>The publicationHandle ({@link #getPublicationHandle()}) that identifies
 * locally the {@link org.omg.dds.pub.DataWriter} that modified the instance.
 * The publicationHandle is the same {@link org.omg.dds.core.InstanceHandle}
 * that is returned by the operation
 * {@link org.omg.dds.sub.DataReader#getMatchedPublications()} on the DataReader
 * and can also be used as a parameter to the operation
 * {@link org.omg.dds.sub.DataReader#getMatchedPublicationData(InstanceHandle)}.
 * </li>
 * </ul>
 *
 * <b>Interpretation of the Counters and Ranks</b>
 * <p>
 * A Sample provides several counters and ranks: the disposedGenerationCount (
 * {@link #getDisposedGenerationCount()}), noWritersGenerationCount (
 * {@link #getNoWritersGenerationCount()}), sampleRank ({@link #getSampleRank()}
 * ), generationRank ({@link #getGenerationRank()}), and absoluteGenerationRank
 * ({@link #getAbsoluteGenerationRank()}). These counters and ranks allow the
 * application to distinguish samples belonging to different 'generations' of
 * the instance. Note that it is possible for an instance to transition from
 * not-alive to alive (and back) several times before the application accesses
 * the data by means of {@link org.omg.dds.sub.DataReader#read()} or
 * {@link org.omg.dds.sub.DataReader#take()}. In this case the returned
 * collection may contain samples that cross generations (i.e., some samples
 * were received before the instance became not-alive, others after the instance
 * reappeared again). Using the information in the Sample the application can
 * anticipate what other information regarding the same instance appears in the
 * returned collection as well as in the infrastructure and thus make
 * appropriate decisions. For example, an application desiring to only consider
 * the most current sample for each instance would only look at samples with
 * sampleRank == 0. Similarly an application desiring to only consider samples
 * that correspond to the latest generation in the collection will only look at
 * samples with generationRank == 0. An application desiring only samples
 * pertaining to the latest generation available will ignore samples for which
 * absoluteGenerationRank != 0. Other application-defined criteria may also be
 * used.
 * <p>
 *
 * @param <TYPE>
 *            The concrete type of the data encapsulated by this Sample.
 */
public interface Sample<TYPE> extends Cloneable, Serializable, DDSObject
{
    // -----------------------------------------------------------------------
    // Methods
    // -----------------------------------------------------------------------

    // --- Sample data: ------------------------------------------------------

    /**
     * Get the data associated with this Sample, if any.
     * <p>
     * Normally each Sample contains both meta-data ("Sample Info") and some
     * data. However there are situations where a Sample contains only the
     * Sample Info and does not have any associated data. This occurs when the
     * Service notifies the application of a change of state for an instance
     * that was caused by some internal mechanism (such as a timeout) for which
     * there is no associated data. An example of this situation is when the
     * Service detects that an instance has no writers and changes the
     * corresponding instanceState to
     * {@link org.omg.dds.sub.InstanceState#NOT_ALIVE_NO_WRITERS}.
     * <p>
     * The actual set of scenarios under which the middleware returns Samples
     * containing no data is implementation dependent. The application can
     * distinguish whether a particular Sample has data by examining the value
     * returned by this method. If the result is not null, then the Sample
     * contains valid data. If it is null, the Sample contains no data.
     * <p>
     * To ensure correctness and portability, the application must check for a
     * null result from this method prior to using it. If the data is null, the
     * application should access only the Sample Info.
     *
     * @return the data associated with this sample. This method will return
     *         null if this sample contains no valid data.
     */
    public TYPE getData();


    // --- Sample meta-data: -------------------------------------------------

    /**
     * For each sample received, the middleware internally maintains a
     * sampleState relative to each {@link org.omg.dds.sub.DataReader}. The
     * sampleState can either be {@link org.omg.dds.sub.SampleState#READ} or
     * {@link org.omg.dds.sub.SampleState#NOT_READ}.
     *
     * <ul>
     * <li>READ indicates that the DataReader has already accessed that sample
     * by means of {@link org.omg.dds.sub.DataReader#read()}. (Had the sample
     * been accessed by {@link org.omg.dds.sub.DataReader#take()}, it would no
     * longer be available to the DataReader.)</li>
     * <li>NOT_READ indicates that the DataReader has not accessed that sample
     * before.</li>
     * </ul>
     *
     * The sampleState will, in general, be different for each sample in the
     * collection returned by {@link org.omg.dds.sub.DataReader#read()} or
     * {@link org.omg.dds.sub.DataReader#take()}.
     * @return the SampleState
     */
    public SampleState getSampleState();

    /**
     * For each instance (identified by the key), the middleware internally
     * maintains a viewState relative to each {@link org.omg.dds.sub.DataReader}
     * . The viewState can either be {@link org.omg.dds.sub.ViewState#NEW} or
     * {@link org.omg.dds.sub.ViewState#NOT_NEW}.
     *
     * <ul>
     * <li>NEW indicates that either this is the first time that the DataReader
     * has ever accessed samples of that instance, or else that the DataReader
     * has accessed previous samples of the instance, but the instance has since
     * been reborn (i.e., become not-alive and then alive again). These two
     * cases are distinguished by examining the disposedGenerationCount and the
     * noWritersGenerationCount.</li>
     * <li>NOT_NEW indicates that the DataReader has already accessed samples of
     * the same instance and that the instance has not been reborn since.</li>
     * </ul>
     *
     * The viewState available in the Sample is a snapshot of the viewState of
     * the instance relative to the DataReader used to access the samples at the
     * time the collection was obtained (i.e., at the time
     * {@link org.omg.dds.sub.DataReader#read()} or
     * {@link org.omg.dds.sub.DataReader#take()} was called). The viewState is
     * therefore the same for all samples in the returned collection that refer
     * to the same instance.
     * @return the ViewState
     *
     * @see #getDisposedGenerationCount()
     * @see #getNoWritersGenerationCount()
     */
    public ViewState getViewState();

    /**
     * For each instance the middleware internally maintains an instanceState.
     * The instanceState can be {@link org.omg.dds.sub.InstanceState#ALIVE},
     * {@link org.omg.dds.sub.InstanceState#NOT_ALIVE_DISPOSED}, or
     * {@link org.omg.dds.sub.InstanceState#NOT_ALIVE_NO_WRITERS}.
     *
     * <ul>
     * <li>ALIVE indicates that (a) samples have been received for the instance,
     * (b) there are live {@link org.omg.dds.pub.DataWriter} entities writing
     * the instance, and (c) the instance has not been explicitly disposed (or
     * else more samples have been received after it was disposed).</li>
     * <li>NOT_ALIVE_DISPOSED indicates the instance was explicitly disposed by
     * a DataWriter by means of
     * {@link org.omg.dds.pub.DataWriter#dispose(InstanceHandle)}.</li>
     * <li>NOT_ALIVE_NO_WRITERS indicates the instance has been declared as
     * not-alive by the {@link org.omg.dds.sub.DataReader} because it detected
     * that there are no live DataWriter entities writing that instance.
     * </ul>
     *
     * The precise behavior events that cause the instanceState to change
     * depends on the setting of the {@link org.omg.dds.core.policy.Ownership}:
     *
     * <ul>
     * <li>If {@link org.omg.dds.core.policy.Ownership#getKind()} is
     * {@link org.omg.dds.core.policy.Ownership.Kind#EXCLUSIVE}, then the
     * instanceState becomes NOT_ALIVE_DISPOSED only if the DataWriter that
     * "owns" the instance explicitly disposes it. The instanceState becomes
     * ALIVE again only if the DataWriter that owns the instance writes it.</li>
     * <li>If {@link org.omg.dds.core.policy.Ownership#getKind()} is
     * {@link org.omg.dds.core.policy.Ownership.Kind#SHARED}, then the
     * instanceState becomes NOT_ALIVE_DISPOSED if any DataWriter explicitly
     * disposes the instance. The instanceState becomes ALIVE as soon as any
     * DataWriter writes the instance again.</li>
     * </ul>
     *
     * The instanceState available in the Sample is a snapshot of the
     * instanceState of the instance at the time the collection was obtained
     * (i.e., at the time {@link org.omg.dds.sub.DataReader#read()} or
     * {@link org.omg.dds.sub.DataReader#take()} was called). The instanceState
     * is therefore the same for all samples in the returned collection that
     * refer to the same instance.
     * @return the InstanceState
     */
    public InstanceState getInstanceState();
    /**
     * Gets the timestamp of the sample.
     * This is the timestamp provided by the DataWriter at the time the sample was produced.
     * @return the timestamp
     */
    public Time getSourceTimestamp();
    /**
     * Gets the InstanceHandle of the associated data Sample.
     *
     * @return the InstanceHandle of the sample
     */
    public InstanceHandle getInstanceHandle();
    /**
     * Gets the InstanceHandle of the associated publication.
     *
     * @return the PublicationHandle
     */
    public InstanceHandle getPublicationHandle();

    /**
     * For each instance the middleware internally maintains two counts: the
     * disposedGenerationCount and noWritersGenerationCount, relative to each
     * {@link org.omg.dds.sub.DataReader}:
     *
     * <ul>
     * <li>The disposedGenerationCount and noWritersGenerationCount are
     * initialized to zero when the DataReader first detects the presence of a
     * never-seen-before instance.</li>
     * <li>The disposedGenerationCount is incremented each time the
     * instanceState of the corresponding instance changes from
     * NOT_ALIVE_DISPOSED to ALIVE.</li>
     * <li>The noWritersGenerationCount is incremented each time the
     * instanceState of the corresponding instance changes from
     * NOT_ALIVE_NO_WRITERS to ALIVE.</li>
     * </ul>
     *
     * The disposedGenerationCount and noWritersGenerationCount available in the
     * Sample capture a snapshot of the corresponding counters at the time the
     * sample was received.
     * @return the disposed generation count
     * @see #getNoWritersGenerationCount()
     */
    public int getDisposedGenerationCount();

    /**
     * For each instance the middleware internally maintains two counts: the
     * disposedGenerationCount and noWritersGenerationCount, relative to each
     * {@link org.omg.dds.sub.DataReader}:
     *
     * <ul>
     * <li>The disposedGenerationCount and noWritersGenerationCount are
     * initialized to zero when the DataReader first detects the presence of a
     * never-seen-before instance.</li>
     * <li>The disposedGenerationCount is incremented each time the
     * instanceState of the corresponding instance changes from
     * NOT_ALIVE_DISPOSED to ALIVE.</li>
     * <li>The noWritersGenerationCount is incremented each time the
     * instanceState of the corresponding instance changes from
     * NOT_ALIVE_NO_WRITERS to ALIVE.</li>
     * </ul>
     *
     * The disposedGenerationCount and noWritersGenerationCount available in the
     * Sample capture a snapshot of the corresponding counters at the time the
     * sample was received.
     * @return the no writers generation count
     * @see #getDisposedGenerationCount()
     */
    public int getNoWritersGenerationCount();

    /**
     * The sampleRank and generationRank available in the Sample are computed
     * based solely on the actual samples in the ordered collection returned by
     * {@link org.omg.dds.sub.DataReader#read()} or
     * {@link org.omg.dds.sub.DataReader#take()}.
     *
     * <ul>
     * <li>The sampleRank indicates the number of samples of the same instance
     * that follow the current one in the collection.</li>
     * <li>The generationRank available in the Sample indicates the difference
     * in 'generations' between the sample (S) and the Most Recent Sample of the
     * same instance that appears In the returned Collection (MRSIC). That is,
     * it counts the number of times the instance transitioned from not-alive to
     * alive in the time from the reception of S to the reception of MRSIC.</li>
     * </ul>
     * @return the sample rank
     * @see #getAbsoluteGenerationRank()
     * @see #getGenerationRank()
     */
    public int getSampleRank();

    /**
     * The sampleRank and generationRank available in the Sample are computed
     * based solely on the actual samples in the ordered collection returned by
     * {@link org.omg.dds.sub.DataReader#read()} or
     * {@link org.omg.dds.sub.DataReader#take()}.
     *
     * <ul>
     * <li>The sampleRank indicates the number of samples of the same instance
     * that follow the current one in the collection.</li>
     * <li>The generationRank available in the Sample indicates the difference
     * in 'generations' between the sample (S) and the Most Recent Sample of the
     * same instance that appears In the returned Collection (MRSIC). That is,
     * it counts the number of times the instance transitioned from not-alive to
     * alive in the time from the reception of S to the reception of MRSIC.</li>
     * </ul>
     *
     * The generationRank is computed using the formula:
     * <p>
     * <code>
     * generationRank = (MRSIC.disposedGenerationCount
     *                  + MRSIC.noWritersGenerationCount)
     *                  - (S.disposedGenerationCount
     *                  + S.noWritersGenerationCount)
     * </code>
     * @return the generation rank
     * @see #getAbsoluteGenerationRank()
     * @see #getSampleRank()
     */
    public int getGenerationRank();

    /**
     * The sampleRank and generationRank available in the Sample are computed
     * based solely on the actual samples in the ordered collection returned by
     * {@link org.omg.dds.sub.DataReader#read()} or
     * {@link org.omg.dds.sub.DataReader#take()}.
     *
     * <ul>
     * <li>The sampleRank indicates the number of samples of the same instance
     * that follow the current one in the collection.</li>
     * <li>The generationRank available in the Sample indicates the difference
     * in 'generations' between the sample (S) and the Most Recent Sample of the
     * same instance that appears In the returned Collection (MRSIC). That is,
     * it counts the number of times the instance transitioned from not-alive to
     * alive in the time from the reception of S to the reception of MRSIC.</li>
     * </ul>
     *
     * The absoluteGenerationRank available in the Sample indicates the
     * difference in 'generations' between the sample (S) and the Most Recent
     * Sample of the same instance that the middleware has received (MRS). That
     * is, it counts the number of times the instance transitioned from
     * not-alive to alive in the time from the reception of S to the time when
     * {@link org.omg.dds.sub.DataReader#read()} or
     * {@link org.omg.dds.sub.DataReader#take()} was called.
     * <p>
     * <code>
     * absoluteGenerationRank = (MRS.disposedGenerationCount
     *                          + MRS.noWritersGenerationCount)
     *                          - (S.disposedGenerationCount
     *                          + S.noWritersGenerationCount)
     * </code>
     * @return the absolute generation rank
     * @see #getGenerationRank()
     * @see #getSampleRank()
     */
    public int getAbsoluteGenerationRank();


    // --- From Object: ------------------------------------------------------

    public Sample<TYPE> clone();



    // -----------------------------------------------------------------------
    // Types
    // -----------------------------------------------------------------------

    public static interface Iterator<IT_DATA>
    extends java.io.Closeable, ListIterator<Sample<IT_DATA>> {
        /**
         * This operation indicates to that the application is done accessing
         * the list of Samples obtained by some earlier invocation of
         * {@link org.omg.dds.sub.DataReader#read()} or
         * {@link org.omg.dds.sub.DataReader#take()}.
         * <p>
         * The operation allows implementations to "loan" buffers from the
         * DataReader to the application and in this manner provide "zero-copy"
         * access to the data. During the loan, the DataReader will guarantee
         * that the data and sample information are not modified.
         * <p>
         * It is not necessary for an application to return the loans
         * immediately after the read or take calls. However, as these buffers
         * correspond to internal resources inside the DataReader, the
         * application should not retain them indefinitely.
         * <p>
         * The use of the operation is only necessary if the read or take calls
         * "loaned" buffers to the application. The situations in which this
         * occurs are described in the documentation for
         * {@link org.omg.dds.sub.DataReader#read()} and
         * {@link org.omg.dds.sub.DataReader#take()}. However, calling close on
         * a collection that does not have a loan is safe and has no side
         * effects.
         *
         * @see DataReader#read()
         * @see DataReader#take()
         */
        @Override
        public abstract void close() throws java.io.IOException;

        // --- From ListIterator: --------------------------------------------
        /**
         * @exception UnsupportedOperationException always.
         */
        @Override
        public void remove();

        /**
         * @exception UnsupportedOperationException always.
         */
        @Override
        public void set(Sample<IT_DATA> o);

        /**
         * @exception UnsupportedOperationException always.
         */
        @Override
        public void add(Sample<IT_DATA> o);
    }

}
